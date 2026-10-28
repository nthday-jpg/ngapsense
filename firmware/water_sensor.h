#ifndef WATER_SENSOR_H
#define WATER_SENSOR_H

#define TRIG_PIN 5
#define ECHO_PIN 18

// Distance from the sensor (mounted at the top of the tank) to the bottom, in cm.
const float TOTAL_HEIGHT = 100.0;
// Speed of sound in air at ~20 C, in cm/us.
const float SOUND_SPEED_CM_PER_US = 0.0343;
const unsigned long ECHO_TIMEOUT_US = 30000UL;

// HC-SR04 usable measuring range. Readings outside it are treated as failures,
// not as valid water levels.
const float MIN_VALID_DISTANCE_CM = 2.0;
const float MAX_VALID_DISTANCE_CM = 400.0;

// Negative return codes so callers can distinguish the failure reason.
const float WATER_LEVEL_NO_ECHO = -1.0f;
const float WATER_LEVEL_OUT_OF_RANGE = -2.0f;

inline void setupWaterSensor() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
}

// Returns the water level in cm above the bottom of the tank, or a negative
// code if the reading failed.
inline float readWaterLevel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) {
    return WATER_LEVEL_NO_ECHO;
  }

  float distance = (duration * SOUND_SPEED_CM_PER_US) / 2.0;
  if (distance < MIN_VALID_DISTANCE_CM || distance > MAX_VALID_DISTANCE_CM) {
    return WATER_LEVEL_OUT_OF_RANGE;
  }

  float level = TOTAL_HEIGHT - distance;

  if (level < 0) return 0;
  if (level > TOTAL_HEIGHT) return TOTAL_HEIGHT;
  return level;
}

#endif