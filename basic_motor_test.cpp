//basic library to test motor controls

#include <Adafruit_MotorShield.h>

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();

//set which jack the motor is plugged into
//example of setting up 3 motors
Adafruit_DCMotor *main_motor_left  = AFMS.getMotor(1);
Adafruit_DCMotor *main_motor_right = AFMS.getMotor(2);
Adafruit_DCMotor *secondary_motor  = AFMS.getMotor(3);




void setup() {
    Serial.begin(9600);
    Serial.println("attempting to start motor shield");
    try {           //try to start the motor shield
        AFMS.begin();
    } catch (int e) {
        Serial.println("failed to start motor shield");#
        while (1);          //halts the program if error occurs
    }
    Serial.println("motor shield started");

}

//create class to control main motors
class MainMotors {
    public:
        int max_speed = 255;
        int min_speed = 0;
        int speed = 0;

        void set_speed(int speed) {
            if (speed > max_speed) {
                speed = max_speed;
            } else if (speed < min_speed) {
                speed = min_speed;
            }
            main_motor_left.setSpeed(speed);
            main_motor_right.setSpeed(speed);
        }


        void forward(int speed) {
            set_speed(speed);
            main_motor_left.run(FORWARD);
            main_motor_right.run(FORWARD);
        }

        void backward(int speed) {
            set_speed(speed);
            main_motor_left.run(BACKWARD);
            main_motor_right.run(BACKWARD);
        }

        void stop() {
            set_speed(0);
            main_motor_left.run(RELEASE);
            main_motor_right.run(RELEASE);
        }


}
