#include "eink_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <linux/gpio.h>
#include <stdbool.h>
#include <gpiod.h>
#include "lodepng.h"
#include <sys/stat.h>

static int spi_fd = -1;
static uint8_t old_data[BUFFER_SIZE];

static epd_mode_t current_mode = MODE_FAST;

// Add at the top with other globals
static struct gpiod_chip *chip;
static struct gpiod_line *dc_line;
static struct gpiod_line *rst_line;
static struct gpiod_line *busy_line;
static size_t total_spi_bytes = 0;

// Add at the top after includes and before any function definitions
static void delay_ms(int ms);
static void gpio_write(int pin, int value);
static int gpio_read(int pin);
static void lcd_chkstatus(void);
static void spi_delay(void);
static void write_cmd(uint8_t cmd);
static void write_data(uint8_t data);
static void spi_write_keep_cs(const uint8_t* data, size_t size);
static void write_full_lut(void);
static void save_pixels_as_png(const uint8_t* px_map, uint32_t width, uint32_t height, const char* filename);

// Add to the top with other static variables
static epd_mode_t previous_mode = MODE_NONE;  // Track previous mode

// Mode switching function
void eink_set_mode(epd_mode_t new_mode) {
    printf("Debug: Switching mode from %d to %d\n", previous_mode, new_mode);
    
    if (new_mode == previous_mode) {
        printf("Debug: Already in requested mode\n");
        return;
    }

    // Clear display when switching modes
    eink_clear(false);

    // Initialize the new mode
    switch(new_mode) {
        case MODE_NORMAL:
            EPD_init();
            break;
        case MODE_4GRAY:
            epd_w21_init_4g();
            break;
        case MODE_FAST:
            EPD_init_Fast();
            break;
        case MODE_PARTIAL:
            EPD_init_Part();
            break;
        default:
            printf("Error: Invalid mode %d\n", new_mode);
            return;
    }

    previous_mode = new_mode;
    current_mode = new_mode;
}

// Update the convert_2bit_to_1bit function to properly handle LVGL's I2 format
static void convert_2bit_to_1bit(const uint8_t* input, uint8_t* output, size_t pixel_count) {
    printf("Debug: Converting %zu pixels from 2-bit indexed to 1-bit\n", pixel_count);
    memset(output, 0xFF, (pixel_count + 7) / 8);  // Initialize to white (0xFF)

    // For I2 format:
    // 00 = Darkest (Black)
    // 01 = Dark Gray
    // 10 = Light Gray
    // 11 = White
    
    for(uint32_t i = 0; i < pixel_count; i++) {
        uint8_t byte_idx = i / 4;
        uint8_t bit_pos = (i % 4) * 2;
        uint8_t pixel = (input[byte_idx] >> bit_pos) & 0x03;
        
        // For e-ink: 1=white, 0=black
        // Convert based on threshold - treat dark gray as black
        if (pixel < 2) {  // If pixel is 00 (black) or 01 (dark gray)
            uint8_t out_byte_idx = i / 8;
            uint8_t out_bit_pos = 7 - (i % 8);
            output[out_byte_idx] &= ~(1 << out_bit_pos);  // Set to black (0)
        }
        // else leave as white (1)
        
        if (i < 32) {  // Debug first few pixels
            printf("Debug: Pixel %d: input[%d]=%02X, bit_pos=%d, value=%d -> %d\n", 
                   i, byte_idx, input[byte_idx], bit_pos, pixel, 
                   (output[i/8] >> (7 - (i%8))) & 1);
        }
    }
}

// Helper function to convert 2-bit indexed to 4-gray format
static void convert_2bit_to_4gray(const uint8_t* input, uint8_t* output, size_t pixel_count) {
    printf("Debug: Converting %zu pixels to 4-gray format\n", pixel_count);
    
    // For I2 format:
    // 00 = Darkest (Black)
    // 01 = Dark Gray
    // 10 = Light Gray
    // 11 = White
    
    for(size_t i = 0; i < pixel_count/4; i++) {
        uint8_t temp3 = 0;
        for(int j = 0; j < 2; j++) {
            uint8_t temp1 = input[i*2 + j];
            for(int k = 0; k < 4; k++) {
                uint8_t pixel = (temp1 >> (6 - k*2)) & 0x03;
                
                // Direct mapping - no need to invert
                temp3 = (temp3 << 1) | (pixel >> 1);  // Map to 4-gray format
            }
        }
        output[i] = temp3;
    }
}

// Basic GPIO and SPI functions
static void delay_ms(int ms) {
    usleep(ms * 1000);
}

