#include "water_sensor.h"

void setup() {
  Serial.begin(115200);
  setupWaterSensor();
}

void loop() {
  float level = readWaterLevel();
  Serial.print("Water level: ");
  Serial.print(level);
  Serial.println(" cm");
  delay(1000);
}