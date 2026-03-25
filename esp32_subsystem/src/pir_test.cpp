#include <Arduino.h>
// PIR sensor pin
const int PIR_PIN = 33;

// Variable to store sensor state
int pirState = LOW;

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);

  Serial.println("PIR Sensor Test Starting...");
  delay(2000); // Give sensor time to stabilize
}

void loop() {
  pirState = digitalRead(PIR_PIN);

  if (pirState == HIGH) {
    Serial.println("Motion detected!");
  } else {
    Serial.println("No motion");
  }

  delay(500); // Adjust for readability
}