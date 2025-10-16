#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include "esp_camera.h"

// Configuration constants
#define WIFI_CONNECT_TIMEOUT_MS 50000  // 50 seconds
#define WIFI_RETRY_DELAY_MS 500
#define HTTP_TIMEOUT_MS 10000

// Data structure for sensor readings
struct Dati {
    float terra;
    float acqua;
    int distanza;
    float umidita;
    float temperatura;
    camera_fb_t* fb;
};

// Network functions
bool connectToWiFi(const String& ssid, const String& password, int maxAttempts);
void checkWiFiConnection();
bool sendImageData(const String& serverUrl, camera_fb_t* fb);
bool sendSensorData(const String& serverUrl, const Dati& dati);
bool sendAllData(const String& ip, const String& port, Dati& dati);

#endif
