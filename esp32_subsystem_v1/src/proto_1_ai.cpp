#include <Arduino.h>

/*
AI SYSTEM TEST
Receives posture data from Python over USB serial.
Expected format:
"score,avg_score,neck_angle,torso_angle\n"
*/

String incomingLine = "";

void setup() {
  Serial.begin(115200);   // MUST match Python baudrate
  delay(2000);            // Allow time for serial monitor

  Serial.println("ESP32 Ready. Waiting for posture data...");
}

void loop() {

  // Check if data is available
  while (Serial.available()) {
    char c = Serial.read();

    // Build the line until newline
    if (c == '\n') {
      processLine(incomingLine);
      incomingLine = "";  // Clear for next message
    } else {
      incomingLine += c;
    }
  }
}

void processLine(String line) {

  int score, avg_score, neck_angle, torso_angle;

  // Parse CSV
  int parsed = sscanf(line.c_str(), "%d,%d,%d,%d",
                      &score,
                      &avg_score,
                      &neck_angle,
                      &torso_angle);

  if (parsed == 4) {
    Serial.println("---- Posture Data Received ----");
    Serial.print("Score: ");
    Serial.println(score);

    Serial.print("Average Score: ");
    Serial.println(avg_score);

    Serial.print("Neck Angle: ");
    Serial.println(neck_angle);

    Serial.print("Torso Angle: ");
    Serial.println(torso_angle);

    Serial.println("------------------------------");
  } else {
    Serial.println("Failed to parse incoming data.");
  }
}