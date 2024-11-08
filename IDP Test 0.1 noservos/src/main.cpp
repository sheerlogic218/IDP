#include <Arduino.h>

// Wiring Instructions:
// Connect an analog sensor (e.g., potentiometer) to analog pin A0

void setup() {
  Serial.begin(9600); // Initialize serial communication at 115200 baud
}

void loop() {
  Serial.println("Hello, world!");
  delay(1000);
}