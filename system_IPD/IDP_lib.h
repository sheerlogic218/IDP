#include <Adafruit_MotorShield.h>
#include <Arduino_LSM6DS3.h>
#include <Servo.h>
#include "Wire.h"
#include "DFRobot_VL53L0X.h"
//15046
//1828
//setup for leds
short int led1pin = 8;   //red
short int led2pin = 2;   //green
short int led3pin = 11;    //blue

DFRobot_VL53L0X tof_sensor;

const byte hallPinLeft = A0; // AH3503 sensor connected to A0 5volt and ground
const byte hallPinRight = A1; // AH3503 sensor connected to A0 5volt and ground
const int offsetL = 500; // calibrate zero
const int offsetR = 490; // calibrate zero
const float span = 0.3617; // calibrate A/D > mT
const float sensitivity = 3.125; // mV/Gauss
float valueL, valueR;
const int magnet_threshold = 30;
bool is_magnet = false; // Magnetic is recyclable is centre with chimmney

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *main_motor_left  = AFMS.getMotor(1); //motor pin 1
Adafruit_DCMotor *main_motor_right = AFMS.getMotor(2); //motor pin 2

Servo left_servo;
Servo right_servo;

volatile byte state = LOW;

//create class to control main motors
class MainMotors {
  public:
    //int max_speed = 255;
    //int min_speed = 0;
    int speed = 0;
    int current_speed = 0;
    int l_speed = 0;
    int r_speed = 0;

    //FORWARD = 1
    //BACKWARD = 2
    float Pi = 3.14159;

    //test values
    float wheel_radius = 32.5; //mm
    int wheel_base = 170; //mm
    //max angular speed of wheel, radians per second
    float max_wheel_angular_speed = 1.55*Pi;
    //(5*360)/(5*2*Pi);
    //double wheel_angular_speed_100 = 0;
    float max_wheel_speed = max_wheel_angular_speed*wheel_radius; //max linear speed of wheel mm/s
    int sensor_wheel_dist = 110;//mm
    

  int clamp_val(int val){
    if (val >= 255){return 255;}
    else if (val <= 0){return 0;}
    return val;
  }

  //simple interface for using built in functions -- verified
  void set_ML_speed(int val) {
    l_speed = clamp_val(val);
    main_motor_left->setSpeed(l_speed);
  }
  void set_MR_speed(int val) {
    r_speed = clamp_val(val);
    main_motor_right->setSpeed(r_speed);
  }

  void set_speed(int val) {
    if (val != current_speed || l_speed != r_speed || r_speed != current_speed) {
      current_speed = speed = clamp_val(val);
      set_ML_speed(speed);
      set_MR_speed(speed);
    }
  }

  void change_speed(int delta){
    set_speed(speed+delta);
  }
  void change_ML_speed(int delta){
    set_ML_speed(l_speed+delta);
  }
  void change_MR_speed(int delta){
    set_MR_speed(r_speed+delta);
  }

  //due to the way the motors are wired they may have to be reversed, -- verified for current arrangement, ie forward is forward and turns are correct
  void ML_run(int direction) {
    //motor left run
    //swap if direction is reversed
    if (direction == BACKWARD) {
      main_motor_left->run(FORWARD);
    } else if (direction == FORWARD) {
      main_motor_left->run(BACKWARD);
    }
  }

  void MR_run(int direction) {
    //motor right run
    //swap if direction is reversed
    if (direction == BACKWARD) {
      main_motor_right->run(FORWARD);
    } else if (direction == FORWARD) {
      main_motor_right->run(BACKWARD);
    }
  }
  // sends both motors forward -- verified
  void go_forward() {
    if(state) {
      //both motors forward
      ML_run(FORWARD);
      MR_run(FORWARD);
    } else {stop();}
  }
  // sends both motors backward -- verified
  void go_backward() {
    //both motors backward
    ML_run(BACKWARD);
    MR_run(BACKWARD);
  }
  //cuts power and motors come to a stop -- verified
  void stop() {
    //cuts power to both motors
    set_speed(0);
    main_motor_left->run(RELEASE);
    main_motor_right->run(RELEASE);
  }
  //simple turn functions -- verified
  // void turn_left(int speed) {
  //   set_speed(speed);
  //   ML_run(BACKWARD);
  //   MR_run(FORWARD);
  // }
  // void turn_right(int speed) {
  //   set_speed(speed);
  //   ML_run(FORWARD);
  //   MR_run(BACKWARD);
  // }

