#ifndef MQTT_CONNECTION_H
#define MQTT_CONNECTION_H

#include <PubSubClient.h>
#include <stdio.h>

#include "config.h"

inline void setupMqtt(PubSubClient &client,
                      void (*callback)(char *, uint8_t *, unsigned int)) {
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(callback);
}

inline bool ensureMqtt(PubSubClient &client) {
  if (client.connected()) {
    return true;
  }

  static unsigned long lastAttempt = 0;
  unsigned long now = millis();
  if (lastAttempt != 0 && now - lastAttempt < MQTT_RETRY_INTERVAL_MS) {
    return false;
  }
  lastAttempt = now;

  Serial.print("Connecting to MQTT broker...");

  // The last-will message marks the device offline if it disconnects.
  bool connected = client.connect(
      DEVICE_ID,
      nullptr,
      nullptr,
      TOPIC_STATUS,
      1,
      true,
      "{\"online\":false}");

  if (!connected) {
    Serial.print(" failed, rc=");
    Serial.println(client.state());
    return false;
  }

  Serial.println(" connected");
  client.publish(
      TOPIC_STATUS,
      "{\"online\":true,\"fw\":\"" FIRMWARE_VERSION "\"}",
      true);
  client.subscribe(TOPIC_CMD);
  return true;
}

inline bool publishTelemetry(PubSubClient &client, float waterLevel,
                             float rainMm, float rainRateMmH, int rssi,
                             unsigned long uptimeMs) {
  if (!client.connected()) {
    return false;
  }

  char payload[200];
  int written = snprintf(
      payload,
      sizeof(payload),
      "{\"device_id\":\"%s\",\"water_level\":%.2f,\"rain_mm\":%.2f,"
      "\"rain_rate\":%.2f,\"rssi\":%d,\"uptime_ms\":%lu}",
      DEVICE_ID,
      waterLevel,
      rainMm,
      rainRateMmH,
      rssi,
      uptimeMs);

  if (written < 0 || written >= static_cast<int>(sizeof(payload))) {
    Serial.println("Telemetry payload too large");
    return false;
  }

  return client.publish(TOPIC_TELEMETRY, payload);
}

#endif