// Update gpio_write function
static void gpio_write(int pin, int value) {
    struct gpiod_line *line = NULL;
    
    if (pin == DC_PIN) {
        line = dc_line;
    } else if (pin == RST_PIN) {
        line = rst_line;
    }
    
    if (line) {
        if (gpiod_line_set_value(line, value) < 0) {
            printf("Error: Failed to set GPIO %d to %d\n", pin, value);
        }
    }
}

// Update gpio_read function
static int gpio_read(int pin) {
    if (pin == BUSY_PIN) {
        return gpiod_line_get_value(busy_line);
    }
    return -1;
}

// Wait for display to be ready
static void lcd_chkstatus(void) {
    while (gpio_read(BUSY_PIN) == 0) {
        delay_ms(10);
    }
}

// Add at the top with other static functions
static void spi_delay(void) {
    usleep(1);  // 1 microsecond delay, equivalent to time.sleep(0.000001)
}

// Update write_cmd function
static void write_cmd(uint8_t cmd) {
    spi_delay();
    gpio_write(DC_PIN, 0);
    
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)&cmd,
        .rx_buf = 0,
        .len = 1,
        .speed_hz = 30000000,
        .bits_per_word = 8,
        .delay_usecs = 0,
        .cs_change = 1,
    };
    
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        perror("Error in SPI command transfer");
    }
}

// Update write_data function
static void write_data(uint8_t data) {
    spi_delay();
    gpio_write(DC_PIN, 1);
    
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)&data,
        .rx_buf = 0,
        .len = 1,
        .speed_hz = 30000000,
        .bits_per_word = 8,
        .delay_usecs = 0,
        .cs_change = 1,
    };
    
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        perror("Error in SPI data transfer");
    }
}

// SPI communication
void EPD_W21_WriteCMD(unsigned char command) {
    write_cmd(command);
}

void EPD_W21_WriteDATA(unsigned char data) {
    write_data(data);
}

// Initialize display
void EPD_W21_Init(void) {
    printf("Debug: EPD_W21_Init - Starting reset sequence\n");
    delay_ms(100);  // At least 10ms delay
    gpio_write(RST_PIN, 0);  // Module reset
    delay_ms(20);   // At least 10ms delay
    gpio_write(RST_PIN, 1);
    delay_ms(20);   // At least 10ms delay
    printf("Debug: Reset sequence complete\n");
}

void EPD_Display(unsigned char *Image) {
    unsigned int Width, Height, i, j;
    Width = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1);
    Height = EPD_HEIGHT;

    EPD_W21_WriteCMD(0x10);
    for (j = 0; j < Height; j++) {
        for (i = 0; i < Width; i++) {
            EPD_W21_WriteDATA(Image[i + j * Width]);
        }
    }

    EPD_W21_WriteCMD(0x13);
    for (j = 0; j < Height; j++) {
        for (i = 0; i < Width; i++) {
            EPD_W21_WriteDATA(0x00);
        }
    }
    EPD_W21_WriteCMD(0x12);
    delay_ms(1);
    lcd_chkstatus();
}

void EPD_init(void) {
    EPD_W21_Init();

    EPD_W21_WriteCMD(0x04);  // Power on
    lcd_chkstatus();        // Waiting for the electronic paper IC to release the idle signal

    EPD_W21_WriteCMD(0x50);  // VCOM AND DATA INTERVAL SETTING
    EPD_W21_WriteDATA(0x97); // WBmode:VBDF 17|D7 VBDW 97 VBDB 57  WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
}

void EPD_init_Fast(void) {
    printf("Debug: EPD_init_Fast - Starting EPD_W21_Init\n");
    EPD_W21_Init();

    printf("Debug: Power on command\n");
    EPD_W21_WriteCMD(0x04);  // Power on
    lcd_chkstatus();        // Waiting for the electronic paper IC to release the idle signal

    printf("Debug: Setting E0 register\n");
    EPD_W21_WriteCMD(0xE0);
    EPD_W21_WriteDATA(0x02);

    printf("Debug: Setting E5 register\n");
    EPD_W21_WriteCMD(0xE5);
    EPD_W21_WriteDATA(0x5A);
    
    printf("Debug: EPD_init_Fast complete\n");
}

void EPD_init_Part(void) {
    EPD_W21_Init();

    EPD_W21_WriteCMD(0x04);  // Power on
    lcd_chkstatus();        // Waiting for the electronic paper IC to release the idle signal

    EPD_W21_WriteCMD(0xE0);
    EPD_W21_WriteDATA(0x02);

    EPD_W21_WriteCMD(0xE5);
    EPD_W21_WriteDATA(0x6E);

    EPD_W21_WriteCMD(0x50);
    EPD_W21_WriteDATA(0xD7);
}

