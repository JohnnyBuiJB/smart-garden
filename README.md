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
│   LiPo 1000mAh       LiPo 1000mAh          LiPo 1000mAh     │
│        │                  │                     │            │
│        └──────────────────┴──── ESP-NOW ────────┘            │
│                                     │                        │
│                          [Central Controller]                │
│                            ESP32-WROOM-32                    │
│                            10 solenoid valves                │
│                            1 submersible pump                │
│                            LiPo 10 000 mAh                  │
└─────────────────────────────────────────────────────────────┘
```

### Sensor Nodes (×10)

- **MCU:** ESP32-C3 mini
- **Sensor:** Capacitive soil moisture (3.3 V, analog output)
- **Battery:** LiPo 1 000 mAh with TP4056 charging module
- **Enclosure:** IP65, cable gland for sensor probe

**Cycle (every 30 min):**
1. Wake from deep sleep
2. Power-on sensor → settle → read ADC → convert to %
3. Build `SensorPacket` and transmit to central via ESP-NOW (up to 3 retries)
4. Deep sleep for remainder of 30-minute interval

### Central Controller (×1)

- **MCU:** ESP32-WROOM-32
- **Actuators:** 10 normally-closed solenoid valves + 1 submersible pump (2× 8-ch relay modules)
- **Battery:** LiPo 10 000 mAh with BMS + boost converter
- **Enclosure:** IP65

**Cycle (every 30 min):**
1. Wake from deep sleep
2. Open ESP-NOW receive window (60 s) — collect packets from all nodes
3. For each plant with moisture < threshold → open valve → run pump → close valve
4. Skip irrigation if battery voltage < 3.5 V
5. Deep sleep, compensating for active time to stay in sync with nodes

---

## Communication Protocol

Wireless link: **ESP-NOW** (peer-to-peer Wi-Fi, no router needed).

```c
// src/common/espnow_packet.h
struct SensorPacket {
    uint8_t  plant_id;      // 0–9
    float    moisture_pct;  // 0.0–100.0 (calibrated %)
    float    battery_v;     // node LiPo voltage
    uint32_t uptime_s;      // seconds since last cold-boot (diagnostics)
};
```

Nodes broadcast; the central controller is registered as a peer and acknowledges each packet. If no ACK is received the node retries up to `ESPNOW_RETRIES` (default 3) times.

---

## Power Budget

### Sensor Node (1 000 mAh)

| State | Current | Duty | Average |
|-------|---------|------|---------|
| Active (boot + read + TX, ~5 s) | 120 mA | 0.3 % | 0.36 mA |
| Deep sleep | 0.005 mA | 99.7 % | ~0 mA |
| **Total** | | | **~0.36 mA** |

**Estimated lifetime:** 1 000 mAh ÷ 0.36 mA ≈ **115 days** (>1 month ✓)

### Central Controller (10 000 mAh)

| State | Current | Duty | Average |
|-------|---------|------|---------|
| Active (receive window + watering, ~65 s) | 150 mA | 3.6 % | 5.4 mA |
| Deep sleep | 0.01 mA | 96.4 % | ~0 mA |
| **Total** | | | **~5.4 mA** |

**Estimated lifetime:** 10 000 mAh ÷ 5.4 mA ≈ **77 days** (>1 month ✓)

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
| `BATTERY_LOW_VOLTS` | `3.5 V` | Skip irrigation below this voltage |
