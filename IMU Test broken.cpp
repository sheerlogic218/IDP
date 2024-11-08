#include <Arduino.h>
#include <Wire.h>
#include <DFRobot_BMI160.h>

// Wiring Instructions:
// Connect BMI160 VCC to Arduino 5V
// Connect BMI160 GND to Arduino GND
// Connect BMI160 SDA to Arduino SDA (A4)
// Connect BMI160 SCL to Arduino SCL (A5)

DFRobot_BMI160 bmi160;
const int8_t i2c_addr = 0x69;

float posX = 0, posY = 0, posZ = 0;
float velX = 0, velY = 0, velZ = 0;
unsigned long prevTime;

const int buttonPin = 2; // Pin number for the button

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize the button pin
  pinMode(buttonPin, INPUT_PULLUP); // Use internal pull-up resistor
  
  // Initialize the BMI160 sensor
  if (bmi160.softReset() != BMI160_OK) {
    Serial.println("Reset failed");
    while (1);
  }
  
  if (bmi160.I2cInit(i2c_addr) != BMI160_OK) {
    Serial.println("Initialization failed");
    while (1);
  }
  
  prevTime = millis();
}

void loop() {
  // Variables for acceleration
  float ax = 0.0, ay = 0.0, az = 0.0;

  // Check if the button is pressed
  if (digitalRead(buttonPin) == LOW) {
    // Reset velocities and positions
    velX = velY = velZ = 0.0;
    posX = posY = posZ = 0.0;
    ax = ay = az = 0.0;
  } else {
    int16_t accelData[3] = {0};
    int rslt;

    // Read accelerometer data
    rslt = bmi160.getAccelGyroData(accelData);
    if (rslt == 0) {
      // Convert raw data to acceleration in g
      ax = accelData[0] / 16384.0;
      ay = accelData[1] / 16384.0;
      az = accelData[2] / 16384.0;

      // Convert acceleration from g to m/s²
      ax *= 9.80665;
      ay *= 9.80665;
      az *= 9.80665;

      // Print acceleration values
      Serial.print("Acceleration (m/s²): ax=");
      Serial.print(ax);
      Serial.print(" ay=");
      Serial.print(ay);
      Serial.print(" az=");
      Serial.println(az);
    } else {
      Serial.println("Error reading accelerometer data");
    }
  }

  unsigned long currentTime = millis();
  float dt = (currentTime - prevTime) / 1000.0; // Convert milliseconds to seconds
  prevTime = currentTime;

  // Integrate acceleration to get velocity
  velX += ax * dt;
  velY += ay * dt;
  velZ += az * dt;

  // Print velocity values
  Serial.print("Velocity (m/s): vx=");
  Serial.print(velX);
  Serial.print(" vy=");
  Serial.print(velY);
  Serial.print(" vz=");
  Serial.println(velZ);

  // Integrate velocity to get position
  posX += velX * dt;
  posY += velY * dt;
  posZ += velZ * dt;

  // Print position values
  Serial.print("Position (cm): x=");
  Serial.print(posX * 100);
  Serial.print(" y=");
  Serial.print(posY * 100);
  Serial.print(" z=");
  Serial.println(posZ * 100);

  delay(100);
}