void EPD_sleep(void) {
    EPD_W21_WriteCMD(0x02);  // Power off
    lcd_chkstatus();        // Waiting for the electronic paper IC to release the idle signal
    EPD_W21_WriteCMD(0x07);  // Deep sleep
    EPD_W21_WriteDATA(0xA5);
}

void PIC_display(const unsigned char *new_data) {
    unsigned int i;
    
    // Write Data
    EPD_W21_WriteCMD(0x10);        // Transfer old data
    for(i = 0; i < BUFFER_SIZE; i++) {
        EPD_W21_WriteDATA(old_data[i]);  // Transfer the actual displayed data
    }
    
    EPD_W21_WriteCMD(0x13);        // Transfer new data
    for(i = 0; i < BUFFER_SIZE; i++) {
        EPD_W21_WriteDATA(new_data[i]);  // Transfer the actual displayed data
        old_data[i] = new_data[i];
    }
    
    // Refresh
    EPD_W21_WriteCMD(0x12);        // DISPLAY REFRESH
    delay_ms(1);                    // The delay here is necessary, 200uS at least
    lcd_chkstatus();               // Waiting for the electronic paper IC to release the idle signal
}

void PIC_display_Clear(void) {
    unsigned int i;
    
    // Write Data
    EPD_W21_WriteCMD(0x10);        // Transfer old data
    for(i = 0; i < BUFFER_SIZE; i++) {
        EPD_W21_WriteDATA(old_data[i]);
    }
    
    EPD_W21_WriteCMD(0x13);        // Transfer new data
    for(i = 0; i < BUFFER_SIZE; i++) {
        EPD_W21_WriteDATA(0xFF);   // Transfer the actual displayed data
        old_data[i] = 0xFF;
    }
    
    // Refresh
    EPD_W21_WriteCMD(0x12);        // DISPLAY REFRESH
    delay_ms(1);                    // The delay here is necessary, 200uS at least
    lcd_chkstatus();               // Waiting for the electronic paper IC to release the idle signal
}

void pic_display_4G(const unsigned char *datas) {
    unsigned int i, j, k;
    unsigned char temp1, temp2, temp3;

    // Old data
    EPD_W21_WriteCMD(0x10);
    for(i = 0; i < BUFFER_SIZE; i++) {
        temp3 = 0;
        for(j = 0; j < 2; j++) {
            temp1 = datas[i*2+j];
            for(k = 0; k < 4; k++) {
                temp2 = temp1 & 0xC0;
                if(temp2 == 0xC0)
                    temp3 |= 0x01;      // White
                else if(temp2 == 0x00)
                    temp3 |= 0x00;      // Black
                else if(temp2 == 0x80)
                    temp3 |= 0x01;      // Gray1
                else if(temp2 == 0x40)
                    temp3 |= 0x00;      // Gray2

                if(j == 0) {
                    temp1 <<= 2;
                    temp3 <<= 1;
                }
                if(j == 1 && k != 3) {
                    temp1 <<= 2;
                    temp3 <<= 1;
                }
            }
        }
        EPD_W21_WriteDATA(temp3);
    }

    // New data
    EPD_W21_WriteCMD(0x13);
    for(i = 0; i < BUFFER_SIZE; i++) {
        temp3 = 0;
        for(j = 0; j < 2; j++) {
            temp1 = datas[i*2+j];
            for(k = 0; k < 4; k++) {
                temp2 = temp1 & 0xC0;
                if(temp2 == 0xC0)
                    temp3 |= 0x01;      // White
                else if(temp2 == 0x00)
                    temp3 |= 0x00;      // Black
                else if(temp2 == 0x80)
                    temp3 |= 0x00;      // Gray1
                else if(temp2 == 0x40)
                    temp3 |= 0x01;      // Gray2

                if(j == 0) {
                    temp1 <<= 2;
                    temp3 <<= 1;
                }
                if(j == 1 && k != 3) {
                    temp1 <<= 2;
                    temp3 <<= 1;
                }
            }
        }
        EPD_W21_WriteDATA(temp3);
    }

    // Refresh
    EPD_W21_WriteCMD(0x12);        // DISPLAY REFRESH
    delay_ms(1);                    // The delay here is necessary, 200uS at least
    lcd_chkstatus();               // Waiting for the electronic paper IC to release the idle signal
}

