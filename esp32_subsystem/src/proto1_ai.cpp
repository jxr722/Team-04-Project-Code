#include <Arduino.h>
#include "process_serial.h"

/*
AI SYSTEM TEST
Receives posture data from Python over USB serial.
Expected format:
"score,avg_score\n"
*/

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("ESP32 Ready. Waiting for posture data...");
}

void loop() {
  readSerial();

  if (currentScore >= 0) {
    Serial.print("Score: ");
    Serial.print(currentScore);
    Serial.print(", Avg Score: ");
    Serial.println(currentAvgScore);
  }

  delay(100);
}