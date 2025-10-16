/*
 * S.A.M. Cisterna V2 - Storage Module Implementation
 */

#include "storage.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "../config/config.h"

bool initStorage() {
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
        Serial.println("[STORAGE] Failed to mount SPIFFS");
        return false;
    }
    Serial.println("[STORAGE] SPIFFS mounted successfully");
    return true;
}

bool writeConfig(const char* path, const WifiConfig& config) {
    Serial.printf("[STORAGE] Writing config to: %s\n", path);

    File file = SPIFFS.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("[STORAGE] Failed to open file for writing");
        return false;
    }

    DynamicJsonDocument jsonDoc(1024);
    jsonDoc["ssid"] = config.ssid;
    jsonDoc["password"] = config.password;
    jsonDoc["ip"] = config.ip;
    jsonDoc["port"] = config.port;

    String jsonString;
    serializeJson(jsonDoc, jsonString);

    bool success = file.print(jsonString);
    file.close();

    if (success) {
        Serial.println("[STORAGE] Config written successfully");
    } else {
        Serial.println("[STORAGE] Failed to write config");
    }

    return success;
}

bool readConfig(const char* path, WifiConfig& config) {
    Serial.printf("[STORAGE] Reading config from: %s\n", path);

    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.println("[STORAGE] Failed to open file for reading");
        return false;
    }

    String jsonString = file.readString();
    file.close();

    DynamicJsonDocument jsonDoc(2048);
    DeserializationError error = deserializeJson(jsonDoc, jsonString);

    if (error) {
        Serial.print("[STORAGE] JSON parse failed: ");
        Serial.println(error.c_str());
        return false;
    }

    // Read values with defaults
    config.ssid = jsonDoc.containsKey("ssid") ? jsonDoc["ssid"].as<String>() : "";
    config.password = jsonDoc.containsKey("password") ? jsonDoc["password"].as<String>() : "";
    config.ip = jsonDoc.containsKey("ip") ? jsonDoc["ip"].as<String>() : "";
    config.port = jsonDoc.containsKey("port") ? jsonDoc["port"].as<String>() : "";

    Serial.println("[STORAGE] Config loaded successfully:");
    Serial.printf("  SSID: %s\n", config.ssid.c_str());
    Serial.printf("  IP: %s\n", config.ip.c_str());
    Serial.printf("  Port: %s\n", config.port.c_str());

    return true;
}
