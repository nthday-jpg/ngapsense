
// --- BEGIN INCLUDED FILE: water_sensor.h ---
#ifndef WATER_SENSOR_H
#define WATER_SENSOR_H

#define TRIG_PIN 5
#define ECHO_PIN 18

// Distance from the sensor (mounted at the top of the tank) to the bottom, in cm.
const float TOTAL_HEIGHT = 100.0;
// Speed of sound in air at ~20 C, in cm/us.
const float SOUND_SPEED_CM_PER_US = 0.0343;
const unsigned long ECHO_TIMEOUT_US = 30000UL;

inline void setupWaterSensor() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
}

// Returns the water level in cm above the bottom of the tank, or -1.0 if the
// sensor did not return an echo (out of range or disconnected).
inline float readWaterLevel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) {
    return -1.0;
  }

  float distance = (duration * SOUND_SPEED_CM_PER_US) / 2.0;
  float level = TOTAL_HEIGHT - distance;

  if (level < 0) return 0;
  if (level > TOTAL_HEIGHT) return TOTAL_HEIGHT;
  return level;
}

#endif
// --- END INCLUDED FILE: water_sensor.h ---

void setup() {
  Serial.begin(115200);
  setupWaterSensor();
}

void loop() {
  float level = readWaterLevel();
  if (level < 0) {
    Serial.println("Water level: sensor error (no echo)");
  } else {
    Serial.print("Water level: ");
    Serial.print(level);
    Serial.println(" cm");
  }
  delay(1000);
}
