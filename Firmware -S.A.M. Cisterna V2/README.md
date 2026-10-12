# S.A.M. Cisterna V2

![Version](https://img.shields.io/badge/version-2.0.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-orange.svg)

Sistema di monitoraggio intelligente per cisterne d'acqua basato su ESP32, con sensori ultrasonici per il livello dell'acqua e sensori ambientali per temperatura e umidità.

## Caratteristiche

- 📏 Misurazione livello acqua tramite sensore ultrasonico
- 🌡️ Monitoraggio temperatura e umidità ambientale (DHT22)
- 📡 Connessione WiFi con riconnessione automatica
- 🌐 Invio dati a server remoto via HTTP POST
- ⚙️ Interfaccia web per configurazione WiFi e server
- 🔒 Watchdog timer per stabilità del sistema
- 🛡️ Gestione errori completa e logging strutturato
- 💾 Configurazione persistente su SPIFFS

## Requisiti Hardware

### Componenti Principali
- **ESP32 Dev Module** (qualsiasi variante con WiFi)
- **Sensore Ultrasonico HC-SR04** (o compatibile)
- **Sensore DHT22** (temperatura e umidità)
- **Alimentazione**: 5V USB o alimentatore esterno

### Configurazione Pin

| Componente | Pin GPIO | Tipo |
|-----------|----------|------|
| DHT22 Data | GPIO 32 | Input |
| Ultrasonico TRIG | GPIO 26 | Output |
| Ultrasonico ECHO | GPIO 27 | Input |

## Installazione

### Arduino IDE

1. **Installare supporto ESP32**
   - File → Preferenze → URL aggiuntive per il gestore schede
   - Aggiungi: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Strumenti → Scheda → Gestore schede → Cerca "ESP32" e installa

2. **Installare librerie richieste**
   - Sketch → Include Library → Manage Libraries
   - Installa le seguenti librerie:
     - `DHT sensor library` by Adafruit
     - `ArduinoJson` by Benoit Blanchon
     - `ESPAsyncWebServer` by me-no-dev
     - `AsyncTCP` by me-no-dev (dipendenza di ESPAsyncWebServer)

3. **Aprire e caricare il firmware**
   - Apri `src/main.ino`
   - Seleziona: Strumenti → Scheda → ESP32 Dev Module
   - Seleziona la porta COM corretta
   - Clicca su "Carica"

### PlatformIO

Crea un file `platformio.ini` con questa configurazione:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 9600

lib_deps =
    adafruit/DHT sensor library
    bblanchon/ArduinoJson
    me-no-dev/ESPAsyncWebServer
    me-no-dev/AsyncTCP
```

Quindi esegui:
```bash
pio run --target upload
pio device monitor -b 9600
```

## Configurazione

### Prima Configurazione

1. **Carica il firmware** sul dispositivo ESP32
2. **Apri il Serial Monitor** a 9600 baud
3. Il dispositivo entrerà automaticamente in **modalità configurazione** se non trova una configurazione salvata
4. **Connettiti al WiFi**:
   - SSID: `SAM-CISTERNA`
   - Password: `<la-tua-password>` (va scelta dall'utente e impostata in `src/config/config.h`, `CONFIG_AP_PASSWORD`, prima del flash: il sorgente contiene solo un valore provvisorio di sviluppo)
5. **Apri il browser** e vai a: `http://192.168.4.1`
6. **Compila il form** con:
   - SSID della tua rete WiFi
   - Password WiFi
   - Indirizzo IP del server (es: `192.168.1.100`)
   - Porta del server (es: `5000`)
7. **Salva la configurazione** e riavvia il dispositivo

### Entrare in Modalità Configurazione

Puoi rientrare in modalità configurazione in due modi:

1. **Manualmente**: Invia il carattere `C` o `c` via Serial Monitor
2. **Automaticamente**: Se la connessione WiFi fallisce dopo 100 tentativi

## API Server

Il dispositivo invia dati al server tramite richieste HTTP POST.

### Endpoint
```
POST http://{ip}:{port}/api/cisterna
```

### Formato Richiesta
```json
{
  "distanza": 45,
  "temperatura": 23.5,
  "umidita": 65.2
}
```

### Campi
- `distanza` (int): Distanza in centimetri dal sensore ultrasonico (sempre presente)
- `temperatura` (float, opzionale): Temperatura in °C (omesso se errore di lettura)
- `umidita` (float, opzionale): Umidità relativa in % (omesso se errore di lettura)

### Risposta Attesa
```
HTTP/1.1 200 OK
```

### Esempio Server Flask

Vedi `examples/test-server/` per un server di test completo.

## Utilizzo

### Monitoraggio via Serial

Apri il Serial Monitor a 9600 baud per vedere:
- Stato inizializzazione sistema
- Connessione WiFi
- Letture dei sensori ogni 5 secondi
- Stato invio dati al server
- Eventuali errori

### Output Esempio
```
================================
S.A.M. Cisterna V2
Firmware Version: 2.0.0
================================

[MAIN] Initializing watchdog timer...
[STORAGE] SPIFFS mounted successfully
[SENSORS] Sensors initialized successfully
[NETWORK] Connecting to WiFi...
[NETWORK] WiFi connected successfully
[NETWORK] IP Address: 192.168.1.50

[SENSORS] Distance: 45 cm
[SENSORS] Temperature: 23.5 °C, Humidity: 65.2 %
[NETWORK] Data sent successfully
```

## Risoluzione Problemi

### Il dispositivo non si connette al WiFi
- Verifica SSID e password nella configurazione
- Controlla che la rete sia a 2.4 GHz (ESP32 non supporta 5 GHz)
- Avvicinati al router WiFi
- Riprova entrando in modalità configurazione (invia 'C' via serial)

### Il sensore DHT22 restituisce errori
- Controlla il collegamento del pin dati (GPIO 32)
- Verifica l'alimentazione del sensore (3.3V o 5V)
- Attendi almeno 2 secondi tra le letture
- Sostituisci il sensore se continua a dare errori

### Il sensore ultrasonico non rileva la distanza
- Verifica i collegamenti TRIG (GPIO 26) e ECHO (GPIO 27)
- Assicurati che non ci siano ostacoli nel campo del sensore
- Controlla che la superficie dell'acqua sia entro 4 metri
- Verifica l'alimentazione (5V per HC-SR04)

### Il dispositivo si riavvia continuamente
- Il watchdog timer potrebbe attivarsi: controlla gli errori via Serial Monitor
- Verifica che il server sia raggiungibile (timeout HTTP impostato a 10 secondi)
- Controlla l'alimentazione (ESP32 richiede corrente stabile)

### I dati non arrivano al server
- Verifica che l'IP e la porta del server siano corretti
- Controlla che il server sia avviato e in ascolto
- Verifica firewall e regole di rete
- Testa l'endpoint con curl:
  ```bash
  curl -X POST -H "Content-Type: application/json" \
       -d '{"distanza":100}' \
       http://192.168.1.100:5000/api/cisterna
  ```

## Architettura del Codice

Il firmware è organizzato in moduli separati:

```
src/
├── main.ino              # Entry point e loop principale
├── config/
│   ├── pins.h            # Definizioni pin hardware
│   └── config.h          # Costanti di configurazione
└── modules/
    ├── storage.cpp/h     # Gestione SPIFFS e configurazione
    ├── sensors.cpp/h     # Lettura sensori
    ├── network.cpp/h     # WiFi e HTTP
    └── configurator.cpp/h # Web server configurazione
```

Per dettagli architetturali completi, vedi [`CLAUDE.md`](CLAUDE.md).

## Correzioni Bug (v2.0.0)

Questa versione corregge **9 bug critici** presenti nella v1.0.0:

1. ✅ `Serial.begin()` chiamato solo in `setup()` (era in 4 posizioni)
2. ✅ WiFi sleep mode disabilitato (causava timeout HTTP)
3. ✅ Aggiunta logica di riconnessione WiFi automatica
4. ✅ Rimosso delay di 20 minuti bloccante
5. ✅ Aggiunto timeout HTTP (10 secondi)
6. ✅ Implementato watchdog timer (30 secondi)
7. ✅ Rimossi credenziali hardcoded dal codice
8. ✅ Aggiunta gestione errori completa
9. ✅ Corretto lifecycle del server web AsyncWebServer

## Contribuire

I contributi sono benvenuti! Per favore:
1. Fai un fork del progetto
2. Crea un branch per la tua feature (`git checkout -b feature/AmazingFeature`)
3. Committa le modifiche (`git commit -m 'Add some AmazingFeature'`)
4. Push al branch (`git push origin feature/AmazingFeature`)
5. Apri una Pull Request

## Licenza

Questo progetto è rilasciato sotto licenza MIT. Vedi il file [`LICENSE`](LICENSE) per i dettagli.

## Versioni

Vedi [`CHANGELOG.md`](CHANGELOG.md) per la storia completa delle versioni.

## Supporto

Per problemi, domande o suggerimenti, apri una issue su GitHub.

---

**Versione Firmware**: 2.0.0
**Data Rilascio**: 16 Ottobre 2025
**Piattaforma**: ESP32
