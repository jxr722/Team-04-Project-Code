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

void processLine(String line) {
  line.trim();
  if (line.length() == 0) return;

  // Parse CSV
  int idx1 = line.indexOf(',');
  int idx2 = line.indexOf(',', idx1 + 1);
  int idx3 = line.indexOf(',', idx2 + 1);

  if (idx1 < 0 || idx2 < 0 || idx3 < 0) {
    Serial.print("Bad format: ");
    Serial.println(line);
    return;
  }

  int score      = line.substring(0, idx1).toInt();
  int avg_score  = line.substring(idx1 + 1, idx2).toInt();
  int neck_angle = line.substring(idx2 + 1, idx3).toInt();
  int torso_angle = line.substring(idx3 + 1).toInt();

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