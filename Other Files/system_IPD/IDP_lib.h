#include <Adafruit_MotorShield.h>
#include <Arduino_LSM6DS3.h>
#include <Servo.h>

//setup for leds
int led1pin = 11;
int led2pin = 8;



//magnetic sensor stuff
const byte hallPinLeft = A0; // AH3503 sensor connected to A0 5volt and ground
const byte hallPinRight = A1; // AH3503 sensor connected to A0 5volt and ground
const int offsetL = 500; // calibrate zero
const int offsetR = 490; // calibrate zero
const float span = 0.3617; // calibrate A/D > mT
const float sensitivity = 3.125; // mV/Gauss
float valueL, valueR;
const int magnet_threshold = 30;

//setup for motors
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *main_motor_left  = AFMS.getMotor(1); //motor pin 1
Adafruit_DCMotor *main_motor_right = AFMS.getMotor(2); //motor pin 2

//setup for servos
Servo left_servo;
Servo right_servo;

//setup for interrupt
volatile byte state = LOW;

//create class to control main motors
class MainMotors {
    public:
        int max_speed = 255;
        int min_speed = 0;
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
        float max_wheel_angular_speed = 1.78*Pi;//(5*360)/(5*2*Pi); //max angular speed of wheel, radians per second
        //double wheel_angular_speed_100 = 0;
        float max_wheel_speed = max_wheel_angular_speed*wheel_radius; //max linear speed of wheel mm/s
        int sensor_wheel_dist = 110;//mm
        int sensor_forward_offset = sensor_wheel_dist - wheel_base/2;
        

    //simple interface for using built in functions -- verified
    void set_ML_speed(int l_speed) {
        if (l_speed > max_speed) {
          l_speed = max_speed;
        }
        else if (l_speed < min_speed) {
          l_speed = min_speed;
        }
        main_motor_left->setSpeed(l_speed);
    }
    void set_MR_speed(int r_speed) {
        if (r_speed > max_speed) {
          r_speed = max_speed;
        }
        else if (r_speed < min_speed) {
          r_speed = min_speed;
        }
        main_motor_right->setSpeed(r_speed);
    }

    //joint function to set speed of both motors -- verified -- test for values out of range, i would assume they would be clipped to the max/min values by default
    void set_speed(int speed) {
        if(speed != current_speed || l_speed != r_speed || r_speed != current_speed)  //HOPEFULLY AN OPTIMISATION TRICK
        {
          if (speed > max_speed) {
            speed = max_speed;
          }
          else if (speed < min_speed) {
            speed = min_speed;
          }
          l_speed = r_speed = speed;
          set_ML_speed(l_speed);
          set_MR_speed(r_speed);
          current_speed = speed;
        }
    }

