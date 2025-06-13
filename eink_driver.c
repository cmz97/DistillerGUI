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
#include "lvgl/lvgl.h"

// Debug option to enable/disable PNG saving (resource intensive)
#define EINK_DEBUG_SAVE_PNG 0  // Set to 0 to disable PNG saving, 1 to enable

static int spi_fd = -1;

static epd_mode_t current_mode = MODE_PARTIAL;
static epd_mode_t previous_mode = MODE_NONE;

// Version check for libgpiod
#ifdef GPIOD_V2
static struct gpiod_chip *chip;
static struct gpiod_line_request *dc_request;
static struct gpiod_line_request *rst_request;
static struct gpiod_line_request *busy_request;
#else
static struct gpiod_chip *chip;
static struct gpiod_line *dc_line;
static struct gpiod_line *rst_line;
static struct gpiod_line *busy_line;
#endif

// Watchdog counter for busy checking
static int watchdog_counter = 0;

// Frame counter for mode switching
static uint32_t frame_counter = 0;
static bool first_frame = true;

// Forward declarations
static void delay_ms(int ms);
static void gpio_write(int pin, int value);
static int gpio_read(int pin);
static void lcd_chkstatus(void);
static void spi_delay(void);
static void epd_w21_write_cmd(uint8_t cmd);
static void epd_w21_write_data(uint8_t data);

// Image processing functions (currently unused - working with bit-packed data directly)
// TODO: Implement bit-level transformations if needed for orientation correction

// Basic utility functions
static void delay_ms(int ms) {
    usleep(ms * 1000);
}

static void gpio_write(int pin, int value) {
#ifdef GPIOD_V2
    struct gpiod_line_request *request = NULL;
    
    if (pin == DC_PIN) {
        request = dc_request;
    } else if (pin == RST_PIN) {
        request = rst_request;
    }
    
    if (request) {
        if (gpiod_line_request_set_value(request, pin, value) < 0) {
            printf("Error: Failed to set GPIO %d to %d\n", pin, value);
        }
    }
#else
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
#endif
}

static int gpio_read(int pin) {
#ifdef GPIOD_V2
    if (pin == BUSY_PIN && busy_request) {
        return gpiod_line_request_get_value(busy_request, pin);
    }
#else
    if (pin == BUSY_PIN && busy_line) {
        return gpiod_line_get_value(busy_line);
    }
#endif
    return -1;
}

static void spi_delay(void) {
    usleep(10);  // 10 microseconds delay
}

static void lcd_chkstatus(void) {
    watchdog_counter = 0;
    while (gpio_read(BUSY_PIN) == 1 && watchdog_counter < 1000) {  // =1 BUSY
        delay_ms(10);
        watchdog_counter++;
    }
    printf("Debug: Busy wait counter: %d\n", watchdog_counter);
    watchdog_counter = 0;
}

static void epd_w21_write_cmd(uint8_t cmd) {
    spi_delay();
    gpio_write(DC_PIN, 0);
    
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)&cmd,
        .rx_buf = 0,
        .len = 1,
        .speed_hz = 40000000,  // Match Python baudrate
        .bits_per_word = 8,
        .delay_usecs = 0,
        .cs_change = 1,
    };
    
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        perror("Error in SPI command transfer");
    }
}

static void epd_w21_write_data(uint8_t data) {
    spi_delay();
    gpio_write(DC_PIN, 1);
    
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)&data,
        .rx_buf = 0,
        .len = 1,
        .speed_hz = 40000000,  // Match Python baudrate
        .bits_per_word = 8,
        .delay_usecs = 0,
        .cs_change = 1,
    };
    
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        perror("Error in SPI data transfer");
    }
}