// Update pic_display_fast to remove the text file saving
void pic_display_fast(const uint8_t* data, size_t size) {
    total_spi_bytes = 0;  // Reset counter at start
    printf("Debug: Starting pic_display_fast\n");
    if (!data || size == 0 || size > BUFFER_SIZE) {
        printf("Error: Invalid data or size\n");
        return;
    }

    // For fast mode, data should already be in 1-bit format
    EPD_init_Fast();
    
    // Send old data
    EPD_W21_WriteCMD(0x10);
    gpio_write(DC_PIN, 1);
    spi_write_keep_cs(old_data, size);

    // Send new data
    EPD_W21_WriteCMD(0x13);
    gpio_write(DC_PIN, 1);
    spi_write_keep_cs(data, size);

    // Cache the data
    memcpy(old_data, data, size);

    // Refresh
    EPD_W21_WriteCMD(0x12);
    delay_ms(1);
    lcd_chkstatus();

    printf("Debug: pic_display_fast complete - Total SPI bytes sent: %zu\n", total_spi_bytes);
}

// Partial mode display (1-bit)
void pic_display_partial(const uint8_t* data, size_t size) {
    total_spi_bytes = 0;  // Reset counter at start
    printf("Debug: Starting pic_display_partial\n");
    if (!data || size == 0 || size > BUFFER_SIZE) {
        printf("Error: Invalid data or size\n");
        return;
    }

    EPD_init_Part();
    
    // Same as fast mode but with different initialization
    EPD_W21_WriteCMD(0x10);
    gpio_write(DC_PIN, 1);
    spi_write_keep_cs(old_data, size);

    EPD_W21_WriteCMD(0x13);
    gpio_write(DC_PIN, 1);
    spi_write_keep_cs(data, size);

    memcpy(old_data, data, size);

    EPD_W21_WriteCMD(0x12);
    delay_ms(1);
    lcd_chkstatus();

    printf("Debug: pic_display_partial complete - Total SPI bytes sent: %zu\n", total_spi_bytes);
}

// 4 Gray mode display (2-bit)
void pic_display_4g(const uint8_t* data, size_t size) {
    total_spi_bytes = 0;  // Reset counter at start
    printf("Debug: Starting pic_display_4g\n");
    printf("Debug: Starting 4G display with %zu bytes\n", size);
    
    // Command to start transmitting old data
    EPD_W21_WriteCMD(0x10);
    gpio_write(DC_PIN, 1);

    printf("Debug: Start Old Data Transmission\n");
    uint8_t buffer[BUFFER_SIZE];
    
    // Convert and send old data
    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        uint8_t temp3 = 0;
        for (int j = 0; j < 2; j++) {
            uint8_t temp1 = data[i * 2 + j];
            for (int k = 0; k < 4; k++) {
                uint8_t temp2 = temp1 & 0xC0;
                if (temp2 == 0xC0)
                    temp3 |= 0x01;      // White
                else if (temp2 == 0x00)
                    temp3 |= 0x00;      // Black
                else if (temp2 == 0x80)
                    temp3 |= 0x01;      // Gray1
                else if (temp2 == 0x40)
                    temp3 |= 0x00;      // Gray2

                if (j == 0) {
                    temp1 <<= 2;
                    temp3 <<= 1;
                }
                if (j == 1 && k != 3) {
                    temp1 <<= 2;
                    temp3 <<= 1;
                }
            }
        }
        buffer[i] = temp3;
    }
    
    // Send old data in chunks
    spi_write_keep_cs(buffer, BUFFER_SIZE);

    printf("Debug: Start New Data Transmission\n");
    // Command to start transmitting new data
    EPD_W21_WriteCMD(0x13);
    gpio_write(DC_PIN, 1);

    // Convert and send new data
    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        uint8_t temp3 = 0;
        for (int j = 0; j < 2; j++) {
            uint8_t temp1 = data[i * 2 + j];
            for (int k = 0; k < 4; k++) {
                uint8_t temp2 = temp1 & 0xC0;
                if (temp2 == 0xC0)
                    temp3 |= 0x01;      // White
                else if (temp2 == 0x00)
                    temp3 |= 0x00;      // Black
                else if (temp2 == 0x80)
                    temp3 |= 0x00;      // Gray1
                else if (temp2 == 0x40)
                    temp3 |= 0x01;      // Gray2

                if (j == 0) {
                    temp1 <<= 2;
                    temp3 <<= 1;
                }
                if (j == 1 && k != 3) {
                    temp1 <<= 2;
                    temp3 <<= 1;
                }
            }
        }
        buffer[i] = temp3;
    }
    
    // Send new data in chunks
    spi_write_keep_cs(buffer, BUFFER_SIZE);

    // Refresh command
    printf("Debug: Refreshing\n");
    EPD_W21_WriteCMD(0x12);
    delay_ms(1);
    lcd_chkstatus();

    printf("Debug: pic_display_4g complete - Total SPI bytes sent: %zu\n", total_spi_bytes);
}

