/*
 * S.A.M. Cisterna V2 - Main Configuration
 * System-wide constants and configuration values
 */

#ifndef CONFIG_H
#define CONFIG_H

// Firmware version
#define FIRMWARE_VERSION "2.0.0"

// Serial configuration
#define SERIAL_BAUD_RATE 9600

// Network configuration
#define WIFI_CONNECT_TIMEOUT_MS 50000
#define WIFI_MAX_RETRY_ATTEMPTS 100
#define HTTP_TIMEOUT_MS 10000

// Configuration AP settings
#define CONFIG_AP_SSID "SAM-CISTERNA"
#define CONFIG_AP_PASSWORD "password123"
#define CONFIG_AP_CHANNEL 1
#define CONFIG_AP_MAX_CONNECTIONS 1

// Timing configuration
#define SENSOR_READ_INTERVAL_MS 5000
#define WATCHDOG_TIMEOUT_S 30

// SPIFFS configuration
#define CONFIG_FILE_PATH "/wifi_config.txt"
#define FORMAT_SPIFFS_IF_FAILED true

// Sensor configuration
#define ULTRASONIC_MAX_DISTANCE_CM 400
#define ULTRASONIC_TIMEOUT_US 30000

// Error values (sentinel values for invalid readings)
#define ERROR_TEMPERATURE -999.0
#define ERROR_HUMIDITY -999.0
#define ERROR_DISTANCE -1

#endif
