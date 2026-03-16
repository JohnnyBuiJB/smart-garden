# Smart Garden — Phase 1

Automated plant watering system powered entirely by batteries. Up to 10 plants are monitored and watered independently with no mains power required.

## Requirements

| ID | Requirement |
|----|-------------|
| F1 | Water each plant when its soil moisture falls below a configurable threshold |
| NF1 | Battery-powered (no mains connection — safer, portable) |
| NF2 | Reliable for 1 month unattended |

---

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        Garden bed                           │
│                                                             │
│  [Sensor Node 0]  [Sensor Node 1]  …  [Sensor Node 9]      │
│   ESP32-C3           ESP32-C3              ESP32-C3          │
│   Capacitive         Capacitive            Capacitive        │
│   moisture           moisture              moisture          │
│   3× AA + LDO        3× AA + LDO           3× AA + LDO      │
│        │                  │                     │            │
│        └──────────────────┴──── ESP-NOW ────────┘            │
│                                     │                        │
│                          [Central Controller]                │
│                            ESP32-WROOM-32                    │
│                            10 solenoid valves                │
│                            1 submersible pump                │
│                            12× AA (4S3P) + LDOs             │
└─────────────────────────────────────────────────────────────┘
```

### Sensor Nodes (×10)

- **MCU:** ESP32-C3 mini
- **Sensor:** Capacitive soil moisture (3.3 V, analog output)
- **Battery:** 3× AA alkaline in series (4.5 V) → MCP1700 3.3 V LDO
- **Enclosure:** IP65, cable gland for sensor probe

**Cycle (every 30 min):**
1. Wake from deep sleep
2. Power-on sensor → settle → read ADC → convert to %
3. Build `SensorPacket` and transmit to central via ESP-NOW (up to 3 retries)
4. Deep sleep for remainder of 30-minute interval

### Central Controller (×1)

- **MCU:** ESP32-WROOM-32
- **Actuators:** 10 normally-closed 5 V solenoid valves + 1 submersible pump (2× 8-ch relay modules)
- **Battery:** 12× AA alkaline, 4S3P (4 in series × 3 in parallel = 6 V, ~7 500 mAh) → LD1117 3.3 V LDO + 7805 5 V LDO
- **Enclosure:** IP65

**Cycle (every 30 min):**
1. Wake from deep sleep
2. Open ESP-NOW receive window (60 s) — collect packets from all nodes
3. For each plant with moisture < threshold → open valve → run pump → close valve
4. Skip irrigation if battery voltage < 4.4 V
5. Deep sleep, compensating for active time to stay in sync with nodes

---

## Communication Protocol

Wireless link: **ESP-NOW** (peer-to-peer Wi-Fi, no router needed).

```c
// src/common/espnow_packet.h
struct SensorPacket {
    uint8_t  plant_id;      // 0–9
    float    moisture_pct;  // 0.0–100.0 (calibrated %)
    float    battery_v;     // node battery voltage (AA pack)
    uint32_t uptime_s;      // seconds since last cold-boot (diagnostics)
};
```

Nodes broadcast; the central controller is registered as a peer and acknowledges each packet. If no ACK is received the node retries up to `ESPNOW_RETRIES` (default 3) times.

---

## Power Budget

### Sensor Node (3× AA = ~2 500 mAh)

| State | Current | Duty | Average |
|-------|---------|------|---------|
| Active (boot + read + TX, ~5 s) | 120 mA | 0.3 % | 0.36 mA |
| Deep sleep | 0.005 mA | 99.7 % | ~0 mA |
| **Total** | | | **~0.36 mA** |

**Estimated lifetime:** 2 500 mAh ÷ 0.36 mA ≈ **115 days** (>1 month ✓)

### Central Controller (12× AA 4S3P = ~7 500 mAh)

| State | Current | Duty | Average |
|-------|---------|------|---------|
| Active (receive window + watering, ~65 s) | 150 mA | 3.6 % | 5.4 mA |
| Deep sleep | 0.01 mA | 96.4 % | ~0 mA |
| **Total** | | | **~5.4 mA** |

**Estimated lifetime:** 7 500 mAh ÷ 5.4 mA ≈ **57 days** (>1 month ✓)

> 4S3P = 4 cells in series (6 V) × 3 parallel banks. Wire three 4×AA holders in parallel.

---

## Repository Layout

```
smart-garden/
├── platformio.ini              # PlatformIO build environments
└── src/
    ├── common/
    │   └── espnow_packet.h     # Shared packet struct (node ↔ central)
    ├── sensor_node/
    │   ├── main.cpp            # Wake → read → transmit → sleep
    │   ├── config.h            # PLANT_ID, pin map, timing, ADC calibration
    │   ├── sensor.{h,cpp}      # ADC read + percent conversion
    │   ├── transmitter.{h,cpp} # ESP-NOW send with retries
    │   └── power.{h,cpp}       # Battery ADC, deep sleep helper
    └── central/
        ├── main.cpp            # Wake → receive → irrigate → sleep
        ├── config.h            # NUM_PLANTS, valve pins, thresholds, timing
        ├── receiver.{h,cpp}    # ESP-NOW receive window
        ├── irrigation.{h,cpp}  # Valve + pump sequencing
        ├── storage.{h,cpp}     # NVS missed-packet counters
        └── power.{h,cpp}       # Battery ADC, low-battery guard, deep sleep
