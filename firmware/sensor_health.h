#ifndef SENSOR_HEALTH_H
#define SENSOR_HEALTH_H

#include <Arduino.h>

// Sensor health is inferred from evidence rather than from a self-test:
//   1. read errors       - no echo / out-of-range distance
//   2. valid data range  - the distance must be usable by the HC-SR04
//   3. stale data        - no valid reading for WATER_STALE_MS
//
// Reported status values:
//   HEALTHY   recent data passes all checks
//   DEGRADED  a few read errors, or no reading yet
//   ERROR     repeated read errors or no valid data for too long
//
// OFFLINE is not a sensor state: it is reported separately by the MQTT
// last-will message when the device loses its connection.

const unsigned long WATER_STALE_MS = 15000UL;
const unsigned int WATER_MAX_CONSECUTIVE_ERRORS = 3;
const unsigned long RAIN_STUCK_MS = 300000UL;

struct WaterSensorHealth {
  unsigned long lastOkMs;
  unsigned int consecutiveErrors;
  bool hasReading;
};

WaterSensorHealth waterHealth = {0, 0, false};

inline void recordWaterReading(float levelCm, unsigned long now) {
  if (levelCm < 0) {
    waterHealth.consecutiveErrors++;
    return;
  }

  waterHealth.lastOkMs = now;
  waterHealth.consecutiveErrors = 0;
  waterHealth.hasReading = true;
}

inline const char *waterHealthStatus(unsigned long now) {
  if (waterHealth.consecutiveErrors >= WATER_MAX_CONSECUTIVE_ERRORS) {
    return "ERROR";
  }
  if (waterHealth.hasReading && now - waterHealth.lastOkMs > WATER_STALE_MS) {
    return "ERROR";
  }
  if (waterHealth.consecutiveErrors > 0 || !waterHealth.hasReading) {
    return "DEGRADED";
  }
  return "HEALTHY";
}

struct RainSensorHealth {
  int lastPinState;
  unsigned long pinLowSinceMs;
};

RainSensorHealth rainHealth = {HIGH, 0};

inline void recordRainPinState(int reading, unsigned long now) {
  if (reading == LOW) {
    if (rainHealth.lastPinState != LOW) {
      rainHealth.pinLowSinceMs = now;
    }
  } else {
    rainHealth.pinLowSinceMs = 0;
  }
  rainHealth.lastPinState = reading;
}

inline const char *rainHealthStatus(unsigned long now) {
  // A reed switch closes only briefly for each bucket tip. Staying closed for
  // minutes means the contact is stuck.
  if (rainHealth.lastPinState == LOW &&
      now - rainHealth.pinLowSinceMs >= RAIN_STUCK_MS) {
    return "ERROR";
  }
  return "HEALTHY";
}

#endif
