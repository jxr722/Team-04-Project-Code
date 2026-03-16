#include <Arduino.h>
#include "process_serial.h"
#include "sensor.h"

bool lastOccupied = false;

void setup() {

  Serial.begin(115200);
  delay(2000);

  setupUltrasonic();

  Serial.println("System Ready.");
}

void loop() {

  // Read ultrasonic
  float distance = measureDistance();
  bool occupied = distance < OCCUPIED_DISTANCE_CM;

  // Handle state change
  if (occupied && !lastOccupied) {
    Serial.println("AI START");
  }
  if (!occupied && lastOccupied) {
    Serial.println("AI STOP");
    currentScore = -1;     // reset score
    currentAvgScore = -1;
  }
  lastOccupied = occupied;

  // Read AI serial data (only when occupied)
  if (occupied) {
    readSerial();
  }
  else {
    // Discards all incoming serial bytes without processing them
    while (Serial.available()) {
      Serial.read();
    }
  }

  // Print status
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm, ");
  if (occupied) {
    if (currentScore >= 0) {
      Serial.print("OCCUPIED, Score: ");
      Serial.print(currentScore);
      Serial.print(", Avg Score: ");
      Serial.println(currentAvgScore);
    } else {
      Serial.println("OCCUPIED, Score: --, Avg Score: --");
    }
  } else {
    Serial.println("NOT OCCUPIED");
  }

  delay(100);
}
