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
#include <ctype.h>
#include <stdlib.h>
#include "system_monitor.h"

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

// Add these variables at the top with other static variables
static char *current_text_for_tts = NULL;  // Store the full text
static char *next_sentence_ptr = NULL;     // Track next sentence position
static bool tts_in_progress = false;       // Track TTS state

// Add this near the top with other static variables
static bool system_monitoring_enabled = false;  // Set to false to disable monitoring

// Add this debug macros at the top
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
void handle_tts_end(void);  // Add this declaration

// Add this function declaration at the top with other declarations
void handle_stream_end(void);

// Add this function declaration at the top with other declarations
static void queue_status_update(const char* text, lv_color_t bg_color, lv_color_t text_color);

// Improve the get_next_sentence function to better handle sentence boundaries
static char* get_next_sentence(const char* text, const char* current_pos) {
    if (!text || !*text) return NULL;
    
    // If current_pos is NULL, start from beginning
    const char* start = current_pos ? current_pos : text;
    
    // Skip any leading whitespace
    while (*start && isspace(*start)) start++;
    if (!*start) return NULL;
    
    // Find the end of the sentence - look for ., !, ? followed by space or end of string
    // Also consider section markers like "Thinking >" and "Answer >" as sentence boundaries
    const char* end = start;
    while (*end) {
        // Check for sentence terminators
        if ((*end == '.' || *end == '!' || *end == '?') && 
            (end[1] == '\0' || isspace(end[1]))) {
            end++;
            // Skip any trailing whitespace to position at the start of next sentence
            while (*end && isspace(*end)) end++;
            break;
        }
        
        // Check for section markers (treat them as sentence boundaries)
        if (strncmp(end, "\n\nThinking >", 12) == 0 || 
            strncmp(end, "\n\nAnswer >", 10) == 0) {
            // Don't include the newlines in the current sentence
            break;
        }
        
        end++;
    }
    
    // If we reached the end without finding a sentence terminator
    if (*end == '\0' && end > start) {
        // Use the whole remaining text as one sentence
        end = start + strlen(start);
    }
    
    // If we found no text, return NULL
    if (end == start) return NULL;
    
    // Allocate and copy the sentence
    size_t len = end - start;
    char* sentence = malloc(len + 1);
    if (!sentence) return NULL;
    
    strncpy(sentence, start, len);
    sentence[len] = '\0';
    
    // Trim trailing whitespace
    char* p = sentence + len - 1;
    while (p >= sentence && isspace(*p)) *p-- = '\0';
    
    UI_DEBUG_PRINT("Extracted sentence: '%s'", sentence);
    UI_DEBUG_PRINT("Next position will be at offset: %ld", end - text);
    
    return sentence;
}

// Add these timer callback functions at an appropriate place in the file
static void reset_status_timer_cb(lv_timer_t *timer) {
    queue_status_update("PRESS LEFT BUTTON TO RECORD", lv_color_hex(0xDDDDDD), lv_color_hex(0x000000));
}

