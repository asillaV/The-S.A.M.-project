# API Reference

## Server Endpoints

The S.A.M. Box V2 sends data to a remote server via HTTP POST requests to two endpoints.

## Endpoint 1: Image Upload

### Request
```http
POST /data HTTP/1.1
Host: {server-ip}:{port}
Content-Type: image/jpeg
Content-Length: {size}

[Binary JPEG Image Data]
```

### Response
```http
HTTP/1.1 200 OK
Content-Type: text/plain

OK
```

### Description
Uploads a JPEG image captured by the ESP32-CAM module. The image is sent as raw binary data in the request body.

### Error Codes
- `200 OK` - Image uploaded successfully
- `400 Bad Request` - Invalid image data
- `500 Internal Server Error` - Server processing error
- `503 Service Unavailable` - Server temporarily unavailable

### Example cURL
```bash
curl -X POST \
  -H "Content-Type: image/jpeg" \
  --data-binary @image.jpg \
  http://192.168.1.100:5000/data
```

---

## Endpoint 2: Sensor Data

### Request
```http
POST /api/data HTTP/1.1
Host: {server-ip}:{port}
Content-Type: application/json
Content-Length: {size}

{
  "terra": 45.2,
  "acqua": 78.5,
  "distanza": 120,
  "umidita": 65.3,
  "temperatura": 22.5
}
```

### Response
```http
HTTP/1.1 200 OK
Content-Type: application/json

{
  "status": "success",
  "message": "Data received"
}
```

### JSON Fields

| Field | Type | Unit | Range | Description |
|-------|------|------|-------|-------------|
| `terra` | float | % | 0-100 | Soil moisture percentage |
| `acqua` | float | % | 0-100 | Water level percentage |
| `distanza` | int | cm | 0-400 | Distance measurement (future use) |
| `umidita` | float | % | 0-100 | Air humidity percentage |
| `temperatura` | float | °C | -40 to 80 | Air temperature in Celsius |

### Special Values
- `-1.0` - Sensor read timeout or error
- `-999.0` - DHT22 sensor failure (NaN reading)

### Example cURL
```bash
curl -X POST \
  -H "Content-Type: application/json" \
  -d '{
    "terra": 45.2,
    "acqua": 78.5,
    "distanza": 120,
    "umidita": 65.3,
    "temperatura": 22.5
  }' \
  http://192.168.1.100:5000/api/data
```

### Error Codes
- `200 OK` - Data received successfully
- `400 Bad Request` - Invalid JSON format
- `422 Unprocessable Entity` - Valid JSON but invalid field values
- `500 Internal Server Error` - Server processing error

---

## Sample Server Implementation

### Flask Example

See `examples/server-flask/app.py` for a complete implementation:

```python
from flask import Flask, request, jsonify
import os
from datetime import datetime

app = Flask(__name__)

@app.route('/data', methods=['POST'])
def upload_image():
    """Receive and save JPEG image"""
    try:
        image_data = request.get_data()
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"image_{timestamp}.jpg"

        with open(os.path.join('uploads', filename), 'wb') as f:
            f.write(image_data)

        print(f"Image saved: {filename}")
        return "OK", 200
    except Exception as e:
        print(f"Error: {e}")
        return "Error", 500

@app.route('/api/data', methods=['POST'])
def receive_sensor_data():
    """Receive and process sensor data"""
    try:
        data = request.get_json()

        # Validate data
        required_fields = ['terra', 'acqua', 'distanza', 'umidita', 'temperatura']
        if not all(field in data for field in required_fields):
            return jsonify({"error": "Missing fields"}), 400

        # Process data (save to database, etc.)
        print(f"Sensor data received: {data}")

        # Log to file
        with open('sensor_log.txt', 'a') as f:
            timestamp = datetime.now().isoformat()
            f.write(f"{timestamp},{data}\n")

        return jsonify({"status": "success", "message": "Data received"}), 200
    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    os.makedirs('uploads', exist_ok=True)
    app.run(host='0.0.0.0', port=5000, debug=True)
```

---

## Communication Flow

```
S.A.M. Box V2                          Server
     │                                   │
     ├─ Capture Image ──────────────────>│
     │  POST /data                       │
     │  Content-Type: image/jpeg         │
     │                                   │
     │<────────────────────── 200 OK ────┤
     │                                   │
     ├─ Send Sensor Data ───────────────>│
     │  POST /api/data                   │
     │  Content-Type: application/json   │
     │                                   │
     │<────────────────────── 200 OK ────┤
     │                                   │
     ├─ Wait 5 seconds                   │
     │                                   │
     └─ Repeat                           │
```

## Timing

- **Data Collection Cycle**: ~5 seconds
  - Camera capture: ~2.5s
  - Sensor reading: ~2.5s
  - HTTP transmission: variable (depends on network)

- **HTTP Timeout**: 10 seconds per request
- **Retry Logic**: No automatic retry (failed requests are logged)

## Network Requirements

- **Protocol**: HTTP/1.1
- **WiFi**: 802.11 b/g/n
- **Security**: Unencrypted (consider HTTPS for production)
- **Bandwidth**: ~50-200 KB per image + ~200 bytes JSON data per cycle

## Configuration

Configure server endpoint in device configuration:
1. Enter configuration mode (press 'C' or use AP mode)
2. Set Server IP (e.g., `192.168.1.100`)
3. Set Server Port (e.g., `5000`)

Configuration is stored in SPIFFS at `/wifi_config.txt`:
```json
{
  "ssid": "YourWiFiNetwork",
  "password": "YourPassword",
  "ip": "192.168.1.100",
  "port": "5000"
}
```

## Testing the API

### Using Postman

1. **Image Upload**
   - Method: POST
   - URL: `http://{your-ip}:5000/data`
   - Headers: `Content-Type: image/jpeg`
   - Body: Binary → Select JPEG file

2. **Sensor Data**
   - Method: POST
   - URL: `http://{your-ip}:5000/api/data`
   - Headers: `Content-Type: application/json`
   - Body: Raw JSON
   ```json
   {
     "terra": 45.2,
     "acqua": 78.5,
     "distanza": 120,
     "umidita": 65.3,
     "temperatura": 22.5
   }
   ```

### Using Python Requests

```python
import requests

# Send sensor data
url = "http://192.168.1.100:5000/api/data"
data = {
    "terra": 45.2,
    "acqua": 78.5,
    "distanza": 120,
    "umidita": 65.3,
    "temperatura": 22.5
}
response = requests.post(url, json=data)
print(response.status_code, response.text)

# Send image
url = "http://192.168.1.100:5000/data"
with open("test.jpg", "rb") as f:
    response = requests.post(url, data=f, headers={"Content-Type": "image/jpeg"})
print(response.status_code, response.text)
```

## Security Considerations

⚠️ **Current Implementation**
- Unencrypted HTTP communication
- No authentication required
- WiFi credentials stored in plaintext on SPIFFS

🔒 **Production Recommendations**
- Use HTTPS for encrypted transmission
- Implement API key authentication
- Encrypt WiFi credentials on device
- Use WPA2/WPA3 WiFi security
- Implement rate limiting on server
- Validate and sanitize all input data

## Troubleshooting

### Connection Refused
- Verify server is running
- Check firewall settings
- Verify IP address and port

### Timeout Errors
- Check network latency
- Increase timeout in `network.h`
- Verify WiFi signal strength

### Invalid JSON
- Check for NaN values in sensor readings
- Verify JSON formatting
- Enable server-side logging

### Image Upload Fails
- Check camera buffer memory
- Verify JPEG quality settings
- Monitor available RAM
