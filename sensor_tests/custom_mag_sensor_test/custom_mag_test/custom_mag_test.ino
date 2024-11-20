const byte hallPinLeft = A0; // AH3503 sensor connected to A0 5volt and ground
const byte hallPinRight = A1; // AH3503 sensor connected to A0 5volt and ground
const int offsetL = 481; // calibrate zero
const int offsetR = 485; // calibrate zero
const float span = 0.3617; // calibrate A/D > mT
const float sensitivity = 3.125; // mV/Gauss
float valueL, valueR;

void setup() {
  Serial.begin (9600);
}

void loop() {
  //value = (analogRead(hallPin) - offset) * span;
  valueL = analogRead(hallPinLeft) - offsetL;
  valueR = analogRead(hallPinRight) - offsetR;
  Serial.print("Left Value: ");
  Serial.print(valueL, 1);
  Serial.print("    Right Value: ");
  Serial.println(valueR, 1);
  //Serial.println(" mT");
  delay(500);
}