// Update debug_pixel_data function to look for text
static void debug_pixel_data(const uint8_t* px_map, size_t size, uint32_t width, uint32_t height) {
    int black_count = 0;
    int white_count = 0;
    bool found_text = false;

    printf("\nDetailed Pixel Analysis:\n");
    
    // Skip palette
    px_map += 8;
    
    // For I1 format, each bit is one pixel
    size_t total_pixels = width * height;
    
    for(size_t i = 0; i < total_pixels; i++) {
        size_t byte_idx = i / 8;
        uint8_t bit_pos = 7 - (i % 8);  // MSB first
        uint8_t pixel = (px_map[byte_idx] >> bit_pos) & 0x01;
        
        if (pixel == 0) {
            black_count++;
            if (!found_text) {
                found_text = true;
                printf("Found black pixel at byte[%zu] bit %d\n", byte_idx, bit_pos);
            }
        } else {
            white_count++;
        }
    }

    printf("\nPixel Distribution:\n");
    float total = (float)total_pixels;
    printf("Black: %d (%.1f%%)\n", black_count, (black_count * 100.0f) / total);
    printf("White: %d (%.1f%%)\n", white_count, (white_count * 100.0f) / total);
    printf("Total pixels: %zu\n", total_pixels);
    
    if (black_count + white_count != total_pixels) {
        printf("WARNING: Pixel count mismatch! Sum = %d, Expected = %zu\n", 
               black_count + white_count, total_pixels);
    }
}

static void dump_buffer(const uint8_t* buf, size_t size, const char* label) {
    printf("\nDumping first 32 bytes of %s:\n", label);
    for(size_t i = 0; i < 32 && i < size; i++) {
        printf("%02X ", buf[i]);
        if((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

// Add this helper function
static void ensure_output_dir(void) {
    struct stat st = {0};
    if (stat("output", &st) == -1) {
        // Create output directory if it doesn't exist
        if (mkdir("output", 0755) == -1) {
            perror("Error creating output directory");
        } else {
            printf("Debug: Created output directory\n");
        }
    }
}

// Add this helper function for flipping bytes
static uint8_t flip_byte(uint8_t b) {
    // No need to flip bits within byte since we want to preserve horizontal order
    return b;
}

// Add this function to flip image vertically
static void flip_buffer_vertical(uint8_t* buffer, uint32_t width, uint32_t height) {
    const uint32_t bytes_per_row = (width + 7) / 8;  // Round up to nearest byte
    uint8_t* temp_row = malloc(bytes_per_row);
    
    if (!temp_row) {
        printf("Error: Failed to allocate temp buffer for flipping\n");
        return;
    }

    // Only need to swap half the rows
    for (uint32_t i = 0; i < height/2; i++) {
        uint8_t* top_row = buffer + (i * bytes_per_row);
        uint8_t* bottom_row = buffer + ((height - 1 - i) * bytes_per_row);
        
        // Save top row
        memcpy(temp_row, top_row, bytes_per_row);
        
        // Copy bottom row to top
        memcpy(top_row, bottom_row, bytes_per_row);
        
        // Copy saved top row to bottom
        memcpy(bottom_row, temp_row, bytes_per_row);
    }

    free(temp_row);
}

void eink_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map) {
    printf("Debug: Flush callback entered\n");
    static int flush_count = 0;
    printf("\n=== Flush #%d ===\n", ++flush_count);
    
    uint32_t width = area->x2 - area->x1 + 1;
    uint32_t height = area->y2 - area->y1 + 1;
    uint32_t size = (width * height) / 8;  // Size in bytes since it's 1-bit per pixel

    printf("Area: (%d,%d) to (%d,%d), size: %dx%d\n", 
           area->x1, area->y1, area->x2, area->y2, width, height);

    // Skip palette bytes
    px_map += 8;

    // Flip the buffer vertically
    flip_buffer_vertical(px_map, width, height);

    // Save PNG file (will show flipped result)
    // char filename[64];
    // snprintf(filename, sizeof(filename), "output/flush_%d.png", flush_count);
    // save_pixels_as_png(px_map - 8, width, height, filename);  // Subtract 8 to include palette

    // Display the buffer according to current mode
    switch(current_mode) {
        case MODE_NORMAL:
            pic_display(px_map, size);  // px_map is already 1-bit
            break;
            
        case MODE_4GRAY:
            pic_display_4g(px_map, size);  // For 4-gray, needs special handling
            break;
            
        case MODE_FAST:
            pic_display_fast(px_map, size);  // px_map is already 1-bit
            break;
            
        case MODE_PARTIAL:
            pic_display_partial(px_map, size);  // px_map is already 1-bit
            break;
    }

    lv_display_flush_ready(disp);
}

// Initialize the display
void eink_init(void) {
    printf("Debug: Starting eink_init\n");
    
    // Create output directory
    ensure_output_dir();
    
    // Initialize SPI
    spi_fd = open("/dev/spidev0.0", O_RDWR);
    if (spi_fd < 0) {
        perror("Error opening SPI device");
        return;
    }
    printf("Debug: SPI opened successfully\n");

    // Configure SPI
    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = 30000000;

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Error setting SPI mode");
        return;
    }
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        perror("Error setting SPI bits");
        return;
    }
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Error setting SPI speed");
        return;
    }
    printf("Debug: SPI configured successfully\n");

    // Initialize GPIO using libgpiod
    printf("Debug: Initializing GPIO\n");
    
    chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip) {
        perror("Error opening GPIO chip");
        return;
    }

    // Get GPIO lines
    dc_line = gpiod_chip_get_line(chip, DC_PIN);
    rst_line = gpiod_chip_get_line(chip, RST_PIN);
    busy_line = gpiod_chip_get_line(chip, BUSY_PIN);

    if (!dc_line || !rst_line || !busy_line) {
        printf("Error: Failed to get GPIO lines\n");
        return;
    }

    // Configure GPIO directions
    if (gpiod_line_request_output(dc_line, "eink_dc", 0) < 0 ||
        gpiod_line_request_output(rst_line, "eink_rst", 0) < 0 ||
        gpiod_line_request_input(busy_line, "eink_busy") < 0) {
        printf("Error: Failed to configure GPIO directions\n");
        return;
    }

    printf("Debug: GPIO initialized successfully\n");

    // Initialize old_data buffer to white (0xFF) since display starts white
    memset(old_data, 0xFF, BUFFER_SIZE);
    
    printf("Debug: Display buffer initialized to white\n");
    
    printf("Debug: Starting EPD_init_Fast\n");
    EPD_init_Fast();
    printf("Debug: eink_init complete\n");
}

