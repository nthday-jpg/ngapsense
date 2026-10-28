#ifndef RAIN_SENSOR_H
#define RAIN_SENSOR_H

#include "sensor_health.h"

// Wokwi has no rain gauge part, so a tipping-bucket gauge is emulated with a
// pushbutton: one press = one bucket tip. A real tipping bucket closes a reed
// switch once per fixed volume of rain; 0.2794 mm per tip is the usual value
// for a 0.011 inch bucket. The firmware therefore reports rainfall in mm (and
// mm/h), not a wetness percentage.
//
// GPIO 23 is on the right-hand header, next to the ESP32's GND rail feed, and
// supports the internal pull-up (unlike the input-only GPIO 34-39 pins).

#define RAIN_PIN 23

const float RAIN_MM_PER_TIP = 0.2794f;

// Ignore contact bounce from the reed switch (or from the simulated button).
const unsigned long RAIN_DEBOUNCE_MS = 50UL;

// Rainfall rate is measured over this trailing window.
const unsigned long RAIN_RATE_WINDOW_MS = 60000UL;

// Number of recent tip timestamps kept for the rate calculation. 64 tips in
// 60 s is about 18 mm/h, well above any demo value.
const int RAIN_HISTORY_SIZE = 64;

// Accumulated rainfall since boot, in mm.
float rainTotalMm = 0.0f;
unsigned long rainTipCount = 0;

unsigned long rainTipTimes[RAIN_HISTORY_SIZE];
int rainTipHead = 0;
int rainTipHistoryCount = 0;

inline void setupRainSensor() {
  pinMode(RAIN_PIN, INPUT_PULLUP);
}

inline void registerRainTip(unsigned long now) {
  rainTotalMm += RAIN_MM_PER_TIP;
  rainTipCount++;
  rainTipTimes[rainTipHead] = now;
  rainTipHead = (rainTipHead + 1) % RAIN_HISTORY_SIZE;
  if (rainTipHistoryCount < RAIN_HISTORY_SIZE) {
    rainTipHistoryCount++;
  }

  Serial.print("Rain tip #");
  Serial.print(rainTipCount);
  Serial.print(" - total ");
  Serial.print(rainTotalMm, 2);
  Serial.println(" mm");
}

// Call on every loop iteration. Counts one tip each time the button goes from
// released (HIGH) to pressed (LOW), after the signal has been stable for
// RAIN_DEBOUNCE_MS.
inline void updateRainSensor() {
  static int lastReading = HIGH;
  static int stableReading = HIGH;
  static unsigned long lastChangeMs = 0;

  int reading = digitalRead(RAIN_PIN);
  unsigned long now = millis();

  if (reading != lastReading) {
    lastReading = reading;
    lastChangeMs = now;
  }

  if (now - lastChangeMs >= RAIN_DEBOUNCE_MS && reading != stableReading) {
    stableReading = reading;
    if (stableReading == LOW) {
      registerRainTip(now);
    }
  }

  recordRainPinState(stableReading, now);
}

// Number of tips recorded inside the rate window.
inline unsigned long rainTipsInWindow(unsigned long now) {
  unsigned long count = 0;
  for (int i = 0; i < rainTipHistoryCount; i++) {
    int index = (rainTipHead - 1 - i + RAIN_HISTORY_SIZE) % RAIN_HISTORY_SIZE;
    if (now - rainTipTimes[index] < RAIN_RATE_WINDOW_MS) {
      count++;
    }
  }
  return count;
}

// Accumulated rainfall since boot, in mm.
inline float readRainTotalMm() {
  return rainTotalMm;
}

// Current rainfall intensity in mm/h, over the trailing rate window.
inline float readRainRateMmPerHour(unsigned long now) {
  unsigned long tips = rainTipsInWindow(now);
  return tips * RAIN_MM_PER_TIP * (3600000.0f / RAIN_RATE_WINDOW_MS);
}

#endif
