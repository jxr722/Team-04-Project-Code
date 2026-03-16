#include <Arduino.h>
#include "process_serial.h"
#include "sensor.h"

// CONSTANTS
const unsigned long SENSOR_INTERVAL = 1000; // read from sensor every 1 sec
const unsigned long FORTY_MIN = 40UL * 60UL * 1000UL;
const unsigned long TEN_SEC = 10000UL;
const unsigned long EMPTY_RESET_DELAY = 60000UL;

// Posture score thresholds (0-100, higher = better posture)
const int HEALTHY_MIN = 70;
const int MODERATE_MIN = 40;

// STATE VARIABLES
bool seatOccupied = false;
bool lastSeatOccupied = false;

// TIMERS
unsigned long lastSensorRead = 0;

unsigned long timerBStart = 0; // Timer B: seat occupancy timer
bool timerBRunning = false;

unsigned long timerAStart = 0; // Timer A: poor posture timer
bool timerARunning = false;

unsigned long healthyStart = 0; // timer to ensure healthy posture for >10s
bool healthyTimerRunning = false;

unsigned long seatEmptyStart = 0; // timer to ensure seat empty long enough to count as a "break"
bool seatEmptyDelayRunning = false;

// SETUP
void setup() {
  Serial.begin(115200);
  delay(2000);

  setupUltrasonic();
  setupLEDs();
  setupServo();

  setLED(false, false, false);

  Serial.println("System Ready.");
}

// MAIN LOOP
void loop() {

  unsigned long now = millis();

  // READ FROM ULTRASONIC EVERY SECOND
  if (now - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = now;

    float distance = measureDistance();
    seatOccupied = distance < OCCUPIED_DISTANCE_CM;
  }

  // SEAT OCCUPIED LOGIC
  if (seatOccupied) {

    // Start timer B if not running already
    if (!timerBRunning) {
      timerBStart = now;
      timerBRunning = true;
      Serial.println("AI START");
    }

    seatEmptyDelayRunning = false;

    // Check if seating duration is 40 minutes
    if (timerBRunning && now - timerBStart >= FORTY_MIN) {
      servoTap(3, 600); // 3 slow taps
      timerBStart = now;
    }

    // Read AI serial data
    readSerial();
    int score = currentScore;

    if (score >= 0) {

      // If healthy posture (green)
      if (score >= HEALTHY_MIN) {

        setLED(true, false, false);

        // Start healthy posture timer if it hasn't started already
        if (!healthyTimerRunning) {
          healthyStart = now;
          healthyTimerRunning = true;
        }

        // Stop timer A if healthy posture for more than 10 seconds
        if (healthyTimerRunning && now - healthyStart >= TEN_SEC) {
          timerARunning = false;
        }
      }

      // If moderately bad posture (yellow)
      else if (score >= MODERATE_MIN) {

        setLED(false, true, false);
        healthyTimerRunning = false;

        if (!timerARunning) {
          timerAStart = now;
          timerARunning = true;
        }
      }

      // If bad posture (red)
      else {

        setLED(false, false, true);
        healthyTimerRunning = false;

        if (!timerARunning) {
          timerAStart = now;
          timerARunning = true;
        }

        servoTap(5, 120); // 5 quick taps
      }
    }
  }

  // SEAT NOT OCCUPIED LOGIC
  else {

    // When the seat just got empty
    if (lastSeatOccupied) {
      seatEmptyStart = now;
      seatEmptyDelayRunning = true;
      Serial.println("AI STOP");
      currentScore = -1;
      currentAvgScore = -1;
    }

    // Stop Timer A
    timerARunning = false;
    healthyTimerRunning = false;

    setLED(false, false, false);
    tapServo.write(neutralPos);

    // Discard serial buffer while not occupied
    while (Serial.available()) {
      Serial.read();
    }

    // Reset Timer B after 60 seconds of seat empty (break threshold)
    if (seatEmptyDelayRunning && now - seatEmptyStart >= EMPTY_RESET_DELAY) {
      timerBRunning = false;
      seatEmptyDelayRunning = false;
    }
  }

  // DEBUG print
  Serial.print("Time(ms): ");
  Serial.print(now);
  Serial.print(", Occupied: ");
  Serial.print(seatOccupied ? "YES" : "NO");

  Serial.print(", TimerA(ms): ");
  if (timerARunning) Serial.print(now - timerAStart);
  else Serial.print("OFF");

  Serial.print(", TimerB(ms): ");
  if (timerBRunning) Serial.print(now - timerBStart);
  else Serial.print("OFF");

  Serial.print(", Score: ");
  Serial.print(currentScore);

  Serial.println();

  lastSeatOccupied = seatOccupied;
}