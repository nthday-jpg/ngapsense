#ifndef WATER_SENSOR_H
#define WATER_SENSOR_H

#define TRIG_PIN 5
#define ECHO_PIN 18
const float TOTAL_HEIGHT = 100.0;

inline void setupWaterSensor() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

inline float readWaterLevel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 0;

  float distance = duration * 0.0343 / 2.0;
  float level = TOTAL_HEIGHT - distance;
  return (level < 0) ? 0 : level;
}

#endif