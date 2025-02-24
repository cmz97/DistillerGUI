#ifndef UART_INPUT_H
#define UART_INPUT_H

#include "lvgl/lvgl.h"

// Initialize the UART input device
void uart_input_init(void);

// Cleanup UART input
void uart_input_deinit(void);

// Get the input device instance (needed for main.c)
lv_indev_t* uart_input_get_indev(void);

// Get the input group (needed for main.c)
lv_group_t* uart_input_get_group(void);

// Add this declaration
int uart_input_get_button_state(void);

#endif /* UART_INPUT_H */ 