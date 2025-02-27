#define _GNU_SOURCE  // Add this at the very top of the file
#include "api_client.h"
#include "ui_callbacks.h"  // Add this include
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG_PRINT(fmt, ...) printf("API Debug: " fmt "\n", ##__VA_ARGS__)

// Structure to hold response data
struct ResponseData {
    char *data;
    size_t size;
};

// Add at the top with other structs
struct StreamContext {
    char *accumulated_answer;
    size_t answer_size;
    size_t answer_capacity;
};

// Callback function to handle response data
static size_t handle_response(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct ResponseData *resp = (struct ResponseData *)userp;

    char *ptr = realloc(resp->data, resp->size + realsize + 1);
    if (!ptr) {
        DEBUG_PRINT("Failed to allocate memory for response");
        return 0;
    }

    resp->data = ptr;
    memcpy(&(resp->data[resp->size]), contents, realsize);
    resp->size += realsize;
    resp->data[resp->size] = 0;

    return realsize;
}

// Modify the callback to handle streaming data
static size_t handle_stream_response(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    const char *data = (const char *)contents;
    struct {
        stream_callback cb;
        void *user_data;
        char *partial_line;
    } *stream_data = userp;
    
    // Process the incoming data line by line
    const char *start = data;
    const char *end = data + realsize;
    const char *newline;
    char *combined = NULL;  // Declare at the start of function
    
    // Handle any partial line from previous callback
    if (stream_data->partial_line) {
        size_t partial_len = strlen(stream_data->partial_line);
        combined = malloc(partial_len + realsize + 1);
        memcpy(combined, stream_data->partial_line, partial_len);
        memcpy(combined + partial_len, data, realsize);
        combined[partial_len + realsize] = '\0';
        
        free(stream_data->partial_line);
        stream_data->partial_line = NULL;
        
        start = combined;
        end = combined + partial_len + realsize;
    }
    
    while (start < end) {
        newline = memchr(start, '\n', end - start);
        if (!newline) {
            // Save partial line for next callback
            size_t remaining = end - start;
            stream_data->partial_line = malloc(remaining + 1);
            memcpy(stream_data->partial_line, start, remaining);
            stream_data->partial_line[remaining] = '\0';
            break;
        }
        
        // Process complete line
        size_t line_len = newline - start;
        if (line_len > 0) {
            char *line = malloc(line_len + 1);
            memcpy(line, start, line_len);
            line[line_len] = '\0';
            
            // Skip "data: " prefix if present
            const char *message = line;
            if (strncmp(line, "data: ", 6) == 0) {
                message += 6;
            }
            
            // Call user callback with the message
            if (stream_data->cb) {
                stream_data->cb(message, stream_data->user_data);
            }
            
            free(line);
        }
        
        start = newline + 1;
    }
    
    if (combined) {  // Free combined buffer if it was allocated
        free(combined);
    }
    
    return realsize;
}

bool api_client_init(void) {
    curl_global_init(CURL_GLOBAL_ALL);
    return true;
}

void api_client_cleanup(void) {
    curl_global_cleanup();
}

bool api_send_audio_file(const char *filename) {
    DEBUG_PRINT("Sending audio file to server: %s", filename);
    
    CURL *curl;
    CURLcode res;
    struct ResponseData response_data = {0};
    FILE *fd;
    bool success = false;

    curl = curl_easy_init();
    if (!curl) {
        DEBUG_PRINT("Failed to initialize CURL");
        return false;
    }

    fd = fopen(filename, "rb");
    if (!fd) {
        DEBUG_PRINT("Failed to open audio file");
        curl_easy_cleanup(curl);
        return false;
    }

    // Get file size
    fseek(fd, 0, SEEK_END);
    long file_size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: audio/wav");
    headers = curl_slist_append(headers, "Accept: text/event-stream");

    // Set up the request
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000/speech-chat");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, fd);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, file_size);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        DEBUG_PRINT("Failed to send request: %s", curl_easy_strerror(res));
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            if (response_data.data) {
                // Process the streaming response
                char *line = strtok(response_data.data, "\n");
                while (line) {
                    if (strncmp(line, "data: ", 6) == 0) {
                        DEBUG_PRINT("Stream: %s", line + 6);
                    }
                    line = strtok(NULL, "\n");
                }
            }
            success = true;
        } else {
            DEBUG_PRINT("Server returned error code: %ld", response_code);
            if (response_data.data) {
                DEBUG_PRINT("Error response: %s", response_data.data);
            }
        }
    }

    if (response_data.data) {
        free(response_data.data);
    }
    curl_slist_free_all(headers);
    fclose(fd);
    curl_easy_cleanup(curl);

    return success;
}

