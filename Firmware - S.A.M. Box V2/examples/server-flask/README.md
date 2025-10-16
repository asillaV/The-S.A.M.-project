# Flask Server Example for S.A.M. Box V2

Simple Flask server to receive data from S.A.M. Box V2 device.

## Installation

```bash
# Install Python dependencies
pip install -r requirements.txt
```

## Running the Server

```bash
python app.py
```

Server will start on `http://0.0.0.0:5000`

## Endpoints

- `POST /data` - Receives JPEG images
- `POST /api/data` - Receives sensor data (JSON)
- `GET /api/latest` - Returns most recent sensor reading

## Configuration

Edit `app.py` to change:
- `UPLOAD_FOLDER` - Where images are saved
- `LOG_FILE` - Sensor data log file
- `DB_FILE` - JSON database file
- Port (default: 5000)

## Testing

```bash
# Test sensor data endpoint
curl -X POST http://localhost:5000/api/data \
  -H "Content-Type: application/json" \
  -d '{"terra":45.2,"acqua":78.5,"distanza":120,"umidita":65.3,"temperatura":22.5}'

# Get latest reading
curl http://localhost:5000/api/latest
```
