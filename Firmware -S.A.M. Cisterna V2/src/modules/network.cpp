/*
 * S.A.M. Cisterna V2 - Network Module Implementation
 * FIXES: WiFi sleep mode disabled, timeout added, reconnection logic
 */

#include "network.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "../config/config.h"

bool connectToWiFi(const String& ssid, const String& password, int maxAttempts) {
    Serial.println("[NETWORK] Connecting to WiFi...");
    Serial.printf("[NETWORK] SSID: %s\n", ssid.c_str());

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    // ✅ FIX: Disable WiFi sleep to prevent HTTP timeouts
    WiFi.setSleep(false);

    int attempts = 0;
    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        Serial.print(".");
        attempts++;

        // Check timeout
        if (millis() - startTime > WIFI_CONNECT_TIMEOUT_MS) {
            Serial.println("\n[NETWORK] WiFi connection timeout");
            return false;
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[NETWORK] WiFi connected successfully");
        Serial.printf("[NETWORK] IP Address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("[NETWORK] Signal strength: %d dBm\n", WiFi.RSSI());
        return true;
    }

    Serial.println("\n[NETWORK] WiFi connection failed");
    return false;
}

void checkWiFiConnection(const String& ssid, const String& password) {
    // ✅ FIX: Added WiFi reconnection logic
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[NETWORK] WiFi disconnected, attempting reconnection...");

        // Try to reconnect
        if (!connectToWiFi(ssid, password, 20)) {
            Serial.println("[NETWORK] Reconnection failed, restarting device...");
            delay(3000);
            ESP.restart();
        }
    }
}

bool sendDataToServer(const String& serverUrl, const String& jsonData) {
    // Check WiFi connection first
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[NETWORK] Cannot send: WiFi not connected");
        return false;
    }

    HTTPClient http;

    // ✅ FIX: Set HTTP timeout to prevent indefinite blocking
    http.setTimeout(HTTP_TIMEOUT_MS);

    // ✅ FIX: Add error checking for http.begin()
    if (!http.begin(serverUrl)) {
        Serial.println("[NETWORK] Failed to initialize HTTP connection");
        return false;
    }

    http.addHeader("Content-Type", "application/json");

    Serial.printf("[NETWORK] Sending POST to: %s\n", serverUrl.c_str());
    Serial.printf("[NETWORK] Data: %s\n", jsonData.c_str());

    int httpResponseCode = http.POST(jsonData);

    bool success = false;
    if (httpResponseCode > 0) {
        Serial.printf("[NETWORK] HTTP Response code: %d\n", httpResponseCode);

        if (httpResponseCode == 200) {
            String response = http.getString();
            Serial.printf("[NETWORK] Server response: %s\n", response.c_str());
            success = true;
        } else {
            Serial.printf("[NETWORK] Server returned error code: %d\n", httpResponseCode);
        }
    } else {
        Serial.printf("[NETWORK] HTTP POST failed: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
    return success;
}

void disconnectWiFi() {
    Serial.println("[NETWORK] Disconnecting WiFi...");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}
