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

void setup() {
  Serial.begin(115200);
  delay(1000);
  

  //init the hardware bmin160  
  if (bmi160.softReset() != BMI160_OK){
    Serial.println("reset false");
    while(1);
  }
  
  //set and init the bmi160 i2c address
  if (bmi160.I2cInit(i2c_addr) != BMI160_OK){
    Serial.println("init false");
    while(1);
  }
  
  prevTime = millis();
}

void loop() {
  int16_t accelData[3] = {0};
  int rslt;

  // Read accelerometer data
  rslt = bmi160.getAccelGyroData(accelData);
  if (rslt == 0) {
    float ax = accelData[0]; // Convert to g
    float ay = accelData[1];
    float az = accelData[2];
    
    unsigned long currentTime = millis();
    float dt = (currentTime - prevTime) / 1000.0; // Convert to seconds
    prevTime = currentTime;
    
    // Integrate acceleration to get velocity
    velX += ax * dt;
    velY += ay * dt;
    velZ += az * dt;
    
    // Integrate velocity to get position
    posX += velX * dt;
    posY += velY * dt;
    posZ += velZ * dt;
    
    // Convert position to centimeters
    Serial.print("Position (cm): X=");
    Serial.print(velX * 100);
    Serial.print(" Y=");
    Serial.print(velY * 100);
    Serial.print(" Z=");
    Serial.println(velZ * 100);
  } else {
    Serial.println("Error reading accelerometer data");
  }
  
  delay(100);
}