// Update the TTS button handler to better manage state
void handle_tts_button(void) {
    UI_DEBUG_PRINT("TTS button pressed");
    
    // Check if we're busy with other operations
    if (is_recording || ai_is_processing) {
        UI_DEBUG_PRINT("Cannot start TTS: system busy with recording or AI processing");
        return;
    }
    
    // If TTS is already in progress, stop it and return
    if (tts_in_progress) {
        UI_DEBUG_PRINT("TTS in progress, stopping");
        // Stop any ongoing TTS
        audio_stop_playback();
        tts_in_progress = false;
        queue_status_update("TTS STOPPED", lv_color_hex(0xFF0000), lv_color_hex(0xFFFFFF));
        
        // Wait a moment then reset status
        lv_timer_t *reset_timer = lv_timer_create(reset_status_timer_cb, 1000, NULL);
        lv_timer_set_repeat_count(reset_timer, 1);
        return;
    }
    
    // Get the current text from chat panel
    const char* chat_text = lv_label_get_text(chat_content);
    if (!chat_text || !*chat_text) {
        UI_DEBUG_PRINT("No text to play");
        queue_status_update("NO TEXT TO READ", lv_color_hex(0xFF0000), lv_color_hex(0xFFFFFF));
        
        // Wait a moment then reset status
        lv_timer_t *reset_timer = lv_timer_create(reset_status_timer_cb, 1000, NULL);
        lv_timer_set_repeat_count(reset_timer, 1);
        return;
    }
    
    // Store the text and initialize sentence pointer if needed
    if (!current_text_for_tts) {
        UI_DEBUG_PRINT("Starting new TTS session");
        current_text_for_tts = strdup(chat_text);
        next_sentence_ptr = current_text_for_tts;
    }
    
    // If we have a next sentence, play it
    char* sentence = get_next_sentence(current_text_for_tts, next_sentence_ptr);
    if (sentence) {
        tts_in_progress = true;
        // Update status bar using queue system
        queue_status_update("TTS IN PROGRESS", lv_color_hex(0x000000), lv_color_hex(0xFFFFFF));
        
        // Calculate the next position correctly
        // The next position should be where the current sentence ends in the original text
        size_t current_offset = next_sentence_ptr - current_text_for_tts;
        size_t sentence_len = strlen(sentence);
        
        // Find this sentence in the original text starting from current_offset
        const char* found_pos = strstr(next_sentence_ptr, sentence);
        if (found_pos) {
            // Set next_sentence_ptr to just after this sentence in the original text
            size_t new_offset = (found_pos - current_text_for_tts) + sentence_len;
            next_sentence_ptr = current_text_for_tts + new_offset;
            UI_DEBUG_PRINT("Found sentence at offset %ld, next position: %ld", 
                          found_pos - current_text_for_tts, 
                          next_sentence_ptr - current_text_for_tts);
        } else {
            // If we can't find it (shouldn't happen), just advance by the sentence length
            next_sentence_ptr += sentence_len;
            UI_DEBUG_PRINT("Sentence not found in text, advancing to offset: %ld", 
                          next_sentence_ptr - current_text_for_tts);
        }
        
        UI_DEBUG_PRINT("Playing sentence: '%s'", sentence);
        
        // Play the sentence
        if (!audio_speak_text(sentence)) {
            UI_DEBUG_PRINT("TTS playback failed");
            handle_tts_end();
        }
        
        free(sentence);
    } else {
        // No more sentences, reset
        UI_DEBUG_PRINT("No more sentences to play");
        if (current_text_for_tts) {
            free(current_text_for_tts);
            current_text_for_tts = NULL;
        }
        next_sentence_ptr = NULL;
        tts_in_progress = false;
        
        // Show completion message
        queue_status_update("TTS COMPLETE", lv_color_hex(0x00AA00), lv_color_hex(0xFFFFFF));
        
        // Wait a moment then reset status
        lv_timer_t *reset_timer = lv_timer_create(reset_status_timer_cb, 1000, NULL);
        lv_timer_set_repeat_count(reset_timer, 1);
    }
}

// Update the TTS end handler
void handle_tts_end(void) {
    UI_DEBUG_PRINT("TTS playback completed");
    tts_in_progress = false;
    
    // Reset status bar using queue system
    queue_status_update("PRESS LEFT BUTTON TO RECORD", lv_color_hex(0xDDDDDD), lv_color_hex(0x000000));
}

// Add this near the top with other static variables
#define MAX_STATUS_UPDATES 16

typedef struct {
    char text[64];
    lv_color_t bg_color;
    lv_color_t text_color;
} status_update_info_t;

typedef struct {
    status_update_info_t updates[MAX_STATUS_UPDATES];
    int head;
    int tail;
    lv_timer_t *process_timer;
} status_update_queue_t;

static status_update_queue_t status_queue = {0};

// Update the process_status_updates function to avoid using internal LVGL fields
static void process_status_updates(lv_timer_t *timer) {
    if (status_queue.head == status_queue.tail) {
        return;  // Queue is empty
    }
    
    // Process one update - removed the rendering check since it's not accessible
    status_update_info_t *update = &status_queue.updates[status_queue.tail];
    lv_obj_set_style_bg_color(status_bar, update->bg_color, 0);
    lv_obj_set_style_text_color(status_label, update->text_color, 0);
    lv_label_set_text(status_label, update->text);
    
    // Move tail forward
    status_queue.tail = (status_queue.tail + 1) % MAX_STATUS_UPDATES;
}

// Keep the implementation the same
static void queue_status_update(const char* text, lv_color_t bg_color, lv_color_t text_color) {
    // Create or ensure timer exists first
    if (!status_queue.process_timer) {
        status_queue.process_timer = lv_timer_create(process_status_updates, 50, NULL);
        lv_timer_set_repeat_count(status_queue.process_timer, -1);
    }
    
    // Then queue the update
    int next_head = (status_queue.head + 1) % MAX_STATUS_UPDATES;
    if (next_head == status_queue.tail) {
        UI_DEBUG_PRINT("Status update queue full, dropping update");
        return;
    }
    
    // Add update to queue
    status_update_info_t *update = &status_queue.updates[status_queue.head];
    strncpy(update->text, text, sizeof(update->text) - 1);
    update->text[sizeof(update->text) - 1] = '\0';
    update->bg_color = bg_color;
    update->text_color = text_color;
    
    // Move head forward
    status_queue.head = next_head;
}

