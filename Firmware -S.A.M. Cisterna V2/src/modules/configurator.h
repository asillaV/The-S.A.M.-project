/*
 * S.A.M. Cisterna V2 - Configurator Module
 * Provides web interface for WiFi and server configuration
 */

#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <Arduino.h>
#include "storage.h"

// Function declarations
void startConfigurationMode(WifiConfig& config);
void stopConfigurationMode();

#endif
