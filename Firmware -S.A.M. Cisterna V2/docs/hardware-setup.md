# Hardware Setup Guide

Guida completa per l'assemblaggio hardware del sistema S.A.M. Cisterna V2.

## Bill of Materials (BOM)

| Componente | Quantità | Specifiche | Prezzo Indicativo |
|-----------|----------|------------|-------------------|
| ESP32 Dev Module | 1 | ESP32-WROOM-32 o compatibile | €5-10 |
| Sensore Ultrasonico HC-SR04 | 1 | Range: 2cm - 400cm | €2-5 |
| Sensore DHT22 | 1 | Temperatura: -40~80°C, Umidità: 0~100% | €3-8 |
| Cavi Dupont | ~10 | Maschio-Maschio e Maschio-Femmina | €2-5 |
| Breadboard | 1 | 830 punti (opzionale per test) | €3-5 |
| Alimentatore 5V | 1 | Micro-USB, almeno 1A | €3-10 |
| Contenitore IP65 | 1 | Per installazione esterna (opzionale) | €5-15 |

**Costo totale stimato**: €25-60

## Schema di Connessione

### ESP32 Pinout

```
                    ESP32 Dev Module
                 ┌─────────────────┐
                 │                 │
           3V3 ──┤ 3V3         GND ├── GND
           GND ──┤ GND         D27 ├── ECHO (Ultrasonico)
                 │                 │
                 │             D26 ├── TRIG (Ultrasonico)
                 │                 │
                 │             D32 ├── DATA (DHT22)
                 │                 │
                 └─────────────────┘
```

### DHT22 - Sensore Temperatura/Umidità

```
DHT22          ESP32
┌───┐
│ 1 │ VCC ──── 3.3V o 5V
│ 2 │ DATA ─── GPIO 32
│ 3 │ NC   ──── (non collegato)
│ 4 │ GND  ──── GND
└───┘

Nota: Alcuni moduli DHT22 hanno già la resistenza pull-up integrata.
Se necessario, collegare una resistenza da 10kΩ tra VCC e DATA.
```

### HC-SR04 - Sensore Ultrasonico

```
HC-SR04        ESP32
┌───────┐
│  VCC  │ ──── 5V
│  TRIG │ ──── GPIO 26
│  ECHO │ ──── GPIO 27 (*)
│  GND  │ ──── GND
└───────┘

(*) IMPORTANTE: ECHO del HC-SR04 emette segnale a 5V.
    L'ESP32 accetta massimo 3.3V sui pin GPIO.

    SOLUZIONE: Usa un partitore di tensione:

    ECHO ─┬─── Resistenza 1kΩ ───┬─── GPIO 27
          │                      │
          └─ Resistenza 2kΩ ─── GND

    Questo riduce 5V a ~3.3V proteggendo l'ESP32.
```

## Schema Completo

```
                                    ESP32
                                ┌────────────┐
                                │            │
DHT22                           │        GND ├──┬── GND (Comune)
┌────┐                          │            │  │
│VCC ├────────────────────────► │ 3.3V   D27 ├──┤
│DATA├────────────────────────► │        D26 ├──┤
│GND ├──┬───────────────────────┤        D32 ├──┤
└────┘  │                       │            │  │
        │                       └────────────┘  │
        │                                       │
        │   HC-SR04                             │
        │   ┌──────┐                            │
        │   │ VCC  ├──── 5V (da USB)            │
        │   │ TRIG ├────────────────────────────┘
        │   │ ECHO ├──── [Partitore tensione] ──┘
        └───┤ GND  │
            └──────┘
```

## Istruzioni di Assemblaggio

### Passo 1: Preparazione Componenti

1. Verifica di avere tutti i componenti della BOM
2. Controlla che l'ESP32 non sia danneggiato
3. Testa i sensori con un multimetro se possibile

### Passo 2: Montaggio su Breadboard (Test)

1. **Inserisci l'ESP32** sulla breadboard (al centro per avere spazio)
2. **Collega le alimentazioni**:
   - Collega la riga positiva (+) della breadboard a 3.3V dell'ESP32
   - Collega la riga negativa (-) della breadboard a GND dell'ESP32

3. **Collega il DHT22**:
   - VCC → 3.3V
   - DATA → GPIO 32
   - GND → GND

4. **Collega l'HC-SR04** (con partitore di tensione):
   - VCC → 5V (da USB o alimentatore esterno)
   - TRIG → GPIO 26
   - ECHO → GPIO 27 **attraverso partitore di tensione**
   - GND → GND comune

### Passo 3: Verifica Collegamenti

Prima di alimentare, verifica con un multimetro:

1. **Test continuità**:
   - Verifica che GND sia comune a tutti i componenti
   - Verifica che non ci siano cortocircuiti tra VCC e GND

2. **Test tensioni**:
   - Alimenta l'ESP32 via USB
   - Misura 3.3V sul pin 3.3V
   - Misura 5V sul pin VIN (se presente)

### Passo 4: Upload Firmware

1. Collega l'ESP32 al PC via USB
2. Carica il firmware usando Arduino IDE o PlatformIO
3. Apri il Serial Monitor a 9600 baud
4. Verifica l'output di inizializzazione