  void move_forward(int dist) {
    stop();
    int move_speed = 230;
    set_speed(move_speed);
    unsigned long t = ( 1000.0*dist )/( (move_speed/255.0)*max_wheel_speed );
    go_forward();
    delay(t);
    stop();
  }

  void move_backward(int dist) {
    stop();
    int move_speed = 200;
    set_speed(move_speed);
    unsigned long t = ( 1000.0*dist )/( (move_speed/255.0)*max_wheel_speed );
    go_backward();
    delay(t);
    stop();
  }

  void turn_90_left(bool move = true) {
    if (move) {
    move_forward(20);
    }
    stop();
    int turn_speed = 200;
    set_MR_speed(turn_speed);
    set_ML_speed(0);
    double factor = (turn_speed/255.0)*max_wheel_angular_speed*(wheel_radius/wheel_base);
    unsigned long t = 1000.0*Pi/(2*factor);
    go_forward();
    delay(t);
    stop();
  }

  void turn_90_right(bool move = true) {
    if (move) {
    move_forward(20);
    }
    stop();
    int turn_speed = 200;
    set_ML_speed(turn_speed);
    set_MR_speed(0);
    double factor = (turn_speed/255.0)*max_wheel_angular_speed*(wheel_radius/wheel_base);
    unsigned long t = 1000.0*Pi/(2*factor);
    go_forward();
    delay(t);
    stop();
  }
};

MainMotors main_motors; //create main motors object

class Servo_claws {
  private:
  int servo_time = 20;
  int min_angle = 4;
  int max_angle = 82;
  int current_angle = min_angle;
  int open_angle = 50;
  //sets the servos to their "0" point
  void go_zero(){
    for (int angle = current_angle; angle >= min_angle; angle -= 1){
      //left has "0" at 270 due to being mirrored
      left_servo.write(max_angle-angle);
      right_servo.write(angle);
      //controlls the speed of the servo rotations
      delay(servo_time);
    }
    // for (int angle = current_angle; angle >= min_angle; angle -=1 ){
    //   left_servo.write(max_angle - angle);
    //   delay(servo_time);
    // }
    // for (int angle = current_angle; angle >= min_angle; angle -=1 ){
    //   right_servo.write(angle);
    //   delay(servo_time);
    // }

    current_angle = min_angle;
  }
  

  //turns the servos to a target angle relative to their "0"
  void steady_turn(int target_angle){
    if (target_angle<max_angle && target_angle > min_angle){
      for (int angle = min_angle; angle <= target_angle; angle += 1){
        //left has "0" at 270 due to being mirrored
        left_servo.write(max_angle-angle);
        right_servo.write(angle);
        //controlls the speed of the servo rotations
        delay(servo_time);
      }
      // for (int angle = min_angle; angle <= target_angle; angle += 1){
      //   right_servo.write(angle);
      //   delay(servo_time);
      // }
      // for (int angle = min_angle; angle <= target_angle; angle += 1){
      //   left_servo.write(max_angle-angle);
      //   delay(servo_time);
      // }
      current_angle = target_angle;
      }
  }
  public:
  //means people cant mess up stuff by accessing turns directly
  void open(){
  steady_turn(open_angle);
  }
  void close(){
  go_zero();
  }
  void wide_open(){
  steady_turn(70);
  }
};

Servo_claws Claws;

