/*
 * S.A.M. Cisterna V2 - Pin Configuration
 * Defines all GPIO pin assignments for hardware components
 */

#ifndef PINS_H
#define PINS_H

// DHT22 Temperature/Humidity Sensor
#define DHT_PIN 32
#define DHT_TYPE DHT22

// Ultrasonic Sensor (HC-SR04)
#define ULTRASONIC_TRIG_PIN 26
#define ULTRASONIC_ECHO_PIN 27

// Optional: I2C pins for OLED display (if used)
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

#endif
