#include <Adafruit_MotorShield.h>
#include <Arduino_LSM6DS3.h>


Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *main_motor_left  = AFMS.getMotor(1); //motor pin 1
Adafruit_DCMotor *main_motor_right = AFMS.getMotor(2); //motor pin 2

volatile byte state = LOW;

//create class to control main motors
class MainMotors {
    public:
        int max_speed = 255;
        int min_speed = 0;
        int speed = 0;
        int l_speed = 0;
        int r_speed = 0;

        //FORWARD = 1
        //BACKWARD = 2
        float Pi = 3.14159;

        //test values
        float wheel_radius = 32.5; //mm
        int wheel_base = 170; //mm
        float max_wheel_angular_speed = 1.95*Pi;//(5*360)/(5*2*Pi); //max angular speed of wheel, radians per second
        //double wheel_angular_speed_100 = 0;
        float max_wheel_speed = max_wheel_angular_speed*wheel_radius; //max linear speed of wheel mm/s
        int sensor_wheel_dist = 110;//mm
        

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
        if (speed > max_speed) {
          speed = max_speed;
        }
        else if (speed < min_speed) {
          speed = min_speed;
        }
        l_speed = r_speed = speed;
        set_ML_speed(l_speed);
        set_MR_speed(r_speed);
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

//set to safe pins
int far_left_sensor = 4;
int left_sensor = 5; 
//int center_sensor = 5;
int right_sensor = 6;
int far_right_sensor = 7;

//initialize the states of the sensors
int fls_state = 0;
int ls_state = 0;
//int cs_state = 0;
int rs_state = 0;
int frs_state = 0;


int soft_turn_rate = 10;

//function to read data from the sensors
void read_sensors(){
    //0 is black, 1 is white
    fls_state = digitalRead(far_left_sensor);
    ls_state = digitalRead(left_sensor);
    //cs_state = 0;   // digitalRead(center_sensor);
    rs_state = digitalRead(right_sensor);
    frs_state = digitalRead(far_right_sensor);
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
    pinMode(3,INPUT);
    while ( !digitalRead(3) );
    Serial.begin(9600);
    Serial.println("a");
    attachInterrupt(digitalPinToInterrupt(3), interrupt_function, RISING);
    if (AFMS.begin()){
      Serial.println("AFMS connected");
    }
    else{
      Serial.println("AFMS not connected");
      while(1);
    }
    main_motors.stop();
    pinMode(left_sensor, INPUT);
    //pinMode(center_sensor, INPUT);
    pinMode(right_sensor, INPUT);
    pinMode(far_left_sensor,INPUT);
    pinMode(far_right_sensor, INPUT);
    
}




