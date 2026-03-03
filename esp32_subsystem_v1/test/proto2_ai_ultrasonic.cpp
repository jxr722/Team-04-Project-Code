#include <Arduino.h>

//ULTRASONIC PIN DEFINITIONS -- CHANGE AFTER WIRING CIRCUIT
#define TRIG_PIN 26 //A0
#define ECHO_PIN 25 //A1

const float OCCUPIED_DISTANCE_CM = 10.0; //person is 80cm away = occupied CHANGE THIS AFTER TESTING

bool lastOccupied = false;

//return ultrasonic sensor distance
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

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // TODO: Initialize AI communication
}

void loop() {

  float distance = readDistanceCM();
  bool occupied = distance < OCCUPIED_DISTANCE_CM;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | ");

  if (occupied) {

    Serial.print("OCCUPIED | ");

    if (!lastOccupied) {
      Serial.println("AI START");
      // TODO: send AI start signal
    }

    //TODO: GET POSTURE DEVIATION SCORE
    int score = random(0, 100);  // placeholder

    Serial.print("Score: ");
    Serial.println(score);
  }

  else {

    Serial.print("NOT OCCUPIED");

    if (lastOccupied) {
      Serial.print(" | AI STOP");
      // TODO: send AI stop signal
    }

    Serial.println();
  }

  lastOccupied = occupied;

  delay(1000);
}