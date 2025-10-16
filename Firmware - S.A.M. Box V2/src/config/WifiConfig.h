#ifndef WIFICONFIG_H
#define WIFICONFIG_H

#include <Arduino.h>

struct WifiConfig {
    String ssid;
    String password;
    String ip;
    String port;
};

extern WifiConfig wifiConfig;
void configuratore(WifiConfig& config);
void writeFile(const char *path, const WifiConfig &data);
void readFile(const char *path, WifiConfig &data);

#endif
