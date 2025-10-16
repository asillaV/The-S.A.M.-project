/*
 * S.A.M. Cisterna V2 - Main Firmware
 * Water tank monitoring system with ultrasonic and environmental sensors
 *
 * Version: 2.0.0
 * License: MIT
 *
 * Hardware:
 * - ESP32 Dev Module
 * - HC-SR04 Ultrasonic Sensor (GPIO 26/27)
 * - DHT22 Temperature/Humidity Sensor (GPIO 32)
 *
 * Features:
 * - WiFi connectivity with auto-reconnection
 * - HTTP POST to remote server
 * - Web-based configuration interface
 * - Watchdog timer for system stability
 * - Comprehensive error handling
 */

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "config/config.h"
#include "config/pins.h"
#include "modules/storage.h"
#include "modules/network.h"
#include "modules/sensors.h"
#include "modules/configurator.h"

// Global configuration
WifiConfig wifiConfig;

// State variables
bool systemInitialized = false;
unsigned long lastSensorRead = 0;

void setup() {
    // ✅ FIX: Serial.begin() called ONLY ONCE here
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);

    Serial.println("\n\n================================");
    Serial.println("S.A.M. Cisterna V2");
    Serial.printf("Firmware Version: %s\n", FIRMWARE_VERSION);
    Serial.println("================================\n");

    // ✅ FIX: Initialize watchdog timer
    Serial.println("[MAIN] Initializing watchdog timer...");
    esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);
    esp_task_wdt_add(NULL);
    Serial.printf("[MAIN] Watchdog timeout: %d seconds\n", WATCHDOG_TIMEOUT_S);

    // Initialize SPIFFS
    Serial.println("[MAIN] Initializing storage...");
    if (!initStorage()) {
        Serial.println("[MAIN] CRITICAL: Storage initialization failed!");
        delay(3000);
        ESP.restart();
    }

    // Initialize sensors
    Serial.println("[MAIN] Initializing sensors...");
    if (!initSensors()) {
        Serial.println("[MAIN] CRITICAL: Sensor initialization failed!");
        delay(3000);
        ESP.restart();
    }

    // Load configuration
    Serial.println("[MAIN] Loading configuration...");
    if (!readConfig(CONFIG_FILE_PATH, wifiConfig)) {
        Serial.println("[MAIN] No configuration found, entering config mode");
        enterConfigurationMode();
        return;
    }

    // Validate configuration
    if (wifiConfig.ssid.length() == 0 || wifiConfig.ip.length() == 0) {
        Serial.println("[MAIN] Invalid configuration, entering config mode");
        enterConfigurationMode();
        return;
    }

    // Connect to WiFi
    Serial.println("[MAIN] Connecting to WiFi...");
    if (!connectToWiFi(wifiConfig.ssid, wifiConfig.password, WIFI_MAX_RETRY_ATTEMPTS)) {
        Serial.println("[MAIN] WiFi connection failed, entering config mode");
        enterConfigurationMode();
        return;
    }

    systemInitialized = true;
    Serial.println("\n[MAIN] System initialized successfully");
    Serial.println("[MAIN] Entering main loop...\n");
}

void loop() {
    // ✅ FIX: Reset watchdog every loop iteration
    esp_task_wdt_reset();

    // Check if system is initialized
    if (!systemInitialized) {
        // In configuration mode, just wait
        delay(100);
        return;
    }

    // ✅ FIX: Check WiFi connection and reconnect if needed
    checkWiFiConnection(wifiConfig.ssid, wifiConfig.password);

    // Check for configuration command via serial
    if (Serial.available() > 0) {
        char inputChar = Serial.read();
        if (inputChar == 'C' || inputChar == 'c') {
            Serial.println("[MAIN] Manual configuration mode requested");
            enterConfigurationMode();
            return;
        }
    }

    // Read sensors at defined interval
    unsigned long currentTime = millis();
    if (currentTime - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
        lastSensorRead = currentTime;

        // Read all sensors
        Serial.println("\n[MAIN] Reading sensors...");
        SensorData data = readAllSensors();

        // Create JSON payload
        String jsonData = createJsonPayload(data);

        // Send data to server
        String serverUrl = "http://" + wifiConfig.ip + ":" + wifiConfig.port + "/api/cisterna";
        Serial.println("[MAIN] Sending data to server...");

        if (sendDataToServer(serverUrl, jsonData)) {
            Serial.println("[MAIN] Data sent successfully\n");
        } else {
            Serial.println("[MAIN] Failed to send data\n");
        }
    }

    // Small delay to prevent busy-waiting
    delay(100);
}

void enterConfigurationMode() {
    Serial.println("\n[MAIN] ========================================");
    Serial.println("[MAIN] ENTERING CONFIGURATION MODE");
    Serial.println("[MAIN] ========================================\n");

    // Stop normal operations
    systemInitialized = false;

    // Start configuration web server
    startConfigurationMode(wifiConfig);

    // ✅ FIX: Don't use long blocking delay - system will restart after config
    Serial.println("[MAIN] Configuration mode active");
    Serial.println("[MAIN] Device will restart after configuration is saved");

    // Keep servicing the web server in a loop
    while (true) {
        esp_task_wdt_reset();  // Keep watchdog happy
        delay(100);
    }
}

String createJsonPayload(const SensorData& data) {
    // Create JSON with all sensor data
    String json = "{";

    // Always include distance (primary data)
    json += "\"distanza\":" + String(data.distance);

    // Include temperature and humidity if valid
    if (data.temperature != ERROR_TEMPERATURE) {
        json += ",\"temperatura\":" + String(data.temperature, 1);
    }

    if (data.humidity != ERROR_HUMIDITY) {
        json += ",\"umidita\":" + String(data.humidity, 1);
    }

    json += "}";

    return json;
}
