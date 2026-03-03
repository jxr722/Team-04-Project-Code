#include <Arduino.h>

// ---------- ULTRASONIC ----------
#define TRIG_PIN 26
#define ECHO_PIN 25
const float OCCUPIED_DISTANCE_CM = 10.0;

bool lastOccupied = false;

// ---------- AI SERIAL ----------
String incomingLine = "";
int currentScore = -1;  // store latest score


// ------------------ ULTRASONIC ------------------
float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 999;

  return duration * 0.034 / 2.0;
}


// ------------------ AI PARSER ------------------
void processLine(String line) {
  line.trim();
  if (line.length() == 0) return;

  int idx1 = line.indexOf(',');
  int idx2 = line.indexOf(',', idx1 + 1);
  int idx3 = line.indexOf(',', idx2 + 1);

  if (idx1 < 0 || idx2 < 0 || idx3 < 0) {
    Serial.print("Bad format: ");
    Serial.println(line);
    return;
  }

  currentScore = line.substring(0, idx1).toInt();
}


// ------------------ SETUP ------------------
void setup() {

  Serial.begin(115200);   // MATCH PYTHON
  delay(2000);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("System Ready.");
}


// ------------------ LOOP ------------------
void loop() {

  // ===== 1️⃣ READ ULTRASONIC =====
  float distance = readDistanceCM();
  bool occupied = distance < OCCUPIED_DISTANCE_CM;

  // ===== 2️⃣ HANDLE STATE CHANGES =====
  if (occupied && !lastOccupied) {
    Serial.println("AI START");
    // Optional: Serial.println("START");  // send to Python if needed
  }

  if (!occupied && lastOccupied) {
    Serial.println("AI STOP");
    currentScore = -1;  // reset score
    // Optional: Serial.println("STOP");
  }

  lastOccupied = occupied;


  // ===== 3️⃣ READ AI SERIAL DATA (ONLY WHEN OCCUPIED) =====
  if (occupied) {
    while (Serial.available()) {
      char c = Serial.read();

      if (c == '\n') {
        if (occupied) {
          processLine(incomingLine);   // ✅ ONLY process when occupied
        }
        incomingLine = "";
      } else {
        incomingLine += c;
      }
    }
  }
  else {
    // Clear buffer if not occupied
    while (Serial.available()) Serial.read();
  }


  // ===== 4️⃣ PRINT STATUS =====
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | ");

  if (occupied) {
    Serial.print("OCCUPIED | Score: ");
    Serial.println(currentScore);
  } else {
    Serial.println("NOT OCCUPIED");
  }

  delay(200);
}