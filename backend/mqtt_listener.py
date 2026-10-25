"""Minimal MQTT listener for local development.

This is a temporary test client, not the full FastAPI backend. It connects to
the same public broker as the Wokwi ESP32 and prints incoming messages, so you
can verify that the device-to-server path works.

Run from the repository root:

    uv run python backend/mqtt_listener.py
"""

import json
import os
from pathlib import Path

import paho.mqtt.client as mqtt
from dotenv import load_dotenv

load_dotenv(Path(__file__).with_name(".env"))

MQTT_HOST = os.getenv("MQTT_HOST", "broker.emqx.io")
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))
MQTT_USERNAME = os.getenv("MQTT_USERNAME") or None
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD") or None
MQTT_TOPIC_ROOT = os.getenv("MQTT_TOPIC_ROOT", "ngapsense")


def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code != 0:
        print(f"MQTT connection failed: {reason_code}")
        return

    print(f"Connected to {MQTT_HOST}:{MQTT_PORT}")
    for suffix in ("telemetry", "status", "alert", "cmd/ack"):
        topic = f"{MQTT_TOPIC_ROOT}/+/{suffix}"
        client.subscribe(topic)
        print(f"Subscribed to {topic}")


def on_message(client, userdata, message):
    payload = message.payload.decode("utf-8", errors="replace")

    try:
        parsed = json.loads(payload)
        payload = json.dumps(parsed, indent=2, ensure_ascii=False)
    except json.JSONDecodeError:
        pass

    print(f"\n[{message.topic}]\n{payload}")


def main():
    client = mqtt.Client(
        mqtt.CallbackAPIVersion.VERSION2,
        client_id=f"ngapsense-backend-{os.getpid()}",
    )

    if MQTT_USERNAME:
        client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)

    if os.getenv("MQTT_TLS", "false").lower() == "true":
        client.tls_set()

    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(MQTT_HOST, MQTT_PORT, keepalive=60)

    try:
        client.loop_forever()
    except KeyboardInterrupt:
        print("\nStopping listener")
    finally:
        client.disconnect()


if __name__ == "__main__":
    main()
