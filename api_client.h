#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <stdbool.h>
#include <stddef.h>

// Add this callback type for streaming responses
typedef void (*stream_callback)(const char *data, void *user_data);

// Initialize the API client
bool api_client_init(void);

// Clean up the API client
void api_client_cleanup(void);

// Send audio file to speech-to-text endpoint
bool api_send_audio_file(const char *filename);

// Send audio buffer to speech-to-text endpoint
bool api_send_audio_buffer(const char *buffer, size_t buffer_size);

// Send message to chat endpoint
bool api_send_chat_message(const char *message);

// Send TTS request
bool api_send_tts_request(const char *text, float speed, const char *language, char **audio_data, size_t *audio_size);

// Update the function declarations to support streaming
bool api_send_audio_buffer_streaming(const char *buffer, size_t buffer_size, 
                                   stream_callback cb, void *user_data);
bool api_send_audio_file_streaming(const char *filename,
                                 stream_callback cb, void *user_data);

// Add this function declaration
void handle_stream_message(const char *message, void *user_data);

// Add this structure definition
struct ApiThreadData {
    char *buffer;
    size_t buffer_size;
    bool is_file;
    char filename[256];
};

// Add this function declaration
void *api_send_thread_func(void *arg);

#endif // API_CLIENT_H 