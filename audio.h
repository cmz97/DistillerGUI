#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

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

#endif // AUDIO_H 