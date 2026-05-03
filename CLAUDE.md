# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Vision

The long-term goal is a **balcony robotic greenhouse** — a compact, apartment-scale system where robot arms handle all farming tasks (seeding, watering, trimming, harvesting, orienting plants toward sunlight). The slogan: *"every home deserves a garden."*

**Phased roadmap:**
- **Phase 1** (current) — Autonomous watering: battery-powered ESP32 sensor nodes + central irrigation hub over ESP-NOW
- **Phase 2** — ROS2, simulation & robot arm: companion computer (Raspberry Pi 4/5) bridges Brain data to ROS2 topics; Gazebo simulation with MoveIt2 validates motion planning before hardware; deploy physical arm; migrate ESP32s to micro-ROS
- **Phase 3** — Full autonomy: computer vision, task scheduling, sunlight orientation; simulation migrates to **NVIDIA Isaac Sim + Isaac Lab** for photorealistic synthetic training data and RL-based dexterous manipulation (requires NVIDIA GPU)

## Build & Flash Commands

```bash
# Build only
pio run -e sensor_node
pio run -e central

# Build and flash (board connected via USB)
pio run -e sensor_node -t upload
pio run -e central -t upload

# Monitor serial output (115200 baud)
pio device monitor

# Clean build artifacts
pio run -t clean
```

## Architecture

Two separate firmware targets share a common packet definition:

```
src/common/espnow_packet.h      # Shared SensorPacket struct
src/sensor_node/                # ESP32-C3 per-plant sensor nodes
src/central/                    # ESP32-WROOM-32 central hub
```

**Sensor node boot cycle** (single shot → deep sleep 30 min):
`main.cpp` → read ADC (`sensor.cpp`) → transmit ESP-NOW (`transmitter.cpp`) → sleep (`power.cpp`)

**Central controller boot cycle** (single shot → deep sleep):
`main.cpp` → init relays (`irrigation.cpp`) → 60 s RX window (`receiver.cpp`) → irrigate if threshold crossed → sleep (`power.cpp`)

Neither firmware has a meaningful `loop()` — everything happens in `setup()` and the MCU deep sleeps before `loop()` runs.

**Persistent state on central:**
- `receiver.cpp` uses `RTC_DATA_ATTR` to buffer packets across sleep cycles
- `storage.cpp` uses NVS `Preferences` for long-term per-plant state (last-watered timestamps, missed-packet counters)

## First-Time Setup Sequence

1. Flash central; read its MAC via serial (`WiFi.macAddress()`)
2. Set `CENTRAL_MAC` in `src/sensor_node/config.h`
3. For each node: set unique `PLANT_ID` (0–9) in `src/sensor_node/config.h`, then flash
4. Tune per-plant `MOISTURE_THRESHOLD` and `WATERING_DURATION_MS` arrays in `src/central/config.h`

## Key Configuration Files

| File | What to edit |
|------|-------------|
| `src/sensor_node/config.h` | `PLANT_ID`, `CENTRAL_MAC`, sleep interval, ADC calibration (`SENSOR_DRY_RAW` / `SENSOR_WET_RAW`) |
| `src/central/config.h` | `MOISTURE_THRESHOLD[10]`, `WATERING_DURATION_MS[10]`, `RECEIVE_WINDOW_MS`, valve GPIO pins, `BATTERY_LOW_VOLTS` |

## Hardware Notes

- Valve and pump relays are **active LOW** — `LOW` opens valve/starts pump
- Central skips irrigation entirely when battery < 4.4 V (safety guard)
- Sensor power is gated by a P-FET (`PIN_SENSOR_POWER`): drive LOW to enable, HIGH to cut power
- `PIN_BATTERY_ADC` on central is GPIO 34 (input-only pin on ESP32 — do not reassign to output)

## PlatformIO Environments

`platformio.ini` uses `build_src_filter` to keep the two targets isolated. The `knolleary/PubSubClient` library is listed as a central dependency but is not yet wired up in code.
