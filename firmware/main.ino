#include <string.h>

#include "config.h"
#include "wifi_connection.h"
#include "mqtt_connection.h"
#include "sensor_health.h"
#include "water_sensor.h"
#include "rain_sensor.h"

WiFiClient netClient;
PubSubClient mqttClient(netClient);

unsigned long lastSensorRead = 0;
unsigned long lastTelemetry = 0;
unsigned long lastHealth = 0;

void onMqttMessage(char *topic, uint8_t *payload, unsigned int length) {
  char message[128];
  unsigned int copyLength = length < sizeof(message) - 1
                                ? length
                                : sizeof(message) - 1;
  memcpy(message, payload, copyLength);
  message[copyLength] = '\0';

  Serial.print("MQTT command on ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  // The servo/gate logic is not implemented yet, but acknowledge receipt so
  // the backend knows the command reached the device.
  mqttClient.publish(TOPIC_CMD_ACK, "{\"result\":\"received\"}");
}

void setup() {
  Serial.begin(115200);
  delay(100);

  setupWaterSensor();
  setupRainSensor();
  setupWiFi();
  setupMqtt(mqttClient, onMqttMessage);
}

void loop() {
  updateRainSensor();

  if (ensureWiFi()) {
    ensureMqtt(mqttClient);
    mqttClient.loop();
  }

  unsigned long now = millis();
  if (now - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = now;

    float level = readWaterLevel();
    recordWaterReading(level, now);
    float rainTotal = readRainTotalMm();
    float rainRate = readRainRateMmPerHour(now);

    Serial.print("Rain: ");
    Serial.print(rainTotal, 2);
    Serial.print(" mm (");
    Serial.print(rainRate, 2);
    Serial.println(" mm/h)");

    if (level == WATER_LEVEL_NO_ECHO) {
      Serial.println("Water level: sensor error (no echo)");
    } else if (level == WATER_LEVEL_OUT_OF_RANGE) {
      Serial.println("Water level: sensor error (out of range)");
    } else {
      Serial.print("Water level: ");
      Serial.print(level, 2);
      Serial.println(" cm");
    }

    if (level >= 0 && mqttClient.connected() &&
        now - lastTelemetry >= TELEMETRY_INTERVAL_MS) {
      lastTelemetry = now;
      if (publishTelemetry(mqttClient, level, rainTotal, rainRate, WiFi.RSSI(),
                           now)) {
        Serial.println("Telemetry published");
      } else {
        Serial.println("Telemetry publish failed");
      }
    }
  }

  // Publish a health report on every (re)connect and then at a fixed interval.
  // This keeps running even when the water level sensor stops returning data.
  static bool mqttWasConnected = false;
  bool mqttConnected = mqttClient.connected();

  if (mqttConnected &&
      (!mqttWasConnected || now - lastHealth >= HEALTH_INTERVAL_MS)) {
    lastHealth = now;
    const char *waterStatus = waterHealthStatus(now);
    const char *rainStatus = rainHealthStatus(now);

    if (publishHealth(mqttClient, waterStatus, rainStatus, WiFi.RSSI(), now)) {
      Serial.print("Health report: water=");
      Serial.print(waterStatus);
      Serial.print(", rain=");
      Serial.println(rainStatus);
    } else {
      Serial.println("Health report publish failed");
    }
  }
  mqttWasConnected = mqttConnected;

  delay(10);
}