// Cleanup function
void eink_cleanup(void) {
    if (spi_fd >= 0) {
        close(spi_fd);
    }

    // Release GPIO lines
    if (dc_line) gpiod_line_release(dc_line);
    if (rst_line) gpiod_line_release(rst_line);
    if (busy_line) gpiod_line_release(busy_line);
    
    // Close GPIO chip
    if (chip) gpiod_chip_close(chip);
}

// LUT data from Python driver
const uint8_t LUT_ALL[216] = {
    0x01,	0x05,	0x20,	0x19,	0x0A,	0x01,	0x01,
    0x05,	0x0A,	0x01,	0x0A,	0x01,	0x01,	0x01,
    0x05,	0x09,	0x02,	0x03,	0x04,	0x01,	0x01,
    0x01,	0x04,	0x04,	0x02,	0x00,	0x01,	0x01,
    0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,
    0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,
    0x01,	0x05,	0x20,	0x19,	0x0A,	0x01,	0x01,
    0x05,	0x4A,	0x01,	0x8A,	0x01,	0x01,	0x01,
    0x05,	0x49,	0x02,	0x83,	0x84,	0x01,	0x01,
    0x01,	0x84,	0x84,	0x82,	0x00,	0x01,	0x01,
    0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,
    0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,
    0x01,	0x05,	0x20,	0x99,	0x8A,	0x01,	0x01,
    0x05,	0x4A,	0x01,	0x8A,	0x01,	0x01,	0x01,
    0x05,	0x49,	0x82,	0x03,	0x04,	0x01,	0x01,
    0x01,	0x04,	0x04,	0x02,	0x00,	0x01,	0x01,
    0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,
    0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,
    0x01,	0x85,	0x20,	0x99,	0x0A,	0x01,	0x01,
    0x05,	0x4A,	0x01,	0x8A,	0x01,	0x01,	0x01,
    0x05,	0x49,	0x02,	0x83,	0x04,	0x01,	0x01,
    0x01,	0x04,	0x04,	0x02,	0x00,	0x01,	0x01,
    0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,
    0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,
    0x01,	0x85,	0xA0,	0x99,	0x0A,	0x01,	0x01,
    0x05,	0x4A,	0x01,	0x8A,	0x01,	0x01,	0x01,
    0x05,	0x49,	0x02,	0x43,	0x04,	0x01,	0x01,
    0x01,	0x04,	0x04,	0x42,	0x00,	0x01,	0x01,
    0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,
    0x01,	0x00,	0x00,	0x00,	0x00,	0x01,	0x01,
    0x09,	0x10,	0x3F,	0x3F,	0x00,	0x0B,
};

