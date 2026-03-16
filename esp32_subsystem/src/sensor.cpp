#include "sensor.h"

// Ultrasonic configurations
const int TRIG_PIN = A0;   // Sends ultrasonic pulse
const int ECHO_PIN = A1;   // Receives reflected pulse
const float OCCUPIED_DISTANCE_CM = 40.0;

void setupUltrasonic() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

// Measure distance and return distance in centimeters
float measureDistance() {
  // Send a 10 microsecond HIGH pulse to the Trig pin to start measurement
  digitalWrite(TRIG_PIN, LOW);   // Set Trig LOW first
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);  // Set Trig HIGH to send ultrasonic pulse.
  delayMicroseconds(10);         // Keep for 10 microseconds
  digitalWrite(TRIG_PIN, LOW);   // Set Trig back to LOW

  // After sound bounce off the object, return, and detect by the ultrasonic sensor,
  // the ECHO pin will goes low
  // Measure how long the Echo pin stays HIGH (in microseconds)
  // Therefore, this duration corresponds to the time for the sound wave to travel
  long duration = pulseIn(ECHO_PIN, HIGH, 240000);  // Timeout after 240ms (return a 0)

  // If timeout, assume a large distance
  if (duration == 0) {
    return 999.0;
  }

  // Speed of sound = 343 m/s = 0.0343 cm/us
  // Distance = (duration * 0.0343) / 2
  float distance = (duration * 0.0343) / 2.0;

  return distance;
}

// LEDs configurations
const int RED_PIN    = 27;
const int YELLOW_PIN = 12;
const int GREEN_PIN  = 13;

void setupLEDs() {
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
}

void setLED(bool g, bool y, bool r) {
  digitalWrite(GREEN_PIN, g);
  digitalWrite(YELLOW_PIN, y);
  digitalWrite(RED_PIN, r);
}

// Servo configurations
const int SERVO_PIN = 32;
Servo tapServo;
int neutralPos = 90;
int tapAngle = 45;

void setupServo() {
  tapServo.attach(SERVO_PIN);
  tapServo.write(neutralPos);
}

void servoTap(int taps, int delayMs) {
  for (int i = 0; i < taps; i++) {
    tapServo.write(neutralPos - tapAngle);
    delay(delayMs);
    tapServo.write(neutralPos);
    delay(delayMs);
  }
}