### Passo 5: Test Funzionale

1. **Test DHT22**:
   - Verifica che temperatura e umidità vengano lette
   - Prova a soffiare sul sensore per vedere il cambio di temperatura

2. **Test HC-SR04**:
   - Metti la mano davanti al sensore
   - Verifica che la distanza cambi nel Serial Monitor
   - Range normale: 2cm - 400cm

### Passo 6: Installazione Finale

Una volta testato tutto:

1. **Montaggio permanente** (opzionale):
   - Saldare i collegamenti su una PCB prototipazione
   - Usare connettori JST per collegamenti removibili
   - Inserire tutto in un contenitore IP65 per uso esterno

2. **Posizionamento sensori**:
   - **HC-SR04**: Montare verticalmente sopra la cisterna
   - **DHT22**: Posizionare in area ventilata, lontano dalla luce diretta

## Posizionamento Fisico

### Installazione Cisterna

```
         ┌───────────────────────────────┐
         │  [ESP32 in contenitore IP65]  │
         │         ╔═══════╗             │
         │         ║HC-SR04║             │
         │         ╚═══════╝             │
         │             │                 │
         │             ▼ ultrasuoni      │
         └─────────────┬─────────────────┘
                       │
                       │ Distanza misurata
                       │
         ┌─────────────▼─────────────────┐
         │                               │
         │   ╔═════════════════════╗     │
         │   ║                     ║     │
         │   ║   Superficie acqua  ║     │
         │   ║                     ║     │
         │   ╚═════════════════════╝     │
         │                               │
         └───────────────────────────────┘
              Cisterna / Serbatoio
```

### Raccomandazioni:
- Montare HC-SR04 **perpendicolarmente** alla superficie dell'acqua
- Distanza minima sensore-acqua: **2 cm**
- Distanza massima sensore-acqua: **400 cm**
- Evitare ostacoli nel cono di rilevamento (±15°)
- Proteggere l'elettronica dall'umidità
- Posizionare DHT22 dove vuoi monitorare temperatura ambiente

## Test e Calibrazione

### Test di Funzionamento

1. **Verifica letture sensori**:
   ```
   [SENSORS] Distance: 45 cm
   [SENSORS] Temperature: 23.5 °C, Humidity: 65.2 %
   ```

2. **Verifica connessione WiFi**:
   ```
   [NETWORK] WiFi connected successfully
   [NETWORK] IP Address: 192.168.1.50
   ```

3. **Verifica invio dati**:
   ```
   [NETWORK] Data sent successfully
   ```

### Calibrazione (Opzionale)

Per convertire la distanza in volume/percentuale di riempimento:

1. Misura l'altezza totale della cisterna (H_totale)
2. Misura la distanza sensore-fondo cisterna (D_max)
3. Calcola il livello: `Livello = D_max - Distanza_misurata`
4. Calcola percentuale: `% = (Livello / H_totale) * 100`

Implementa questa logica nel server che riceve i dati.

## Troubleshooting Hardware

### Il sensore ultrasonico dà letture errate

**Possibili cause**:
- Superficie dell'acqua troppo agitata → Attendi che si calmi
- Presenza di schiuma → Può assorbire gli ultrasuoni
- Angolazione non perpendicolare → Raddrizza il sensore
- Ostacoli nel percorso → Libera il campo di rilevamento

**Soluzioni**:
- Usa una media di più letture consecutive
- Monta il sensore su supporto rigido anti-vibrazione
- Aggiungi un tubo guida per gli ultrasuoni

### Il DHT22 non legge o dà NaN

**Possibili cause**:
- Collegamento dati instabile
- Alimentazione insufficiente
- Sensore difettoso

**Soluzioni**:
- Verifica i collegamenti con multimetro
- Aggiungi condensatore 100µF tra VCC e GND vicino al sensore
- Prova con un altro sensore DHT22

### L'ESP32 si riavvia casualmente

**Possibili cause**:
- Alimentazione instabile o insufficiente
- Cortocircuito intermittente
- Watchdog timer che scatta

**Soluzioni**:
- Usa alimentatore da almeno 1A
- Verifica tutti i collegamenti
- Controlla il Serial Monitor per errori prima del riavvio
- Aggiungi condensatore 1000µF sull'alimentazione

## Note di Sicurezza

⚠️ **ATTENZIONE**:
- Non immergere l'elettronica in acqua
- Usa contenitori IP65/IP67 per installazioni esterne
- Non alimentare con tensioni superiori a 5V
- Rispetta la polarità dei componenti
- ECHO dell'HC-SR04 può danneggiare l'ESP32 se collegato direttamente

## Manutenzione

### Controlli Periodici (ogni 3-6 mesi)

- Pulisci il sensore ultrasonico da polvere e ragnatele
- Verifica che i collegamenti non si siano allentati
- Controlla l'assenza di ossidazione sui contatti
- Verifica tenuta del contenitore se installato all'esterno
- Aggiorna il firmware se disponibile

## Foto e Diagrammi

*Sezione da completare con foto reali dell'assemblaggio*

---

Per supporto tecnico o domande, consulta il README.md o apri una issue su GitHub.