void eink_clear(bool poweroff) {
    EPD_W21_WriteCMD(0x10);
    gpio_write(DC_PIN, 1);
    
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)old_data,
        .rx_buf = 0,
        .len = sizeof(old_data),
        .speed_hz = 30000000,
        .bits_per_word = 8,
        .delay_usecs = 0,
    };
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);

    EPD_W21_WriteCMD(0x13);
    gpio_write(DC_PIN, 1);
    
    uint8_t clear_data[12480] = {0};
    tr.tx_buf = (unsigned long)clear_data;
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);

    memcpy(old_data, clear_data, sizeof(clear_data));

    EPD_W21_WriteCMD(0x12);
    delay_ms(1);
    lcd_chkstatus();

    if (poweroff) {
        EPD_W21_WriteCMD(0x02);
        lcd_chkstatus();
    }
}

// Test function to display white screen
void eink_test_white(void) {
    printf("Debug: Testing display with white screen\n");
    
    // Initialize fast mode first
    EPD_init_Fast();
    
    // // Then initialize partial refresh mode
    // epd_init_part();
    
    // Create white data (all 0xFF)
    // For 240x416 display in 1-bit mode: (240 * 416) / 8 = 12480 bytes
    uint8_t white_data[12480];  // Each byte holds 8 pixels
    memset(white_data, 0xFF, sizeof(white_data));
    
    printf("Debug: Sending white data to display (%zu bytes)\n", sizeof(white_data));
    pic_display(white_data, sizeof(white_data));
    
    // Put display to sleep
    EPD_sleep();
}

void pic_display(const uint8_t* data, size_t size) {
    total_spi_bytes = 0;  // Reset counter at start
    printf("Debug: Starting pic_display (normal mode)\n");
    if (!data || size == 0 || size > BUFFER_SIZE) {
        printf("Error: Invalid data or size in pic_display\n");
        return;
    }

    // Send old data (what's currently on display)
    EPD_W21_WriteCMD(0x10);
    gpio_write(DC_PIN, 1);
    spi_write_keep_cs(old_data, size);  // Send current display contents

    // Send new data
    EPD_W21_WriteCMD(0x13);
    gpio_write(DC_PIN, 1);
    spi_write_keep_cs(data, size);

    // Update old_data buffer with new contents
    memcpy(old_data, data, size);

    // Refresh display
    EPD_W21_WriteCMD(0x12);
    delay_ms(1);
    lcd_chkstatus();
    
    printf("Debug: pic_display complete - Total SPI bytes sent: %zu\n", total_spi_bytes);
}

// Update spi_write_keep_cs function to track bytes
static void spi_write_keep_cs(const uint8_t* data, size_t size) {
    const size_t MAX_CHUNK = 4096;  // Maximum SPI transfer size
    size_t remaining = size;
    size_t offset = 0;
    
    total_spi_bytes += size;  // Track total bytes sent
    printf("Debug: SPI sending %zu bytes (total: %zu)\n", size, total_spi_bytes);

    while (remaining > 0) {
        size_t chunk_size = (remaining > MAX_CHUNK) ? MAX_CHUNK : remaining;
        
        spi_delay();  // Add delay before transmission
        struct spi_ioc_transfer tr = {
            .tx_buf = (unsigned long)(data + offset),
            .rx_buf = 0,
            .len = chunk_size,
            .speed_hz = 30000000,
            .bits_per_word = 8,
            .delay_usecs = 0,
            .cs_change = (remaining <= MAX_CHUNK) ? 1 : 0,  // Release CS on last chunk
        };
        
        int ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
        if (ret < 0) {
            perror("Error in SPI transfer");
            return;
        }

        remaining -= chunk_size;
        offset += chunk_size;
    }
}

// Implement write_full_lut function
static void write_full_lut(void) {
    EPD_W21_WriteCMD(0x20);  // Write VCOM register
    for (int i = 0; i < 42; i++) {
        EPD_W21_WriteDATA(LUT_ALL[i]);
    }

    EPD_W21_WriteCMD(0x21);  // Write LUTWW register
    for (int i = 42; i < 84; i++) {
        EPD_W21_WriteDATA(LUT_ALL[i]);
    }

    EPD_W21_WriteCMD(0x22);  // Write LUTR register
    for (int i = 84; i < 126; i++) {
        EPD_W21_WriteDATA(LUT_ALL[i]);
    }

    EPD_W21_WriteCMD(0x23);  // Write LUTW register
    for (int i = 126; i < 168; i++) {
        EPD_W21_WriteDATA(LUT_ALL[i]);
    }

    EPD_W21_WriteCMD(0x24);  // Write LUTB register
    for (int i = 168; i < 210; i++) {
        EPD_W21_WriteDATA(LUT_ALL[i]);
    }
}

