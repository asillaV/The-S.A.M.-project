/*
 * S.A.M. Cisterna V2 - Storage Module
 * Handles SPIFFS filesystem operations and configuration persistence
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

// WiFi Configuration structure
struct WifiConfig {
    String ssid;
    String password;
    String ip;
    String port;
};

// Function declarations
bool initStorage();
bool writeConfig(const char* path, const WifiConfig& config);
bool readConfig(const char* path, WifiConfig& config);

#endif
