#ifndef CONFIG_H
#define CONFIG_H

// WiFi configuration for the Wokwi simulator.
// For real hardware, replace these with your own network credentials.
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6  // Wokwi-GUEST runs on channel 6; use 0 for a real AP
#define WIFI_RETRY_INTERVAL_MS 10000UL

// Public EMQX test broker (no authentication).
// Do NOT send private or sensitive data through this broker.
#define MQTT_HOST "broker.emqx.io"
#define MQTT_PORT 1883
#define MQTT_RETRY_INTERVAL_MS 5000UL

// Change this to a unique value. Topics on a public broker are visible to
// everyone, so a unique device ID avoids collisions with other projects.
#define DEVICE_ID "esp32-01-7f3a9c"

#define TOPIC_ROOT      "ngapsense"
#define TOPIC_TELEMETRY TOPIC_ROOT "/" DEVICE_ID "/telemetry"
#define TOPIC_STATUS    TOPIC_ROOT "/" DEVICE_ID "/status"
#define TOPIC_ALERT     TOPIC_ROOT "/" DEVICE_ID "/alert"
#define TOPIC_CMD       TOPIC_ROOT "/" DEVICE_ID "/cmd"
#define TOPIC_CMD_ACK   TOPIC_ROOT "/" DEVICE_ID "/cmd/ack"

#define FIRMWARE_VERSION "0.1.0"
#define SENSOR_INTERVAL_MS 1000UL
#define TELEMETRY_INTERVAL_MS 5000UL

#endif
