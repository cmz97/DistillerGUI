#include "audio.h"
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include "api_client.h"
#include "ui_callbacks.h"

#define AUDIO_DEVICE "hw:0,0"
#define SAMPLE_RATE 24000
#define CHANNELS 2
#define FORMAT SND_PCM_FORMAT_S16_LE
#define SAVE_WAV 0  // Set to 1 to save WAV files, 0 to send directly to server

// Add debug macro
#define DEBUG_PRINT(fmt, ...) printf("Audio Debug: " fmt "\n", ##__VA_ARGS__)

// Add function prototypes
static void print_audio_info(snd_pcm_t *handle, snd_pcm_hw_params_t *params);
static void *recording_thread_func(void *arg);

// Add these structs
struct ResponseData {
    char *data;
    size_t size;
};

static snd_pcm_t *capture_handle = NULL;
static pthread_t recording_thread;
static volatile bool is_recording = false;
static char current_filename[256];

// Add mutex for thread safety
static pthread_mutex_t audio_mutex = PTHREAD_MUTEX_INITIALIZER;

// Update the WAV header structure with proper bit fields
typedef struct {
    char riff_header[4];    // Contains "RIFF"
    uint32_t wav_size;      // Size of WAV file
    char wave_header[4];    // Contains "WAVE"
    char fmt_header[4];     // Contains "fmt "
    uint32_t fmt_chunk_size;// Size of fmt chunk
    uint16_t audio_format;  // Audio format 1=PCM
    uint16_t num_channels;  // Mono = 1, Stereo = 2
    uint32_t sample_rate;   // Sampling Frequency in Hz
    uint32_t byte_rate;     // bytes per second
    uint16_t block_align;   // 2=16-bit mono, 4=16-bit stereo
    uint16_t bits_per_sample;// Number of bits per sample
    char data_header[4];    // Contains "data"
    uint32_t data_bytes;    // Number of bytes in data
} __attribute__((packed)) wav_header;

static void write_wav_header(FILE *file, int data_size) {
    wav_header header;
    memset(&header, 0, sizeof(header));  // Clear the structure first
    
    // Write RIFF header
    memcpy(header.riff_header, "RIFF", 4);
    header.wav_size = data_size + sizeof(wav_header) - 8;
    memcpy(header.wave_header, "WAVE", 4);
    
    // Write fmt chunk
    memcpy(header.fmt_header, "fmt ", 4);
    header.fmt_chunk_size = 16;
    header.audio_format = 1;             // PCM = 1
    header.num_channels = CHANNELS;      // Mono = 1
    header.sample_rate = SAMPLE_RATE;    // 16000
    header.bits_per_sample = 16;         // 16 bits
    header.block_align = header.num_channels * header.bits_per_sample / 8;
    header.byte_rate = header.sample_rate * header.block_align;
    
    // Write data chunk
    memcpy(header.data_header, "data", 4);
    header.data_bytes = data_size;
    
    // Write the header to file
    fwrite(&header, sizeof(wav_header), 1, file);
    
    DEBUG_PRINT("WAV Header written: format=%d, channels=%d, rate=%d, bits=%d",
                header.audio_format,
                header.num_channels,
                header.sample_rate,
                header.bits_per_sample);
}