// Main initialization functions - Based on Python eink_dsp.py
void EPD_init(void) {
    printf("Debug: EPD_init - Full screen refresh initialization\n");
    
    // Module reset
    gpio_write(RST_PIN, 0);
    delay_ms(10);  // At least 10ms delay
    gpio_write(RST_PIN, 1);
    delay_ms(10);  // At least 10ms delay
    
    lcd_chkstatus();
    epd_w21_write_cmd(0x12);  // SWRESET
    lcd_chkstatus();
    
    epd_w21_write_cmd(0x01);  // Driver output control
    epd_w21_write_data((EPD_HEIGHT-1) % 256);
    epd_w21_write_data((EPD_HEIGHT-1) / 256);
    epd_w21_write_data(0x00);

    epd_w21_write_cmd(0x11);  // data entry mode
    epd_w21_write_data(0x01);  // Normal mode

    epd_w21_write_cmd(0x44);  // set Ram-X address start/end position
    epd_w21_write_data(0x00);                    // Start first
    epd_w21_write_data(EPD_WIDTH/8-1);           // End second

    epd_w21_write_cmd(0x45);  // set Ram-Y address start/end position
    epd_w21_write_data((EPD_HEIGHT-1) % 256);    // Start with height-1
    epd_w21_write_data((EPD_HEIGHT-1) / 256);
    epd_w21_write_data(0x00);                    // End with 0
    epd_w21_write_data(0x00);

    epd_w21_write_cmd(0x3C);  // BorderWavefrom
    epd_w21_write_data(0x05);

    epd_w21_write_cmd(0x21);  // Display update control
    epd_w21_write_data(0x00);
    epd_w21_write_data(0x80);  // Normal mode

    epd_w21_write_cmd(0x18);  // Read built-in temperature sensor
    epd_w21_write_data(0x80);

    epd_w21_write_cmd(0x4E);  // set RAM x address count
    epd_w21_write_data(0x00);  // Start at 0
        
    epd_w21_write_cmd(0x4F);  // set RAM y address count
    epd_w21_write_data((EPD_HEIGHT-1) % 256);  // Start at height-1
    epd_w21_write_data((EPD_HEIGHT-1) / 256);
    lcd_chkstatus();
}

// COMMENTED OUT - Fast refresh no longer used, only partial refresh
/*
void EPD_init_Fast(void) {
    printf("Debug: EPD_init_Fast - Fast refresh initialization (adapted from Python epd_init_fast)\n");
    
    // Module reset
    gpio_write(RST_PIN, 0);
    delay_ms(10);  // At least 10ms delay
    gpio_write(RST_PIN, 1);
    delay_ms(10);  // At least 10ms delay
    
    epd_w21_write_cmd(0x12);  // SWRESET
    lcd_chkstatus();
    
    epd_w21_write_cmd(0x18);  // Read built-in temperature sensor
    epd_w21_write_data(0x80);
    
    epd_w21_write_cmd(0x22);  // Load temperature value
    epd_w21_write_data(0xB1);
    epd_w21_write_cmd(0x20);
    lcd_chkstatus();

    epd_w21_write_cmd(0x1A);  // Write to temperature register
    epd_w21_write_data(0x5A);  // Fast2 value (0x5A vs 0x64 in original fast) - matches Python
    epd_w21_write_data(0x00);
    
    epd_w21_write_cmd(0x22);  // Load temperature value
    epd_w21_write_data(0x91);
    epd_w21_write_cmd(0x20);
    lcd_chkstatus();
}
*/

void EPD_init_Part(void) {
    printf("Debug: EPD_init_Part - Partial refresh initialization\n");
    
    // Module reset - adapted from Arduino code
    gpio_write(RST_PIN, 0);  // EPD_W21_RST_0
    delay_ms(10);  // At least 10ms delay
    gpio_write(RST_PIN, 1);  // EPD_W21_RST_1
    delay_ms(10);  // At least 10ms delay
    
    lcd_chkstatus();  // Epaper_READBUSY()
    epd_w21_write_cmd(0x12);  // SWRESET
    lcd_chkstatus();  // Epaper_READBUSY()
    
    epd_w21_write_cmd(0x01);  // Driver output control
    epd_w21_write_data((EPD_HEIGHT-1) % 256);
    epd_w21_write_data((EPD_HEIGHT-1) / 256);
    epd_w21_write_data(0x00);

    epd_w21_write_cmd(0x11);  // data entry mode
    epd_w21_write_data(0x01);

    epd_w21_write_cmd(0x44);  // set Ram-X address start/end position
    epd_w21_write_data(0x00);
    epd_w21_write_data(EPD_WIDTH/8-1);

    epd_w21_write_cmd(0x45);  // set Ram-Y address start/end position
    epd_w21_write_data((EPD_HEIGHT-1) % 256);
    epd_w21_write_data((EPD_HEIGHT-1) / 256);
    epd_w21_write_data(0x00);
    epd_w21_write_data(0x00);

    epd_w21_write_cmd(0x3C);  // BorderWavefrom
    epd_w21_write_data(0x05);

    epd_w21_write_cmd(0x21);  // Display update control
    epd_w21_write_data(0x00);
    epd_w21_write_data(0x80);

    epd_w21_write_cmd(0x18);  // Read built-in temperature sensor
    epd_w21_write_data(0x80);

    epd_w21_write_cmd(0x4E);  // set RAM x address count to 0
    epd_w21_write_data(0x00);
    epd_w21_write_cmd(0x4F);  // set RAM y address count to 0X199
    epd_w21_write_data((EPD_HEIGHT-1) % 256);
    epd_w21_write_data((EPD_HEIGHT-1) / 256);
    lcd_chkstatus();  // Epaper_READBUSY()
    
    printf("Debug: EPD_init_Part initialization complete\n");
}

