int ledPin = 13;                // choose the pin for the LED
int inputPin = 3;               // choose the input pin
int val = 0;                    // variable for reading the pin status
void setup() {
  pinMode(ledPin, OUTPUT);      // declare LED as output
  pinMode(inputPin, INPUT);     // declare pushbutton as input
}
void loop(){
  val = digitalRead(inputPin);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    digitalWrite(ledPin, LOW);  // turn LED OFF
    Serial.write(val);
  } else {
    digitalWrite(ledPin, HIGH); // turn LED ON
    Serial.write(val);  
  }
}