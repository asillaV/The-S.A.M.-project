# Hardware Setup Guide

## Bill of Materials (BOM)

| Component | Quantity | Description |
|-----------|----------|-------------|
| ESP32 WROVER Module | 1 | Main microcontroller with PSRAM |
| ESP32-CAM Module | 1 | Camera module (OV2640) |
| DHT22 | 1 | Temperature and humidity sensor |
| Soil Moisture Sensor | 1 | Capacitive or resistive type |
| Water Level Sensor | 1 | Analog water level sensor |
| ATtiny85 | 1 | Sensor interface microcontroller |
| 10kΩ Resistor | 1 | Pull-up for DHT22 |
| Power Supply | 1 | 5V/2A minimum |
| Jumper Wires | ~20 | For connections |

## Pin Connections

### ESP32 WROVER to Sensors

#### DHT22 Temperature/Humidity Sensor
```
DHT22 Pin 1 (VCC)  → ESP32 3.3V
DHT22 Pin 2 (DATA) → ESP32 GPIO 33 (with 10kΩ pull-up to 3.3V)
DHT22 Pin 3 (NC)   → Not connected
DHT22 Pin 4 (GND)  → ESP32 GND
```

#### Soil Moisture Sensor
```
Sensor VCC → ESP32 3.3V
Sensor GND → ESP32 GND
Sensor OUT → ESP32 GPIO 32 (or connect via ATtiny85)
```

#### Water Level Sensor
```
Sensor VCC → ESP32 3.3V
Sensor GND → ESP32 GND
Sensor OUT → ESP32 GPIO 13 (or connect via ATtiny85)
```

#### ATtiny85 (for sensor multiplexing)
```
ATtiny85 VCC → ESP32 3.3V
ATtiny85 GND → ESP32 GND
ATtiny85 TX  → ESP32 RX (GPIO 3)
ATtiny85 RX  → ESP32 TX (GPIO 1)
```

### ESP32-CAM Pin Configuration

The camera uses the following GPIO pins (defined in `camera_pins.h`):

```
Y9  → GPIO 35
Y8  → GPIO 34
Y7  → GPIO 39
Y6  → GPIO 36
Y5  → GPIO 19
Y4  → GPIO 18
Y3  → GPIO 5
Y2  → GPIO 4
VSYNC → GPIO 25
HREF  → GPIO 23
PCLK  → GPIO 22
XCLK  → GPIO 21
SIOD  → GPIO 26 (I2C SDA)
SIOC  → GPIO 27 (I2C SCL)
```

## Wiring Diagram

```
                    ┌─────────────────┐
                    │   ESP32 WROVER  │
                    │                 │
    DHT22 ────────> │ GPIO 33         │
    Soil ─────────> │ GPIO 32         │
    Water ────────> │ GPIO 13         │
                    │                 │
    ATtiny85 <────> │ RX/TX (Serial)  │
                    │                 │
    Camera <──────> │ GPIO 4-39       │
                    │   (multiple)    │
                    │                 │
    Power 5V ─────> │ 5V/3.3V         │
    GND ──────────> │ GND             │
                    └─────────────────┘
```

## Assembly Instructions

### 1. ESP32-CAM Module Assembly
1. Ensure the OV2640 camera module is properly seated
2. Verify PSRAM chip is present on the board
3. Connect FTDI programmer for initial firmware upload
4. Upload firmware via Arduino IDE or PlatformIO

### 2. Sensor Connections
1. Connect DHT22 with pull-up resistor (10kΩ) between DATA and VCC
2. Connect soil moisture sensor analog output to GPIO 32
3. Connect water level sensor analog output to GPIO 13
4. Ensure all sensors share common GND with ESP32

### 3. ATtiny85 Setup (Optional)
If using ATtiny85 for sensor multiplexing:
1. Program ATtiny85 with sensor reading firmware
2. Connect serial communication pins (TX/RX)
3. Configure to respond to 'w' (water) and 's' (soil) commands
4. Set baud rate to 9600

### 4. Power Supply
1. Use regulated 5V power supply (minimum 2A)
2. ESP32-CAM requires stable power for reliable operation
3. Consider adding decoupling capacitors (100µF) near ESP32
4. Avoid powering from USB if camera is active (current draw too high)

## Testing

### 1. Serial Monitor Test
```
- Open Serial Monitor at 9600 baud
- Power on device
- Verify boot messages appear
- Check for PSRAM detection message
- Verify camera initialization success
```

### 2. Sensor Test
```
- Press 'V' to check firmware version
- Observe sensor readings in Serial Monitor
- Verify temperature, humidity values are reasonable
- Check soil/water sensor values (0-100% range)
```

### 3. Camera Test
```
- Check for "Camera frame captured successfully" message
- Verify no memory errors
- Confirm JPEG frames are being captured
```

### 4. WiFi Test
```
- Device should connect to configured network
- Check IP address in Serial Monitor
- Verify HTTP POST messages show success (200 OK)
```

## Troubleshooting

### Camera Not Detected
- Check all camera pin connections
- Verify PSRAM is detected (see Serial output)
- Try lowering XCLK frequency to 10MHz
- Check camera module is not damaged

### Sensor Reading Errors
- Verify sensor power connections (3.3V/GND)
- Check DHT22 pull-up resistor is installed
- Test sensors individually with multimeter
- Verify GPIO pins are not already in use

### Power Issues
- Use external 5V power supply (not USB)
- Add bulk capacitors (100-470µF) near ESP32
- Check for voltage drops during camera operation
- Verify current supply is at least 2A

### Serial Communication Issues
- Verify baud rate is 9600 (both ESP32 and ATtiny85)
- Check TX/RX connections are not swapped
- Test with Serial loopback first
- Clear serial buffer before reading

## Safety Notes

⚠️ **Important Safety Information**

- Do not exceed 3.3V on any GPIO pin
- ESP32-CAM can draw high current during transmission
- Use proper ESD protection when handling modules
- Keep water sensors away from ESP32 circuit board
- Do not expose electronics to moisture
- Use appropriate wire gauge for power connections

## Additional Resources

- ESP32-CAM Datasheet: [Espressif Documentation](https://www.espressif.com)
- DHT22 Datasheet: Search for "DHT22 datasheet"
- Soil Moisture Sensor Calibration Guide
- ATtiny85 Programming Guide

## Schematic

See `hardware/schematic.pdf` for complete schematic diagram.
