#include "lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eink_driver.h"
#include "font_distiller.h"
#include "system_monitor.h"

static void hal_init(void);
static void *tick_thread(void *data);

// Debug callback for LVGL - only for errors
static void my_log_cb(lv_log_level_t level, const char * buf)
{
    if(level <= LV_LOG_LEVEL_ERROR) {
        printf("[LVGL-Error] %s\n", buf);
    }
}

// Essential render callback for e-ink display alignment
static void my_rounder_cb(lv_event_t *e)
{
    lv_area_t *area = lv_event_get_param(e);
    
    // Round coordinates to byte boundaries (multiples of 8) as per LVGL I1 documentation
    // x1 and y1 coordinates are rounded down, x2 and y2 coordinates are rounded up
    area->x1 = (area->x1 & ~0x7);  // Round down to multiple of 8
    area->x2 = (area->x2 | 0x7);   // Round up to multiple of 8  
    area->y1 = (area->y1 & ~0x7);  // Round down to multiple of 8
    area->y2 = (area->y2 | 0x7);   // Round up to multiple of 8
}

int main(void)
{
    printf("Debug: Starting main()\n");
    
    // Initialize LVGL
    lv_init();
    lv_log_register_print_cb(my_log_cb);
    printf("Debug: LVGL initialized\n");

    // Initialize e-ink hardware first
#ifndef LOCAL_DEBUG
    eink_init();
#endif

    // Initialize system monitor
    if (!system_monitor_init()) {
        printf("Error: Failed to initialize system monitor\n");
        return -1;
    }
    printf("Debug: System monitor initialized\n");

    // Initialize LVGL hardware abstraction layer
    hal_init();
    
    printf("Debug: Creating simple WHITE background with BLACK text\n");
    
    // Clear screen first and FORCE white background
    lv_obj_clean(lv_scr_act());
    
    // Force screen to WHITE background
    lv_obj_t * scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    // FORCE refresh to apply white background BEFORE adding text
    lv_obj_invalidate(scr);
    lv_refr_now(NULL);
    lv_timer_handler();
    
    printf("Debug: Applied WHITE background\n");
    
    // Create counter label in the center with BLACK color
    lv_obj_t * label = lv_label_create(scr);
    lv_obj_set_style_text_font(label, &font_distiller, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_center(label);
    
    // System monitor display loop - update every 2 seconds
    char monitor_text[64];
    int update_count = 0;
    
    while (update_count < 50) {  // Run for about 100 seconds (50 updates × 2 seconds)
        // Update system monitor data
        system_monitor_update();
        
        // Get individual values
        float ram_usage = system_monitor_get_ram_usage();
        float cpu_usage = system_monitor_get_cpu_usage();
        float temperature = system_monitor_get_temperature();
        
        // Format the text for display
        snprintf(monitor_text, sizeof(monitor_text), 
                "RAM: %.1f%%\nCPU: %.1f%%\nTEMP: %.1f°C", 
                ram_usage, cpu_usage, temperature);
        
        lv_label_set_text(label, monitor_text);
        lv_obj_center(label);  // Re-center as text width may change
        
        // Force render to display the system info
        lv_obj_invalidate(scr);
        lv_refr_now(NULL);
        lv_timer_handler();
        
        printf("Debug: System monitor update %d - RAM: %.1f%%, CPU: %.1f%%, Temp: %.1f°C\n", 
               update_count + 1, ram_usage, cpu_usage, temperature);
        
        // // Wait 2 seconds before next update
        // sleep(1);
        update_count++;
    }
    
    printf("Debug: System monitor display finished - %d updates completed\n", update_count);
    
    // Clean up system monitor and e-ink display before exit
    printf("Debug: Cleaning up system monitor\n");
    system_monitor_cleanup();
    
#ifndef LOCAL_DEBUG
    printf("Debug: Cleaning up e-ink display before exit\n");
    eink_cleanup();
#endif
    
    return 0;
}

static void hal_init(void)
{
    printf("Debug: Starting hal_init()\n");
    
    // Create a display with LVGL dimensions (landscape mode)
    lv_display_t * disp = lv_display_create(LVGL_WIDTH, LVGL_HEIGHT);
    if (!disp) {
        printf("Failed to create display!\n");
        return;
    }
    
    printf("Debug: Display created with %dx%d resolution (LVGL landscape)\n", LVGL_WIDTH, LVGL_HEIGHT);
    printf("Debug: Physical display: %dx%d (portrait)\n", EPD_WIDTH, EPD_HEIGHT);
    
    // Calculate buffer size for RGB565 (2 bytes per pixel)
    uint32_t buf_size = LVGL_WIDTH * LVGL_HEIGHT * 2;  // Each pixel needs 2 bytes for RGB565
    
    printf("Debug: Required buffer: %d bytes\n", buf_size);
    
    // Allocate draw buffer - full resolution for LVGL landscape mode with RGB565
    static uint8_t buf1[250 * 128 * 2];  // LVGL resolution, 2 bytes per pixel for RGB565
    memset(buf1, 0xFF, sizeof(buf1));  // Initialize to white in RGB565
    
    printf("Debug: Allocated buffer size: %zu bytes\n", sizeof(buf1));
    
    // Set up the display - order is important!
    lv_display_set_flush_cb(disp, eink_flush_cb);              // Set flush callback first
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565); // Use RGB565 for proper text rendering
    lv_display_set_buffers(disp, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_FULL);
    
    // TEMPORARILY DISABLE rounder callback to test if it's causing diagonal line issue
    // lv_display_add_event_cb(disp, my_rounder_cb, LV_EVENT_INVALIDATE_AREA, disp);
    
    printf("Debug: Display setup complete (rounder callback DISABLED to test diagonal line issue)\n");
    
    printf("Debug: Using custom Distiller font (no theme - explicit colors only)\n");
    
    // Skip theme setup - use explicit colors only to avoid conflicts
    // This prevents theme from overriding our explicit white background
    
    printf("Debug: Skipping theme setup to avoid color conflicts\n");
    
    // Force a clean state and refresh
    lv_obj_invalidate(lv_scr_act());
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

