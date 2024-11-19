const byte hallPin = A0; // AH3503 sensor connected to A0 5volt and ground
const int offset = 483; // calibrate zero
const float span = 0.3617; // calibrate A/D > mT
const float sensitivity = 3.125; // mV/Gauss
float value;

void setup() {
  Serial.begin (9600);
}

void loop() {
  //value = (analogRead(hallPin) - offset) * span;
  value = analogRead(hallPin) - offset;
  Serial.print("Value: ");
  Serial.println(value, 1);
  //Serial.println(" mT");
  delay(500);
}