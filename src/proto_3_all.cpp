#include <Arduino.h>
#include <ESP32Servo.h>

// PINS: CHANGE BASED ON CIRCUIT WIRING
#define TRIG_PIN 0 //ultrasonic sensor
#define ECHO_PIN 0 //ultrasonic sensor

#define LED_GREEN 0
#define LED_YELLOW 0
#define LED_RED 0

#define SERVO_PIN 0

// CONSTANTS
const float OCCUPIED_DISTANCE_CM = 80.0; //person is 80cm away = occupied CHANGE THIS AFTER TESTING

const unsigned long SENSOR_INTERVAL = 1000; // read from sensor every 1 sec
const unsigned long FORTY_MIN = 40UL * 60UL * 1000UL;
const unsigned long TEN_SEC = 10000UL;
const unsigned long EMPTY_RESET_DELAY = 60000UL;

//arbitrary posture deviation thresholds CHANGE THESE AFTER AI IS DONE
const int HEALTHY_MAX = 30;
const int MODERATE_MAX = 60;

//SERVO MOTOR
Servo tapServo;
int neutralPos = 90;
int tapAngle = 45;

//STATE VARIABLES
bool seatOccupied = false;
bool lastSeatOccupied = false;

//TIMERS
unsigned long lastSensorRead = 0;

unsigned long timerBStart = 0; //Timer B: seat occupancy timer
bool timerBRunning = false;

unsigned long timerAStart = 0; //Timer A: poor posture timer
bool timerARunning = false;

unsigned long healthyStart = 0; //timer to ensure healthy posture for >10s
bool healthyTimerRunning = false;

unsigned long seatEmptyStart = 0; //timer to ensure seat empty long enough to count as a "break"
bool seatEmptyDelayRunning = false;

//HELPER FUNCTIONS

//read ultrasonic sensor distance
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

//set LED color
void setLED(bool g, bool y, bool r) {
  digitalWrite(LED_GREEN, g);
  digitalWrite(LED_YELLOW, y);
  digitalWrite(LED_RED, r);
}

//set # and speed of servo motor taps
void servoTap(int taps, int delayMs) {
  for (int i = 0; i < taps; i++) {
    tapServo.write(neutralPos - tapAngle);
    delay(delayMs);
    tapServo.write(neutralPos);
    delay(delayMs);
  }
}


//[insert function to recieve posture deviation score from AI system]

//SETUP

void setup() {
  Serial.begin(9600);
  
  //[insert setup for AI subsystem also?]

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  tapServo.attach(SERVO_PIN);
  tapServo.write(neutralPos);

  setLED(false, false, false);
}

//MAIN LOOP

void loop() {

  unsigned long now = millis();

  //READ FROM ULTRASONIC EVERY SECOND
  if (now - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = now;

    float distance = readDistanceCM();
    seatOccupied = distance < OCCUPIED_DISTANCE_CM;
  }

  //SEAT OCCUPIED LOGIC
  if (seatOccupied) {

    //start timer B if not running already
    if (!timerBRunning) {
      timerBStart = now;
      timerBRunning = true;
      Serial.println("AI START");
      //[insert code for telling AI system to start running]
    }

    seatEmptyDelayRunning = false;

    //Check if seating duration is 40 minutes
    if (timerBRunning && now - timerBStart >= FORTY_MIN) {
      servoTap(3, 600); //3 slow taps
      timerBStart = now;
    }

    //Get posture score from AI system 
    int score = random(0, 100);  // placeholder

    if (score >= 0) {

      //if healthy posture (green)
      if (score <= HEALTHY_MAX) {

        setLED(true, false, false);

        //start healthy posture timer if it hasn't started already
        if (!healthyTimerRunning) {
          healthyStart = now;
          healthyTimerRunning = true;
        }

        //stop timer A if healthy posture for more than 10 seconds
        if (healthyTimerRunning && now - healthyStart >= TEN_SEC) {
          timerARunning = false;
        }
      }

      // if moderately bad posture (yellow)
      else if (score <= MODERATE_MAX) {

        setLED(false, true, false);
        healthyTimerRunning = false; //stop healthy posture timer

        if (!timerARunning) { //start timerA if it hasn't started already
          timerAStart = now;
          timerARunning = true;
        }
      }

      // if bad posture (red)
      else {

        setLED(false, false, true);
        healthyTimerRunning = false; //stop healthy posture timer

        if (!timerARunning) { //start timerA if it hasn't started already
          timerAStart = now;
          timerARunning = true;
        }

        servoTap(5, 120); // 5 quick taps
      }
    }
  }

  //SEAT NOT OCCUPIED LOGIC
  else {

    //when the seat just got empty 
    if (lastSeatOccupied) {
      seatEmptyStart = now;
      seatEmptyDelayRunning = true;
      Serial.print("AI STOP");
      //[insert code to tell AI system to stop running]
    }

    //Stop Timer A
    timerARunning = false;
    healthyTimerRunning = false;

    setLED(false, false, false);
    tapServo.write(neutralPos);

    //Reset Timer B after 60 seconds of seat empty (break threshold)
    if (seatEmptyDelayRunning && now - seatEmptyStart >= EMPTY_RESET_DELAY) {
      timerBRunning = false;
      seatEmptyDelayRunning = false;
    }
  }

  //DEBUG print timer values/seat occupied status
  Serial.print("Time(ms): ");
  Serial.print(now);
  Serial.print(" | Occupied: ");
  Serial.print(seatOccupied ? "YES" : "NO");

  Serial.print(" | TimerA(ms): ");
  if (timerARunning) Serial.print(now - timerAStart);
  else Serial.print("OFF");

  Serial.print(" | TimerB(ms): ");
  if (timerBRunning) Serial.print(now - timerBStart);
  else Serial.print("OFF");
  
  Serial.println();

  lastSeatOccupied = seatOccupied;
}