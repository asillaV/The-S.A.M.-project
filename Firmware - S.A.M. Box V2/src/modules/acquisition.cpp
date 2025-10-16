#include "acquisition.h"
#include "../config/sensor_pins.h"
#include "esp_camera.h"
#include <DHT.h>

// DHT sensor instance
DHT dht(DHTPIN, DHTTYPE);

// Serial communication timeout
#define SERIAL_TIMEOUT_MS 100
#define SERIAL_RESPONSE_DELAY_MS 50

// Initialize DHT sensor (call once in setup)
void initSensors() {
    dht.begin();
}

// Capture camera frame
bool capture_and_send(Dati& dati) {
    // Free previous frame if it exists
    if (dati.fb != nullptr) {
        esp_camera_fb_return(dati.fb);
        dati.fb = nullptr;
    }

    // Capture new frame
    dati.fb = esp_camera_fb_get();

    if (!dati.fb) {
        Serial.println("Camera capture failed");
        return false;
    }

    return true;
}

// Read sensor data from DHT22 and ATtiny85
bool leggi_dati(Dati& dati) {
    // Initialize values to invalid state
    dati.terra = -1.0;
    dati.acqua = -1.0;
    dati.distanza = -1;

    // Read DHT22 sensor
    dati.temperatura = dht.readTemperature();
    dati.umidita = dht.readHumidity();

    // Check if DHT reading was successful
    if (isnan(dati.temperatura) || isnan(dati.umidita)) {
        Serial.println("Error reading DHT sensor!");
        dati.temperatura = -999.0;
        dati.umidita = -999.0;
    }

    // Clear serial buffer
    while (Serial.available() > 0) {
        Serial.read();
    }

    // Request water sensor reading from ATtiny85
    Serial.println('w');
    unsigned long startTime = millis();

    // Wait for response with timeout
    while (!Serial.available() && (millis() - startTime < SERIAL_TIMEOUT_MS)) {
        delay(10);
    }

    if (Serial.available() > 0) {
        dati.acqua = Serial.parseInt();
        Serial.printf("Water sensor: %.2f\n", dati.acqua);
    } else {
        Serial.println("Timeout waiting for water sensor data");
    }

    // Small delay before next request
    delay(SERIAL_RESPONSE_DELAY_MS);

    // Clear serial buffer again
    while (Serial.available() > 0) {
        Serial.read();
    }

    // Request soil sensor reading from ATtiny85
    Serial.println('s');
    startTime = millis();

    // Wait for response with timeout
    while (!Serial.available() && (millis() - startTime < SERIAL_TIMEOUT_MS)) {
        delay(10);
    }

    if (Serial.available() > 0) {
        dati.terra = Serial.parseInt();
        Serial.printf("Soil sensor: %.2f\n", dati.terra);
    } else {
        Serial.println("Timeout waiting for soil sensor data");
    }

    // Return true if at least DHT data was read successfully
    return !isnan(dati.temperatura) && !isnan(dati.umidita);
}
