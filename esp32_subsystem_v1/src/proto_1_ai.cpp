#include <Arduino.h>

/*
AI SYSTEM ONLY TEST
Reads posture score every second and prints it.
*/

void setup() {
  Serial.begin(9600);

  // TODO: Initialize AI communication here
  // Example:
  // Serial2.begin(115200);
}

void loop() {

  // ===== REQUEST SCORE FROM AI =====
  // TODO: Replace this with real AI communication

  int score = -1;

  // Example placeholder:
  // if (Serial2.available()) score = Serial2.parseInt();

  // TEMP TEST VALUE:
  score = random(0, 100);

  Serial.print("Received posture score: ");
  Serial.println(score);

  delay(1000);
}