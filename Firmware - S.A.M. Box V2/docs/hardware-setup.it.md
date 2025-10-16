# Guida al Setup Hardware

## Distinta base (BOM)

| Componente | Quantità | Descrizione |
|-----------|----------|-------------|
| Modulo ESP32 WROVER | 1 | Microcontrollore principale con PSRAM |
| Modulo ESP32-CAM | 1 | Modulo fotocamera (OV2640) |
| DHT22 | 1 | Sensore di temperatura e umidità |
| Sensore di umidità del suolo | 1 | Tipo capacitivo o resistivo |
| Sensore livello acqua | 1 | Sensore analogico di livello |
| ATtiny85 | 1 | Microcontrollore di interfaccia sensori |
| Resistenza 10kΩ | 1 | Pull-up per DHT22 |
| Alimentatore | 1 | 5V/2A minimo |
| Cavi jumper | ~20 | Per i collegamenti |

## Connessioni dei pin

### ESP32 WROVER verso sensori

#### Sensore DHT22 Temperatura/Umidità
```
DHT22 Pin 1 (VCC)  → ESP32 3.3V
DHT22 Pin 2 (DATA) → ESP32 GPIO 33 (con pull-up 10kΩ a 3.3V)
DHT22 Pin 3 (NC)   → Non connesso
DHT22 Pin 4 (GND)  → ESP32 GND
```

#### Sensore di umidità del suolo
```
VCC sensore → ESP32 3.3V
GND sensore → ESP32 GND
OUT sensore → ESP32 GPIO 32 (o collega tramite ATtiny85)
```

#### Sensore livello acqua
```
VCC sensore → ESP32 3.3V
GND sensore → ESP32 GND
OUT sensore → ESP32 GPIO 13 (o collega tramite ATtiny85)
```

#### ATtiny85 (per multiplexing sensori)
```
ATtiny85 VCC → ESP32 3.3V
ATtiny85 GND → ESP32 GND
ATtiny85 TX  → ESP32 RX (GPIO 3)
ATtiny85 RX  → ESP32 TX (GPIO 1)
```

### Configurazione pin ESP32-CAM

La fotocamera utilizza i seguenti GPIO (definiti in `camera_pins.h`):

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

## Schema di cablaggio

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

## Istruzioni di assemblaggio

### 1. Assemblaggio modulo ESP32-CAM
1. Assicurarsi che il modulo fotocamera OV2640 sia inserito correttamente
2. Verificare la presenza del chip PSRAM sulla scheda
3. Connettere il programmatore FTDI per il primo upload del firmware
4. Caricare il firmware tramite Arduino IDE o PlatformIO

### 2. Collegamenti sensori
1. Collegare il DHT22 con resistenza di pull-up (10kΩ) tra DATA e VCC
2. Collegare l'uscita analogica del sensore suolo a GPIO 32
3. Collegare l'uscita analogica del sensore acqua a GPIO 13
4. Assicurarsi che tutti i sensori condividano la massa (GND) con l'ESP32

### 3. Setup ATtiny85 (opzionale)
Se si usa ATtiny85 per il multiplexing dei sensori:
1. Programmare ATtiny85 con il firmware di lettura sensori
2. Collegare i pin di comunicazione seriale (TX/RX)
3. Configurarlo per rispondere ai comandi 'w' (water) e 's' (soil)
4. Impostare il baud rate a 9600

### 4. Alimentazione
1. Usare un alimentatore a 5V regolato (almeno 2A)
2. ESP32-CAM richiede alimentazione stabile per funzionamento affidabile
3. Considerare l'aggiunta di condensatori di disaccoppiamento (100µF) vicino all'ESP32
4. Evitare l'alimentazione da USB se la fotocamera è attiva (assorbimento troppo elevato)

## Test

### 1. Test Monitor Seriale
```
- Aprire il Monitor Seriale a 9600 baud
- Accendere il dispositivo
- Verificare la comparsa dei messaggi di boot
- Controllare il messaggio di rilevamento PSRAM
- Verificare l'inizializzazione della fotocamera
```

### 2. Test sensori
```
- Premere 'V' per verificare la versione firmware
- Osservare le letture dei sensori nel Monitor Seriale
- Verificare che temperatura e umidità siano plausibili
- Controllare i valori suolo/acqua (intervallo 0–100%)
```

### 3. Test fotocamera
```
- Controllare il messaggio "Camera frame captured successfully"
- Verificare assenza di errori di memoria
- Confermare che i frame JPEG vengano acquisiti
```

### 4. Test WiFi
```
- Il dispositivo deve connettersi alla rete configurata
- Controllare l'indirizzo IP nel Monitor Seriale
- Verificare che le richieste HTTP POST abbiano esito 200 OK
```

## Troubleshooting

### Fotocamera non rilevata
- Controllare tutte le connessioni dei pin della fotocamera
- Verificare il rilevamento della PSRAM (vedi output seriale)
- Provare a ridurre la frequenza XCLK a 10 MHz
- Assicurarsi che il modulo fotocamera non sia danneggiato

### Errori di lettura sensori
- Verificare i collegamenti di alimentazione dei sensori (3.3V/GND)
- Controllare che la resistenza di pull-up del DHT22 sia installata
- Testare i sensori singolarmente con un multimetro
- Verificare che i GPIO non siano già in uso

### Problemi di alimentazione
- Usare un alimentatore esterno 5V (non USB)
- Aggiungere condensatori bulk (100–470µF) vicino all'ESP32
- Controllare eventuali cadute di tensione durante l'uso della fotocamera
- Verificare una corrente disponibile di almeno 2A

### Problemi di comunicazione seriale
- Verificare baud rate 9600 (sia su ESP32 che su ATtiny85)
- Controllare che TX/RX non siano invertiti
- Testare prima con loopback seriale
- Svuotare il buffer seriale prima di leggere

## Note di sicurezza

⚠️ **Informazioni importanti**

- Non superare 3,3V su nessun pin GPIO
- ESP32-CAM può assorbire corrente elevata durante la trasmissione
- Usare protezione ESD quando si maneggiano i moduli
- Tenere i sensori d'acqua lontani dalla scheda ESP32
- Non esporre l'elettronica all'umidità
- Usare una sezione di cavo adeguata per le linee di alimentazione

## Risorse aggiuntive

- Datasheet ESP32-CAM: [Documentazione Espressif](https://www.espressif.com)
- Datasheet DHT22: Cerca "DHT22 datasheet"
- Guida alla calibrazione del sensore di umidità del suolo
- Guida alla programmazione di ATtiny85

## Schema elettrico

Vedi `hardware/schematic.pdf` per lo schema completo.