```

---

## Build & Flash

Prerequisites: [PlatformIO CLI](https://docs.platformio.org/en/latest/core/installation/index.html)

```bash
# Flash a sensor node (set PLANT_ID 0–9 in src/sensor_node/config.h first)
pio run -e sensor_node -t upload

# Flash the central controller
pio run -e central -t upload
```

### First-time setup

1. Flash the central controller and note its MAC address printed on Serial:
   ```
   Serial.println(WiFi.macAddress());
   ```
2. Set `CENTRAL_MAC` in [src/sensor_node/config.h](src/sensor_node/config.h).
3. Set a unique `PLANT_ID` (0–9) per node, then flash each node.
4. Adjust `MOISTURE_THRESHOLD` and `WATERING_DURATION_MS` in
   [src/central/config.h](src/central/config.h) per plant type.

---

## Configuration Reference

### Sensor Node — [src/sensor_node/config.h](src/sensor_node/config.h)

| Constant | Default | Description |
|----------|---------|-------------|
| `PLANT_ID` | `0` | Unique node ID (0–9). **Set before flashing.** |
| `CENTRAL_MAC` | `FF:FF:…` | MAC of central ESP32. **Set before flashing.** |
| `SLEEP_INTERVAL_US` | 30 min | Deep sleep duration |
| `SENSOR_DRY_RAW` | `2800` | ADC count in dry air (calibrate per sensor) |
| `SENSOR_WET_RAW` | `1200` | ADC count submerged in water |
| `ESPNOW_RETRIES` | `3` | TX retry count |

### Central Controller — [src/central/config.h](src/central/config.h)

| Constant | Default | Description |
|----------|---------|-------------|
| `NUM_PLANTS` | `10` | Number of sensor nodes |
| `MOISTURE_THRESHOLD[i]` | `40.0 %` | Water plant `i` below this level |
| `WATERING_DURATION_MS[i]` | `5000 ms` | Pump run time per watering event |
| `RECEIVE_WINDOW_MS` | `60 000 ms` | How long to listen for packets each cycle |
| `BATTERY_LOW_VOLTS` | `4.4 V` | Skip irrigation below this voltage (= 4× 1.1 V AA cutoff) |

---

## Bill of Materials & Where to Buy

### Recommended Retailers

| Retailer | Best for | Ships from |
|----------|----------|------------|
| **AliExpress** | Lowest price, bulk quantities | China (1–4 weeks) |
| **Amazon** | Fast delivery, returns | Local warehouse |
| **Adafruit** | Quality modules, good docs | USA |
| **SparkFun** | Quality modules, good docs | USA |
| **Mouser / Digi-Key** | Industrial-grade parts, datasheets | USA/EU |
| **LCSC** | Cheapest bare components | China |

### Microcontrollers

| Part | Qty | Search term | Recommended source |
|------|-----|-------------|--------------------|
| ESP32-C3 SuperMini | 10 | `ESP32-C3 SuperMini` | AliExpress (~$2 each) |
| ESP32-WROOM-32 DevKit | 1 | `ESP32-DevKitC-32E` | Amazon / Mouser |

> The **ESP32-C3 SuperMini** is the smallest/cheapest C3 board and has a 32.768 kHz crystal pad for accurate RTC.

### Sensors

| Part | Qty | Search term | Recommended source |
|------|-----|-------------|--------------------|
| Capacitive soil moisture v1.2 | 10 | `Capacitive Soil Moisture Sensor v1.2` | AliExpress (~$1 each) |

> Avoid resistive sensors — they corrode in soil within weeks.

### Batteries & Power Regulation

| Part | Qty | Search term | Recommended source |
|------|-----|-------------|--------------------|
| AA alkaline batteries | 42 | — | Supermarket / Amazon bulk pack |
| 3×AA battery holder (with leads) | 10 | `3 AA battery holder with wire` | AliExpress / Amazon (~$0.50 each) |
| 4×AA battery holder (with leads) | 3 | `4 AA battery holder with wire` | AliExpress / Amazon (~$0.50 each) |
| MCP1700-3302E/TO (3.3 V LDO) | 10 | `MCP1700-3302E TO-92` | Mouser / Digi-Key (~$0.40 each) |
| LD1117V33 (3.3 V LDO, TO-220) | 1 | `LD1117V33 TO-220` | Mouser / LCSC (~$0.20) |
| 7805 (5 V regulator, TO-220) | 1 | `7805 5V regulator TO-220` | Mouser / LCSC (~$0.20) |

> 42 AA = 30 for sensor nodes (3× per node) + 12 for central controller (4S3P).
> No charging circuits needed — replace batteries when depleted (~2–4 months).

### Actuators

| Part | Qty | Search term | Recommended source |
|------|-----|-------------|--------------------|
| 5V NC solenoid valve (G1/4" or G1/2") | 10 | `5V solenoid valve normally closed G1/4` | AliExpress (~$3 each) |
| 5V mini submersible pump | 1 | `5V mini submersible water pump DC` | Amazon / AliExpress |
| 8-channel 5V relay module | 2 | `8 channel 5V relay module Arduino` | Amazon / AliExpress |

> Use **5V NC solenoids** — compatible with the 7805-regulated 5 V rail and fail safe (closed) on power loss.
> **Not 12V** — the AA pack only provides 6 V, not enough headroom for a 12 V boost.

### Enclosures & Connectors

| Part | Qty | Search term | Recommended source |
|------|-----|-------------|--------------------|
| IP65 junction box ~100×68×50mm | 11 | `IP65 ABS junction box 100x68` | AliExpress / Amazon |
| PG7 cable glands | 20 | `PG7 cable gland nylon` | AliExpress (~$0.20 each) |
| JST-PH 2mm connector kit | 1 | `JST PH 2mm connector kit` | Amazon / Adafruit |
| Silicone wire 24AWG | 1 roll | `24AWG silicone wire` | Amazon |

### Passives (Mouser / LCSC)

- 100 kΩ resistors × 20 (battery voltage dividers)
- 10 kΩ resistors × 5 (pull-ups for GPIO0 + relay inputs)
- 100 nF decoupling caps × 20

### Approximate Total Cost

| Category | Est. cost |
|----------|-----------|
| 10× sensor nodes (MCU + sensor + AA batteries + LDO) | ~$55 |
| Central controller (MCU + relays + pump + valves + AA batteries + LDOs) | ~$50 |
| Enclosures + connectors + wire | ~$30 |
| **Total** | **~$135** |

AliExpress cuts this by ~30% if you can wait for shipping.
AA batteries from a supermarket bulk pack (~$0.30/cell) keep costs low and replacements easy.
