/*
 * S.A.M. Cisterna V2 - Sensors Module
 * Handles ultrasonic distance sensor and DHT22 temperature/humidity sensor
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Sensor data structure
struct SensorData {
    int distance;        // Distance in cm (-1 if error)
    float temperature;   // Temperature in °C (-999.0 if error)
    float humidity;      // Humidity in % (-999.0 if error)
};

// Function declarations
bool initSensors();
int readUltrasonicDistance();
bool readTemperatureHumidity(float& temperature, float& humidity);
SensorData readAllSensors();

#endif
