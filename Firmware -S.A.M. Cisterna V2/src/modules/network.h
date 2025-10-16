/*
 * S.A.M. Cisterna V2 - Network Module
 * Handles WiFi connection, reconnection, and HTTP communication
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>

// Function declarations
bool connectToWiFi(const String& ssid, const String& password, int maxAttempts);
void checkWiFiConnection(const String& ssid, const String& password);
bool sendDataToServer(const String& serverUrl, const String& jsonData);
void disconnectWiFi();

#endif
