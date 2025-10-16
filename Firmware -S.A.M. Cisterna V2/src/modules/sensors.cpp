/*
 * S.A.M. Cisterna V2 - Sensors Module Implementation
 */

#include "sensors.h"
#include <DHT.h>
#include "../config/pins.h"
#include "../config/config.h"

// DHT sensor instance
DHT dht(DHT_PIN, DHT_TYPE);

bool initSensors() {
    Serial.println("[SENSORS] Initializing sensors...");

    // Initialize DHT22
    dht.begin();

    // Initialize ultrasonic sensor pins
    pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
    pinMode(ULTRASONIC_ECHO_PIN, INPUT);

    Serial.println("[SENSORS] Sensors initialized successfully");
    return true;
}

int readUltrasonicDistance() {
    // Send ultrasonic pulse
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

    // Read echo pulse duration
    long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, ULTRASONIC_TIMEOUT_US);

    // Check for timeout or invalid reading
    if (duration == 0) {
        Serial.println("[SENSORS] Ultrasonic sensor timeout");
        return ERROR_DISTANCE;
    }

    // Calculate distance in cm (speed of sound = 340 m/s = 0.034 cm/µs)
    int distance = duration * 0.034 / 2;

    // Validate reading
    if (distance < 0 || distance > ULTRASONIC_MAX_DISTANCE_CM) {
        Serial.printf("[SENSORS] Invalid distance reading: %d cm\n", distance);
        return ERROR_DISTANCE;
    }

    Serial.printf("[SENSORS] Distance: %d cm\n", distance);
    return distance;
}

bool readTemperatureHumidity(float& temperature, float& humidity) {
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    // Check for read errors
    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("[SENSORS] DHT22 read error");
        temperature = ERROR_TEMPERATURE;
        humidity = ERROR_HUMIDITY;
        return false;
    }

    // Validate reasonable ranges
    if (temperature < -40.0 || temperature > 80.0 ||
        humidity < 0.0 || humidity > 100.0) {
        Serial.println("[SENSORS] DHT22 values out of range");
        temperature = ERROR_TEMPERATURE;
        humidity = ERROR_HUMIDITY;
        return false;
    }

    Serial.printf("[SENSORS] Temperature: %.1f °C, Humidity: %.1f %%\n",
                  temperature, humidity);
    return true;
}

SensorData readAllSensors() {
    SensorData data;

    data.distance = readUltrasonicDistance();
    readTemperatureHumidity(data.temperature, data.humidity);

    return data;
}
