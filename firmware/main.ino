#include "water_sensor.h"

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