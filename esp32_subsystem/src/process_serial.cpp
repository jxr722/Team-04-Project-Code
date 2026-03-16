#include "process_serial.h"

String incomingLine = "";
int currentScore = -1;
int currentAvgScore = -1;

void processLine(String line) {
  line.trim();
  if (line.length() == 0) return;

  // Parse CSV
  int idx1 = line.indexOf(',');

  if (idx1 < 0) {
    Serial.print("Bad format: ");
    Serial.println(line);
    return;
  }

  currentScore    = line.substring(0, idx1).toInt();
  currentAvgScore = line.substring(idx1 + 1).toInt();
}

void readSerial() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      processLine(incomingLine);
      incomingLine = "";
    } 
    else {
      incomingLine += c;
    }
  }
}
