#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

// Ultrasonic
extern const int TRIG_PIN;
extern const int ECHO_PIN;
extern const float OCCUPIED_DISTANCE_CM;
void setupUltrasonic();
float measureDistance();

// LEDs
extern const int RED_PIN;
extern const int YELLOW_PIN;
extern const int GREEN_PIN;
void setupLEDs();
void setLED(bool g, bool y, bool r);

// Servo
extern const int SERVO_PIN;
extern Servo tapServo;
extern int neutralPos;
extern int tapAngle;
void setupServo();
void servoTap(int taps, int delayMs);
