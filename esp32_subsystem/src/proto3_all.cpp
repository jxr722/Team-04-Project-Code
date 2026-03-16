#include <Arduino.h>
#include "process_serial.h"
#include "sensor.h"

// Configurations
// User-configurable thresholds
const unsigned long TIMER_B_THRESHOLD = 1UL * 60UL * 1000UL; // sitting too long
const unsigned long TIMER_A_THRESHOLD = 10UL * 1000UL;         // poor posture too long
const unsigned long EMPTY_RESET_DELAY = 5UL * 1000UL;         // seat empty before Timer B resets
// Posture score thresholds (0-100, higher = better posture)
const int HEALTHY_MIN = 55;
const int MODERATE_MIN = 40;


// STATE VARIABLES
bool seatOccupied = false;
bool lastSeatOccupied = false;

// Timer B: how long the user has been sitting
unsigned long timerBStart = 0;
bool timerBRunning = false;
bool timerBTapped = false; // already tapped for this sitting period

// Timer A: how long the user has had poor posture
unsigned long timerAStart = 0;
bool timerARunning = false;
bool timerATapped = false; // already tapped for this poor posture period

// Seat empty delay: wait before resetting Timer B
unsigned long seatEmptyStart = 0;
bool seatEmptyDelayRunning = false;

void setup() {
  Serial.begin(115200);
  delay(2000);

  setupUltrasonic();
  setupLEDs();
  setupServo();

  setLED(false, false, false);

  Serial.println("System Ready.");
}

void loop() {

  unsigned long now = millis();
  float distance = measureDistance();
  seatOccupied = distance < OCCUPIED_DISTANCE_CM;

  // SEAT OCCUPIED
  if (seatOccupied) {

    if (!lastSeatOccupied) {
      Serial.println("AI START");
      seatEmptyDelayRunning = false;
    }

    // Start Timer B if not running
    if (!timerBRunning) {
      timerBStart = now;
      timerBRunning = true;
      timerBTapped = false;
    }

    // Timer B: sitting too long, slow taps
    if (timerBRunning && !timerBTapped && now - timerBStart >= TIMER_B_THRESHOLD) {
      servoTap(3, 600);
      timerBTapped = true;
    }

    // Read posture score
    readSerial();
    int score = currentScore;

    if (score >= 0) {

      // Healthy posture (green)
      if (score >= HEALTHY_MIN) {
        setLED(true, false, false);

        // Reset Timer A: posture is good
        timerARunning = false;
        timerATapped = false;
      }

      // Moderate bad posture (yellow)
      else if (score >= MODERATE_MIN) {
        setLED(false, true, false);

        // Start or continue Timer A
        if (!timerARunning) {
          timerAStart = now;
          timerARunning = true;
          timerATapped = false;
        }
      }

      // Bad posture (red)
      else {
        setLED(false, false, true);

        // Start or continue Timer A
        if (!timerARunning) {
          timerAStart = now;
          timerARunning = true;
          timerATapped = false;
        }
      }

      // Timer A exceeded: poor posture too long, tap.
      if (timerARunning && !timerATapped && now - timerAStart >= TIMER_A_THRESHOLD) {
        servoTap(5, 120);
        timerATapped = true;
      }
    }
  }

  // SEAT NOT OCCUPIED
  else {

    // Seat just became empty
    if (lastSeatOccupied) {
      Serial.println("AI STOP");
      seatEmptyStart = now;
      seatEmptyDelayRunning = true;
      currentScore = -1;
      currentAvgScore = -1;
    }

    // Stop Timer A, turn off LEDs and servo
    timerARunning = false;
    timerATapped = false;
    setLED(false, false, false);
    tapServo.write(neutralPos);

    // Discard serial buffer
    while (Serial.available()) {
      Serial.read();
    }

    // Reset Timer B after seat empty long enough
    if (seatEmptyDelayRunning && now - seatEmptyStart >= EMPTY_RESET_DELAY) {
      timerBRunning = false;
      timerBTapped = false;
      seatEmptyDelayRunning = false;
    }
  }

  // DEBUG
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm, Occupied: ");
  Serial.print(seatOccupied ? "YES" : "NO");

  Serial.print(", TimerA: ");
  if (timerARunning) { Serial.print(now - timerAStart); Serial.print("ms"); }
  else Serial.print("OFF");

  Serial.print(", TimerB: ");
  if (timerBRunning) { Serial.print(now - timerBStart); Serial.print("ms"); }
  else Serial.print("OFF");

  Serial.print(", Score: ");
  if (currentScore >= 0) Serial.print(currentScore);
  else Serial.print("--");

  Serial.println();

  lastSeatOccupied = seatOccupied;

  delay(200);
}