// Update epd_w21_init_4g function to match Python
void epd_w21_init_4g(void) {
    EPD_W21_Init();  // Reset the e-paper display

    // Panel Setting
    EPD_W21_WriteCMD(0x00);
    EPD_W21_WriteDATA(0xFF);  // LUT from MCU
    EPD_W21_WriteDATA(0x0D);

    // Power Setting
    EPD_W21_WriteCMD(0x01);
    EPD_W21_WriteDATA(0x03);  // Enable internal VSH, VSL, VGH, VGL
    EPD_W21_WriteDATA(LUT_ALL[211]);  // VGH=20V, VGL=-20V
    EPD_W21_WriteDATA(LUT_ALL[212]);  // VSH=15V
    EPD_W21_WriteDATA(LUT_ALL[213]);  // VSL=-15V
    EPD_W21_WriteDATA(LUT_ALL[214]);  // VSHR

    // Booster Soft Start
    EPD_W21_WriteCMD(0x06);
    EPD_W21_WriteDATA(0xD7);  // D7
    EPD_W21_WriteDATA(0xD7);  // D7
    EPD_W21_WriteDATA(0x27);  // 2F

    // PLL Control - Frame Rate
    EPD_W21_WriteCMD(0x30);
    EPD_W21_WriteDATA(LUT_ALL[210]);  // PLL

    // CDI Setting
    EPD_W21_WriteCMD(0x50);
    EPD_W21_WriteDATA(0x57);

    // TCON Setting
    EPD_W21_WriteCMD(0x60);
    EPD_W21_WriteDATA(0x22);

    // Resolution Setting
    EPD_W21_WriteCMD(0x61);
    EPD_W21_WriteDATA(0xF0);  // HRES[7:3] - 240
    EPD_W21_WriteDATA(0x01);  // VRES[15:8] - 320
    EPD_W21_WriteDATA(0xA0);  // VRES[7:0]

    EPD_W21_WriteCMD(0x65);
    EPD_W21_WriteDATA(0x00);  // Additional resolution setting

    // VCOM_DC Setting
    EPD_W21_WriteCMD(0x82);
    EPD_W21_WriteDATA(LUT_ALL[215]);  // -2.0V

    // Power Saving Register
    EPD_W21_WriteCMD(0xE3);
    EPD_W21_WriteDATA(0x88);  // VCOM_W[3:0], SD_W[3:0]

    // LUT Setting
    write_full_lut();

    // Power ON
    EPD_W21_WriteCMD(0x04);
    lcd_chkstatus();  // Check if the display is ready
}

// Helper function to save 1-bit pixels as PNG
static void save_pixels_as_png(const uint8_t* px_map, uint32_t width, uint32_t height, const char* filename) {
    // Create output directory if it doesn't exist
    system("mkdir -p output");
    
    // Skip the palette (first 8 bytes)
    px_map += 8;
    
    // Allocate buffer for RGBA image (4 bytes per pixel)
    uint8_t* rgba = malloc(width * height * 4);
    if (!rgba) {
        printf("Error: Failed to allocate RGBA buffer\n");
        return;
    }

    // For debugging
    printf("Debug: Saving PNG %dx%d to %s\n", width, height, filename);
    printf("Debug: First few bytes of input: ");
    for(int i = 0; i < 16; i++) {
        printf("%02X ", px_map[i]);
    }
    printf("\n");

    // Convert pixels to RGBA
    for(uint32_t y = 0; y < height; y++) {
        for(uint32_t x = 0; x < width; x++) {
            uint32_t i = y * width + x;
            uint32_t rgba_idx = i * 4;
            
            // In I1 format, bits are MSB first in each byte
            uint32_t byte_idx = i / 8;
            uint8_t bit_pos = 7 - (i % 8);  // MSB first (7,6,5,4,3,2,1,0)
            uint8_t pixel = (px_map[byte_idx] >> bit_pos) & 0x01;
            
            // Convert 1-bit value to grayscale (0=black, 1=white)
            uint8_t value = pixel ? 255 : 0;
            
            // Set RGBA values (grayscale)
            rgba[rgba_idx + 0] = value; // R
            rgba[rgba_idx + 1] = value; // G
            rgba[rgba_idx + 2] = value; // B
            rgba[rgba_idx + 3] = 255;   // A (fully opaque)
        }
    }

    // Save as PNG
    unsigned error = lodepng_encode32_file(filename, rgba, width, height);
    if(error) {
        printf("Error saving PNG file: %u: %s\n", error, lodepng_error_text(error));
    } else {
        printf("Debug: Successfully saved pixel data to %s\n", filename);
    }

    free(rgba);
} 
