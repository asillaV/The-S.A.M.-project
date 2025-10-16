# S.A.M. Box V2 - Monitor Agricolo Intelligente

![Version](https://img.shields.io/badge/version-2.0.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-orange.svg)

Dispositivo IoT basato su ESP32 per il monitoraggio agricolo intelligente. Raccoglie dati ambientali da più sensori e acquisisce immagini per il monitoraggio remoto via WiFi.

## Funzionalità

- **Supporto multi-sensore**: temperatura, umidità, umidità del suolo e livello dell'acqua
- **Integrazione ESP32-CAM**: acquisisce e trasmette immagini JPEG
- **Connettività WiFi**: trasmissione dati via HTTP POST a server remoto
- **Configurazione via Web**: setup semplice tramite modalità Access Point
- **Memoria persistente**: configurazione salvata su SPIFFS
- **Auto-riconnessione**: recupero automatico della connessione WiFi
- **Watchdog Timer**: stabilità del sistema e auto-recupero
- **Comandi seriali**: configurazione e debug a runtime

## Requisiti hardware

### Componenti principali
- **Modulo ESP32 WROVER** con PSRAM
- **Modulo ESP32-CAM** (CAMERA_MODEL_WROVER_KIT)
- **DHT22** - Sensore di temperatura e umidità
- **Sensore di umidità del suolo** (analogico)
- **Sensore livello acqua** (analogico)
- **ATtiny85** - Microcontrollore di interfaccia sensori

### Configurazione pin

| Componente | GPIO |
|-----------|------|
| Dati DHT22 | GPIO 33 |
| Umidità suolo | GPIO 32 |
| Sensore acqua | GPIO 13 |
| Modulo fotocamera | Vedi camera_pins.h |

### Comunicazioni
- **Seriale**: 9600 baud (ESP32 ↔ ATtiny85 letture sensori)
- **WiFi**: 802.11 b/g/n

## Installazione

### Arduino IDE

1. **Installa il supporto schede ESP32**
   - Apri Arduino IDE
   - Vai su File → Preferenze
   - Aggiungi a "URL aggiuntive per il Gestore schede":
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
   - Vai su Strumenti → Scheda → Gestore schede
   - Cerca "ESP32" e installa

2. **Installa le librerie richieste**
   - WiFi (inclusa)
   - DHT sensor library di Adafruit
   - HTTPClient (inclusa)
   - esp_camera (inclusa)
   - ArduinoJson di Benoit Blanchon
   - ESPAsyncWebServer di me-no-dev
   - SPIFFS (inclusa)

3. **Seleziona la scheda**
   - Strumenti → Scheda → ESP32 Wrover Module
   - Strumenti → Porta → (seleziona la tua porta COM)

4. **Carica il firmware**
   - Apri `src/main.ino`
   - Clicca Carica

### PlatformIO

```bash
# Clona il repository
git clone <repository-url>
cd Firmware-SAM-Box-V2

# Compila e carica
pio run --target upload

# Monitora l'output seriale
pio device monitor -b 9600
```

## Configurazione

### Prima configurazione

1. **Accendi il dispositivo**
2. Se non è presente una configurazione WiFi, il dispositivo entra automaticamente in **Modalità Configurazione**
3. Connettiti alla rete WiFi: `ESP32-Access-Point` (password: `password123`)
4. Apri il browser e vai su: `http://192.168.4.1`
5. Inserisci le credenziali WiFi e i dettagli del server:
   - **SSID**: Nome della tua rete WiFi
   - **Password**: Password della tua rete WiFi
   - **Server IP**: Indirizzo IP del server Flask
   - **Server Port**: Porta del server Flask (predefinita: 5000)
6. Clicca "Salva Configurazione"
7. Il dispositivo si riavvierà e si connetterà alla tua rete

### Modalità configurazione manuale

Premi `C` o `c` nel Monitor Seriale per entrare in qualsiasi momento in modalità configurazione.

### Comandi seriali

| Comando | Azione |
|---------|--------|
| `C` / `c` | Entra in modalità configurazione |
| `R` / `r` | Riavvia il dispositivo |
| `V` / `v` | Mostra la versione del firmware |

## Requisiti del server

Il dispositivo invia i dati a due endpoint HTTP:

### 1. Endpoint immagini
```
POST http://{server-ip}:{port}/data
Content-Type: image/jpeg
Body: JPEG image data
```

### 2. Endpoint dati sensori
```
POST http://{server-ip}:{port}/api/data
Content-Type: application/json
Body: {
  "terra": 45.2,
  "acqua": 78.5,
  "distanza": 120,
  "umidita": 65.3,
  "temperatura": 22.5
}
```

Vedi `examples/server-flask/` per un esempio di implementazione server Flask.

## Struttura del progetto

```
Firmware-SAM-Box-V2/
├── src/
│   ├── main.ino                 # Punto di ingresso principale del firmware
│   ├── config/
│   │   ├── WifiConfig.h         # Struttura di configurazione WiFi
│   │   ├── camera_pins.h        # Definizioni pin fotocamera
│   │   └── sensor_pins.h        # Definizioni pin sensori
│   ├── modules/
│   │   ├── acquisition.cpp/h    # Acquisizione dati sensori
│   │   ├── configurator.cpp     # Server web di configurazione
│   │   └── network.cpp/h        # Funzioni di rete e HTTP
│   └── data/
│       └── wifi_config.txt      # Archiviazione configurazione (SPIFFS)
├── docs/
│   ├── CLAUDE.md                # Linee guida Claude Code
│   ├── hardware-setup.md        # Guida di assemblaggio hardware
│   └── api-reference.md         # Documentazione API
├── examples/
│   └── server-flask/            # Esempio server Flask
├── README.md
├── LICENSE
└── CHANGELOG.md
```

## Risoluzione dei problemi

### Connessione WiFi fallita
- Verifica le credenziali nella configurazione
- Controlla la potenza del segnale WiFi
- Premi `C` per riconfigurare

### Inizializzazione fotocamera fallita
- Controlla i collegamenti del modulo fotocamera
- Verifica che la PSRAM sia rilevata
- Verifica che `camera_pins.h` corrisponda al tuo hardware

### Errori nelle letture dei sensori
- Verifica il collegamento del DHT22 su GPIO 33
- Controlla la comunicazione seriale con ATtiny85 (9600 baud)
- Verifica i collegamenti dei sensori suolo/acqua

### Errori nelle richieste HTTP POST
- Verifica che il server sia in esecuzione e raggiungibile
- Controlla IP e porta del server nella configurazione
- Monitora l'output seriale per i codici di errore

### Crash/Riavvii del dispositivo
- Controlla l'alimentazione (5V/3.3V stabili richiesti)
- Monitora i messaggi del watchdog
- Esamina l'output seriale per eventuali stack trace

## Sviluppo

### Build dai sorgenti
```bash
# Installa PlatformIO
pip install platformio

# Compila il firmware
pio run

# Carica sul dispositivo
pio run --target upload

# Apri il monitor seriale
pio device monitor -b 9600
```

### Debug
Abilita output seriale verboso in `src/main.ino`:
```cpp
#define DEBUG_MODE 1
```

## Contributi

Contributi benvenuti! Per favore:
1. Fai un fork del repository
2. Crea un branch per la feature
3. Fai commit delle modifiche
4. Esegui il push del branch
5. Apri una Pull Request

## Licenza

Questo progetto è rilasciato sotto licenza MIT - vedi il file [LICENSE](LICENSE) per i dettagli.

## Cronologia versioni

Vedi [CHANGELOG.md](CHANGELOG.md) per la cronologia e gli aggiornamenti.

## Autori

Team di progetto S.A.M.

## Supporto

Per problemi e domande:
- Apri una issue su GitHub
- Consulta la documentazione in `docs/`
- Rivedi la sezione di troubleshooting sopra

---

**Versione firmware**: 2.0.0  
**Ultimo aggiornamento**: Gennaio 2025
