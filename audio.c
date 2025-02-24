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

#define AUDIO_DEVICE "hw:0,0"
#define SAMPLE_RATE 48000
#define CHANNELS 2
#define FORMAT SND_PCM_FORMAT_S16_LE

// Add debug macro
#define DEBUG_PRINT(fmt, ...) printf("Audio Debug: " fmt "\n", ##__VA_ARGS__)

// Add function prototypes
static void print_audio_info(snd_pcm_t *handle, snd_pcm_hw_params_t *params);

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
    
    // Check if capture handle is valid
    if (!capture_handle) {
        DEBUG_PRINT("Invalid capture handle");
        pthread_mutex_unlock(&audio_mutex);
        return NULL;
    }
    
    snd_pcm_uframes_t frames = 1024;
    size_t bytes_per_frame = CHANNELS * 2;
    char *buffer = malloc(frames * bytes_per_frame);
    FILE *wav_file = NULL;
    long total_bytes = 0;
    
    if (!buffer) {
        DEBUG_PRINT("Failed to allocate buffer");
        pthread_mutex_unlock(&audio_mutex);
        return NULL;
    }
    
    generate_filename();
    DEBUG_PRINT("Attempting to open file: %s", current_filename);
    wav_file = fopen(current_filename, "wb");
    if (!wav_file) {
        DEBUG_PRINT("Failed to open WAV file: %s", strerror(errno));
        free(buffer);
        pthread_mutex_unlock(&audio_mutex);
        return NULL;
    }
    DEBUG_PRINT("Successfully opened WAV file");
    
    // Write a placeholder header
    wav_header placeholder = {0};
    if (fwrite(&placeholder, sizeof(wav_header), 1, wav_file) != 1) {
        DEBUG_PRINT("Failed to write placeholder header: %s", strerror(errno));
        fclose(wav_file);
        free(buffer);
        pthread_mutex_unlock(&audio_mutex);
        return NULL;
    }
    
    // Check PCM state before preparing
    snd_pcm_state_t state = snd_pcm_state(capture_handle);
    DEBUG_PRINT("PCM state before prepare: %d", state);
    
    // Drop and prepare
    snd_pcm_drop(capture_handle);
    
    // Prepare PCM device
    DEBUG_PRINT("Preparing PCM device");
    int prepare_rc = snd_pcm_prepare(capture_handle);
    if (prepare_rc < 0) {
        DEBUG_PRINT("Failed to prepare PCM device: %s", snd_strerror(prepare_rc));
        fclose(wav_file);
        free(buffer);
        pthread_mutex_unlock(&audio_mutex);
        return NULL;
    }
    
    // Start the PCM device
    DEBUG_PRINT("Starting PCM device");
    int start_rc = snd_pcm_start(capture_handle);
    if (start_rc < 0) {
        DEBUG_PRINT("Failed to start PCM device: %s", snd_strerror(start_rc));
        fclose(wav_file);
        free(buffer);
        pthread_mutex_unlock(&audio_mutex);
        return NULL;
    }
    
    pthread_mutex_unlock(&audio_mutex);
    DEBUG_PRINT("Starting recording loop");
    
    while (is_recording) {
        pthread_mutex_lock(&audio_mutex);
        if (!capture_handle) {
            DEBUG_PRINT("Capture handle became invalid");
            pthread_mutex_unlock(&audio_mutex);
            break;
        }
        
        int rc = snd_pcm_readi(capture_handle, buffer, frames);
        pthread_mutex_unlock(&audio_mutex);
        
        if (rc == -EPIPE) {
            DEBUG_PRINT("Buffer overrun detected");
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
            size_t written = fwrite(buffer, bytes_per_frame, rc, wav_file);
            if (written != (size_t)rc) {
                DEBUG_PRINT("Short write to WAV file: %zu != %d", written, rc);
            }
            total_bytes += written * bytes_per_frame;
        }
        
        usleep(100);
    }
    
    DEBUG_PRINT("Recording loop ended, finalizing file with %ld bytes", total_bytes);
    pthread_mutex_lock(&audio_mutex);
    
    // Stop the PCM device
    if (capture_handle) {
        snd_pcm_drop(capture_handle);
    }
    
    // Write the final header
    if (fseek(wav_file, 0, SEEK_SET) != 0) {
        DEBUG_PRINT("Failed to seek to start of file: %s", strerror(errno));
    }
    write_wav_header(wav_file, total_bytes);
    
    // Verify file size
    fseek(wav_file, 0, SEEK_END);
    long file_size = ftell(wav_file);
    DEBUG_PRINT("Final file size: %ld bytes", file_size);
    
    fclose(wav_file);
    free(buffer);
    pthread_mutex_unlock(&audio_mutex);
    
    DEBUG_PRINT("Recording thread ending, file saved: %s", current_filename);
    return NULL;
}

bool audio_init(void) {
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