// Add these variables at the top with other static variables
static char chat_buffer[8192] = {0};  // Larger buffer for the entire conversation
static bool has_content = false;      // Flag to track if we have any content

// Add these structures to store update data for chat content
typedef struct {
    char buffer[8192];
    bool has_content;
} chat_update_data_t;

// Add this as a static variable
static chat_update_data_t chat_data = {0};
static lv_timer_t *chat_update_timer = NULL;

// Add this callback function for chat updates
static void chat_update_timer_cb(lv_timer_t *timer) {
    UI_DEBUG_PRINT("Chat update timer callback started");
    
    // Update the label with the current buffer content
    lv_label_set_text(chat_content, chat_data.buffer);
    
    // Scroll to the bottom - but don't force refresh
    lv_obj_scroll_to_y(lv_obj_get_parent(chat_content), LV_COORD_MAX, LV_ANIM_OFF);
    
    UI_DEBUG_PRINT("Chat text updated successfully");
    chat_update_timer = NULL;  // Clear the reference
}

// Schedule a chat update using the timer system
static void schedule_chat_update(void) {
    // If a timer is already scheduled, we don't need to create another one
    if (chat_update_timer) {
        return;
    }
    
    // Create a new timer for the update
    chat_update_timer = lv_timer_create(chat_update_timer_cb, 0, NULL);
    if (!chat_update_timer) {
        UI_DEBUG_PRINT("Error: Failed to create chat update timer");
        return;
    }
    lv_timer_set_repeat_count(chat_update_timer, 1);
    UI_DEBUG_PRINT("Chat update timer created successfully");
}

// Add this with other static variables
static lv_obj_t *system_info_label;
static lv_timer_t *system_monitor_timer = NULL;

// Add this structure for system monitor updates
typedef struct {
    char text[64];
} system_info_update_t;

// Add this to the status update queue
static system_info_update_t system_info_data = {0};
static lv_timer_t *system_info_timer = NULL;

// Add this callback function for system info updates
static void system_info_timer_cb(lv_timer_t *timer) {
    UI_DEBUG_PRINT("System info update timer callback started");
    
    // Update the label with the current system info
    lv_label_set_text(system_info_label, system_info_data.text);
    
    UI_DEBUG_PRINT("System info updated successfully");
    system_info_timer = NULL;  // Clear the reference
}

// Schedule a system info update using the timer system
static void schedule_system_info_update(const char *text) {
    // If a timer is already scheduled, we don't need to create another one
    if (system_info_timer) {
        return;
    }
    
    // Copy the text to our static buffer
    strncpy(system_info_data.text, text, sizeof(system_info_data.text) - 1);
    system_info_data.text[sizeof(system_info_data.text) - 1] = '\0';
    
    // Create a new timer for the update
    system_info_timer = lv_timer_create(system_info_timer_cb, 0, NULL);
    if (!system_info_timer) {
        UI_DEBUG_PRINT("Error: Failed to create system info update timer");
        return;
    }
    lv_timer_set_repeat_count(system_info_timer, 1);
    UI_DEBUG_PRINT("System info update timer created successfully");
}

// Modify the system_monitor_timer_cb function to check this flag
static void system_monitor_timer_cb(lv_timer_t *timer) {
    // Skip if monitoring is disabled
    if (!system_monitoring_enabled) {
        return;
    }
    
    // Update system monitoring data
    system_monitor_update();
    
    // Get formatted status string
    char *status = system_monitor_get_status_string();
    if (status) {
        // Schedule the update instead of updating directly
        schedule_system_info_update(status);
        free(status);
    }
}

// Add this structure for stream end handling
typedef struct {
    bool dummy;  // Just a placeholder since we don't need data
} stream_end_data_t;

// Add this to the static variables
static stream_end_data_t stream_end_data = {0};
static lv_timer_t *stream_end_timer = NULL;

