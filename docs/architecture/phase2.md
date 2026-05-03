# Phase 2 — ROS2, Simulation & Robot Arm

```mermaid
graph TD
    subgraph Nodes ["Sensor Nodes (micro-ROS on ESP32)"]
        SN0["Node 0"]
        SN1["Node 1"]
        SNX["Node N..."]
    end

    subgraph Brain ["Brain (ESP32-WROOM-32)"]
        ESPNOW["ESP-NOW / MQTT\nBridge"]
    end

    subgraph Pi ["Companion Computer (Raspberry Pi 4/5)"]
        BRIDGE["ROS2 Bridge Node\nESP32 → ROS2 topics"]
        ROS2["ROS2 Core"]
        MOVEIT["MoveIt2\nMotion Planning"]
        BRIDGE --> ROS2
        ROS2 <--> MOVEIT
    end

    subgraph Sim ["Simulation (Gazebo Gz Sim)"]
        GZ["Garden Bed + Arm\nSim Environment"]
        MOVEIT <-- "ROS2 control" --> GZ
    end

    subgraph Arm ["Robot Arm (Hardware)"]
        ARM["Physical Arm\nEthernet / ROS2 Control"]
        MOVEIT <-- "ROS2 control" --> ARM
    end

    SN0 -- "ESP-NOW" --> ESPNOW
    SN1 -- "ESP-NOW" --> ESPNOW
    SNX -- "ESP-NOW" --> ESPNOW
    ESPNOW -- "Serial / MQTT\n(WiFi)" --> BRIDGE

    ROS2 -- "/soil_moisture\n/battery_voltage\n/irrigation_cmd" --> BRIDGE
```

## ROS2 Topics

| Topic | Direction | Description |
|---|---|---|
| `/soil_moisture` | Brain → ROS2 | Per-plant moisture % |
| `/battery_voltage` | Brain → ROS2 | Per-node battery voltage |
| `/irrigation_cmd` | ROS2 → Brain | Override irrigation commands |
| `/arm/joint_states` | Arm → ROS2 | Current joint positions |
| `/arm/goal` | ROS2 → Arm | MoveIt2 motion goal |

## Transport layers

- **ESP-NOW** — sensor nodes to Brain (2.4 GHz, no router)
- **MQTT over WiFi** — Brain to Pi bridge
- **ROS2 / DDS over WiFi** — Pi to operator laptop
- **Ethernet** — Pi to robot arm controller (real-time control)

## Workflow

1. Simulate arm + garden bed in Gazebo first; validate all motion plans with MoveIt2
2. Wire up Brain → MQTT → ROS2 bridge for live sensor data on ROS2 topics
3. Deploy physical arm; replay validated motion plans on hardware
4. Migrate sensor nodes from ESP-NOW to micro-ROS for native ROS2 pub/sub
