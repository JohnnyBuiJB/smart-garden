# System Architecture Overview

```mermaid
graph LR
    P1["Phase 1\nAutonomous Watering\nESP32 + ESP-NOW"]
    P2["Phase 2\nROS2 + Simulation\n+ Robot Arm"]
    P3["Phase 3\nFull Autonomy\nIsaac Sim + RL"]

    P1 -- "Add Pi + ROS2\nMQTT bridge\nGazebo sim\nPhysical arm" --> P2
    P2 -- "Add CV\nTask scheduler\nIsaac Lab RL\nSim-to-real" --> P3
```

## Detailed diagrams

- [Phase 1 — Autonomous Watering](phase1.md)
- [Phase 2 — ROS2, Simulation & Robot Arm](phase2.md)
- [Phase 3 — Full Autonomy](phase3.md)

## Technology stack by phase

| Layer | Phase 1 | Phase 2 | Phase 3 |
|---|---|---|---|
| Sensors | ESP32 + ESP-NOW | ESP32 + MQTT | micro-ROS |
| Compute | ESP32 Brain | Raspberry Pi 4/5 | Raspberry Pi 4/5 |
| Middleware | — | ROS2 + DDS | ROS2 + DDS |
| Motion planning | — | MoveIt2 + Gazebo | MoveIt2 + Gazebo |
| ML / Training | — | — | Isaac Sim + Isaac Lab |
| Manipulation | — | Robot arm (teleoperated) | Robot arm (autonomous) |