// Update functions - Based on Python eink_dsp.py
static void epd_update(void) {
    // Full screen refresh update function
    epd_w21_write_cmd(0x22);  // Display Update Control
    epd_w21_write_data(0xF7);
    epd_w21_write_cmd(0x20);  // Activate Display Update Sequence
    lcd_chkstatus();
}

// COMMENTED OUT - Fast refresh no longer used, only partial refresh
/*
static void epd_update_fast(void) {
    // Fast refresh update function
    epd_w21_write_cmd(0x22);  // Display Update Control
    epd_w21_write_data(0xC7);
    epd_w21_write_cmd(0x20);  // Activate Display Update Sequence
    lcd_chkstatus();
}
*/

static void epd_update_part(void) {
    // Partial refresh update function
    epd_w21_write_cmd(0x22);  // Display Update Control
    epd_w21_write_data(0xFF);
    epd_w21_write_cmd(0x20);  // Activate Display Update Sequence
    lcd_chkstatus();
}

// Display functions - Based on Python eink_dsp.py
void pic_display(const uint8_t* data, size_t size) {
    printf("Debug: pic_display - Full screen refresh\n");
    if (!data || size != EPD_ARRAY) {
        printf("Error: Invalid data or size in pic_display\n");
        return;
    }

    epd_w21_write_cmd(0x24);  // write RAM for black(0)/white (1)
    for (size_t i = 0; i < size; i++) {
        epd_w21_write_data(data[i]);
    }
    epd_update();
}

// COMMENTED OUT - Fast refresh no longer used, only partial refresh
/*
void pic_display_fast(const uint8_t* data, size_t size) {
    printf("Debug: pic_display_fast - Fast refresh with dual buffer write\n");
    if (!data || size != EPD_ARRAY) {
        printf("Error: Invalid data or size in pic_display_fast\n");
        return;
    }

    // Debug: Check the data being sent to display
    printf("Debug: First 16 bytes to display: ");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    
    // Debug: Count non-white bytes
    int non_white_count = 0;
    for (size_t i = 0; i < size; i++) {
        if (data[i] != 0xFF) non_white_count++;
    }
    printf("Debug: %d/%zu bytes are non-white (have black pixels)\n", non_white_count, size);

    // Fast refresh with dual RAM write - adapted from Python EPD_Display_Fast_Dual()
    // This writes to both RAM buffers for better fast refresh performance
    epd_w21_write_cmd(0x24);  // write RAM for black(0)/white (1)

    for (size_t i = 0; i < size; i++) {
        epd_w21_write_data(data[i]);
    }
    epd_w21_write_cmd(0x26);  // write RAM for black(0)/white (1) - second buffer
    for (size_t i = 0; i < size; i++) {
        epd_w21_write_data(0x00);  // Fill second buffer with black/reference
    }
    epd_update_fast();
}
*/

