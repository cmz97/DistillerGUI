#include "lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include "eink_driver.h"
#include "background.h"
#include "uart_input.h"
#include "audio.h"
#include "ui_callbacks.h"
#include "font_distiller.h"

// Add this declaration before main() if background.h doesn't declare it
extern const lv_image_dsc_t background;  // Declare the external image descriptor

static void hal_init(void);
static void *tick_thread(void *data);
static lv_obj_t *chat_content;  // Add this line

// Add this near the top with other static variables
static bool is_recording = false;
static lv_obj_t *status_bar;
static lv_obj_t *status_label;

// Add this with other static variables at the top
static lv_obj_t *cot_content;   // For chain of thought

// Add this at the top with other static variables
static char thinking_buffer[1024] = {0};  // Buffer for thinking text
static char answer_buffer[4096] = {0};    // Buffer for answer text

// Add these structures to store update data
typedef struct {
    lv_obj_t *label;
    char text[4096];
} label_update_data_t;

// Add these as static variables
static label_update_data_t thinking_data = {0};
static label_update_data_t answer_data = {0};

// Add these as static variables at the top
static lv_timer_t *thinking_timer = NULL;
static lv_timer_t *answer_timer = NULL;

// Move these variable definitions to the top of the file, after the includes
bool ai_is_processing = false;  // Define the variable here
bool got_question = false;      // Define the variable here

// Add these debug macros at the top
#define UI_DEBUG_PRINT(fmt, ...) printf("UI Debug: " fmt "\n", ##__VA_ARGS__)

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

// Add these function declarations
void ui_update_thinking_text(const char *text);
void ui_update_answer_text(const char *text);

// Add this function declaration at the top with other declarations
void handle_stream_end(void);

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
    lv_obj_set_style_text_opa(ip_label, LV_OPA_COVER, 0);  // Full opacity
    lv_obj_set_style_bg_opa(top_status_bar, LV_OPA_COVER, 0);  // Full background opacity

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
    
    // Add padding below header
    lv_obj_set_style_pad_bottom(cot_header, 10, 0);

    // CoT Content - position it below the header
    cot_content = lv_label_create(cot_panel);
    lv_obj_set_width(cot_content, LV_PCT(100));
    lv_label_set_long_mode(cot_content, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(cot_content, lv_color_hex(0x666666), 0);
    lv_obj_align_to(cot_content, cot_header, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);  // Position below header
    lv_label_set_text(cot_content, "Chain of thoughts will appear here...");

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
    lv_label_set_text(chat_content, "AI Response will appear here...");

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
    

    // Test audio
    audio_speak_text("Hello TSVC, how can I help you today?");
    
    // Main loop
    // int counter = 0;
    // char chat_buffer[256];
    
    printf("Debug: Entering while(1) loop\n");
    while(1) {
        // Process LVGL tasks - this is essential for input handling!
        lv_timer_handler();
        usleep(5000);
    }

    audio_cleanup();  // Add this before return
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
    
    printf("Debug: Using custom Distiller font\n");
    
    // Set up the theme with the custom font
    lv_theme_t * theme = lv_theme_default_init(disp,
        lv_color_black(),     // Primary color
        lv_color_white(),     // Secondary color
        false,                // Dark mode = false
        &font_distiller);     // Use custom Distiller font
    
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
    
    printf("Debug: Initializing audio\n");
    if (!audio_init()) {
        printf("Failed to initialize audio!\n");
    }
    printf("Debug: Audio initialized\n");
    
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

// Update the handle_enter_key function
void handle_enter_key(void) {
    if (ai_is_processing) {
        UI_DEBUG_PRINT("AI is still processing, ignoring record button");
        return;
    }

    is_recording = !is_recording;  
    
    if (is_recording) {
        // Start recording
        if (audio_start_recording()) {
            lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x000000), 0);
            lv_obj_set_style_text_color(status_label, lv_color_hex(0xFFFFFF), 0);
            lv_label_set_text(status_label, "RECORDING NOW");
        } else {
            // If recording failed to start
            is_recording = false;
            lv_label_set_text(status_label, "RECORDING FAILED!");
        }
    } else {
        // Stop recording and indicate AI is thinking
        audio_stop_recording();
        ai_is_processing = true;
        got_question = false;
        lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x000000), 0);
        lv_obj_set_style_text_color(status_label, lv_color_hex(0xFFFFFF), 0);
        lv_label_set_text(status_label, "AI IS THINKING");
    }
}

