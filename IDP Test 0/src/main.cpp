//basic library to test motor controls

#include <Adafruit_MotorShield.h>
#include <Arduino_LSM6DS3.h>

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();

//set which jack the motor is plugged into
//example of setting up 3 motors
Adafruit_DCMotor *main_motor_left  = AFMS.getMotor(1);
Adafruit_DCMotor *main_motor_right = AFMS.getMotor(2);
Adafruit_DCMotor *secondary_motor  = AFMS.getMotor(3);


//create class to control main motors
class MainMotors {
    public:
        int max_speed = 255;
        int min_speed = 0;
        int speed = 0;

        //FORWARD = 1
        //BACKWARD = 2

        //test values
        int wheel_radius = 5; //mm
        int wheel_base = 50; //mm
        int max_wheel_speed = 255; //max angular speed of wheel

    void set_ML_speed(int speed) {
        main_motor_left->setSpeed(speed);
    }
    void set_MR_speed(int speed) {
        main_motor_right->setSpeed(speed);
    }

    void set_speed(int speed) {
        speed = speed;
        set_ML_speed(speed);
        set_MR_speed(speed);
    }

    void change_speed(int delta) {
        speed += delta;
        if (speed > max_speed) {
            speed = max_speed;
        } else if (speed < min_speed) {
            speed = min_speed;
        }
        set_speed(speed);
    }


    void ML_run(int direction) {
        //motor left run
        //swap if direction is reversed
        if (direction == FORWARD) {
            main_motor_left->run(FORWARD);
        } else if (direction == BACKWARD) {
            main_motor_left->run(BACKWARD);
        }
    }

    void MR_run(int direction) {
        //motor right run
        //swap if direction is reversed
        if (direction == FORWARD) {
            main_motor_right->run(FORWARD);
        } else if (direction == BACKWARD) {
            main_motor_right->run(BACKWARD);
        }
    }

    void go_forward() {
        //both motors forward
        ML_run(FORWARD);
        MR_run(FORWARD);
    }

    void go_backward() {
        //both motors backward
        ML_run(BACKWARD);
        MR_run(BACKWARD);
    }

    void stop() {
        //cuts power to both motors
        set_speed(0);
        main_motor_left->run(RELEASE);
        main_motor_right->run(RELEASE);
    }

    void hard_stop() {
        //cuts power and brakes both motors
        set_speed(0);
        main_motor_left->run(BRAKE);
        main_motor_right->run(BRAKE);
    }

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

    void 

    
}


int calc_angular_speed() {
    IMU.readGyroscope();
}


void setup() {
    Serial.begin(9600);
    Serial.println("attempting to start motor shield");
    try {           //try to start the motor shield
        AFMS.begin();
    } catch (int e) {
        Serial.println("failed to start motor shield");
        while (1);          //halts the program if error occurs
    }
    Serial.println("motor shield started");
    MainMotors main_motors;
    main_motors.set_speed(0);
    main_motors.stop();
    IMU.begin();

}

void loop() {
    //test motor class
    main_motors.set_speed(100);
    main_motors.go_forward();
    delay(2000);
    main_motors.hard_stop();
    delay(2000);
    main_motors.set_speed(100);
    main_motors.go_backward();
    delay(2000);
    main_motors.stop();
    delay(2000);
    main_motors.turn_left(100);
    delay(1000);
    main_motors.stop();


}