void pic_display_partial(const uint8_t* data, size_t size) {
    printf("Debug: pic_display_partial - Partial refresh (like EPD_Dis_PartAll in demo)\n");
    if (!data || size != EPD_ARRAY) {
        printf("Error: Invalid data or size in pic_display_partial\n");
        return;
    }

    gpio_write(RST_PIN, 0);
    delay_ms(10);
    gpio_write(RST_PIN, 1);
    delay_ms(10);

    // Partial refresh setup - the basemap should already be established
    // No reset needed for partial refresh - basemap is already in RAM2
    
    epd_w21_write_cmd(0x3C);  // BorderWavefrom
    epd_w21_write_data(0x80);  // Partial refresh border setting

    // Write new image to RAM1 (0x24) - this will be compared against basemap in RAM2 (0x26)
    epd_w21_write_cmd(0x24);  // write new image to RAM1
    for (size_t i = 0; i < size; i++) {
        epd_w21_write_data(data[i]);
    }
    
    // The basemap (RAM2/0x26) remains unchanged - that's the key to partial refresh
    // Only pixels that differ from the basemap will be updated
    
    epd_update_part();  // Update only changed pixels
    
    printf("Debug: Partial refresh completed - only changed pixels updated\n");
}
    

    
// Sleep function
void EPD_sleep(void) {
    printf("Debug: Entering deep sleep\n");
    epd_w21_write_cmd(0x10);  // Enter deep sleep
    epd_w21_write_data(0x01);
    delay_ms(100);
}

// Deep sleep function - should be called before program exit
void EPD_DeepSleep(void) {
    printf("Debug: Entering deep sleep before program exit\n");
    epd_w21_write_cmd(0x10);  // Enter deep sleep
    epd_w21_write_data(0x01);
    delay_ms(100);
    printf("Debug: Deep sleep mode activated\n");
}

// PNG debug functions - save to project root and properly unpack bit data
#if EINK_DEBUG_SAVE_PNG
void save_bitpacked_as_png(const char* filename, const uint8_t* packed_data, uint32_t width, uint32_t height) {
    // Save to project root directory
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "./%s", filename);
    
    printf("Debug: Saving bit-packed PNG: %s (%dx%d)\n", full_path, width, height);
    
    // Calculate total pixels and bytes needed for packed data
    uint32_t total_pixels = width * height;
    uint32_t packed_bytes = (total_pixels + 7) / 8;  // Round up to nearest byte
    
    // Allocate RGBA buffer for PNG (4 bytes per pixel)
    uint8_t* rgba_data = malloc(total_pixels * 4);
    if (!rgba_data) {
        printf("Error: Failed to allocate RGBA buffer\n");
        return;
    }
    
    // Unpack bits and convert to RGBA - MSB first (LVGL I1 standard)
    for (uint32_t pixel_idx = 0; pixel_idx < total_pixels; pixel_idx++) {
        // Calculate which byte and bit position this pixel is in
        uint32_t byte_idx = pixel_idx / 8;
        uint8_t bit_pos = 7 - (pixel_idx % 8);  // MSB first - bit 7 is pixel 0
        
        // Extract the bit - LVGL I1 standard: 0=white, 1=black
        uint8_t bit = (packed_data[byte_idx] >> bit_pos) & 1;
        uint8_t gray_value = bit ? 0 : 255;  // 1=black(0), 0=white(255) - proper LVGL I1
        
        // Set RGBA values
        uint32_t rgba_idx = pixel_idx * 4;
        rgba_data[rgba_idx + 0] = gray_value;  // R
        rgba_data[rgba_idx + 1] = gray_value;  // G
        rgba_data[rgba_idx + 2] = gray_value;  // B
        rgba_data[rgba_idx + 3] = 255;         // A (fully opaque)
    }
    
    // Save PNG file
    unsigned error = lodepng_encode32_file(full_path, rgba_data, width, height);
    if (error) {
        printf("Error: PNG encoding failed: %s\n", lodepng_error_text(error));
    } else {
        printf("Debug: Successfully saved %s\n", full_path);
    }
    
    free(rgba_data);
}
#endif // EINK_DEBUG_SAVE_PNG

