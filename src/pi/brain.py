import time
import json
import logging
import paho.mqtt.client as mqtt
from config import (
    MQTT_BROKER, MQTT_PORT,
    NUM_PLANTS, MOISTURE_THRESHOLD,
    WATERING_DURATION_MS, MIN_REWATER_INTERVAL_S,
)

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
log = logging.getLogger(__name__)

last_watered: dict[int, float] = {}


def on_connect(client, userdata, flags, reason_code, properties):
    log.info("Connected to MQTT broker")
    client.subscribe("garden/plants/+/moisture")
    client.subscribe("garden/plants/+/battery")


def on_message(client, userdata, msg):
    parts = msg.topic.split("/")
    if len(parts) != 4:
        return
    _, _, plant_id_str, metric = parts
    try:
        plant_id = int(plant_id_str)
        value = float(msg.payload)
    except (ValueError, TypeError):
        return

    if metric == "battery":
        log.info(f"Plant {plant_id} battery: {value:.2f}V")
    elif metric == "moisture":
        log.info(f"Plant {plant_id} moisture: {value:.1f}%")
        _maybe_irrigate(client, plant_id, value)


def _maybe_irrigate(client, plant_id: int, moisture: float):
    if plant_id >= NUM_PLANTS:
        return
    if moisture >= MOISTURE_THRESHOLD[plant_id]:
        return

    now = time.time()
    if now - last_watered.get(plant_id, 0) < MIN_REWATER_INTERVAL_S:
        log.info(f"Plant {plant_id} dry but skipping (watered {now - last_watered[plant_id]:.0f}s ago)")
        return

    duration = WATERING_DURATION_MS[plant_id]
    client.publish(f"garden/plants/{plant_id}/irrigate", json.dumps({"duration_ms": duration}))
    last_watered[plant_id] = now
    log.info(f"Plant {plant_id} → irrigate for {duration}ms")


def main():
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(MQTT_BROKER, MQTT_PORT)
    client.loop_forever()


if __name__ == "__main__":
    main()
