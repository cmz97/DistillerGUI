#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <stdbool.h>

// Initialize the system monitoring module
bool system_monitor_init(void);

// Get RAM usage as a percentage (0-100)
float system_monitor_get_ram_usage(void);

// Get average CPU utilization across all cores (0-100)
float system_monitor_get_cpu_usage(void);

// Get current CPU temperature in Celsius
float system_monitor_get_temperature(void);

// Get a formatted status string with all system info
// The caller is responsible for freeing the returned string
char* system_monitor_get_status_string(void);

// Update the system monitoring data
void system_monitor_update(void);

// Clean up resources
void system_monitor_cleanup(void);

#endif // SYSTEM_MONITOR_H 