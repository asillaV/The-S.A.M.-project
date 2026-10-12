"""
Flask Server for S.A.M. Box V2
Receives image and sensor data from ESP32 device
"""

from flask import Flask, request, jsonify
import os
from datetime import datetime
import json

app = Flask(__name__)

# Configuration
UPLOAD_FOLDER = 'uploads'
LOG_FILE = 'sensor_log.txt'
DB_FILE = 'sensor_data.json'

# Create upload folder if it doesn't exist
os.makedirs(UPLOAD_FOLDER, exist_ok=True)


@app.route('/')
def index():
    """Home page with status"""
    return """
    <html>
    <head><title>S.A.M. Box V2 Server</title></head>
    <body>
        <h1>S.A.M. Box V2 Data Server</h1>
        <p>Server is running and ready to receive data.</p>
        <h2>Endpoints:</h2>
        <ul>
            <li><code>POST /data</code> - Upload images</li>
            <li><code>POST /api/data</code> - Upload sensor data</li>
            <li><code>GET /api/latest</code> - Get latest sensor reading</li>
        </ul>
    </body>
    </html>
    """


@app.route('/data', methods=['POST'])
def upload_image():
    """
    Receive and save JPEG image from ESP32-CAM
    Expected: Binary JPEG data in request body
    """
    try:
        image_data = request.get_data()

        # Validate it's a JPEG
        if not image_data.startswith(b'\xff\xd8'):
            return "Invalid image format (not JPEG)", 400

        # Generate filename with timestamp
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"image_{timestamp}.jpg"
        filepath = os.path.join(UPLOAD_FOLDER, filename)

        # Save image
        with open(filepath, 'wb') as f:
            f.write(image_data)

        print(f"[{datetime.now()}] Image saved: {filename} ({len(image_data)} bytes)")
        return "OK", 200

    except Exception as e:
        print(f"[{datetime.now()}] Error saving image: {e}")
        return f"Error: {str(e)}", 500


@app.route('/api/data', methods=['POST'])
def receive_sensor_data():
    """
    Receive sensor data from ESP32
    Expected JSON format:
    {
        "terra": float,      # Soil moisture %
        "acqua": float,      # Water level %
        "distanza": int,     # Distance cm
        "umidita": float,    # Humidity %
        "temperatura": float # Temperature °C
    }
    """
    try:
        data = request.get_json()

        if not data:
            return jsonify({"error": "No JSON data received"}), 400

        # Validate required fields
        required_fields = ['terra', 'acqua', 'distanza', 'umidita', 'temperatura']
        missing_fields = [field for field in required_fields if field not in data]

        if missing_fields:
            return jsonify({
                "error": "Missing required fields",
                "missing": missing_fields
            }), 400

        # Add timestamp
        data['timestamp'] = datetime.now().isoformat()

        # Log to console
        print(f"[{data['timestamp']}] Sensor data received:")
        print(f"  Temperature: {data['temperatura']}°C")
        print(f"  Humidity: {data['umidita']}%")
        print(f"  Soil moisture: {data['terra']}%")
        print(f"  Water level: {data['acqua']}%")
        print(f"  Distance: {data['distanza']}cm")

        # Append to text log
        with open(LOG_FILE, 'a') as f:
            log_entry = f"{data['timestamp']},{data['temperatura']},{data['umidita']},{data['terra']},{data['acqua']},{data['distanza']}\n"
            f.write(log_entry)

        # Save to JSON database
        save_to_database(data)

        return jsonify({
            "status": "success",
            "message": "Data received and stored",
            "timestamp": data['timestamp']
        }), 200

    except json.JSONDecodeError:
        return jsonify({"error": "Invalid JSON format"}), 400
    except Exception as e:
        print(f"[{datetime.now()}] Error processing sensor data: {e}")
        return jsonify({"error": str(e)}), 500


@app.route('/api/latest', methods=['GET'])
def get_latest_data():
    """Get the most recent sensor reading"""
    try:
        if not os.path.exists(DB_FILE):
            return jsonify({"error": "No data available"}), 404

        with open(DB_FILE, 'r') as f:
            data = json.load(f)

        if not data:
            return jsonify({"error": "No data available"}), 404

        # Return the most recent entry
        latest = data[-1]
        return jsonify(latest), 200

    except Exception as e:
        return jsonify({"error": str(e)}), 500


def save_to_database(data):
    """Append sensor data to JSON database"""
    try:
        # Read existing data
        if os.path.exists(DB_FILE):
            with open(DB_FILE, 'r') as f:
                database = json.load(f)
        else:
            database = []

        # Append new data
        database.append(data)

        # Keep only last 1000 entries
        if len(database) > 1000:
            database = database[-1000:]

        # Write back to file
        with open(DB_FILE, 'w') as f:
            json.dump(database, f, indent=2)

    except Exception as e:
        print(f"Error saving to database: {e}")


if __name__ == '__main__':
    print("=" * 50)
    print("S.A.M. Box V2 Server Starting...")
    print("=" * 50)
    print(f"Upload folder: {UPLOAD_FOLDER}")
    print(f"Log file: {LOG_FILE}")
    print(f"Database: {DB_FILE}")
    print("=" * 50)
    print("\nServer running on http://0.0.0.0:5000")
    print("Press Ctrl+C to stop\n")

    # Start Flask server. The Werkzeug debugger allows remote code execution:
    # keep it off unless you explicitly set SAM_DEBUG=1 on a trusted network.
    debug = os.environ.get('SAM_DEBUG', '').lower() in ('1', 'true', 'yes')
    app.run(host='0.0.0.0', port=5000, debug=debug)