// Rotate bit-packed data counter-clockwise 90 degrees
void rotate_bitpacked_ccw_90(const uint8_t* src, uint8_t* dst, uint32_t src_width, uint32_t src_height) {
    // Clear destination buffer
    uint32_t dst_bytes = (src_height * src_width + 7) / 8;
    memset(dst, 0, dst_bytes);
    
    // For each pixel in source
    for (uint32_t src_y = 0; src_y < src_height; src_y++) {
        for (uint32_t src_x = 0; src_x < src_width; src_x++) {
            // Get bit from source - MSB first
            uint32_t src_bit_idx = src_y * src_width + src_x;
            uint32_t src_byte_idx = src_bit_idx / 8;
            uint8_t src_bit_pos = 7 - (src_bit_idx % 8);  // MSB first
            uint8_t src_bit = (src[src_byte_idx] >> src_bit_pos) & 1;
            
            // Calculate destination coordinates (counter-clockwise rotation)
            uint32_t dst_x = src_y;
            uint32_t dst_y = src_width - 1 - src_x;
            
            // Set bit in destination - MSB first
            uint32_t dst_bit_idx = dst_y * src_height + dst_x;
            uint32_t dst_byte_idx = dst_bit_idx / 8;
            uint8_t dst_bit_pos = 7 - (dst_bit_idx % 8);  // MSB first
            
            if (src_bit) {
                dst[dst_byte_idx] |= (1 << dst_bit_pos);
            }
        }
    }
}

