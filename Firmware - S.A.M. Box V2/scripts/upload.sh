#!/bin/bash
# Upload firmware to ESP32 using PlatformIO

echo "=========================================="
echo "S.A.M. Box V2 - Firmware Upload Script"
echo "=========================================="

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo "Error: PlatformIO is not installed"
    echo "Install with: pip install platformio"
    exit 1
fi

# Build and upload
echo "Building and uploading firmware..."
pio run --target upload

# Check if successful
if [ $? -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "Upload successful!"
    echo "=========================================="
    echo "Run 'bash scripts/monitor.sh' to view serial output"
else
    echo ""
    echo "=========================================="
    echo "Upload failed!"
    echo "=========================================="
    exit 1
fi
