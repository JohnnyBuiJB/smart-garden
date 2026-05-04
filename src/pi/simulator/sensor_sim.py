"""Simulates ESP32-C3 sensor nodes publishing moisture and battery over MQTT."""
import os
import sys
import time
import random
import paho.mqtt.client as mqtt

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
from config import MQTT_BROKER, MQTT_PORT, NUM_PLANTS

moisture = [random.uniform(50, 80) for _ in range(NUM_PLANTS)]
battery = [random.uniform(3.8, 4.2) for _ in range(NUM_PLANTS)]


def on_message(client, userdata, msg):
    parts = msg.topic.split("/")
    if len(parts) == 4 and parts[3] == "irrigate":
        plant_id = int(parts[2])
        moisture[plant_id] = random.uniform(65, 90)
        print(f"[sim] Plant {plant_id} irrigated → moisture reset to {moisture[plant_id]:.1f}%")


def main():
    client = mqtt.Client(client_id="sensor_sim")
    client.on_message = on_message
    client.connect(MQTT_BROKER, MQTT_PORT)
    client.subscribe("garden/plants/+/irrigate")
    client.loop_start()

    print(f"[sim] Publishing {NUM_PLANTS} plants every 10s. Ctrl-C to stop.")
    while True:
        for plant_id in range(NUM_PLANTS):
            moisture[plant_id] = max(0.0, moisture[plant_id] - random.uniform(0.5, 2.5))
            battery[plant_id] = max(3.0, battery[plant_id] - random.uniform(0.0, 0.002))
            client.publish(f"garden/plants/{plant_id}/moisture", f"{moisture[plant_id]:.1f}")
            client.publish(f"garden/plants/{plant_id}/battery", f"{battery[plant_id]:.2f}")
        time.sleep(10)


if __name__ == "__main__":
    main()
