#ifndef EINK_DRIVER_H
#define EINK_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "lvgl/lvgl.h"

// Display dimensions - Updated to match new Python driver
#define EPD_WIDTH   128
#define EPD_HEIGHT  250

// Buffer size for 1-bit mode (1 bit per pixel, 8 pixels per byte)
#define EPD_ARRAY   ((EPD_WIDTH * EPD_HEIGHT) / 8)  // 4000 bytes
#define BUFFER_SIZE EPD_ARRAY

// LVGL display dimensions (landscape mode for LVGL)
#define LVGL_WIDTH  250  // LVGL width = EPD height (landscape)
#define LVGL_HEIGHT 128  // LVGL height = EPD width (landscape)

// GPIO pins - Keep the same as specified
#define DC_PIN    7  // Data/Command control
#define RST_PIN   13 // Reset
#define BUSY_PIN  9  // Busy status
#define CS_PIN    8  // Chip select

// Full refresh interval - partial mode used everywhere except first frame and every Nth frame
#define FULL_REFRESH_INTERVAL 20

// Display refresh modes
typedef enum {
    MODE_NONE,   // Initial state
    MODE_FAST,   // Fast update (full refresh)
    MODE_PARTIAL // Partial update
} epd_mode_t;

// Basic driver functions
void eink_init(void);
void eink_clear(bool poweroff);
void eink_sleep(void);
void eink_cleanup(void);

// Display functions
void pic_display(const uint8_t* data, size_t size);
void pic_display_fast(const uint8_t* data, size_t size);
void pic_display_partial(const uint8_t* data, size_t size);
void pic_display_4g(const uint8_t* data, size_t size);

// Basemap function for proper partial refresh support
void epd_set_basemap(const uint8_t* data, size_t size);

// E-paper initialization functions - Updated to match Python driver
void EPD_init(void);
void EPD_init_Fast(void);
void EPD_init_Part(void);
void epd_w21_init_4g(void);
void EPD_Display(unsigned char *Image);
void EPD_sleep(void);

// Low-level functions
void EPD_W21_WriteCMD(unsigned char command);
void EPD_W21_WriteDATA(unsigned char data);

// Test function
void eink_test_white(void);

// LVGL integration
void eink_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);
void eink_set_mode(epd_mode_t mode);

// Image processing functions
void rotate_counter_clockwise(const uint8_t* src, uint8_t* dst, uint32_t width, uint32_t height);
void flip_horizontal(uint8_t* data, uint32_t width, uint32_t height);
void pack_pixels_to_bits(const uint8_t* src, uint8_t* dst, uint32_t width, uint32_t height);

#endif // EINK_DRIVER_H 