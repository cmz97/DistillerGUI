#include "system_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEBUG_PRINT(fmt, ...) printf("SysMon Debug: " fmt "\n", ##__VA_ARGS__)

// Static variables to store monitoring data
static float ram_usage = 0.0f;
static float cpu_usage = 0.0f;
static float temperature = 0.0f;

// Variables for CPU usage calculation
static unsigned long long prev_idle = 0;
static unsigned long long prev_total = 0;

bool system_monitor_init(void) {
    DEBUG_PRINT("Initializing system monitor");
    
    // Perform initial update to populate values
    system_monitor_update();
    
    return true;
}

float system_monitor_get_ram_usage(void) {
    return ram_usage;
}

float system_monitor_get_cpu_usage(void) {
    return cpu_usage;
}

float system_monitor_get_temperature(void) {
    return temperature;
}

char* system_monitor_get_status_string(void) {
    char* status = malloc(64);
    if (!status) {
        return NULL;
    }
    
    snprintf(status, 64, "RAM: %.1f%% | CPU: %.1f%% | TEMP: %.1f°C", 
             ram_usage, cpu_usage, temperature);
    
    return status;
}

// Helper function to read CPU stats
static void read_cpu_stats(unsigned long long *idle_time, unsigned long long *total_time) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        DEBUG_PRINT("Failed to open /proc/stat");
        return;
    }
    
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), fp)) {
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
        sscanf(buffer, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
        
        *idle_time = idle + iowait;
        *total_time = user + nice + system + idle + iowait + irq + softirq + steal;
    }
    
    fclose(fp);
}

// Helper function to read temperature
static float read_temperature(void) {
    // Try to read from thermal zone 0 (may vary by system)
    int fd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
    if (fd < 0) {
        DEBUG_PRINT("Failed to open thermal zone");
        return 0.0f;
    }
    
    char buffer[16];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);
    
    if (bytes_read <= 0) {
        DEBUG_PRINT("Failed to read temperature");
        return 0.0f;
    }
    
    buffer[bytes_read] = '\0';
    int temp_millicelsius = atoi(buffer);
    
    // Convert from millicelsius to celsius
    return temp_millicelsius / 1000.0f;
}

void system_monitor_update(void) {
    // Update RAM usage
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        unsigned long total_ram = info.totalram * info.mem_unit;
        unsigned long free_ram = info.freeram * info.mem_unit;
        unsigned long used_ram = total_ram - free_ram;
        
        ram_usage = (float)used_ram * 100.0f / (float)total_ram;
    } else {
        DEBUG_PRINT("Failed to get system info");
    }
    
    // Update CPU usage
    unsigned long long idle, total;
    read_cpu_stats(&idle, &total);
    
    if (prev_total != 0) {
        unsigned long long idle_delta = idle - prev_idle;
        unsigned long long total_delta = total - prev_total;
        
        if (total_delta > 0) {
            cpu_usage = 100.0f * (1.0f - ((float)idle_delta / (float)total_delta));
        }
    }
    
    prev_idle = idle;
    prev_total = total;
    
    // Update temperature
    temperature = read_temperature();
    
    DEBUG_PRINT("Updated - RAM: %.1f%%, CPU: %.1f%%, Temp: %.1f°C", 
                ram_usage, cpu_usage, temperature);
}

void system_monitor_cleanup(void) {
    // Nothing to clean up for now
} 