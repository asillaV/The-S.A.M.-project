#include "network.h"
#include "../config/WifiConfig.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Connect to WiFi with timeout
bool connectToWiFi(const String& ssid, const String& password, int maxAttempts) {
    WiFi.begin(ssid.c_str(), password.c_str());
    WiFi.setSleep(false);  // Disable WiFi sleep for stable HTTP connections

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(WIFI_RETRY_DELAY_MS);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        return true;
    }

    Serial.println("\nWiFi connection failed");
    return false;
}

// Check WiFi connection and reconnect if needed
void checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected. Attempting to reconnect...");
        extern WifiConfig wifiConfig;
        connectToWiFi(wifiConfig.ssid, wifiConfig.password, 20);
    }
}

// Send image data to server
bool sendImageData(const String& serverUrl, camera_fb_t* fb) {
    if (fb == nullptr) {
        Serial.println("Error: Camera frame buffer is NULL");
        return false;
    }

    HTTPClient http;
    http.setTimeout(HTTP_TIMEOUT_MS);
    http.begin(serverUrl);
    http.addHeader("Content-Type", "image/jpeg");

    int httpResponseCode = http.POST((uint8_t*)fb->buf, fb->len);
    http.end();

    if (httpResponseCode == HTTP_CODE_OK) {
        Serial.println("Image sent successfully");
        return true;
    } else {
        Serial.printf("HTTP POST image failed, error: %d\n", httpResponseCode);
        return false;
    }
}

// Send sensor data to server
bool sendSensorData(const String& serverUrl, const Dati& dati) {
    HTTPClient http;
    http.setTimeout(HTTP_TIMEOUT_MS);
    http.begin(serverUrl);

    // Create JSON string
    String dati_json = "{\"terra\": " + String(dati.terra) +
                       ", \"acqua\": " + String(dati.acqua) +
                       ", \"distanza\": " + String(dati.distanza) +
                       ", \"umidita\": " + String(dati.umidita) +
                       ", \"temperatura\": " + String(dati.temperatura) + "}";

    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(dati_json);
    http.end();

    if (httpResponseCode == HTTP_CODE_OK) {
        Serial.println("Sensor data sent successfully");
        return true;
    } else {
        Serial.printf("HTTP POST sensor data failed, error: %d\n", httpResponseCode);
        return false;
    }
}

// Send all data (image + sensors)
bool sendAllData(const String& ip, const String& port, Dati& dati) {
    // Check WiFi connection first
    checkWiFiConnection();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Cannot send data: WiFi not connected");
        return false;
    }

    String serverAddress1 = "http://" + ip + ":" + port + "/data";
    String serverAddress2 = "http://" + ip + ":" + port + "/api/data";

    Serial.println("Sending to: " + serverAddress1);

    bool imageSuccess = false;
    bool sensorSuccess = false;

    // Send image data if frame buffer is valid
    if (dati.fb != nullptr) {
        imageSuccess = sendImageData(serverAddress1, dati.fb);
        // Always free the frame buffer after sending (success or failure)
        esp_camera_fb_return(dati.fb);
        dati.fb = nullptr;
    } else {
        Serial.println("Warning: No camera frame to send");
    }

    // Always try to send sensor data even if image fails
    sensorSuccess = sendSensorData(serverAddress2, dati);

    return imageSuccess && sensorSuccess;
}
