#!/bin/bash
# Monitor serial output from ESP32

echo "=========================================="
echo "S.A.M. Box V2 - Serial Monitor"
echo "=========================================="
echo "Baud rate: 9600"
echo "Press Ctrl+C to exit"
echo "=========================================="
echo ""

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo "Error: PlatformIO is not installed"
    echo "Install with: pip install platformio"
    exit 1
fi

# Start serial monitor
pio device monitor -b 9600