class LED_indicator{
  public:
  void red_on(){
    digitalWrite(led1pin, HIGH);
  }
  void red_off(){
    digitalWrite(led1pin, LOW);
  }
  void red_blink(){
    red_on();
    delay(50);
    red_off();
  }
  void red_blink_async(){
    if(millis() % 500 < 250){red_on();}
    else{red_off();}
  }

  void green_on(){
    digitalWrite(led2pin, HIGH);
  }
  void green_off(){
    digitalWrite(led2pin, LOW);
  }
  void green_blink(){
    green_on();
    delay(50);
    green_off();
  }
  void green_blink_async(){
    if(millis() % 500 < 250) {green_on();}
    else {green_off();}
  }

  void blue_on(){
    digitalWrite(led3pin, HIGH);
  }
  void blue_off(){
    digitalWrite(led3pin, LOW);
  }
  void blue_blink(){
    blue_on();
    delay(50);
    blue_off();
  }
  void blue_blink_async(){
    if(millis() % 500 < 250) {blue_on();}
    else {blue_off();}
  }
};
LED_indicator leds;

//set to safe pins
short int far_left_sensor = 4;
short int left_sensor = 5; 
short int right_sensor = 6;
short int far_right_sensor = 7;

//initialize the states of the sensors
bool fls_state = 0;
bool ls_state = 0;
bool rs_state = 0;
bool frs_state = 0;

int tof_block_distance = 1000;
int soft_turn_rate = 10;

//function to read data from the sensors
void read_line_sensors(){
  //0 is black, 1 is white
  fls_state = digitalRead(far_left_sensor);
  ls_state = digitalRead(left_sensor);
  rs_state = digitalRead(right_sensor);
  frs_state = digitalRead(far_right_sensor);
}

bool read_magnet_sensor(){
  valueL = analogRead(hallPinLeft) - offsetL;
  valueR = analogRead(hallPinRight) - offsetR;
  // Serial.print("Left Value: ");
  // Serial.print(valueL, 1);
  // Serial.print("    Right Value: ");
  // Serial.println(valueR, 1);
  if ( (abs(valueL)+abs(valueR)) >= magnet_threshold ){
    is_magnet = true;
    leds.green_off();
    leds.red_on();
    return is_magnet;
  }
  is_magnet = false;
  leds.green_on();
  leds.red_off();
  return is_magnet;
}

int get_line_state(){
  read_line_sensors();
  if (!fls_state && !frs_state){
    if(ls_state && rs_state){return 1;} // on line
    else if (ls_state){return 2;} // to the right
    else if (rs_state){return 3;} // to the left
    else {return 4;} // off line
  }else{
    if (fls_state && frs_state){ return 7;} // at T junc
    else if (fls_state){return 5;} // at left junc
    else if (frs_state){return 6;} // at right junc
    else {return 0;} // should never run but cant hurt to have
  }
}

void turn_left_until_line(){
  main_motors.move_backward(10);
  main_motors.set_MR_speed(255);
  main_motors.set_ML_speed(0);
  main_motors.go_forward();
  Serial.println("starting turn");
  delay(700);
  while (get_line_state() != 2){//3
    leds.blue_blink_async();
    if(state);
    else{
      main_motors.stop();
      break;}
  }
  delay(80);//90
  // main_motors.set_MR_speed(230);
  // main_motors.set_ML_speed(180);

}

void turn_right_until_line(){
  main_motors.move_backward(10);
  main_motors.set_ML_speed(255);
  main_motors.set_MR_speed(0);
  main_motors.go_forward();
  delay(700);
  while (get_line_state() != 3){//2
    leds.blue_blink_async();
    if(state);
    else{
      main_motors.stop();
      break;}
  }
  delay(80);
  // main_motors.set_ML_speed(230);
  // main_motors.set_MR_speed(180);
}

