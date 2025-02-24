#include "lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include "eink_driver.h"
#include "monorama_14.h"
#include "background.h"
#include "uart_input.h"

// Add this declaration before main() if background.h doesn't declare it
extern const lv_image_dsc_t background;  // Declare the external image descriptor

static void hal_init(void);
static void *tick_thread(void *data);
static lv_obj_t *chat_content;  // Add this line

// Add this near the top with other static variables
static bool is_recording = false;
static lv_obj_t *status_bar;
static lv_obj_t *status_label;

// Debug callback for LVGL - only for errors
static void my_log_cb(lv_log_level_t level, const char * buf)
{
    if(level <= LV_LOG_LEVEL_ERROR) {
        printf("[LVGL-Error] %s\n", buf);
    }
}

// Debug function to print display info
static void print_display_info(lv_display_t * disp)
{
    printf("Display: %dx%d, Color format: %d\n", 
           lv_display_get_horizontal_resolution(disp),
           lv_display_get_vertical_resolution(disp),
           lv_display_get_color_format(disp));
}

static void my_rounder_cb(lv_event_t *e)
{
    lv_area_t *area = lv_event_get_param(e);
    area->x1 = (area->x1 & ~0x7);  // Round down to multiple of 8
    area->x2 = (area->x2 | 0x7);   // Round up to multiple of 8
    area->y1 = (area->y1 & ~0x7);  // Round down to multiple of 8
    area->y2 = (area->y2 | 0x7);   // Round up to multiple of 8
}

// Add at the top with other includes
LV_FONT_DECLARE(monorama_14);  // Declare the font

