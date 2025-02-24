#ifndef EINK_DRIVER_H
#define EINK_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "lvgl/lvgl.h"

// Display dimensions
#define EPD_WIDTH   240
#define EPD_HEIGHT  416

// For 2-bit color (A2), stride needs to be aligned to 4 bytes
#define EPD_STRIDE  ((EPD_WIDTH + 3) & ~3)  // Round up to multiple of 4
#define DISP_BUF_SIZE (EPD_STRIDE * EPD_HEIGHT / 4)  // Each byte holds 4 pixels in A2 mode

// Original 1-bit buffer size for e-ink operations
#define BUFFER_SIZE ((EPD_WIDTH * EPD_HEIGHT) / 8)

// GPIO pins
#define DC_PIN    6  // Data/Command control
#define RST_PIN   13 // Reset
#define BUSY_PIN  9  // Busy status
#define CS_PIN    8  // Chip select

// Display refresh modes
typedef enum {
    MODE_NONE,   // Initial state
    MODE_NORMAL, // Normal mode (EPD_Display)
    MODE_4GRAY,  // 4 gray levels
    MODE_FAST,   // Fast update
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

// E-paper initialization functions
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

#endif // EINK_DRIVER_H 