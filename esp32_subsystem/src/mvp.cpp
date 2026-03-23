#include <Arduino.h>
#include "process_serial.h"
#include "sensor.h"

// Configurations
const unsigned long TIMER_B_THRESHOLD = 1UL * 60UL * 1000UL; // sitting too long
const unsigned long TIMER_A_THRESHOLD = 10UL * 1000UL;       // poor posture too long

// Posture score thresholds (0-100, higher = better posture)
const int HEALTHY_MIN = 55;
const int MODERATE_MIN = 40;

// STATE VARIABLES

// Timer B: how long the user has been sitting
unsigned long timerBStart = 0;
bool timerBRunning = false;
bool timerBTapped = false;

// Timer A: how long the user has had poor posture
unsigned long timerAStart = 0;
bool timerARunning = false;
bool timerATapped = false;

void setup() {
  Serial.begin(115200);
  delay(2000);

  setupLEDs();
  setupServo();

  setLED(false, false, false);

  Serial.println("System Ready.");
  Serial.println("AI START"); // Always active now

  // Start Timer B immediately
  timerBStart = millis();
  timerBRunning = true;
}

void loop() {

  unsigned long now = millis();

  // Timer B: sitting too long, slow taps
  if (timerBRunning && !timerBTapped && now - timerBStart >= TIMER_B_THRESHOLD) {
    servoTap(3, 600);
    timerBTapped = true;
  }

  // Read posture score
  readSerial();
  int score = currentAvgScore;

  if (score >= 0) {

    // Healthy posture (green)
    if (score >= HEALTHY_MIN) {
      setLED(true, false, false);

      // Reset Timer A
      timerARunning = false;
      timerATapped = false;
    }

    // Moderate bad posture (yellow)
    else if (score >= MODERATE_MIN) {
      setLED(false, true, false);

      if (!timerARunning) {
        timerAStart = now;
        timerARunning = true;
        timerATapped = false;
      }
    }

    // Bad posture (red)
    else {
      setLED(false, false, true);

      if (!timerARunning) {
        timerAStart = now;
        timerARunning = true;
        timerATapped = false;
      }
    }

    // Timer A exceeded → posture correction tap
    if (timerARunning && !timerATapped && now - timerAStart >= TIMER_A_THRESHOLD) {
      servoTap(5, 120);
      timerATapped = true;
    }
  }

  // DEBUG
  Serial.print("TimerA: ");
  if (timerARunning) { Serial.print(now - timerAStart); Serial.print("ms"); }
  else Serial.print("OFF");

  Serial.print(", TimerB: ");
  if (timerBRunning) { Serial.print(now - timerBStart); Serial.print("ms"); }
  else Serial.print("OFF");

  Serial.print(", Score: ");
  if (currentScore >= 0) Serial.print(currentScore);
  else Serial.print("--");

  Serial.print(", Avg: ");
  if (currentAvgScore >= 0) Serial.print(currentAvgScore);
  else Serial.print("--");

  Serial.println();

  delay(200);
}