    //function to change speed of both motors -- verified
    void change_speed(int delta) {
        l_speed = r_speed = speed += delta;
        if (speed > max_speed) {
            speed = max_speed;
        } else if (speed < min_speed) {
            speed = min_speed;
        }
        set_speed(speed);
    }
    void change_ML_speed(int delta) {
        l_speed += delta;
        set_ML_speed(l_speed);
    }
    void change_MR_speed(int delta) {
        r_speed += delta;
        set_MR_speed(r_speed);
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
        //both motors forward
        ML_run(FORWARD);
        MR_run(FORWARD);
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
    //test function to allow for faster stops -- unverified
    void hard_stop() {
        //cuts forward power and brakes both motors
        set_speed(0);
        main_motor_left->run(BRAKE);
        main_motor_right->run(BRAKE);
    }
    //simple turn functions -- verified
    void turn_left(int speed) {
        set_speed(speed);
        ML_run(BACKWARD);
        MR_run(FORWARD);
    }
    void turn_right(int speed) {
        set_speed(speed);
        ML_run(FORWARD);
        MR_run(BACKWARD);
    }

    void move_forward(int dist) {
        stop();
        int move_speed = 200;
        set_speed(move_speed);
        unsigned long t = ( 1000.0*dist )/( (move_speed/255.0)*max_wheel_speed );
        go_forward();
        delay(t);
        stop();
    }
    void move_backward(int dist) {
        stop();
        int move_speed = 150;
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

    void turn_90_left_back(bool move = true){
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
    void turn_90_right_back(bool move = true){
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

    void turn_180(){
      stop();
      int turn_speed = 180;
      set_speed(turn_speed);
      unsigned long t = 1000.0*Pi/( 2*(turn_speed/255.0)*max_wheel_angular_speed*(wheel_radius/wheel_base) );
      ML_run(BACKWARD);
      MR_run(FORWARD);
      delay(t);
      stop();
    }



};

MainMotors main_motors; //create main motors object

class Servo_claws {
  private:
  int servo_time = 40;
  int min_angle = 0;
  int max_angle = 82;
  int current_angle = 0;
  int open_angle = 40;
  //sets the servos to their "0" point
  void go_zero(){
      for (int angle = current_angle; angle >= 0; angle -= 1){
        //left has "0" at 270 due to being mirrored
        left_servo.write(max_angle-angle);
        right_servo.write(angle);
        //Serial.println(angle);
        //controlls the speed of the servo rotations
        delay(servo_time);
      }
      current_angle = 0;
  }
  //turns the servos to a target angle relative to their "0"
  void steady_turn(int target_angle){
    if (target_angle<max_angle && target_angle > min_angle){
      for (int angle = 0; angle <= target_angle; angle += 1){
        //left has "0" at 270 due to being mirrored
        left_servo.write(max_angle-angle);
        right_servo.write(angle);
        //Serial.println(angle);
        //controlls the speed of the servo rotations
        delay(servo_time);
      }
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
  void straight_ahead(){
    steady_turn(30);
  }
};
Servo_claws Claws;

class LED_indicator{
  public:
  void blue_on(){
    digitalWrite(led1pin, HIGH);
  }
  void blue_off(){
    digitalWrite(led1pin, LOW);
  }
  void blue_blink(){
    blue_on();
    delay(50);
    blue_off();
  }

  void red_on(){
    digitalWrite(led2pin, HIGH);
  }
  void red_off(){
    digitalWrite(led2pin, LOW);
  }
  void red_blink(){
    red_on();
    delay(50);
    red_off();
  }
  
};
LED_indicator leds;
//set to safe pins
int far_left_sensor = 4;
int left_sensor = 5; 
int right_sensor = 6;
int far_right_sensor = 7;

//initialize the states of the sensors
int fls_state = 0;
int ls_state = 0;
int rs_state = 0;
int frs_state = 0;


int soft_turn_rate = 10;

//function to read data from the sensors
void read_sensors(){
    //0 is black, 1 is white
    fls_state = digitalRead(far_left_sensor);
    ls_state = digitalRead(left_sensor);
    rs_state = digitalRead(right_sensor);
    frs_state = digitalRead(far_right_sensor);
}



int get_line_state(){
    read_sensors();
    if (fls_state == 0 && frs_state == 0){
      if(ls_state == 1 && rs_state == 1){
          return 1; //1 = forward
      } else if (ls_state == 1 && rs_state == 0){
          return 2; //2 = right of line
      } else if (ls_state == 0 && rs_state == 1){
          return 3; //3 = left of line
      } else{
          return 4;//4 = center is off line, correct please
      }
    } else if (fls_state == 1  && frs_state == 0){
        return 5;//at left junction
    } else if (fls_state == 0 && frs_state == 1){
        return 6;//at right junction
    } else{
        return 7; //at T junction
    }
}

void turn_left_until_line(){
  main_motors.move_forward(20);
  main_motors.set_MR_speed(150);
  main_motors.go_forward();
  while (get_line_state() != 5){
    delay(50);
  }
  main_motors.set_MR_speed(100);
  main_motors.go_forward();
  while (get_line_state() != 1){
    delay(50);
  }
  main_motors.stop();
}

void turn_right_until_line(){
  main_motors.move_forward(20);
  main_motors.set_ML_speed(150);
  main_motors.go_forward();
  while (get_line_state() != 6){
    delay(50);
  }
  main_motors.set_ML_speed(100);
  main_motors.go_forward();
  while (get_line_state() != 1){
    delay(50);
  }
  main_motors.stop();
}

String read_magnet_sensor(){
  valueL = analogRead(hallPinLeft) - offsetL;
  valueR = analogRead(hallPinRight) - offsetR;
  Serial.print("Left Value: ");
  Serial.print(valueL, 1);
  Serial.print("    Right Value: ");
  Serial.println(valueR, 1);
  if ( (abs(valueL)+abs(valueR)) >= magnet_threshold ){
    return "magnet";
  }
  return "not magnet";
}

void interrupt_function(){
    volatile static unsigned long last_interrupt = 0;
    unsigned long t = millis();
    if (t - last_interrupt > 500UL){
      state = !state;
    }
    last_interrupt = t;
  }

void setup() {

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
    pinMode(3,INPUT);
    while ( !digitalRead(3) );
    //sets up the servo pins and holds at "0"
    left_servo.attach(9);
    right_servo.attach(10);
    Claws.close();
    delay(1000);
    //sets up serial communication
    Serial.begin(9600);
    Serial.println("a");
    //sets the interrupt pin
    attachInterrupt(digitalPinToInterrupt(3), interrupt_function, RISING);
    //sets up line sensor pins
    pinMode(left_sensor, INPUT);
    pinMode(right_sensor, INPUT);
    pinMode(far_left_sensor,INPUT);
    pinMode(far_right_sensor, INPUT);
    pinMode(led1pin, OUTPUT);
    pinMode(led2pin, OUTPUT);
    leds.blue_on();
    delay(500);
    leds.blue_off();
    delay(500);
    leds.blue_blink();
    leds.red_blink();
    // main_motors.set_speed(255);
    // main_motors.go_forward();
    // delay(5000);
    // main_motors.stop();
    // main_motors.turn_90_left();
    // while(1);
    // Claws.open();
    // Serial.println(read_magnet_sensor());
    // main_motors.move_forward(120);
    // delay(2000);
    // Claws.close();
    // Serial.println(read_magnet_sensor());
    // main_motors.move_backward(120);
    // while(1);


}




