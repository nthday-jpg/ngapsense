#include <string.h>

#include "config.h"
#include "wifi_connection.h"
#include "mqtt_connection.h"
#include "water_sensor.h"
#include "rain_sensor.h"

WiFiClient netClient;
PubSubClient mqttClient(netClient);

unsigned long lastSensorRead = 0;
unsigned long lastTelemetry = 0;

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
    float rainTotal = readRainTotalMm();
    float rainRate = readRainRateMmPerHour(now);

    Serial.print("Rain: ");
    Serial.print(rainTotal, 2);
    Serial.print(" mm (");
    Serial.print(rainRate, 2);
    Serial.println(" mm/h)");

    if (level < 0) {
      Serial.println("Water level: sensor error (no echo)");
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

  delay(10);
}
