/*
 * S.A.M. Box V2 - Smart Agricultural Monitor
 * Firmware Version 2.0.0
 *
 * ESP32-based IoT device for agricultural monitoring
 * Collects temperature, humidity, soil moisture, water level data
 * Captures images with ESP32-CAM and sends to remote server
 *
 * License: MIT
 */

#include "config/WifiConfig.h"
#include "config/camera_pins.h"
#include "config/sensor_pins.h"
#include "modules/acquisition.h"
#include "modules/network.h"
#include <WiFi.h>
#include <DHT.h>
#include "esp_camera.h"
#include "SPIFFS.h"
#include "esp_task_wdt.h"

// Firmware version
#define FIRMWARE_VERSION "2.0.0"

// Configuration constants
#define FORMAT_SPIFFS_IF_FAILED true
#define CAMERA_MODEL_WROVER_KIT
#define SERIAL_BAUD_RATE 9600
#define WIFI_MAX_ATTEMPTS 100
#define SENSOR_READ_INTERVAL_MS 2500
#define CAMERA_CAPTURE_INTERVAL_MS 2500
#define WATCHDOG_TIMEOUT_S 30

// Global data structure
Dati dati;

// Function declarations
void initCamera();
void initWatchdog();
void handleSerialCommands();

void setup() {
    // Initialize serial (ONLY ONCE!)
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println();
    Serial.println("========================================");
    Serial.println("S.A.M. Box V2 - Smart Agricultural Monitor");
    Serial.println("Firmware Version: " FIRMWARE_VERSION);
    Serial.println("========================================");

    // Initialize SPIFFS
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    Serial.println("SPIFFS mounted successfully");

    // Initialize camera
    initCamera();

    // Initialize sensors
    extern void initSensors();
    initSensors();

    // Initialize dati structure
    dati.fb = nullptr;
    dati.terra = 0;
    dati.acqua = 0;
    dati.distanza = 0;
    dati.umidita = 0;
    dati.temperatura = 0;

    // Read WiFi configuration from SPIFFS
    readFile("/wifi_config.txt", wifiConfig);

    // Connect to WiFi
    Serial.println("Connecting to WiFi: " + wifiConfig.ssid);
    bool connected = connectToWiFi(wifiConfig.ssid, wifiConfig.password, WIFI_MAX_ATTEMPTS);

    if (!connected) {
        Serial.println("WiFi connection failed!");
        Serial.println("Starting configuration mode...");
        configuratore(wifiConfig);

        // After configuration, restart the device
        Serial.println("Configuration complete. Restarting in 3 seconds...");
        delay(3000);
        ESP.restart();
    }

    // Initialize watchdog timer
    initWatchdog();

    Serial.println("Setup complete. Starting main loop...");
    delay(500);
}

void loop() {
    // Reset watchdog timer
    esp_task_wdt_reset();

    // Check WiFi connection and reconnect if needed
    checkWiFiConnection();

    // Capture camera image
    if (capture_and_send(dati)) {
        Serial.println("Camera frame captured successfully");
    } else {
        Serial.println("Camera capture failed");
    }

    delay(CAMERA_CAPTURE_INTERVAL_MS);

    // Read sensor data
    if (leggi_dati(dati)) {
        // Print sensor data
        Serial.println("--- Sensor Readings ---");
        Serial.printf("Temperature: %.2f °C\n", dati.temperatura);
        Serial.printf("Humidity: %.2f %%\n", dati.umidita);
        Serial.printf("Soil moisture: %.2f %%\n", dati.terra);
        Serial.printf("Water level: %.2f %%\n", dati.acqua);
        Serial.println("----------------------");
    } else {
        Serial.println("Sensor reading failed");
    }

    delay(SENSOR_READ_INTERVAL_MS);

    // Send all data to server
    if (sendAllData(wifiConfig.ip, wifiConfig.port, dati)) {
        Serial.println("Data transmission successful");
    } else {
        Serial.println("Data transmission failed");
    }

    // Handle serial commands
    handleSerialCommands();

    // Small delay before next iteration
    delay(100);
}

// Initialize camera with proper configuration
void initCamera() {
    camera_config_t config;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    // Adjust settings based on PSRAM availability
    if (config.pixel_format == PIXFORMAT_JPEG) {
        if (psramFound()) {
            Serial.println("PSRAM found - using high quality settings");
            config.jpeg_quality = 10;
            config.fb_count = 2;
            config.grab_mode = CAMERA_GRAB_LATEST;
        } else {
            Serial.println("PSRAM not found - using standard settings");
            config.frame_size = FRAMESIZE_SVGA;
            config.fb_location = CAMERA_FB_IN_DRAM;
        }
    } else {
        config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
        config.fb_count = 2;
#endif
    }

    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return;
    }

    Serial.println("Camera initialized successfully");
}

// Initialize watchdog timer for system stability
void initWatchdog() {
    esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);
    esp_task_wdt_add(NULL);
    Serial.printf("Watchdog timer initialized (%d seconds timeout)\n", WATCHDOG_TIMEOUT_S);
}

// Handle serial commands (e.g., enter configuration mode)
void handleSerialCommands() {
    if (Serial.available() > 0) {
        char inputChar = Serial.read();

        if (inputChar == 'C' || inputChar == 'c') {
            Serial.println("Activating configurator...");
            configuratore(wifiConfig);

            Serial.println("Configuration from main:");
            Serial.println("SSID: " + wifiConfig.ssid);
            Serial.println("IP: " + wifiConfig.ip);
            Serial.println("Port: " + wifiConfig.port);
        } else if (inputChar == 'R' || inputChar == 'r') {
            Serial.println("Restarting device...");
            delay(1000);
            ESP.restart();
        } else if (inputChar == 'V' || inputChar == 'v') {
            Serial.println("Firmware Version: " FIRMWARE_VERSION);
        }
    }
}