// Add this callback function for stream end handling
static void stream_end_timer_cb(lv_timer_t *timer) {
    UI_DEBUG_PRINT("Stream end timer callback started");
    
    // Reset flags
    ai_is_processing = false;
    is_recording = false;
    
    // Force reset the audio recording state
    extern bool is_audio_recording(void);
    extern void audio_reset_state(void);
    
    if (is_audio_recording()) {
        UI_DEBUG_PRINT("Audio still recording after stream end, forcing reset");
        audio_reset_state();
    }
    
    // Queue the status update instead of updating directly
    queue_status_update("PRESS LEFT BUTTON TO RECORD", 
                       lv_color_hex(0xDDDDDD),  // Light gray background
                       lv_color_hex(0x000000)); // Black text
    
    // Make sure the chat panel is scrollable with keyboard
    lv_obj_t *chat_panel = lv_obj_get_parent(chat_content);
    if (chat_panel) {
        lv_obj_add_flag(chat_panel, LV_OBJ_FLAG_SCROLLABLE);
        lv_group_add_obj(uart_input_get_group(), chat_panel);
    }
    
    stream_end_timer = NULL;  // Clear the reference
}

// Schedule a stream end update using the timer system
static void schedule_stream_end(void) {
    // If a timer is already scheduled, we don't need to create another one
    if (stream_end_timer) {
        return;
    }
    
    // Create a new timer for the update
    stream_end_timer = lv_timer_create(stream_end_timer_cb, 0, NULL);
    if (!stream_end_timer) {
        UI_DEBUG_PRINT("Error: Failed to create stream end timer");
        return;
    }
    lv_timer_set_repeat_count(stream_end_timer, 1);
    UI_DEBUG_PRINT("Stream end timer created successfully");
}

