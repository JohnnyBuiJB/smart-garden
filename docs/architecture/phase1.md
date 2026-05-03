# Phase 1 — Autonomous Watering

```mermaid
graph TD
    subgraph Sensor Nodes ["Sensor Nodes (ESP32-C3, one per plant)"]
        SN0["Node 0\nMoisture + Battery ADC\nESP-NOW TX"]
        SN1["Node 1\nMoisture + Battery ADC\nESP-NOW TX"]
        SNX["Node N...\nMoisture + Battery ADC\nESP-NOW TX"]
    end

    subgraph Brain ["Brain (ESP32-WROOM-32)"]
        RX["ESP-NOW Receiver\n60s window"]
        LOGIC["Irrigation Logic\nThreshold check\nBattery guard"]
        NVS["NVS Storage\nLast watered\nMissed packets"]
        RX --> LOGIC
        LOGIC <--> NVS
    end

    subgraph Irrigation ["Irrigation Hardware"]
        PUMP["Water Pump"]
        V0["Valve 0"]
        V1["Valve 1"]
        VX["Valve N..."]
    end

    SN0 -- "ESP-NOW (2.4 GHz)" --> RX
    SN1 -- "ESP-NOW (2.4 GHz)" --> RX
    SNX -- "ESP-NOW (2.4 GHz)" --> RX

    LOGIC -- "GPIO (active LOW relay)" --> PUMP
    LOGIC -- "GPIO (active LOW relay)" --> V0
    LOGIC -- "GPIO (active LOW relay)" --> V1
    LOGIC -- "GPIO (active LOW relay)" --> VX
```

## Cycle

1. Every 30 minutes, each sensor node wakes from deep sleep
2. Reads soil moisture (ADC) and battery voltage
3. Transmits `SensorPacket` to Brain via ESP-NOW; goes back to sleep
4. Brain wakes on the same 30-minute cycle; opens a 60s receive window
5. After window closes, checks each plant's moisture against threshold
6. Opens valve + runs pump for configured duration if moisture is low
7. Brain goes back to deep sleep

## Key constraints

- No WiFi router required — ESP-NOW is peer-to-peer
- Brain skips irrigation entirely when battery < 4.4 V
- Up to 10 plants supported (Node IDs 0–9)
