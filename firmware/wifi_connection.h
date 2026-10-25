#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <WiFi.h>

#include "config.h"

inline void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Connecting to WiFi");
}

inline bool ensureWiFi() {
  static bool wasConnected = false;
  static unsigned long lastAttempt = millis();

  if (WiFi.status() == WL_CONNECTED) {
    if (!wasConnected) {
      Serial.println();
      Serial.print("WiFi connected, IP: ");
      Serial.println(WiFi.localIP());
      wasConnected = true;
    }
    return true;
  }

  wasConnected = false;

  unsigned long now = millis();
  if (now - lastAttempt >= WIFI_RETRY_INTERVAL_MS) {
    lastAttempt = now;
    Serial.println();
    Serial.print("Reconnecting to WiFi");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  }

  return false;
}

#endif