bool api_send_audio_buffer(const char *buffer, size_t buffer_size) {
    DEBUG_PRINT("Sending audio buffer of size %zu bytes", buffer_size);
    
    CURL *curl = curl_easy_init();
    if (!curl) {
        DEBUG_PRINT("Failed to initialize CURL");
        return false;
    }
    
    bool success = false;
    struct curl_slist *headers = NULL;
    struct ResponseData response_data = {0};
    
    headers = curl_slist_append(headers, "Content-Type: audio/wav");
    headers = curl_slist_append(headers, "Accept: text/event-stream");
    
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000/speech-chat");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, buffer_size);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        DEBUG_PRINT("Failed to send audio data: %s", curl_easy_strerror(res));
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            DEBUG_PRINT("Server accepted the audio data");
            if (response_data.data) {
                // Process the streaming response
                char *line = strtok(response_data.data, "\n");
                while (line) {
                    if (strncmp(line, "data: ", 6) == 0) {
                        DEBUG_PRINT("Stream: %s", line + 6);
                    }
                    line = strtok(NULL, "\n");
                }
            }
            success = true;
        } else {
            DEBUG_PRINT("Server returned error code: %ld", response_code);
            if (response_data.data) {
                DEBUG_PRINT("Error response: %s", response_data.data);
            }
        }
    }
    
    if (response_data.data) {
        free(response_data.data);
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return success;
}

bool api_send_tts_request(const char *text, float speed, const char *language, char **audio_data, size_t *audio_size) {
    DEBUG_PRINT("Sending TTS request: text='%s', speed=%.2f, language='%s'", text, speed, language);
    
    CURL *curl = curl_easy_init();
    if (!curl) {
        DEBUG_PRINT("Failed to initialize CURL");
        return false;
    }
    
    bool success = false;
    struct curl_slist *headers = NULL;
    struct ResponseData response_data = {0};
    
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    // Create JSON payload
    char *json_payload;
    size_t payload_size = strlen(text) + 128;  // Extra space for JSON formatting
    json_payload = malloc(payload_size);
    if (!json_payload) {
        DEBUG_PRINT("Failed to allocate memory for JSON payload");
        curl_easy_cleanup(curl);
        return false;
    }
    
    snprintf(json_payload, payload_size, 
             "{\"text\": \"%s\", \"speed\": %.2f, \"language\": \"%s\"}", 
             text, speed, language);
    
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000/tts");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        DEBUG_PRINT("Failed to send TTS request: %s", curl_easy_strerror(res));
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            DEBUG_PRINT("Received audio data of size: %zu bytes", response_data.size);
            *audio_data = response_data.data;
            *audio_size = response_data.size;
            success = true;
            response_data.data = NULL;  // Prevent it from being freed
        } else {
            DEBUG_PRINT("Server returned error code: %ld", response_code);
            if (response_data.data) {
                DEBUG_PRINT("Error response: %s", response_data.data);
            }
        }
    }
    
    free(json_payload);
    if (response_data.data) {
        free(response_data.data);
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return success;
}

bool api_send_audio_buffer_streaming(const char *buffer, size_t buffer_size, 
                                   stream_callback cb, void *user_data) {
    DEBUG_PRINT("Sending audio buffer of size %zu bytes", buffer_size);
    
    CURL *curl = curl_easy_init();
    if (!curl) {
        DEBUG_PRINT("Failed to initialize CURL");
        return false;
    }
    
    bool success = false;
    struct curl_slist *headers = NULL;
    struct {
        stream_callback cb;
        void *user_data;
        char *partial_line;
    } stream_data = {cb, user_data, NULL};
    
    headers = curl_slist_append(headers, "Content-Type: audio/wav");
    headers = curl_slist_append(headers, "Accept: text/event-stream");
    
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000/speech-chat");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, buffer_size);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_stream_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream_data);
    
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        DEBUG_PRINT("Failed to send audio data: %s", curl_easy_strerror(res));
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            success = true;
        } else {
            DEBUG_PRINT("Server returned error code: %ld", response_code);
        }
    }
    
    if (stream_data.partial_line) {
        free(stream_data.partial_line);
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return success;
}

