#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <stddef.h>

// Initialize audio recording system
bool audio_init(void);

// Start recording
bool audio_start_recording(void);

// Stop recording
void audio_stop_recording(void);

// Clean up audio resources
void audio_cleanup(void);

// Check if currently recording
bool is_audio_recording(void);

// Add these function declarations
bool audio_play_buffer(const char *buffer, size_t buffer_size);
bool audio_speak_text(const char *text);

#endif // AUDIO_H 