static void generate_filename(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    // Create output directory if it doesn't exist
    int mkdir_result = mkdir("output", 0755);
    if (mkdir_result == 0) {
        DEBUG_PRINT("Created output directory");
    } else if (errno != EEXIST) {
        DEBUG_PRINT("Failed to create output directory: %s", strerror(errno));
    }
    
    // Get absolute path
    char abs_path[PATH_MAX];
    if (realpath("output", abs_path) != NULL) {
        DEBUG_PRINT("Output directory path: %s", abs_path);
    }
    
    snprintf(current_filename, sizeof(current_filename),
             "output/recording_%04d%02d%02d_%02d%02d%02d.wav",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    
    DEBUG_PRINT("Generated filename: %s", current_filename);
}

static void *recording_thread_func(void *arg) {
    DEBUG_PRINT("Starting recording thread");
    pthread_mutex_lock(&audio_mutex);
    
    if (!capture_handle) {
        DEBUG_PRINT("Invalid capture handle");
        pthread_mutex_unlock(&audio_mutex);
        return NULL;
    }
    
    snd_pcm_uframes_t frames = 1024;
    size_t bytes_per_frame = CHANNELS * 2;
    char *buffer = malloc(frames * bytes_per_frame);
    FILE *wav_file = NULL;
    char *audio_buffer = NULL;
    size_t audio_buffer_size = 0;
    size_t audio_buffer_capacity = 0;
    
    if (!buffer) {
        DEBUG_PRINT("Failed to allocate buffer");
        pthread_mutex_unlock(&audio_mutex);
        return NULL;
    }
    
    if (SAVE_WAV) {
        generate_filename();
        DEBUG_PRINT("Opening WAV file: %s", current_filename);
        wav_file = fopen(current_filename, "wb");
        if (!wav_file) {
            DEBUG_PRINT("Failed to open WAV file: %s", strerror(errno));
            free(buffer);
            pthread_mutex_unlock(&audio_mutex);
            return NULL;
        }
        
        // Write placeholder header
        wav_header placeholder = {0};
        if (fwrite(&placeholder, sizeof(wav_header), 1, wav_file) != 1) {
            DEBUG_PRINT("Failed to write placeholder header");
            fclose(wav_file);
            free(buffer);
            pthread_mutex_unlock(&audio_mutex);
            return NULL;
        }
    } else {
        // Initialize memory buffer for audio data
        audio_buffer_capacity = 1024 * 1024; // Start with 1MB
        audio_buffer = malloc(audio_buffer_capacity);
        if (!audio_buffer) {
            DEBUG_PRINT("Failed to allocate audio buffer");
            free(buffer);
            pthread_mutex_unlock(&audio_mutex);
            return NULL;
        }
        
        // Write WAV header to memory
        wav_header header = {0};
        memcpy(header.riff_header, "RIFF", 4);
        memcpy(header.wave_header, "WAVE", 4);
        memcpy(header.fmt_header, "fmt ", 4);
        header.fmt_chunk_size = 16;
        header.audio_format = 1;
        header.num_channels = CHANNELS;
        header.sample_rate = SAMPLE_RATE;
        header.bits_per_sample = 16;
        header.block_align = header.num_channels * header.bits_per_sample / 8;
        header.byte_rate = header.sample_rate * header.block_align;
        memcpy(header.data_header, "data", 4);
        
        memcpy(audio_buffer, &header, sizeof(wav_header));
        audio_buffer_size = sizeof(wav_header);
    }
    
    // Initialize PCM device
    snd_pcm_drop(capture_handle);
    if (snd_pcm_prepare(capture_handle) < 0 || snd_pcm_start(capture_handle) < 0) {
        DEBUG_PRINT("Failed to prepare/start PCM device");
        if (SAVE_WAV) {
            fclose(wav_file);
        } else {
            free(audio_buffer);
        }
        free(buffer);
        pthread_mutex_unlock(&audio_mutex);
        return NULL;
    }
    
    pthread_mutex_unlock(&audio_mutex);
    DEBUG_PRINT("Starting recording loop");
    
    long total_bytes = 0;
    while (is_recording) {
        pthread_mutex_lock(&audio_mutex);
        if (!capture_handle) {
            pthread_mutex_unlock(&audio_mutex);
            break;
        }
        
        int rc = snd_pcm_readi(capture_handle, buffer, frames);
        pthread_mutex_unlock(&audio_mutex);
        
        if (rc == -EPIPE) {
            pthread_mutex_lock(&audio_mutex);
            if (capture_handle) {
                snd_pcm_prepare(capture_handle);
                snd_pcm_start(capture_handle);
            }
            pthread_mutex_unlock(&audio_mutex);
            continue;
        } else if (rc < 0) {
            DEBUG_PRINT("Error reading from PCM device: %s", snd_strerror(rc));
            break;
        }
        
        if (rc > 0) {
            size_t bytes = rc * bytes_per_frame;
            if (SAVE_WAV) {
                size_t written = fwrite(buffer, 1, bytes, wav_file);
                if (written != bytes) {
                    DEBUG_PRINT("Short write to WAV file");
                }
                total_bytes += written;
            } else {
                // Ensure buffer has enough space
                if (audio_buffer_size + bytes > audio_buffer_capacity) {
                    audio_buffer_capacity *= 2;
                    char *new_buffer = realloc(audio_buffer, audio_buffer_capacity);
                    if (!new_buffer) {
                        DEBUG_PRINT("Failed to resize audio buffer");
                        break;
                    }
                    audio_buffer = new_buffer;
                }
                memcpy(audio_buffer + audio_buffer_size, buffer, bytes);
                audio_buffer_size += bytes;
            }
        }
        usleep(100);
    }
    
    DEBUG_PRINT("Recording ended");
    pthread_mutex_lock(&audio_mutex);
    
    if (SAVE_WAV) {
        // Finalize WAV file
        if (fseek(wav_file, 0, SEEK_SET) == 0) {
            write_wav_header(wav_file, total_bytes);
        }
        fclose(wav_file);
        
        // Create thread data for API sending
        struct ApiThreadData *thread_data = malloc(sizeof(struct ApiThreadData));
        thread_data->is_file = true;
        strncpy(thread_data->filename, current_filename, sizeof(thread_data->filename) - 1);
        
        // Create thread for API sending
        pthread_t api_thread;
        pthread_create(&api_thread, NULL, api_send_thread_func, thread_data);
        pthread_detach(api_thread);
    } else {
        // Update WAV header with final size
        wav_header *final_header = (wav_header *)audio_buffer;
        final_header->data_bytes = audio_buffer_size - sizeof(wav_header);
        final_header->wav_size = audio_buffer_size - 8;
        
        // Create thread data for API sending
        struct ApiThreadData *thread_data = malloc(sizeof(struct ApiThreadData));
        thread_data->is_file = false;
        thread_data->buffer = audio_buffer;
        thread_data->buffer_size = audio_buffer_size;
        
        // Create thread for API sending
        pthread_t api_thread;
        pthread_create(&api_thread, NULL, api_send_thread_func, thread_data);
        pthread_detach(api_thread);
        
        // Don't free audio_buffer here, it will be freed by the API thread
    }
    
    free(buffer);
    pthread_mutex_unlock(&audio_mutex);
    
    DEBUG_PRINT("Recording thread ending");
    return NULL;
}

bool audio_init(void) {
    if (!api_client_init()) {
        DEBUG_PRINT("Failed to initialize API client");
        return false;
    }
    DEBUG_PRINT("Initializing audio");
    pthread_mutex_lock(&audio_mutex);
    
    // Make sure we don't have an existing handle
    if (capture_handle) {
        snd_pcm_close(capture_handle);
        capture_handle = NULL;
    }
    
    int rc;
    snd_pcm_hw_params_t *params;
    
    rc = snd_pcm_open(&capture_handle, AUDIO_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        DEBUG_PRINT("Failed to open audio device: %s", snd_strerror(rc));
        pthread_mutex_unlock(&audio_mutex);
        return false;
    }
    
    DEBUG_PRINT("Audio device opened successfully");
    snd_pcm_hw_params_alloca(&params);
    
    rc = snd_pcm_hw_params_any(capture_handle, params);
    if (rc < 0) {
        DEBUG_PRINT("Failed to get default params: %s", snd_strerror(rc));
        goto error;
    }
    
    // Print hardware capabilities
    print_audio_info(capture_handle, params);
    
    // Set parameters with error checking
    if ((rc = snd_pcm_hw_params_set_access(capture_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        DEBUG_PRINT("Failed to set access: %s", snd_strerror(rc));
        goto error;
    }
    if ((rc = snd_pcm_hw_params_set_format(capture_handle, params, FORMAT)) < 0) {
        DEBUG_PRINT("Failed to set format: %s", snd_strerror(rc));
        goto error;
    }
    if ((rc = snd_pcm_hw_params_set_channels(capture_handle, params, CHANNELS)) < 0) {
        DEBUG_PRINT("Failed to set channels: %s", snd_strerror(rc));
        goto error;
    }
    
    unsigned int rate = SAMPLE_RATE;
    if ((rc = snd_pcm_hw_params_set_rate_near(capture_handle, params, &rate, 0)) < 0) {
        DEBUG_PRINT("Failed to set rate: %s", snd_strerror(rc));
        goto error;
    }
    
    // Set buffer size
    snd_pcm_uframes_t buffer_size = 16384;
    if ((rc = snd_pcm_hw_params_set_buffer_size_near(capture_handle, params, &buffer_size)) < 0) {
        DEBUG_PRINT("Failed to set buffer size: %s", snd_strerror(rc));
        goto error;
    }
    
    rc = snd_pcm_hw_params(capture_handle, params);
    if (rc < 0) {
        DEBUG_PRINT("Failed to set hardware parameters: %s", snd_strerror(rc));
        goto error;
    }
    
    DEBUG_PRINT("Audio initialized successfully");
    pthread_mutex_unlock(&audio_mutex);
    return true;

error:
    if (capture_handle) {
        snd_pcm_close(capture_handle);
        capture_handle = NULL;
    }
    pthread_mutex_unlock(&audio_mutex);
    return false;
}

bool audio_start_recording(void) {
    DEBUG_PRINT("Starting recording");
    pthread_mutex_lock(&audio_mutex);
    if (is_recording) {
        DEBUG_PRINT("Already recording");
        pthread_mutex_unlock(&audio_mutex);
        return false;
    }
    
    is_recording = true;
    int rc = pthread_create(&recording_thread, NULL, recording_thread_func, NULL);
    if (rc != 0) {
        DEBUG_PRINT("Failed to create recording thread: %s", strerror(rc));
        is_recording = false;
        pthread_mutex_unlock(&audio_mutex);
        return false;
    }
    
    DEBUG_PRINT("Recording started successfully");
    pthread_mutex_unlock(&audio_mutex);
    return true;
}

void audio_stop_recording(void) {
    DEBUG_PRINT("Stopping recording");
    pthread_mutex_lock(&audio_mutex);
    if (!is_recording) {
        DEBUG_PRINT("Not recording");
        pthread_mutex_unlock(&audio_mutex);
        return;
    }
    
    is_recording = false;
    pthread_mutex_unlock(&audio_mutex);
    
    DEBUG_PRINT("Waiting for recording thread to finish");
    pthread_join(recording_thread, NULL);
    DEBUG_PRINT("Recording stopped");
    
    // Don't call handle_stream_end here, as it resets the AI processing flag
    // We want to keep the AI IS THINKING status until we get a response
    
    // Instead, we'll let the API client handle the state transitions
    // The AI processing flag will be reset when the stream ends
}

void audio_cleanup(void) {
    DEBUG_PRINT("Cleaning up audio");
    pthread_mutex_lock(&audio_mutex);
    if (is_recording) {
        DEBUG_PRINT("Stopping active recording");
        is_recording = false;
        pthread_mutex_unlock(&audio_mutex);
        pthread_join(recording_thread, NULL);
        pthread_mutex_lock(&audio_mutex);
    }
    
    if (capture_handle) {
        DEBUG_PRINT("Closing audio device");
        snd_pcm_close(capture_handle);
        capture_handle = NULL;
    }
    pthread_mutex_unlock(&audio_mutex);
    DEBUG_PRINT("Audio cleanup complete");
    api_client_cleanup();
}

bool is_audio_recording(void) {
    pthread_mutex_lock(&audio_mutex);
    bool recording = is_recording;
    pthread_mutex_unlock(&audio_mutex);
    return recording;
}

static void print_audio_info(snd_pcm_t *handle, snd_pcm_hw_params_t *params) {
    DEBUG_PRINT("=== Audio Hardware Info ===");
    
    // Get device name
    snd_pcm_info_t *info;
    snd_pcm_info_alloca(&info);
    snd_pcm_info(handle, info);
    DEBUG_PRINT("Device: %s", snd_pcm_info_get_name(info));
    
    // Get channel info
    unsigned int min_channels, max_channels;
    snd_pcm_hw_params_get_channels_min(params, &min_channels);
    snd_pcm_hw_params_get_channels_max(params, &max_channels);
    DEBUG_PRINT("Channels: min=%u, max=%u", min_channels, max_channels);
    
    // Get sample rate range
    unsigned int min_rate, max_rate;
    int dir;
    snd_pcm_hw_params_get_rate_min(params, &min_rate, &dir);
    snd_pcm_hw_params_get_rate_max(params, &max_rate, &dir);
    DEBUG_PRINT("Sample rates: min=%uHz, max=%uHz", min_rate, max_rate);
    
    // Get format info
    DEBUG_PRINT("Supported formats:");
    for (int format = 0; format <= SND_PCM_FORMAT_LAST; format++) {
        if (snd_pcm_hw_params_test_format(handle, params, format) == 0) {
            DEBUG_PRINT("  %s", snd_pcm_format_name(format));
        }
    }
    DEBUG_PRINT("========================");
}

static snd_pcm_t *playback_handle = NULL;

static bool init_playback_device(void) {
    if (playback_handle) {
        return true;  // Already initialized
    }
    
    DEBUG_PRINT("Opening playback device: %s", AUDIO_DEVICE);
    int rc = snd_pcm_open(&playback_handle, AUDIO_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        DEBUG_PRINT("Failed to open audio device for playback: %s", snd_strerror(rc));
        return false;
    }
    
    snd_pcm_hw_params_t *params;
    snd_pcm_hw_params_alloca(&params);
    
    DEBUG_PRINT("Getting default parameters for playback");
    rc = snd_pcm_hw_params_any(playback_handle, params);
    if (rc < 0) {
        DEBUG_PRINT("Failed to get default params: %s", snd_strerror(rc));
        goto error;
    }
    
    // Print playback capabilities
    print_audio_info(playback_handle, params);
    
    DEBUG_PRINT("Setting playback access mode to INTERLEAVED");
    rc = snd_pcm_hw_params_set_access(playback_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0) {
        DEBUG_PRINT("Failed to set access mode: %s", snd_strerror(rc));
        goto error;
    }
    
    DEBUG_PRINT("Setting playback format to S16_LE");
    rc = snd_pcm_hw_params_set_format(playback_handle, params, FORMAT);
    if (rc < 0) {
        DEBUG_PRINT("Failed to set format: %s", snd_strerror(rc));
        goto error;
    }
    
    DEBUG_PRINT("Setting playback channels to 2");
    rc = snd_pcm_hw_params_set_channels(playback_handle, params, CHANNELS);  // Use 2 channels as per hardware
    if (rc < 0) {
        DEBUG_PRINT("Failed to set channels: %s", snd_strerror(rc));
        goto error;
    }
    
    DEBUG_PRINT("Setting playback rate to %d", SAMPLE_RATE);
    unsigned int rate = SAMPLE_RATE;
    int dir = 0;
    rc = snd_pcm_hw_params_set_rate_near(playback_handle, params, &rate, &dir);
    if (rc < 0) {
        DEBUG_PRINT("Failed to set rate: %s", snd_strerror(rc));
        goto error;
    }
    DEBUG_PRINT("Actual rate set to %u", rate);
    
    // Set buffer size
    snd_pcm_uframes_t buffer_size = 16384;
    rc = snd_pcm_hw_params_set_buffer_size_near(playback_handle, params, &buffer_size);
    if (rc < 0) {
        DEBUG_PRINT("Failed to set buffer size: %s", snd_strerror(rc));
        goto error;
    }
    DEBUG_PRINT("Buffer size set to %lu frames", buffer_size);
    
    DEBUG_PRINT("Applying hardware parameters");
    rc = snd_pcm_hw_params(playback_handle, params);
    if (rc < 0) {
        DEBUG_PRINT("Failed to set hardware parameters: %s", snd_strerror(rc));
        goto error;
    }
    
    DEBUG_PRINT("Playback device initialized successfully");
    return true;

error:
    DEBUG_PRINT("Error initializing playback device");
    snd_pcm_close(playback_handle);
    playback_handle = NULL;
    return false;
}

bool audio_play_buffer(const char *buffer, size_t buffer_size) {
    DEBUG_PRINT("Starting playback of %zu bytes", buffer_size);
    if (!init_playback_device()) {
        return false;
    }
    
    // Skip WAV header
    const char *audio_data = buffer + 44;  // Standard WAV header size
    size_t audio_size = buffer_size - 44;
    
    DEBUG_PRINT("Preparing playback device");
    snd_pcm_prepare(playback_handle);
    
    // Convert mono to stereo if needed
    size_t frames = audio_size / 2;  // 16-bit mono audio
    int16_t *stereo_buffer = malloc(frames * 4);  // 2 channels * 2 bytes per sample
    if (!stereo_buffer) {
        DEBUG_PRINT("Failed to allocate stereo buffer");
        return false;
    }
    
    // Copy mono data to both channels
    const int16_t *mono_data = (const int16_t *)audio_data;
    for (size_t i = 0; i < frames; i++) {
        stereo_buffer[i*2] = mono_data[i];     // Left channel
        stereo_buffer[i*2+1] = mono_data[i];   // Right channel
    }
    
    DEBUG_PRINT("Writing %zu frames to playback device", frames);
    snd_pcm_sframes_t written = snd_pcm_writei(playback_handle, stereo_buffer, frames);
    
    if (written < 0) {
        DEBUG_PRINT("Failed to write audio data: %s", snd_strerror(written));
        free(stereo_buffer);
        return false;
    }
    
    DEBUG_PRINT("Successfully wrote %ld frames", written);
    snd_pcm_drain(playback_handle);
    free(stereo_buffer);
    return true;
}

bool audio_speak_text(const char *text) {
    char *audio_data = NULL;
    size_t audio_size = 0;
    
    if (!api_send_tts_request(text, 1.0f, "en-us", &audio_data, &audio_size)) {
        DEBUG_PRINT("Failed to get TTS audio");
        return false;
    }
    
    bool success = audio_play_buffer(audio_data, audio_size);
    free(audio_data);
    
    // Call the TTS end handler
    extern void handle_tts_end(void);
    handle_tts_end();
    
    return success;
}

// Add this function to force reset the recording state
void audio_reset_state(void) {
    DEBUG_PRINT("Forcing audio state reset");
    pthread_mutex_lock(&audio_mutex);
    
    if (is_recording) {
        DEBUG_PRINT("Resetting recording flag");
        is_recording = false;
        pthread_mutex_unlock(&audio_mutex);
        
        // If there's an active recording thread, wait for it to finish
        // Use a safer approach without pthread_kill
        int join_result = pthread_join(recording_thread, NULL);
        if (join_result == 0) {
            DEBUG_PRINT("Recording thread joined successfully");
        } else if (join_result == ESRCH) {
            DEBUG_PRINT("Recording thread does not exist");
        } else {
            DEBUG_PRINT("Failed to join recording thread: %s", strerror(join_result));
        }
    } else {
        pthread_mutex_unlock(&audio_mutex);
    }
    
    DEBUG_PRINT("Audio state reset complete");
} 