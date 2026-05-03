# Phase 3 — Full Autonomy

```mermaid
graph TD
    subgraph Nodes ["Sensor Nodes (micro-ROS on ESP32)"]
        SN["Nodes 0–N\nmicro-ROS pub/sub"]
    end

    subgraph Pi ["Companion Computer (Raspberry Pi 4/5)"]
        ROS2["ROS2 Core"]
        MOVEIT["MoveIt2\nMotion Planning"]
        SCHED["Task Scheduler\nSeed / Water / Trim / Harvest"]
        CV["Computer Vision\nPlant health, sunlight orientation"]
        ROS2 <--> MOVEIT
        ROS2 <--> SCHED
        ROS2 <--> CV
    end

    subgraph IsaacSim ["Training Workstation (NVIDIA GPU)"]
        ISAAC["Isaac Sim\nPhotorealistic Sim"]
        ISAACLABS["Isaac Lab\nRL Policy Training"]
        ISAAC <--> ISAACLABS
        ISAACLABS -- "Trained policies\n(sim-to-real transfer)" --> MOVEIT
    end

    subgraph Hardware ["Hardware"]
        ARM["Robot Arm\nEthernet / ROS2 Control"]
        CAM["Camera(s)\nRGB-D"]
        GARDEN["Garden Bed\nDefined plant slots"]
        MOVEIT <-- "ROS2 control" --> ARM
        CAM -- "RGB-D frames" --> CV
        ARM -- "interacts with" --> GARDEN
    end

    SN -- "micro-ROS\n(WiFi)" --> ROS2
    CV -- "plant poses\nhealth status" --> SCHED
    SCHED -- "task goals" --> MOVEIT
```

## Capabilities

- **Computer vision** — RGB-D camera identifies plant health, growth stage, and optimal sunlight orientation
- **Task scheduling** — autonomous seeding, watering, trimming, harvesting based on plant state
- **RL-trained manipulation** — Isaac Lab trains dexterous policies (grasping stems, trimming, harvesting) using PhysX 5 soft-body/deformable physics; policies transfer to hardware via sim-to-real
- **Sunlight orientation** — arm repositions plants throughout the day to maximize light exposure

## Simulation split

| Tool | Purpose |
|---|---|
| **Gazebo** | ROS2 control integration testing, motion plan validation |
| **Isaac Sim** | Photorealistic synthetic data for vision model training |
| **Isaac Lab** | RL policy development for dexterous manipulation |

## Hardware requirements

- NVIDIA GPU (RTX series) required for Isaac Sim on the training workstation
- Pi handles real-time ROS2 control; GPU workstation handles offline training only
