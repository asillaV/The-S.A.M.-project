# API Reference

Documentazione completa dell'API HTTP utilizzata da S.A.M. Cisterna V2 per comunicare con il server remoto.

## Panoramica

Il dispositivo ESP32 invia periodicamente (ogni 5 secondi) i dati dei sensori a un server HTTP tramite richieste POST in formato JSON.

- **Protocollo**: HTTP/1.1
- **Metodo**: POST
- **Content-Type**: application/json
- **Frequenza**: Configurabile (default: 5 secondi)
- **Timeout**: 10 secondi

## Endpoint

### POST /api/cisterna

Riceve i dati dai sensori del dispositivo S.A.M. Cisterna V2.

#### URL

```
POST http://{server_ip}:{server_port}/api/cisterna
```

Dove:
- `{server_ip}`: Indirizzo IP del server (configurato via web interface)
- `{server_port}`: Porta del server (configurato via web interface)

#### Headers

```http
Content-Type: application/json
```

#### Request Body

```json
{
  "distanza": 45,
  "temperatura": 23.5,
  "umidita": 65.2
}
```

#### Parametri

| Campo | Tipo | Obbligatorio | Descrizione | Range |
|-------|------|--------------|-------------|-------|
| `distanza` | integer | Sì | Distanza in centimetri dal sensore ultrasonico alla superficie dell'acqua | 2-400 cm o -1 per errore |
| `temperatura` | float | No | Temperatura ambiente in gradi Celsius | -40.0 a 80.0 °C |
| `umidita` | float | No | Umidità relativa in percentuale | 0.0 a 100.0 % |

**Note**:
- `distanza` è sempre presente nel payload
- `temperatura` e `umidita` sono omessi se il sensore DHT22 fallisce la lettura
- Un valore di `distanza = -1` indica un errore di lettura del sensore ultrasonico

#### Response

##### Successo (200 OK)

```http
HTTP/1.1 200 OK
Content-Type: application/json

{
  "status": "success",
  "message": "Data received"
}
```

Il corpo della risposta può essere vuoto o contenere qualsiasi dato - il dispositivo accetta qualsiasi risposta HTTP 200.

##### Errore (4xx/5xx)

```http
HTTP/1.1 400 Bad Request
Content-Type: application/json

{
  "status": "error",
  "message": "Invalid data format"
}
```

In caso di errore, il dispositivo logga il codice HTTP e continua il funzionamento normale.

#### Esempi

##### Esempio 1: Tutti i sensori funzionanti

**Request**:
```http
POST /api/cisterna HTTP/1.1
Host: 192.168.1.100:5000
Content-Type: application/json

{
  "distanza": 45,
  "temperatura": 23.5,
  "umidita": 65.2
}
```

**Response**:
```http
HTTP/1.1 200 OK
Content-Type: application/json

{
  "status": "success"
}
```

##### Esempio 2: DHT22 non funzionante

**Request**:
```http
POST /api/cisterna HTTP/1.1
Host: 192.168.1.100:5000
Content-Type: application/json

{
  "distanza": 45
}
```

**Response**:
```http
HTTP/1.1 200 OK
```

##### Esempio 3: Sensore ultrasonico in errore

**Request**:
```http
POST /api/cisterna HTTP/1.1
Host: 192.168.1.100:5000
Content-Type: application/json

{
  "distanza": -1,
  "temperatura": 23.5,
  "umidita": 65.2
}
```

## Codici di Stato HTTP

| Codice | Significato | Comportamento Dispositivo |
|--------|-------------|---------------------------|
| 200 | OK - Dati ricevuti correttamente | Continua normale funzionamento |
| 400 | Bad Request - Formato dati errato | Logga errore, continua funzionamento |
| 404 | Not Found - Endpoint non trovato | Logga errore, continua funzionamento |
| 500 | Internal Server Error | Logga errore, continua funzionamento |
| Timeout | Nessuna risposta entro 10 secondi | Logga timeout, continua funzionamento |

## Implementazione Server

### Esempio: Flask (Python)

```python
from flask import Flask, request, jsonify
from datetime import datetime

app = Flask(__name__)

@app.route('/api/cisterna', methods=['POST'])
def receive_data():
    try:
        data = request.get_json()

        # Validazione dati
        if 'distanza' not in data:
            return jsonify({
                'status': 'error',
                'message': 'Missing required field: distanza'
            }), 400

        distanza = data.get('distanza')
        temperatura = data.get('temperatura', None)
        umidita = data.get('umidita', None)

        # Log dei dati
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        print(f"[{timestamp}] Distanza: {distanza} cm", end='')

        if temperatura is not None:
            print(f", Temperatura: {temperatura} °C", end='')
        if umidita is not None:
            print(f", Umidità: {umidita} %", end='')
        print()

        # Qui puoi salvare i dati in un database
        # save_to_database(distanza, temperatura, umidita, timestamp)

        return jsonify({
            'status': 'success',
            'message': 'Data received and processed'
        }), 200

    except Exception as e:
        print(f"Error: {str(e)}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
```

### Esempio: Node.js (Express)

