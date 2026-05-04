
# Product ideation

**Vision:** Personalized robotic greenhouse for apartment balconies — robot arms handle every farming task (seeding, watering, trimming, harvesting, orienting plants toward sunlight). Acts more as a living decorator than a vegetable producer.

*Slogan: "every home deserves a garden"*

---

## Phased Roadmap

### Phase 1 — Autonomous Watering (current)
- Battery-powered ESP32 sensor nodes per plant (ESP-NOW mesh)
- Central ESP32 hub controls irrigation valves and pump
- No mains power, no Wi-Fi router required
- Target: 1 month unattended operation

### Phase 2 — ROS2, Simulation & Robot Arm
- Introduce a companion computer (Raspberry Pi 4/5) running ROS2
- Bridge node: reads Brain data (serial/MQTT) → publishes ROS2 topics (`/soil_moisture`, `/battery_voltage`, `/irrigation_cmd`)
- Begin **Gazebo (Gz Sim)** simulation of the arm and garden bed — validate motion planning with MoveIt2 before touching hardware
- Deploy physical robot arm informed by Gazebo simulation
- Migrate ESP32 nodes to micro-ROS for native ROS2 topic pub/sub
- MoveIt2 handles motion planning for watering, trimming, plant-orientation tasks
- Stack: ROS2 + MoveIt2 + Gz Sim + micro-ROS on ESP32
- *Gazebo chosen for lower barrier: no GPU required, mature MoveIt2 integration*

#### Progress log
- **2026-05-03** — Redesigned architecture: Raspberry Pi becomes the Brain (all decision logic); ESP32s demoted to thin I/O controllers. Sensor nodes → WiFi/MQTT (3–6 month AA battery life accepted). Laptop used as Pi simulator.
- **2026-05-03** — Set up Pi software (`src/pi/`): Mosquitto broker, `brain.py` (MQTT decision logic), `simulator/sensor_sim.py` (fake sensor nodes). Conda env `smart-garden` created.
- **2026-05-04** — Rewrote Brain ESP32 firmware as irrigation controller: removed ESP-NOW/deep-sleep/storage, added WiFi + PubSubClient MQTT subscribe on `garden/plants/+/irrigate`. Deleted `receiver`, `storage`, `power` modules.
- **TODO** — Update sensor node firmware: replace ESP-NOW transmitter with WiFi/MQTT publish
- **TODO** — End-to-end hardware test: sensor node → Pi brain → irrigation controller
- **TODO** — Set up ROS2 Docker environment + MQTT-to-ROS2 bridge node
- **TODO** — Select and procure robot arm (SO-ARM100 / Koch v1.1 / myCobot 280)

### Phase 4 — Full Autonomy
- Computer vision for plant health assessment and sunlight orientation
- Task scheduling: seeding, harvesting, trimming
- Full structured environment with defined plant slots
- Migrate simulation to **NVIDIA Isaac Sim + Isaac Lab** (requires NVIDIA GPU):
  - Photorealistic rendering for synthetic training data (sim-to-real for vision models)
  - Isaac Lab (RL framework) for learning dexterous manipulation: grasping stems, trimming, harvesting
  - Soft-body/deformable physics for realistic plant interaction (PhysX 5)
- *Gazebo remains for control/integration testing; Isaac Sim used for ML training and RL policy development*

---

## Idea Screening
Filtering ideas based on feasibility, viability, and alignment with business goals.

## Concept Development
Turning selected ideas into detailed, concrete concepts.

## Idea Validation
Validating concepts through techniques like "painted door" tests or building MVPs to gauge market demand.

## Final Selection
Prioritizing and selecting the best ideas for development.