bool api_send_audio_file_streaming(const char *filename, stream_callback cb, void *user_data) {
    DEBUG_PRINT("Sending audio file to server: %s", filename);
    
    CURL *curl;
    CURLcode res;
    FILE *fd;
    bool success = false;

    curl = curl_easy_init();
    if (!curl) {
        DEBUG_PRINT("Failed to initialize CURL");
        return false;
    }

    fd = fopen(filename, "rb");
    if (!fd) {
        DEBUG_PRINT("Failed to open audio file");
        curl_easy_cleanup(curl);
        return false;
    }

    // Get file size
    fseek(fd, 0, SEEK_END);
    long file_size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    struct curl_slist *headers = NULL;
    struct {
        stream_callback cb;
        void *user_data;
        char *partial_line;
    } stream_data = {cb, user_data, NULL};
    
    headers = curl_slist_append(headers, "Content-Type: audio/wav");
    headers = curl_slist_append(headers, "Accept: text/event-stream");

    // Set up the request
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000/speech-chat");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, fd);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, file_size);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_stream_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream_data);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        DEBUG_PRINT("Failed to send request: %s", curl_easy_strerror(res));
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            success = true;
        } else {
            DEBUG_PRINT("Server returned error code: %ld", response_code);
        }
    }

    if (stream_data.partial_line) {
        free(stream_data.partial_line);
    }
    curl_slist_free_all(headers);
    fclose(fd);
    curl_easy_cleanup(curl);

    return success;
}

// Add these function declarations at the top
static void update_thinking_text(const char *text);
static void update_answer_text(const char *text);

// Add this declaration at the top
extern bool ai_is_processing;
extern bool got_question;

// Add this declaration
extern void handle_stream_end(void);

void handle_stream_message(const char *message, void *user_data) {
    DEBUG_PRINT("Stream message: %s", message);
    static struct StreamContext ctx = {0};
    
    // Check for end of stream first (empty message or special token)
    if (!message || strlen(message) == 0 || strcmp(message, "[DONE]") == 0) {
        DEBUG_PRINT("Stream ended");
        // Reset state and update UI
        handle_stream_end();  // This will reset ai_is_processing
        return;
    }
    
    // Check for question prefix
    if (strncmp(message, "question: --->", 13) == 0) {
        const char *question = message + 13;  // Skip prefix
        DEBUG_PRINT("Question: %s", question);
        
        // Format and display the question in chat panel
        char formatted_question[4096];
        snprintf(formatted_question, sizeof(formatted_question), "Question: %s", question);
        update_answer_text(formatted_question);
        got_question = true;
        
        // Reset accumulated answer when new question starts
        if (ctx.accumulated_answer) {
            free(ctx.accumulated_answer);
            ctx.accumulated_answer = NULL;
            ctx.answer_size = 0;
            ctx.answer_capacity = 0;
        }
        return;
    }
    
    // Check for thinking prefix
    if (strncmp(message, "thinking: --->", 13) == 0) {
        const char *thinking = message + 13;  // Skip prefix
        DEBUG_PRINT("Thinking: %s", thinking);
        update_thinking_text(thinking);
        return;
    }
    
    // Check for answer prefix
    if (strncmp(message, "answer: --->", 11) == 0) {
        const char *answer = message + 11;  // Skip prefix
        DEBUG_PRINT("Answer: %s", answer);
        
        // For the new UI, we'll just pass each answer chunk directly
        update_answer_text(answer);
        
        // Check if this is the last answer chunk (ends with a period)
        if (answer[strlen(answer) - 1] == '.') {
            DEBUG_PRINT("Final answer chunk detected, ending stream");
            handle_stream_end();
        }
        return;
    }
}

// Add these functions to update the UI
static void update_thinking_text(const char *text) {
    // Call the appropriate UI update function
    // This should be implemented in your UI code
    extern void ui_update_thinking_text(const char *text);
    ui_update_thinking_text(text);
}

static void update_answer_text(const char *text) {
    // Call the appropriate UI update function
    // This should be implemented in your UI code
    extern void ui_update_answer_text(const char *text);
    ui_update_answer_text(text);
}

// Add this function to handle API sending in a thread
void *api_send_thread_func(void *arg) {
    struct ApiThreadData *thread_data = (struct ApiThreadData *)arg;
    
    if (thread_data->is_file) {
        api_send_audio_file_streaming(thread_data->filename, handle_stream_message, NULL);
    } else {
        api_send_audio_buffer_streaming(thread_data->buffer, thread_data->buffer_size, 
                                      handle_stream_message, NULL);
        free(thread_data->buffer);  // Free the audio buffer after sending
    }
    
    free(thread_data);  // Free the thread data
    return NULL;
}

// Update the cleanup function to ensure state is reset
void api_stream_cleanup(void) {
    struct StreamContext *ctx = &(struct StreamContext){0};
    if (ctx->accumulated_answer) {
        free(ctx->accumulated_answer);
        ctx->accumulated_answer = NULL;
        ctx->answer_size = 0;
        ctx->answer_capacity = 0;
    }
    // Always call handle_stream_end to ensure state is reset
    handle_stream_end();
} 