```javascript
const express = require('express');
const app = express();
const port = 5000;

app.use(express.json());

app.post('/api/cisterna', (req, res) => {
    try {
        const { distanza, temperatura, umidita } = req.body;

        // Validazione
        if (distanza === undefined) {
            return res.status(400).json({
                status: 'error',
                message: 'Missing required field: distanza'
            });
        }

        // Log dei dati
        const timestamp = new Date().toISOString();
        let logMessage = `[${timestamp}] Distanza: ${distanza} cm`;

        if (temperatura !== undefined) {
            logMessage += `, Temperatura: ${temperatura} °C`;
        }
        if (umidita !== undefined) {
            logMessage += `, Umidità: ${umidita} %`;
        }

        console.log(logMessage);

        // Salva in database (esempio)
        // saveToDatabase(distanza, temperatura, umidita, timestamp);

        res.json({
            status: 'success',
            message: 'Data received'
        });

    } catch (error) {
        console.error('Error:', error);
        res.status(500).json({
            status: 'error',
            message: error.message
        });
    }
});

app.listen(port, '0.0.0.0', () => {
    console.log(`Server listening on http://0.0.0.0:${port}`);
});
```

## Test con cURL

### Test Base

```bash
curl -X POST http://192.168.1.100:5000/api/cisterna \
  -H "Content-Type: application/json" \
  -d '{"distanza": 45, "temperatura": 23.5, "umidita": 65.2}'
```

### Test Solo Distanza

```bash
curl -X POST http://192.168.1.100:5000/api/cisterna \
  -H "Content-Type: application/json" \
  -d '{"distanza": 45}'
```

### Test con Verbose Output

```bash
curl -v -X POST http://192.168.1.100:5000/api/cisterna \
  -H "Content-Type: application/json" \
  -d '{"distanza": 45, "temperatura": 23.5, "umidita": 65.2}'
```

## Integrazione Database

### Esempio: MySQL

```python
import mysql.connector
from datetime import datetime

def save_to_mysql(distanza, temperatura, umidita):
    conn = mysql.connector.connect(
        host="localhost",
        user="samuser",
        password="sampassword",
        database="sam_cisterna"
    )
    cursor = conn.cursor()

    query = """
        INSERT INTO misurazioni (timestamp, distanza, temperatura, umidita)
        VALUES (%s, %s, %s, %s)
    """
    values = (datetime.now(), distanza, temperatura, umidita)

    cursor.execute(query, values)
    conn.commit()

    cursor.close()
    conn.close()
```

### Esempio: InfluxDB (Time Series)

```python
from influxdb_client import InfluxDBClient, Point
from datetime import datetime

def save_to_influxdb(distanza, temperatura, umidita):
    client = InfluxDBClient(
        url="http://localhost:8086",
        token="your-token",
        org="your-org"
    )

    write_api = client.write_api()

    point = Point("cisterna") \
        .field("distanza", distanza) \
        .field("temperatura", temperatura) \
        .field("umidita", umidita) \
        .time(datetime.utcnow())

    write_api.write(bucket="sam_data", record=point)
    client.close()
```

## Sicurezza

### Raccomandazioni

1. **Autenticazione**: Implementa un sistema di autenticazione (API key, JWT)
   ```python
   @app.route('/api/cisterna', methods=['POST'])
   def receive_data():
       api_key = request.headers.get('X-API-Key')
       if api_key != VALID_API_KEY:
           return jsonify({'error': 'Unauthorized'}), 401
       # ... resto del codice
   ```

2. **HTTPS**: Usa HTTPS in produzione per criptare i dati
   ```python
   # Flask con HTTPS
   if __name__ == '__main__':
       app.run(host='0.0.0.0', port=5000,
               ssl_context=('cert.pem', 'key.pem'))
   ```

3. **Rate Limiting**: Limita il numero di richieste per prevenire abusi
   ```python
   from flask_limiter import Limiter

   limiter = Limiter(app, default_limits=["100 per minute"])

   @app.route('/api/cisterna', methods=['POST'])
   @limiter.limit("20 per minute")
   def receive_data():
       # ... codice
   ```

4. **Validazione Input**: Valida sempre i dati ricevuti
   ```python
   if not isinstance(distanza, (int, float)) or distanza < -1 or distanza > 400:
       return jsonify({'error': 'Invalid distance value'}), 400
   ```

## Monitoraggio

### Metriche Consigliate

- Numero di richieste ricevute
- Tempo di risposta medio
- Tasso di errori
- Ultima ricezione dati (per rilevare disconnessioni)

### Esempio: Prometheus Metrics

```python
from prometheus_client import Counter, Histogram, generate_latest

requests_total = Counter('cisterna_requests_total', 'Total requests')
request_duration = Histogram('cisterna_request_duration_seconds', 'Request duration')

@app.route('/api/cisterna', methods=['POST'])
@request_duration.time()
def receive_data():
    requests_total.inc()
    # ... resto del codice

@app.route('/metrics')
def metrics():
    return generate_latest()
```

## Troubleshooting

### Il server non riceve dati

1. Verifica che il server sia in ascolto: `netstat -an | grep 5000`
2. Testa con curl dal PC: vedi esempi sopra
3. Controlla firewall: `sudo ufw allow 5000/tcp`
4. Verifica indirizzo IP nel dispositivo: deve corrispondere al server

### Timeout frequenti

1. Verifica latenza di rete: `ping {server_ip}`
2. Riduci il timeout nel firmware se necessario
3. Ottimizza il codice del server per risposte più veloci
4. Considera l'uso di un database più veloce

### Dati mancanti o errati

1. Logga il corpo della richiesta raw nel server
2. Verifica il formato JSON ricevuto
3. Controlla encoding (deve essere UTF-8)
4. Verifica che i campi opzionali siano gestiti correttamente

---

Per ulteriori informazioni, consulta la documentazione completa nel README.md.