// Flush callback - Convert RGB565 to 1-bit for e-ink display
void eink_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map) {
    printf("Debug: Flush callback - area (%d,%d) to (%d,%d)\n", 
           area->x1, area->y1, area->x2, area->y2);
    
    uint32_t lvgl_width = area->x2 - area->x1 + 1;
    uint32_t lvgl_height = area->y2 - area->y1 + 1;
    uint32_t area_pixels = lvgl_width * lvgl_height;
    uint32_t rgb565_bytes = area_pixels * 2;  // 2 bytes per pixel for RGB565
    uint32_t bit_packed_bytes = (area_pixels + 7) / 8;  // 1 bit per pixel packed
    
    printf("Debug: Area size: %dx%d = %d pixels\n", lvgl_width, lvgl_height, area_pixels);
    
    // Allocate buffer for 1-bit conversion
    uint8_t* bit_packed_data = malloc(bit_packed_bytes);
    if (!bit_packed_data) {
        printf("Error: Failed to allocate bit conversion buffer\n");
        lv_display_flush_ready(disp);
        return;
    }
    
    // Initialize bit buffer to white (0 = white in our 1-bit format)
    memset(bit_packed_data, 0x00, bit_packed_bytes);
    
    // Convert RGB565 to 1-bit
    uint16_t* rgb565_pixels = (uint16_t*)px_map;
    int black_pixels = 0;
    
    for (uint32_t pixel_idx = 0; pixel_idx < area_pixels; pixel_idx++) {
        uint16_t rgb565 = rgb565_pixels[pixel_idx];
        
        // Extract RGB components from RGB565
        uint8_t r = (rgb565 >> 11) & 0x1F;  // 5 bits
        uint8_t g = (rgb565 >> 5) & 0x3F;   // 6 bits  
        uint8_t b = rgb565 & 0x1F;          // 5 bits
        
        // Convert to 8-bit values
        r = (r * 255) / 31;
        g = (g * 255) / 63;
        b = (b * 255) / 31;
        
        // Calculate luminance (grayscale)
        uint8_t gray = (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
        
        // Threshold: < 128 = black (1), >= 128 = white (0)
        if (gray < 128) {
            uint32_t byte_idx = pixel_idx / 8;
            uint8_t bit_pos = 7 - (pixel_idx % 8);  // MSB first
            bit_packed_data[byte_idx] |= (1 << bit_pos);
            black_pixels++;
        }
    }
    
    printf("Debug: Converted %d black pixels\n", black_pixels);

#if EINK_DEBUG_SAVE_PNG
    save_bitpacked_as_png("debug_01_lvgl_bitpacked.png", bit_packed_data, lvgl_width, lvgl_height);
#endif
    
    // Horizontal flip (left-right mirror) to fix mirrored display
    printf("Debug: Applying horizontal flip to fix mirrored image\n");
    
    // Allocate buffer for flipped data
    uint8_t* flipped_data = malloc(bit_packed_bytes);
    if (!flipped_data) {
        printf("Error: Failed to allocate flip buffer\n");
        free(bit_packed_data);
        lv_display_flush_ready(disp);
        return;
    }
    
    // Initialize flipped buffer
    memset(flipped_data, 0x00, bit_packed_bytes);
    
    // Flip horizontally: for each row, reverse the column order
    for (uint32_t y = 0; y < lvgl_height; y++) {
        for (uint32_t x = 0; x < lvgl_width; x++) {
            // Get bit from source position
            uint32_t src_bit_idx = y * lvgl_width + x;
            uint32_t src_byte_idx = src_bit_idx / 8;
            uint8_t src_bit_pos = 7 - (src_bit_idx % 8);  // MSB first
            uint8_t src_bit = (bit_packed_data[src_byte_idx] >> src_bit_pos) & 1;
            
            // Calculate flipped x position (mirror horizontally)
            uint32_t flipped_x = lvgl_width - 1 - x;
            
            // Set bit in flipped position
            uint32_t dst_bit_idx = y * lvgl_width + flipped_x;
            uint32_t dst_byte_idx = dst_bit_idx / 8;
            uint8_t dst_bit_pos = 7 - (dst_bit_idx % 8);  // MSB first
            
            if (src_bit) {
                flipped_data[dst_byte_idx] |= (1 << dst_bit_pos);
            }
        }
    }
    
    printf("Debug: Horizontal flip complete\n");

#if EINK_DEBUG_SAVE_PNG
    save_bitpacked_as_png("debug_01b_flipped.png", flipped_data, lvgl_width, lvgl_height);
#endif
    
    // For partial updates, we need to handle them differently
    if (lvgl_width != 250 || lvgl_height != 128) {
        printf("Debug: Skipping partial update\n");
        free(bit_packed_data);
        free(flipped_data);
        lv_display_flush_ready(disp);
        return;
    }
    
    // Allocate buffer for rotated data
    uint32_t rotated_bytes = (lvgl_height * lvgl_width + 7) / 8;  // Same size, just rotated
    uint8_t* rotated_data = malloc(rotated_bytes);
    if (!rotated_data) {
        printf("Error: Failed to allocate rotation buffer\n");
        free(bit_packed_data);
        free(flipped_data);
        lv_display_flush_ready(disp);
        return;
    }
    
    // Initialize rotated buffer to all white (0x00 in our 1-bit format)
    memset(rotated_data, 0x00, rotated_bytes);
    
    printf("Debug: Rotating flipped image 90° CCW\n");
    
    // Rotate the flipped bit-packed data counter-clockwise 90 degrees
    // LVGL landscape (250x128) -> E-ink portrait (128x250)
    rotate_bitpacked_ccw_90(flipped_data, rotated_data, lvgl_width, lvgl_height);

#if EINK_DEBUG_SAVE_PNG
    save_bitpacked_as_png("debug_02_rotated.png", rotated_data, lvgl_height, lvgl_width);
#endif
    
    // Color inversion step: Flip white to black and black to white
    printf("Debug: Applying color inversion\n");
    
    // Invert all bits in the rotated data (flip white<->black)
    for (uint32_t i = 0; i < rotated_bytes; i++) {
        rotated_data[i] = ~rotated_data[i];  // Flip all bits
    }

#if EINK_DEBUG_SAVE_PNG
    save_bitpacked_as_png("debug_03_inverted.png", rotated_data, lvgl_height, lvgl_width);
#endif
    
    printf("Debug: Sending to display\n");
    
    // Pad rotated data to EPD_ARRAY size if needed
    uint8_t* display_data = malloc(EPD_ARRAY);
    if (!display_data) {
        printf("Error: Failed to allocate display buffer\n");
        free(bit_packed_data);
        free(flipped_data);
        free(rotated_data);
        lv_display_flush_ready(disp);
        return;
    }
    
    // Copy rotated data and pad with white (0xFF after inversion means white)
    memset(display_data, 0xFF, EPD_ARRAY);  // Fill with white 
    uint32_t copy_size = (rotated_bytes < EPD_ARRAY) ? rotated_bytes : EPD_ARRAY;
    memcpy(display_data, rotated_data, copy_size);
    
    // Always use partial mode - EPD_init_Part() first, then epd_set_basemap or pic_display_partial
    if (first_frame) {
        // First frame: Initialize partial mode and establish basemap
        printf("Debug: Initializing partial refresh mode for first frame\n");
        EPD_init_Part();  // Initialize partial refresh mode
        printf("Debug: Partial mode initialized, establishing basemap\n");
        epd_set_basemap(display_data, EPD_ARRAY);  // Set basemap
    } else if (frame_counter % FULL_REFRESH_INTERVAL == 0) {
        // Reset basemap every FULL_REFRESH_INTERVAL to prevent ghosting
        printf("Debug: Resetting basemap to prevent ghosting\n");
        epd_set_basemap(display_data, EPD_ARRAY);
    } else {
        // Use partial refresh (like EPD_Dis_PartAll in demo)
        printf("Debug: Using partial refresh against established basemap\n");
        pic_display_partial(display_data, EPD_ARRAY);
    }
    
    // Update frame counter and clear first frame flag
    frame_counter++;
    if (first_frame) {
        first_frame = false;
    }
    
    // Clean up all buffers
    free(bit_packed_data);
    free(flipped_data);
    free(rotated_data);
    free(display_data);
    lv_display_flush_ready(disp);
}

// Initialize the e-ink driver
void eink_init(void) {
    printf("Debug: Starting eink_init\n");
    
    // Initialize SPI
    spi_fd = open("/dev/spidev0.0", O_RDWR);
    if (spi_fd < 0) {
        perror("Error opening SPI device");
        return;
    }
    printf("Debug: SPI opened successfully\n");

    // Configure SPI to match Python settings
    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = 40000000;  // Match Python baudrate

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

#ifdef GPIOD_V2
    // libgpiod v2.x API
    printf("Debug: Using libgpiod v2.x API\n");
    
    // Configure DC pin (output)
    struct gpiod_line_settings *dc_settings = gpiod_line_settings_new();
    if (!dc_settings) {
        printf("Error: Failed to create DC line settings\n");
        return;
    }
    gpiod_line_settings_set_direction(dc_settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(dc_settings, GPIOD_LINE_VALUE_INACTIVE);
    
    struct gpiod_line_config *dc_config = gpiod_line_config_new();
    if (!dc_config) {
        printf("Error: Failed to create DC line config\n");
        gpiod_line_settings_free(dc_settings);
        return;
    }
    unsigned int dc_pin = DC_PIN;
    gpiod_line_config_add_line_settings(dc_config, &dc_pin, 1, dc_settings);
    
    struct gpiod_request_config *dc_req_config = gpiod_request_config_new();
    if (!dc_req_config) {
        printf("Error: Failed to create DC request config\n");
        gpiod_line_settings_free(dc_settings);
        gpiod_line_config_free(dc_config);
        return;
    }
    gpiod_request_config_set_consumer(dc_req_config, "eink_dc");
    
    dc_request = gpiod_chip_request_lines(chip, dc_req_config, dc_config);
    if (!dc_request) {
        printf("Error: Failed to request DC line\n");
        gpiod_line_settings_free(dc_settings);
        gpiod_line_config_free(dc_config);
        gpiod_request_config_free(dc_req_config);
        return;
    }
    
    gpiod_line_settings_free(dc_settings);
    gpiod_line_config_free(dc_config);
    gpiod_request_config_free(dc_req_config);
    
    // Configure RST pin (output)
    struct gpiod_line_settings *rst_settings = gpiod_line_settings_new();
    if (!rst_settings) {
        printf("Error: Failed to create RST line settings\n");
        return;
    }
    gpiod_line_settings_set_direction(rst_settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(rst_settings, GPIOD_LINE_VALUE_INACTIVE);
    
    struct gpiod_line_config *rst_config = gpiod_line_config_new();
    if (!rst_config) {
        printf("Error: Failed to create RST line config\n");
        gpiod_line_settings_free(rst_settings);
        return;
    }
    unsigned int rst_pin = RST_PIN;
    gpiod_line_config_add_line_settings(rst_config, &rst_pin, 1, rst_settings);
    
    struct gpiod_request_config *rst_req_config = gpiod_request_config_new();
    if (!rst_req_config) {
        printf("Error: Failed to create RST request config\n");
        gpiod_line_settings_free(rst_settings);
        gpiod_line_config_free(rst_config);
        return;
    }
    gpiod_request_config_set_consumer(rst_req_config, "eink_rst");
    
    rst_request = gpiod_chip_request_lines(chip, rst_req_config, rst_config);
    if (!rst_request) {
        printf("Error: Failed to request RST line\n");
        gpiod_line_settings_free(rst_settings);
        gpiod_line_config_free(rst_config);
        gpiod_request_config_free(rst_req_config);
        return;
    }
    
    gpiod_line_settings_free(rst_settings);
    gpiod_line_config_free(rst_config);
    gpiod_request_config_free(rst_req_config);
    
    // Configure BUSY pin (input)
    struct gpiod_line_settings *busy_settings = gpiod_line_settings_new();
    if (!busy_settings) {
        printf("Error: Failed to create BUSY line settings\n");
        return;
    }
    gpiod_line_settings_set_direction(busy_settings, GPIOD_LINE_DIRECTION_INPUT);
    
    struct gpiod_line_config *busy_config = gpiod_line_config_new();
    if (!busy_config) {
        printf("Error: Failed to create BUSY line config\n");
        gpiod_line_settings_free(busy_settings);
        return;
    }
    unsigned int busy_pin = BUSY_PIN;
    gpiod_line_config_add_line_settings(busy_config, &busy_pin, 1, busy_settings);
    
    struct gpiod_request_config *busy_req_config = gpiod_request_config_new();
    if (!busy_req_config) {
        printf("Error: Failed to create BUSY request config\n");
        gpiod_line_settings_free(busy_settings);
        gpiod_line_config_free(busy_config);
        return;
    }
    gpiod_request_config_set_consumer(busy_req_config, "eink_busy");
    
    busy_request = gpiod_chip_request_lines(chip, busy_req_config, busy_config);
    if (!busy_request) {
        printf("Error: Failed to request BUSY line\n");
        gpiod_line_settings_free(busy_settings);
        gpiod_line_config_free(busy_config);
        gpiod_request_config_free(busy_req_config);
        return;
    }
    
    gpiod_line_settings_free(busy_settings);
    gpiod_line_config_free(busy_config);
    gpiod_request_config_free(busy_req_config);
    
#else
    // libgpiod v1.x API (legacy)
    printf("Debug: Using libgpiod v1.x API\n");
    
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
#endif

    printf("Debug: GPIO initialized successfully\n");


    
    // Reset frame counter and first frame flag
    frame_counter = 0;
    first_frame = true;
    
    printf("Debug: eink_init complete\n");
}

// Cleanup function - automatically calls deep sleep before cleanup
void eink_cleanup(void) {
    // Put display in deep sleep before cleanup to extend display lifespan
    EPD_DeepSleep();
    
    if (spi_fd >= 0) {
        close(spi_fd);
    }

#ifdef GPIOD_V2
    if (dc_request) gpiod_line_request_release(dc_request);
    if (rst_request) gpiod_line_request_release(rst_request);
    if (busy_request) gpiod_line_request_release(busy_request);
#else
    if (dc_line) gpiod_line_release(dc_line);
    if (rst_line) gpiod_line_release(rst_line);
    if (busy_line) gpiod_line_release(busy_line);
#endif
    
    if (chip) gpiod_chip_close(chip);
    
    printf("Debug: eink_cleanup completed\n");
}



// Clear screen function
void eink_clear(bool poweroff) {
    printf("Debug: Clearing screen\n");
    
    epd_w21_write_cmd(0x24);  // write RAM for black(0)/white (1)
    for (int i = 0; i < EPD_ARRAY; i++) {
        epd_w21_write_data(0xFF);  // White
    }
    epd_update();

    if (poweroff) {
        EPD_sleep();
    }
}



// Basemap function for proper partial refresh support (like EPD_SetRAMValue_BaseMap in demo)
void epd_set_basemap(const uint8_t* data, size_t size) {
    printf("Debug: epd_set_basemap - Setting basemap for partial refresh (like EPD_SetRAMValue_BaseMap)\n");
    if (!data || size != EPD_ARRAY) {
        printf("Error: Invalid data or size in epd_set_basemap\n");
        return;
    }

    // Set basemap in both RAM buffers - this is crucial for partial refresh
    // Write to RAM1 (0x24) - current image
    epd_w21_write_cmd(0x24);  // Write Black and White image to RAM
    for (size_t i = 0; i < size; i++) {
        epd_w21_write_data(data[i]);
    }
    
    // Write to RAM2 (0x26) - previous/base image for comparison
    epd_w21_write_cmd(0x26);  // Write previous image to RAM (basemap)
    for (size_t i = 0; i < size; i++) {
        epd_w21_write_data(data[i]);  // Same data to establish basemap
    }
    
    // Do a full update to establish the basemap (like demo code)
    epd_update();  // Full update to establish basemap on screen
    
    printf("Debug: Basemap established successfully - ready for partial refresh\n");
} 