// Update the timer callbacks
static void thinking_timer_cb(lv_timer_t *timer) {
    UI_DEBUG_PRINT("Thinking timer callback started");
    if (!thinking_data.label) {
        UI_DEBUG_PRINT("Error: thinking_data.label is NULL");
        thinking_timer = NULL;  // Clear the reference
        return;
    }
    UI_DEBUG_PRINT("Setting thinking text: %s", thinking_data.text);
    lv_label_set_text(thinking_data.label, thinking_data.text);
    lv_refr_now(NULL);  // Force refresh
    UI_DEBUG_PRINT("Thinking text set successfully");
    thinking_timer = NULL;  // Clear the reference
}

static void answer_timer_cb(lv_timer_t *timer) {
    UI_DEBUG_PRINT("Answer timer callback started");
    if (!answer_data.label) {
        UI_DEBUG_PRINT("Error: answer_data.label is NULL");
        answer_timer = NULL;  // Clear the reference
        return;
    }
    UI_DEBUG_PRINT("Setting answer text: %s", answer_data.text);
    lv_label_set_text(answer_data.label, answer_data.text);
    lv_refr_now(NULL);  // Force refresh
    UI_DEBUG_PRINT("Answer text set successfully");
    answer_timer = NULL;  // Clear the reference
}

void ui_update_thinking_text(const char *text) {
    UI_DEBUG_PRINT("ui_update_thinking_text called with text: %s", text ? text : "NULL");
    
    if (!text) {
        UI_DEBUG_PRINT("Error: text is NULL");
        return;
    }
    
    // Store label pointer if not already stored
    if (!thinking_data.label) {
        UI_DEBUG_PRINT("Initializing thinking_data.label");
        thinking_data.label = cot_content;
        if (!thinking_data.label) {
            UI_DEBUG_PRINT("Error: cot_content is NULL");
            return;
        }
    }
    
    // Copy text to our static buffer
    UI_DEBUG_PRINT("Copying text to buffer");
    strncpy(thinking_data.text, text, sizeof(thinking_data.text) - 1);
    thinking_data.text[sizeof(thinking_data.text) - 1] = '\0';
    
    // Schedule the UI update for the next timer callback
    UI_DEBUG_PRINT("Creating thinking timer");
    if (thinking_timer) {
        UI_DEBUG_PRINT("Deleting old thinking timer");
        lv_timer_del(thinking_timer);
        thinking_timer = NULL;
    }
    
    thinking_timer = lv_timer_create(thinking_timer_cb, 0, NULL);
    if (!thinking_timer) {
        UI_DEBUG_PRINT("Error: Failed to create thinking timer");
        return;
    }
    lv_timer_set_repeat_count(thinking_timer, 1);
    UI_DEBUG_PRINT("Thinking timer created successfully");
}

void ui_update_answer_text(const char *text) {
    UI_DEBUG_PRINT("ui_update_answer_text called with text: %s", text ? text : "NULL");
    
    if (!text) {
        UI_DEBUG_PRINT("Error: text is NULL");
        return;
    }
    
    // Store label pointer if not already stored
    if (!answer_data.label) {
        UI_DEBUG_PRINT("Initializing answer_data.label");
        answer_data.label = chat_content;
        if (!answer_data.label) {
            UI_DEBUG_PRINT("Error: chat_content is NULL");
            return;
        }
    }
    
    // Copy text to our static buffer
    UI_DEBUG_PRINT("Copying text to buffer");
    strncpy(answer_data.text, text, sizeof(answer_data.text) - 1);
    answer_data.text[sizeof(answer_data.text) - 1] = '\0';
    
    // Schedule the UI update for the next timer callback
    UI_DEBUG_PRINT("Creating answer timer");
    if (answer_timer) {
        UI_DEBUG_PRINT("Deleting old answer timer");
        lv_timer_del(answer_timer);
        answer_timer = NULL;
    }
    
    answer_timer = lv_timer_create(answer_timer_cb, 0, NULL);
    if (!answer_timer) {
        UI_DEBUG_PRINT("Error: Failed to create answer timer");
        return;
    }
    lv_timer_set_repeat_count(answer_timer, 1);
    UI_DEBUG_PRINT("Answer timer created successfully");
}

// Update or remove the font switching function
void set_ui_font(bool use_custom_font) {
    lv_display_t *disp = lv_display_get_default();
    if (!disp) return;
    
    lv_theme_t *theme = lv_theme_default_init(disp,
        lv_color_black(),
        lv_color_white(),
        false,
        &font_distiller);  // Always use the custom font
    
    lv_disp_set_theme(disp, theme);
    lv_obj_invalidate(lv_scr_act());
}

// Add this function implementation
void handle_stream_end(void) {
    UI_DEBUG_PRINT("Stream ended, resetting UI state");
    ai_is_processing = false;
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0xDDDDDD), 0);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x000000), 0);
    lv_label_set_text(status_label, "PRESS LEFT BUTTON TO RECORD");
} 