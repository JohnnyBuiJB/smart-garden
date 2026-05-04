MQTT_BROKER = "localhost"
MQTT_PORT = 1883

NUM_PLANTS = 10

# Trigger irrigation when moisture drops below this (percent)
MOISTURE_THRESHOLD = [40] * NUM_PLANTS

# How long to open the valve per plant (ms)
WATERING_DURATION_MS = [5000] * NUM_PLANTS

# Don't re-water a plant within this window (seconds)
MIN_REWATER_INTERVAL_S = 3600
