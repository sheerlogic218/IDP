#include <Adafruit_MotorShield.h>
#include <Arduino_LSM6DS3.h>


Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *main_motor_left  = AFMS.getMotor(1); //motor pin 1
Adafruit_DCMotor *main_motor_right = AFMS.getMotor(2); //motor pin 2

//create class to control main motors
class MainMotors {
    public:
        // Adafruit_MotorShield AFMS = Adafruit_MotorShield();
        // Adafruit_DCMotor *main_motor_left  = AFMS.getMotor(1); //motor pin 1
        // Adafruit_DCMotor *main_motor_right = AFMS.getMotor(2); //motor pin 2
        int max_speed = 255;
        int min_speed = 0;
        int speed = 0;
        int l_speed;
        int r_speed;

        //FORWARD = 1
        //BACKWARD = 2

        //test values
        int wheel_radius = 5; //mm
        int wheel_base = 50; //mm
        int max_wheel_speed = 255; //max angular speed of wheel

    //simple interface for using built in functions -- verified
    void set_ML_speed(int speed) {
        main_motor_left->setSpeed(speed);
    }
    void set_MR_speed(int speed) {
        main_motor_right->setSpeed(speed);
    }

    //joint function to set speed of both motors -- verified -- test for values out of range, i would assume they would be clipped to the max/min values by default
    void set_speed(int speed) {
        l_speed, r_speed, speed = speed;
        set_ML_speed(speed);
        set_MR_speed(speed);
    }

    //function to change speed of both motors -- verified
    void change_speed(int delta) {
        l_speed, r_speed, speed += delta;
        if (speed > max_speed) {
            speed = max_speed;
        } else if (speed < min_speed) {
            speed = min_speed;
        }
        set_speed(speed);
    }
    void change_ML_speed(int delta) {
        int l_speed = speed + delta;
        if (l_speed > max_speed) {
            l_speed = max_speed;
        } else if (l_speed < min_speed) {
            l_speed = min_speed;
        }
        set_ML_speed(l_speed);
    }
    void change_MR_speed(int delta) {
        int r_speed = speed + delta;
        if (r_speed > max_speed) {
            r_speed = max_speed;
        } else if (r_speed < min_speed) {
            r_speed = min_speed;
        }
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

};

MainMotors main_motors; //create main motors object

//set to safe pins
int left_sensor = 4; 
int center_sensor = 5;
int right_sensor = 3;
int far_left_sensor = 6;
int far_right_sensor = 7;

//initialize the states of the sensors
int ls_state = 0;
int cs_state = 0;
int rs_state = 0;
int fls_state = 0;
int frs_state = 0;


int soft_turn_rate = 10;

//function to read data from the sensors
void read_sensors(){
    //0 is black, 1 is white
    ls_state = digitalRead(left_sensor);
    cs_state = 0;// digitalRead(center_sensor);
    rs_state = digitalRead(right_sensor);
    fls_state = digitalRead(far_left_sensor);
    frs_state = digitalRead(far_right_sensor);
}


void junction(){
    if (fls_state == 1 && frs_state == 0){
        main_motors.change_MR_speed(20);
        main_motors.go_forward();
    }
    else if (fls_state == 0 && frs_state == 1) {
        main_motors.change_ML_speed(20);
        main_motors.go_forward();
    }
    else if (fls_state == 1 && frs_state == 1) {
        main_motors.change_MR_speed(5);
        main_motors.go_forward();
    }
}


void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    AFMS.begin();
    main_motors.set_speed(0);
    pinMode(left_sensor, INPUT);
    pinMode(center_sensor, INPUT);
    pinMode(right_sensor, INPUT);
    pinMode(far_left_sensor,INPUT);
    pinMode(far_right_sensor, INPUT);

}

void loop(){
    read_sensors();
    //test code for 4 sensor following
    if (ls_state == 1 && rs_state == 1) {
        main_motors.set_speed(255);
        main_motors.go_forward();
    }
    else if (ls_state == 1 && rs_state == 0) {
        main_motors.change_MR_speed(10);
        main_motors.go_forward();
    }
    
    else if (ls_state == 0 && rs_state == 1) {
        main_motors.change_ML_speed(10);
        main_motors.go_forward();
    }
    //something gone wrong
    else if (ls_state == 0 && rs_state == 0) {
        main_motors.set_speed(50);
        main_motors.go_backward();
    }
    //junction logic
    else {
        junction();
    }
    
    
}

 

void loopaaaa() {
    read_sensors();
    //if only center sensor on, go straight
    if (ls_state == 0 && cs_state == 1 && rs_state == 0) {
        main_motors.set_speed(200);
        main_motors.go_forward();
    }
    //if left sensor on, turn left to correct
    else if (ls_state == 1 && cs_state == 0 && rs_state == 0) {
        main_motors.stop();
        delay(100);
        main_motors.turn_left(200);
    }

    //if right sensor on, turn right to correct
    else if (ls_state == 0 && cs_state == 0 && rs_state == 1) {
        main_motors.stop();
        delay(100);
        main_motors.turn_right(200);
    }

    //if left and center sensor on, slightly speed up right motor
    else if (ls_state == 1 && cs_state == 1 && rs_state == 0) {
        main_motors.change_MR_speed(soft_turn_rate);
        main_motors.go_forward();
    }
    
    //if right and center sensor on, slightly speed up left motor
    else if (ls_state == 0 && cs_state == 1 && rs_state == 1) {
        main_motors.change_ML_speed(soft_turn_rate);
        main_motors.go_forward();
    }
}