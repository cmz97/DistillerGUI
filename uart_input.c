#include "uart_input.h"
#include <stdio.h>
#include <stdlib.h>     // Add this for atoi()
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <pthread.h>

extern void handle_enter_key(void);  // Add this at the top with other includes

static lv_indev_t *uart_indev;      // Input device instance
static lv_group_t *g;               // Input group for widgets

// UART file descriptor and thread
static int uart_fd = -1;
static pthread_t uart_thread;
static bool uart_running = false;

// Shared state between UART thread and input driver
static volatile uint32_t last_key = 0;
static volatile bool key_pressed = false;
static pthread_mutex_t uart_mutex = PTHREAD_MUTEX_INITIALIZER;

// Button state constants matching Python code
#define BTN_UP      1
#define BTN_DOWN    2
#define BTN_SELECT  4
#define BTN_SHUTDOWN 8

static int current_button_state = 0;

// Initialize UART
static bool uart_init(void) {
    uart_fd = open("/dev/ttyAMA2", O_RDWR | O_NOCTTY | O_NONBLOCK);  // Make non-blocking
    if (uart_fd < 0) {
        printf("Error opening UART\n");
        return false;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    // Get current settings
    if (tcgetattr(uart_fd, &tty) != 0) {
        printf("Error from tcgetattr\n");
        return false;
    }

    // Set baud rate
    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    // 8-bit chars
    tty.c_cflag |= CS8;
    // Disable flow control
    tty.c_cflag &= ~CRTSCTS;
    // Turn on READ & ignore ctrl lines
    tty.c_cflag |= CREAD | CLOCAL;
    // Disable echo, canonical mode, extended processing
    tty.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    // No signaling chars, no echo, no canonical processing
    tty.c_lflag &= ~(ISIG | ECHO | ECHOE | ECHONL | ICANON);

    // Save settings
    if (tcsetattr(uart_fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr\n");
        return false;
    }

    return true;
}

// UART reading thread function
static void* uart_read_thread(void* arg) {
    char buffer[32];
    printf("Debug: UART thread started\n");
    
    while (uart_running) {
        int n = read(uart_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            int button_state = atoi(buffer);
            current_button_state = button_state;
            printf("UART thread received button state: %d\n", button_state);
            
            pthread_mutex_lock(&uart_mutex);
            
            // Check for TTS combination (UP + DOWN = 3) first
            if (button_state == 3) {
                printf("UART thread: TTS button detected (UP+DOWN)\n");
                extern void handle_tts_button(void);
                handle_tts_button();
            }
            else if (button_state & BTN_UP) {
                last_key = LV_KEY_UP;
                key_pressed = true;
                printf("UART thread: UP key detected\n");
            }
            else if (button_state & BTN_DOWN) {
                last_key = LV_KEY_DOWN;
                key_pressed = true;
                printf("UART thread: DOWN key detected\n");
            }
            else if (button_state & BTN_SELECT) {
                last_key = LV_KEY_ENTER;
                key_pressed = true;
                printf("UART thread: ENTER key detected\n");
                handle_enter_key();
            }
            else {
                key_pressed = false;
                printf("UART thread: Key released\n");
            }
            
            pthread_mutex_unlock(&uart_mutex);
        }
        usleep(5000);  // Small delay to prevent busy waiting
    }
    
    printf("Debug: UART thread ending\n");
    return NULL;
}

// Function to read UART input - called by LVGL
static void uart_input_read(lv_indev_t *indev, lv_indev_data_t *data) {
    pthread_mutex_lock(&uart_mutex);
    
    if (key_pressed) {
        data->key = last_key;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
        data->key = last_key;
    }
    
    pthread_mutex_unlock(&uart_mutex);
}

void uart_input_init(void) {
    // Initialize UART first
    if (!uart_init()) {
        printf("Failed to initialize UART\n");
        return;
    }

    // Create and initialize input device
    uart_indev = lv_indev_create();
    lv_indev_set_type(uart_indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(uart_indev, uart_input_read);
    
    // Create a group for widgets
    g = lv_group_create();
    lv_group_set_default(g);
    
    // Assign the group to the input device
    lv_indev_set_group(uart_indev, g);

    // Start UART reading thread
    uart_running = true;
    if (pthread_create(&uart_thread, NULL, uart_read_thread, NULL) != 0) {
        printf("Failed to create UART thread!\n");
        return;
    }
}

// Add cleanup function
void uart_input_deinit(void) {
    uart_running = false;
    pthread_join(uart_thread, NULL);
    close(uart_fd);
}

lv_indev_t* uart_input_get_indev(void) {
    return uart_indev;
}

lv_group_t* uart_input_get_group(void) {
    return g;
}

int uart_input_get_button_state(void) {
    return current_button_state;
} 