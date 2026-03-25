
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

### Phase 2 — ROS2 Integration & Simulation
- Introduce a companion computer (Raspberry Pi 4/5) running ROS2
- Bridge node: reads ESP32 central data (serial/MQTT) → publishes ROS2 topics (`/soil_moisture`, `/battery_voltage`, `/irrigation_cmd`)
- Begin Gazebo (Gz Sim) simulation of the arm and garden bed — no hardware needed yet
- Validate motion planning concepts using MoveIt2 in sim

### Phase 3 — Robot Arm (Hardware)
- Deploy physical robot arm informed by Gazebo simulation
- Migrate ESP32 nodes to micro-ROS for native ROS2 topic pub/sub
- MoveIt2 handles motion planning for watering, trimming, plant-orientation tasks
- Stack: ROS2 + MoveIt2 + Gz Sim + micro-ROS on ESP32

### Phase 4 — Full Autonomy
- Computer vision for plant health assessment and sunlight orientation
- Task scheduling: seeding, harvesting, trimming
- Full structured environment with defined plant slots

---

## Idea Screening
Filtering ideas based on feasibility, viability, and alignment with business goals.

## Concept Development
Turning selected ideas into detailed, concrete concepts.

## Idea Validation
Validating concepts through techniques like "painted door" tests or building MVPs to gauge market demand.

## Final Selection
Prioritizing and selecting the best ideas for development.