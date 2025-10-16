# Riferimento API

## Endpoint del server

La S.A.M. Box V2 invia dati a un server remoto tramite richieste HTTP POST verso due endpoint.

## Endpoint 1: Caricamento immagine

### Richiesta
```http
POST /data HTTP/1.1
Host: {server-ip}:{port}
Content-Type: image/jpeg
Content-Length: {size}

[Binary JPEG Image Data]
```

### Risposta
```http
HTTP/1.1 200 OK
Content-Type: text/plain

OK
```

### Descrizione
Carica un'immagine JPEG acquisita dal modulo ESP32-CAM. L'immagine viene inviata come dati binari grezzi nel corpo della richiesta.

### Codici di errore
- `200 OK` - Immagine caricata con successo
- `400 Bad Request` - Dati immagine non validi
- `500 Internal Server Error` - Errore di elaborazione sul server
- `503 Service Unavailable` - Server temporaneamente non disponibile

### Esempio cURL
```bash
curl -X POST \
  -H "Content-Type: image/jpeg" \
  --data-binary @image.jpg \
  http://192.168.1.100:5000/data
```

---

## Endpoint 2: Dati sensori

### Richiesta
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

### Risposta
```http
HTTP/1.1 200 OK
Content-Type: application/json

{
  "status": "success",
  "message": "Data received"
}
```

### Campi JSON

| Campo | Tipo | Unità | Intervallo | Descrizione |
|-------|------|------|-------|-------------|
| `terra` | float | % | 0-100 | Percentuale di umidità del suolo |
| `acqua` | float | % | 0-100 | Percentuale del livello dell'acqua |
| `distanza` | int | cm | 0-400 | Misura di distanza (uso futuro) |
| `umidita` | float | % | 0-100 | Percentuale di umidità dell'aria |
| `temperatura` | float | °C | -40 a 80 | Temperatura dell'aria in gradi Celsius |

### Valori speciali
- `-1.0` - Timeout o errore di lettura del sensore
- `-999.0` - Guasto del sensore DHT22 (lettura NaN)

### Esempio cURL
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

### Codici di errore
- `200 OK` - Dati ricevuti con successo
- `400 Bad Request` - Formato JSON non valido
- `422 Unprocessable Entity` - JSON valido ma valori dei campi non validi
- `500 Internal Server Error` - Errore di elaborazione sul server

---

## Implementazione server di esempio

### Esempio Flask

Vedi `examples/server-flask/app.py` per un'implementazione completa:

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

## Flusso di comunicazione

```
S.A.M. Box V2                          Server
     │                                   │
     ├─ Acquisisci immagine ────────────>│
     │  POST /data                       │
     │  Content-Type: image/jpeg         │
     │                                   │
     │<────────────────────── 200 OK ────┤
     │                                   │
     ├─ Invia dati sensori ─────────────>│
     │  POST /api/data                   │
     │  Content-Type: application/json   │
     │                                   │
     │<────────────────────── 200 OK ────┤
     │                                   │
     ├─ Attendi 5 secondi                │
     │                                   │
     └─ Ripeti                           │
```

## Tempistiche

- **Ciclo di raccolta dati**: ~5 secondi
  - Acquisizione fotocamera: ~2,5s
  - Lettura sensori: ~2,5s
  - Trasmissione HTTP: variabile (dipende dalla rete)

- **Timeout HTTP**: 10 secondi per richiesta
- **Logica di retry**: Nessun retry automatico (le richieste fallite vengono loggate)

## Requisiti di rete

- **Protocollo**: HTTP/1.1
- **WiFi**: 802.11 b/g/n
- **Sicurezza**: Non cifrato (considerare HTTPS in produzione)
- **Banda**: ~50–200 KB per immagine + ~200 byte di dati JSON per ciclo

## Configurazione

Configura l'endpoint del server nella configurazione del dispositivo:
1. Entra in modalità configurazione (premi 'C' o usa la modalità AP)
2. Imposta IP del server (es. `192.168.1.100`)
3. Imposta Porta del server (es. `5000`)

La configurazione è memorizzata in SPIFFS in `/wifi_config.txt`:
```json
{
  "ssid": "YourWiFiNetwork",
  "password": "YourPassword",
  "ip": "192.168.1.100",
  "port": "5000"
}
```

## Test dell'API

### Con Postman

1. **Caricamento immagine**
   - Metodo: POST
   - URL: `http://{your-ip}:5000/data`
   - Intestazioni: `Content-Type: image/jpeg`
   - Body: Binario → Seleziona file JPEG

2. **Dati sensori**
   - Metodo: POST
   - URL: `http://{your-ip}:5000/api/data`
   - Intestazioni: `Content-Type: application/json`
   - Body: JSON Raw
   ```json
   {
     "terra": 45.2,
     "acqua": 78.5,
     "distanza": 120,
     "umidita": 65.3,
     "temperatura": 22.5
   }
   ```

### Con Python Requests

```python
import requests

# Invia dati sensori
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

# Invia immagine
url = "http://192.168.1.100:5000/data"
with open("test.jpg", "rb") as f:
    response = requests.post(url, data=f, headers={"Content-Type": "image/jpeg"})
print(response.status_code, response.text)
```

## Considerazioni sulla sicurezza

⚠️ **Implementazione attuale**
- Comunicazione HTTP non cifrata
- Nessuna autenticazione richiesta
- Credenziali WiFi memorizzate in chiaro su SPIFFS

🔒 **Raccomandazioni per la produzione**
- Usa HTTPS per la trasmissione cifrata
- Implementa autenticazione tramite API key
- Cifra le credenziali WiFi sul dispositivo
- Usa sicurezza WiFi WPA2/WPA3
- Implementa rate limiting sul server
- Valida e sanitizza tutti i dati in ingresso

## Risoluzione dei problemi

### Connessione rifiutata
- Verifica che il server sia in esecuzione
- Controlla le impostazioni del firewall
- Verifica indirizzo IP e porta

### Timeout
- Controlla la latenza di rete
- Aumenta il timeout in `network.h`
- Verifica la potenza del segnale WiFi

### JSON non valido
- Verifica la presenza di valori NaN nelle letture dei sensori
- Controlla la formattazione del JSON
- Abilita il logging lato server

### Caricamento immagine fallito
- Controlla la memoria buffer della fotocamera
- Verifica le impostazioni di qualità JPEG
- Monitora la RAM disponibile
