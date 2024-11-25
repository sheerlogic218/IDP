int left_sensor = 6; //set to safe pins
int center_sensor = 7;
int right_sensor = 8;

//initialize the states of the sensors
int ls_state = 0;
int cs_state = 0;
int rs_state = 0;


//function to read data from the sensors
void read_sensors(){
  ls_state = digitalRead(left_sensor);
  cs_state = digitalRead(center_sensor);
  rs_state = digitalRead(right_sensor);
}

void setup() {
  //initialize the sensors
  Serial.begin(9600);
  pinMode(left_sensor, INPUT);
  pinMode(center_sensor, INPUT);
  pinMode(right_sensor, INPUT);
  Serial.println("Initialized");
}

void loop() {
  read_sensors();
  Serial.print("Left Sensor: ");
  Serial.print(ls_state);
  Serial.print(" Center Sensor: ");
  Serial.print(cs_state);
  Serial.print(" Right Sensor: ");
  Serial.println(rs_state);
  delay(100);
}