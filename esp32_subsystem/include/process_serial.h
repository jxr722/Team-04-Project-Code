#pragma once
#include <Arduino.h>

// Buffer for incoming serial data
extern String incomingLine;

// Latest parsed values (-1 = no data yet)
extern int currentScore;
extern int currentAvgScore;

// Parse incoming line.
void processLine(String line);

// Read available serial bytes into incoming line.
void readSerial();