int main(void)
{
    printf("Debug: Starting main()\n");
    
    // Initialize LVGL
    lv_init();
    lv_log_register_print_cb(my_log_cb);
    printf("Debug: LVGL initialized\n");

    // Initialize e-ink hardware first
    eink_init();

    // Initialize LVGL hardware abstraction layer
    hal_init();
    
    printf("Debug: Creating UI\n");
    
    // Create the UI layout
    lv_obj_t * bg = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(bg);
    lv_obj_set_size(bg, LV_PCT(100), LV_PCT(100));
    // lv_obj_set_style_bg_color(bg, lv_color_hex(0xFFFFFF), 0);
    // lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, 0);
    
    lv_obj_set_style_bg_image_src(bg, &background, 0);
    lv_obj_set_style_bg_image_tiled(bg, true, 0);
    lv_obj_set_style_bg_image_recolor(bg, lv_color_hex(0xDDDDDD), 0);
    lv_obj_set_style_bg_image_recolor_opa(bg, LV_OPA_10, 0);

    // Top status bar
    lv_obj_t * top_status_bar = lv_obj_create(bg);
    lv_obj_set_size(top_status_bar, LV_PCT(100), 38);
    lv_obj_align(top_status_bar, LV_ALIGN_TOP_MID, 0, -8);
    lv_obj_set_style_bg_color(top_status_bar, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(top_status_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(top_status_bar, 1, 0);
    lv_obj_set_style_border_color(top_status_bar, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(top_status_bar, 8, 0);
    lv_obj_clear_flag(top_status_bar, LV_OBJ_FLAG_SCROLLABLE);

    // IP address indicator
    lv_obj_t * ip_label = lv_label_create(top_status_bar);
    lv_label_set_text(ip_label, "OFFLINE QA AGENT DEMO");
    lv_obj_align(ip_label, LV_ALIGN_CENTER, 0, 4);
    lv_obj_set_style_text_color(ip_label, lv_color_hex(0xFFFFFF), 0);

    // Chain of Thoughts panel - adjust height to be slightly smaller
    lv_obj_t * cot_panel = lv_obj_create(bg);
    lv_obj_set_size(cot_panel, LV_PCT(94), LV_PCT(28));  // Reduced from 33% to 28%
    lv_obj_align_to(cot_panel, top_status_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_bg_color(cot_panel, lv_color_hex(0xEEEEEE), 0);
    lv_obj_set_style_bg_opa(cot_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cot_panel, 1, 0);
    lv_obj_set_style_border_color(cot_panel, lv_color_hex(0x000000), 0);
    lv_obj_set_style_pad_all(cot_panel, 10, 0);

    // CoT Header
    lv_obj_t * cot_header = lv_label_create(cot_panel);
    lv_label_set_text(cot_header, "CHAIN OF THOUGHTS");
    lv_obj_align(cot_header, LV_ALIGN_TOP_MID, 0, 0);

    // CoT Content
    lv_obj_t * cot_content = lv_label_create(cot_panel);
    lv_obj_set_width(cot_content, LV_PCT(100));
    lv_label_set_long_mode(cot_content, LV_LABEL_LONG_WRAP);
    lv_label_set_text(cot_content, "THINKING PROCESS WILL APPEAR HERE...");
    lv_obj_align(cot_content, LV_ALIGN_TOP_LEFT, 0, 25);
    lv_obj_set_style_text_color(cot_content, lv_color_hex(0x666666), 0);

    // Chat panel - reduce gap and adjust height
    lv_obj_t * chat_panel = lv_obj_create(bg);
    lv_obj_remove_style_all(chat_panel);
    lv_obj_set_size(chat_panel, LV_PCT(94), LV_PCT(50));  // Reduced from 55% to 50%
    lv_obj_align_to(chat_panel, cot_panel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // Keep the 10px gap
    
    // Add styling to chat panel
    lv_obj_set_style_bg_color(chat_panel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(chat_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(chat_panel, 1, 0);
    lv_obj_set_style_border_color(chat_panel, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(chat_panel, 8, 0);  // Rounded corners
    lv_obj_set_style_pad_all(chat_panel, 10, 0);  // Add padding

    // Configure scrolling
    lv_obj_set_style_pad_right(chat_panel, 5, 0);  // Add padding for scrollbar
    lv_obj_set_scrollbar_mode(chat_panel, LV_SCROLLBAR_MODE_AUTO);  // Show scrollbar when needed
    lv_obj_set_scroll_dir(chat_panel, LV_DIR_VER);  // Only allow vertical scrolling
    lv_obj_clear_flag(chat_panel, LV_OBJ_FLAG_SCROLL_CHAIN);  // Prevent scroll bubbling
    lv_obj_add_flag(chat_panel, LV_OBJ_FLAG_SCROLLABLE);      // Make sure scrolling is enabled

    // Create a label for chat content - declare as global/static at top of file
    chat_content = lv_label_create(chat_panel);  // Initialize here
    lv_obj_set_width(chat_content, LV_PCT(100));
    lv_label_set_long_mode(chat_content, LV_LABEL_LONG_WRAP);  // Enable text wrapping
    lv_obj_set_style_text_color(chat_content, lv_color_hex(0x000000), 0);
    lv_label_set_text(chat_content, "AI RESPONSE WILL APPEAR HERE...AI RESPONSE WILL APPEAR HERE...AI RESPONSE WILL APPEAR HERE...AI RESPONSE WILL APPEAR HERE...AI RESPONSE WILL APPEAR HERE...AI RESPONSE WILL APPEAR HERE...AI RESPONSE WILL APPEAR HERE...AI RESPONSE WILL APPEAR HERE...AI RESPONSE WILL APPEAR HERE...AI RESPONSE WILL APPEAR HERE...AI RESPONSE WILL APPEAR HERE...AI RESPONSE WILL APPEAR HERE...");

    // Add chat panel to input group here
    lv_group_add_obj(uart_input_get_group(), chat_panel);

    // Bottom status bar - keep at the very bottom
    status_bar = lv_obj_create(bg);  // Make it use our static variable
    lv_obj_set_size(status_bar, LV_PCT(100), 30);
    lv_obj_align(status_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0xDDDDDD), 0);
    lv_obj_set_style_bg_opa(status_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(status_bar, 1, 0);
    lv_obj_set_style_border_color(status_bar, lv_color_hex(0x000000), 0);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);

    // Status indicator
    status_label = lv_label_create(status_bar);  // Make it use our static variable
    lv_label_set_text(status_label, "PRESS LEFT BUTTON TO RECORD");
    lv_obj_center(status_label);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x000000), 0);

    printf("Debug: UI creation complete\n");
    printf("Debug: Forcing initial render\n");
    lv_obj_invalidate(bg);
    lv_refr_now(NULL);
    lv_timer_handler();
    
    printf("Debug: Initial render complete\n");
    printf("Debug: Starting main loop\n");
    
    // Main loop
    // int counter = 0;
    // char chat_buffer[256];
    
    printf("Debug: Entering while(1) loop\n");
    while(1) {
        // Process LVGL tasks - this is essential for input handling!
        lv_timer_handler();
        usleep(5000);
    }

    return 0;
}

static void hal_init(void)
{
    printf("Debug: Starting hal_init()\n");
    
    // Create a display
    lv_display_t * disp = lv_display_create(EPD_WIDTH, EPD_HEIGHT);
    if (!disp) {
        printf("Failed to create display!\n");
        return;
    }
    
    printf("Debug: Display created with %dx%d resolution\n", EPD_WIDTH, EPD_HEIGHT);
    
    // Calculate buffer size for FULL mode (1 byte per pixel)
    uint32_t buf_size = EPD_WIDTH * EPD_HEIGHT;  // Each pixel needs 1 byte in LVGL
    
    printf("Debug: Required buffer: %d bytes\n", buf_size);
    
    // Allocate draw buffer - simple buffer, no descriptor
    static uint8_t buf1[240 * 416];  // Full resolution, 1 byte per pixel
    memset(buf1, 0xFF, sizeof(buf1));  // Initialize to white
    
    printf("Debug: Allocated buffer size: %zu bytes\n", sizeof(buf1));
    
    // Set up the display - order is important!
    lv_display_set_flush_cb(disp, eink_flush_cb);          // Set flush callback first
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_I1); // Set format
    lv_display_set_buffers(disp, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_FULL);
    
    // Add rounder callback for byte-aligned updates
    lv_display_add_event_cb(disp, my_rounder_cb, LV_EVENT_INVALIDATE_AREA, disp);
    
    printf("Debug: Display setup complete\n");
    
    // Set up the theme with inverted colors (since e-ink is naturally white)
    lv_theme_t * theme = lv_theme_default_init(disp,
        lv_color_black(),     // Primary color
        lv_color_white(),     // Secondary color
        false,                // Dark mode = false
        &monorama_14);       // Use Monorama font instead of Montserrat
    
    lv_disp_set_theme(disp, theme);
    
    // Set white background for screen and ensure it's applied
    lv_obj_t * scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    printf("Debug: Theme and screen setup complete\n");
    
    // Force a clean state and refresh
    lv_obj_invalidate(scr);
    lv_refr_now(disp);
    lv_timer_handler(); // Process any pending tasks
    
    printf("Debug: Display initialization complete\n");

    printf("Debug: Creating tick thread\n");
    // Create and start the tick thread
    pthread_t tick_thread_id;
    if (pthread_create(&tick_thread_id, NULL, tick_thread, NULL) != 0) {
        printf("Failed to create tick thread!\n");
        return;
    }
    printf("Debug: Tick thread created\n");

    printf("Debug: Initializing UART input\n");
    // Replace the input device initialization with:
    uart_input_init();
    printf("Debug: UART input initialized\n");
    
    printf("Debug: hal_init complete\n");
}

static void *tick_thread(void *data)
{
    (void)data;

    while(1) {
        usleep(5000);  // 5 ms delay
        lv_tick_inc(5); // Tell LVGL that 5 milliseconds have passed
    }

    return NULL;
}

// Add this function to handle the enter key press
void handle_enter_key(void) {
    is_recording = !is_recording;  // Toggle recording state
    
    if (is_recording) {
        // Recording mode
        lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x000000), 0);
        lv_obj_set_style_text_color(status_label, lv_color_hex(0xFFFFFF), 0);
        lv_label_set_text(status_label, "RECORDING NOW");
    } else {
        // Normal mode
        lv_obj_set_style_bg_color(status_bar, lv_color_hex(0xDDDDDD), 0);
        lv_obj_set_style_text_color(status_label, lv_color_hex(0x000000), 0);
        lv_label_set_text(status_label, "PRESS LEFT BUTTON TO RECORD");
    }
} 