// Function for line tracking forward
// void line_track_forward2(int follow_speed = 220 ) {
//   read_line_sensors();
//   leds.blue_blink_async();
//   // Test code for 4 sensor following
//   if (ls_state == 1 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
//     main_motors.set_speed(follow_speed);
//     main_motors.go_forward();
//   } else if (ls_state == 1 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
//     main_motors.change_MR_speed(15);
//     main_motors.go_forward();
//   } else if (ls_state == 0 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
//     main_motors.change_ML_speed(15);
//     main_motors.go_forward();
//   } else if (ls_state == 0 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
//     // Something went wrong
//     main_motors.move_backward(10);
//     main_motors.stop();
//   }
//   CODE MOVEMENT
//   Block logic moved to system.ino
// }

void line_track_forward(int follow_speed = 240){
  read_line_sensors();
  leds.blue_blink_async();
  //Serial.println(get_line_state());
  switch (get_line_state()){
    case 1:
      //centered
      Serial.println("on line");
      main_motors.change_speed(5);
      //main_motors.set_speed(follow_speed);
      main_motors.go_forward();
      break;
    case 2:
      //right of line
      Serial.println("right of line");
      main_motors.change_MR_speed(5);
      main_motors.change_ML_speed(-2);
      main_motors.go_forward();
      break;
    case 3:
      //left of line
      Serial.println("left of line");
      main_motors.change_ML_speed(5);
      main_motors.change_MR_speed(-2);
      main_motors.go_forward();
      break;
    case 4:
      main_motors.move_backward(5);
      break;
    default: // at junction
      break;
  }
  
}

//Less accurate distance than regular move_forward due to turning, 
void move_forward_tracking(int dist, int move_speed = 240){
  //New function, bailen pls fix if errors exist
  main_motors.stop();
  main_motors.set_speed(move_speed);
  //I think the maths below is incorrect since the speed is nonlinear with power, good simple version
  unsigned long t = ( 1000.0*dist )/( (move_speed/255.0)*main_motors.max_wheel_speed );
  main_motors.go_forward();
  unsigned long move_forward_tracking_start = millis();
  while( millis() < move_forward_tracking_start + t ) {
    line_track_forward(move_speed);
  }
  main_motors.stop();
}

void interrupt_function() {
  volatile static unsigned long last_interrupt = 0;
  if (millis() - last_interrupt > 500UL){
    state = !state;
  }
  last_interrupt = millis();
}

void IDP_setup() {
  //LED
  pinMode(led1pin, OUTPUT);
  pinMode(led2pin, OUTPUT);
  pinMode(led3pin, OUTPUT);
  leds.blue_on();

  //stops motors straight away
  if (AFMS.begin()){
    Serial.println("AFMS connected");
  }
  else{
    Serial.println("AFMS not connected");
    while(1);
  }
  main_motors.stop();

  //waits for button press to start program
  leds.red_on();
  pinMode(3,INPUT);
  while ( !digitalRead(3) );
  leds.red_off();

  //sets up the servo pins and holds at "0"
  left_servo.attach(9);
  right_servo.attach(10);
  Claws.close();
  delay(300);

  //sets up serial communication
  Serial.begin(9600);
  Serial.println("Serial communication started");

  //sets the interrupt pin
  attachInterrupt(digitalPinToInterrupt(3), interrupt_function, RISING);

  //sets up line sensor pins
  pinMode(left_sensor, INPUT);
  pinMode(right_sensor, INPUT);
  pinMode(far_left_sensor,INPUT);
  pinMode(far_right_sensor, INPUT);

  //tof sensor 
  Wire.begin();
  tof_sensor.begin(0x50); //Set I2C sub-device address
  tof_sensor.setMode(tof_sensor.eContinuous,tof_sensor.eHigh); //Set to Back-to-back mode and high precision mode
  tof_sensor.start(); //Laser rangefinder begins to work



  leds.blue_off();
  //Bailen ill change back the LEDS but i assumed the blinking was cool and we want cool, so i made it cool and data informative ish.
}

// CODE MOVEMENT
// Block logic moved to system.ino