// Update the handle_stream_end function to use the timer
void handle_stream_end(void) {
    UI_DEBUG_PRINT("Stream ended, scheduling UI state reset");
    schedule_stream_end();
}

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

    // Initialize system monitor
    system_monitor_init();

    // System info label
    system_info_label = lv_label_create(top_status_bar);
    lv_label_set_text(system_info_label, "Initializing system monitor...");
    lv_obj_align(system_info_label, LV_ALIGN_CENTER, 0, 4);
    lv_obj_set_style_text_color(system_info_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_opa(system_info_label, LV_OPA_COVER, 0);

    // Create timer for system monitor updates only if enabled
    if (system_monitoring_enabled) {
        system_monitor_timer = lv_timer_create(system_monitor_timer_cb, 2000, NULL);
        lv_timer_set_repeat_count(system_monitor_timer, -1);  // Run indefinitely
    } else {
        // Set a static message when monitoring is disabled
        lv_label_set_text(system_info_label, "System monitoring disabled");
    }

    // Remove the Chain of Thoughts panel and make the chat panel taller
    // Chat panel - now starts where CoT used to start
    lv_obj_t * chat_panel = lv_obj_create(bg);
    lv_obj_remove_style_all(chat_panel);
    // Adjust the height to account for equal margins top and bottom
    lv_obj_set_size(chat_panel, LV_PCT(94), LV_PCT(80));  // Reduced from 83% to 80% to allow equal margins
    lv_obj_align_to(chat_panel, top_status_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // Keep 10px top margin
    
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

    // Bottom status bar - keep at the very bottom with same margin as top
    status_bar = lv_obj_create(bg);  // Make it use our static variable
    lv_obj_set_size(status_bar, LV_PCT(100), 30);
    // Position it with the same 10px margin from the chat panel
    lv_obj_align_to(status_bar, chat_panel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // 10px bottom margin
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
    // audio_speak_text("Hello TSVC, how can I help you today?");
    
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
    // Clean up system monitor
    system_monitor_cleanup();
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
    
    // Start the status update processor
    status_queue.process_timer = lv_timer_create(process_status_updates, 50, NULL);
    lv_timer_set_repeat_count(status_queue.process_timer, -1);  // Run indefinitely
    
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

// Update the handle_enter_key function to reset chat panel when starting a new recording
void handle_enter_key(void) {
    if (ai_is_processing) {
        UI_DEBUG_PRINT("AI is still processing, ignoring record button");
        return;
    }

    // Toggle recording state
    is_recording = !is_recording;  
    
    if (is_recording) {
        // Clear the chat panel when starting a new recording
        chat_data.buffer[0] = '\0';
        chat_data.has_content = false;
        schedule_chat_update();
        
        // Start recording
        if (!audio_start_recording()) {  // Check return value and handle failure
            UI_DEBUG_PRINT("Failed to start recording");
            is_recording = false;  // Reset flag if recording failed
            queue_status_update("RECORDING FAILED!", lv_color_hex(0xFF0000), lv_color_hex(0xFFFFFF));
            return;
        }
        
        // Recording started successfully
        queue_status_update("RECORDING NOW", lv_color_hex(0x000000), lv_color_hex(0xFFFFFF));
    } else {
        // Only set AI processing flag if we were actually recording
        if (is_audio_recording()) {
            // Stop recording and indicate AI is thinking
            audio_stop_recording();
            ai_is_processing = true;  // Set this flag to indicate AI is processing
            got_question = false;
            
            // Make sure we update the status to show AI is thinking
            queue_status_update("AI IS THINKING", lv_color_hex(0x000000), lv_color_hex(0xFFFFFF));
        } else {
            // Reset state if we weren't actually recording
            is_recording = false;
            queue_status_update("PRESS LEFT BUTTON TO RECORD", lv_color_hex(0xDDDDDD), lv_color_hex(0x000000));
        }
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

// Update the ui_update_thinking_text function to improve formatting
void ui_update_thinking_text(const char *text) {
    UI_DEBUG_PRINT("ui_update_thinking_text called with text: %s", text ? text : "NULL");
    
    if (!text) {
        UI_DEBUG_PRINT("Error: text is NULL");
        return;
    }
    
    // If this is the first thinking text after a question, add a newline and "Thinking > " prefix
    if (got_question && !strstr(chat_data.buffer, "Thinking > ")) {
        strncat(chat_data.buffer, "\n\nThinking > ", sizeof(chat_data.buffer) - strlen(chat_data.buffer) - 1);
    } else if (strstr(chat_data.buffer, "Thinking > ")) {
        // If we already have thinking text, just append with a space
        strncat(chat_data.buffer, " ", sizeof(chat_data.buffer) - strlen(chat_data.buffer) - 1);
    } else {
        // If we don't have a question yet but received thinking text, start with "Thinking > "
        strncat(chat_data.buffer, "Thinking > ", sizeof(chat_data.buffer) - strlen(chat_data.buffer) - 1);
    }
    
    // Append the new thinking text
    strncat(chat_data.buffer, text, sizeof(chat_data.buffer) - strlen(chat_data.buffer) - 1);
    
    // Schedule the update instead of forcing it immediately
    schedule_chat_update();
    
    chat_data.has_content = true;
}

// Update the ui_update_answer_text function to improve formatting
void ui_update_answer_text(const char *text) {
    UI_DEBUG_PRINT("ui_update_answer_text called with text: %s", text ? text : "NULL");
    
    if (!text) {
        UI_DEBUG_PRINT("Error: text is NULL");
        return;
    }
    
    // Check if this is a new question
    if (strncmp(text, "Question:", 9) == 0) {
        // If we already have content, add extra newlines for separation
        if (chat_data.has_content) {
            strncat(chat_data.buffer, "\n\n\n", sizeof(chat_data.buffer) - strlen(chat_data.buffer) - 1);
        }
        
        // Replace "Question:" with "Question >"
        char formatted_question[4096];
        snprintf(formatted_question, sizeof(formatted_question), "Question %s", text + 9);
        
        // Clear buffer for new conversation if it's getting too large
        if (strlen(chat_data.buffer) > sizeof(chat_data.buffer) - 5000) {
            chat_data.buffer[0] = '\0';
            chat_data.has_content = false;
        }
        
        // Add the question to the buffer
        strncat(chat_data.buffer, formatted_question, sizeof(chat_data.buffer) - strlen(chat_data.buffer) - 1);
    } 
    // Check if this is an answer (not a question and not already in the buffer)
    else if (!strstr(chat_data.buffer, "Answer > ")) {
        // Add a newline and "Answer > " prefix with extra spacing
        strncat(chat_data.buffer, "\n\n\nAnswer > ", sizeof(chat_data.buffer) - strlen(chat_data.buffer) - 1);
        
        // Add the answer text
        strncat(chat_data.buffer, text, sizeof(chat_data.buffer) - strlen(chat_data.buffer) - 1);
    } 
    // Otherwise, just append to the existing answer
    else {
        // First check if we need to add a space
        size_t len = strlen(chat_data.buffer);
        if (len > 0 && chat_data.buffer[len-1] != ' ' && chat_data.buffer[len-1] != '\n') {
            strncat(chat_data.buffer, " ", sizeof(chat_data.buffer) - strlen(chat_data.buffer) - 1);
        }
        
        // Then append the new text
        strncat(chat_data.buffer, text, sizeof(chat_data.buffer) - strlen(chat_data.buffer) - 1);
    }
    
    // Schedule the update instead of forcing it immediately
    schedule_chat_update();
    
    chat_data.has_content = true;
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