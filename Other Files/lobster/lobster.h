#include <Adafruit_MotorShield.h>
#include <Arduino_LSM6DS3.h>
#include <Servo.h>
#include "Wire.h"
#include "DFRobot_VL53L0X.h"

// The following class handles all the main motor situation
class MainMotors
{
public:
    int speed = 0;
    int current_speed = 0;
    int l_speed = 0;
    int r_speed = 0;

    float Pi = 3.14159;

    // Vehicle values
    float wheel_radius = 32.5; // mm
    int wheel_base = 170;      // mm
    // max angular speed of wheel, radians per second
    float max_wheel_angular_speed = 1.55 * Pi;
    float max_wheel_speed = max_wheel_angular_speed * wheel_radius; // max linear speed of wheel mm/s

    // Clamp_val makes all values within the acceptable reigon
    int clamp_val(int val)
    {
        if (val >= 255)
        {
            return 255;
        }
        else if (val <= 0)
        {
            return 0;
        }
        return val;
    }

    // simple interface for using built in functions -- verified
    void set_ML_speed(int val)
    {
        l_speed = clamp_val(val);
        main_motor_left->setSpeed(l_speed);
    }
    void set_MR_speed(int val)
    {
        r_speed = clamp_val(val);
        main_motor_right->setSpeed(r_speed);
    }

    void set_speed(int val)
    {
        if (val != current_speed || l_speed != r_speed || r_speed != current_speed)
        {
            current_speed = speed = clamp_val(val);
            set_ML_speed(speed);
            set_MR_speed(speed);
        }
    }

    void change_speed(int delta)
    {
        set_speed(speed + delta);
    }
    void change_ML_speed(int delta)
    {
        set_ML_speed(l_speed + delta);
    }
    void change_MR_speed(int delta)
    {
        set_MR_speed(r_speed + delta);
    }

    // due to the way the motors are wired they may have to be reversed, -- verified for current arrangement, ie forward is forward and turns are correct
    void ML_run(int direction)
    {
        // motor left run
        // swap if direction is reversed
        if (direction == BACKWARD)
        {
            main_motor_left->run(FORWARD);
        }
        else if (direction == FORWARD)
        {
            main_motor_left->run(BACKWARD);
        }
    }

    void MR_run(int direction)
    {
        // motor right run
        // swap if direction is reversed
        if (direction == BACKWARD)
        {
            main_motor_right->run(FORWARD);
        }
        else if (direction == FORWARD)
        {
            main_motor_right->run(BACKWARD);
        }
    }
    // sends both motors forward -- verified
    void go_forward()
    {
        if (state)
        {
            // both motors forward
            ML_run(FORWARD);
            MR_run(FORWARD);
        }
        else
        {
            stop();
        }
    }
    // sends both motors backward -- verified
    void go_backward()
    {
        // both motors backward
        ML_run(BACKWARD);
        MR_run(BACKWARD);
    }
    // cuts power and motors come to a stop -- verified
    void stop()
    {
        // cuts power to both motors
        set_speed(0);
        main_motor_left->run(RELEASE);
        main_motor_right->run(RELEASE);
    }

    void move_forward(int dist)
    {
        stop();
        int move_speed = 255;
        set_speed(move_speed);
        unsigned long t = (1000.0 * dist) / ((move_speed / 255.0) * max_wheel_speed);
        go_forward();
        delay(t);
        stop();
    }

    void move_backward(int dist)
    {
        stop();
        int move_speed = 255;
        set_speed(move_speed);
        unsigned long t = (1000.0 * dist) / ((move_speed / 255.0) * max_wheel_speed);
        go_backward();
        delay(t);
        stop();
    }

    void turn_90_left(bool move = true)
    {
        if (move)
        {
            move_forward(20);
        }
        stop();
        int turn_speed = 240;
        set_MR_speed(turn_speed);
        set_ML_speed(0);
        double factor = (turn_speed / 255.0) * max_wheel_angular_speed * (wheel_radius / wheel_base);
        unsigned long t = 1000.0 * Pi / (2 * factor);
        go_forward();
        delay(t);
        stop();
    }

    void turn_90_right(bool move = true)
    {
        if (move)
        {
            move_forward(20);
        }
        stop();
        int turn_speed = 240;
        set_ML_speed(turn_speed);
        set_MR_speed(0);
        double factor = (turn_speed / 255.0) * max_wheel_angular_speed * (wheel_radius / wheel_base);
        unsigned long t = 1000.0 * Pi / (2 * factor);
        go_forward();
        delay(t);
        stop();
    }
};

class Servo_claws
{
public:
    void open()
    {
        steady_turn(open_angle);
    }
    void close()
    {
        go_zero();
    }
    void wide_open()
    {
        steady_turn(70);
    }

private:
    int servo_time = 20;
    int min_angle = 4;
    int max_angle = 82;
    int current_angle = min_angle;
    int open_angle = 50;
    // sets the servos to their "0" point
    void go_zero()
    {
        for (int angle = current_angle; angle >= min_angle; angle -= 1)
        {
            // left has "0" at 270 due to being mirrored
            left_servo.write(max_angle - angle);
            right_servo.write(angle);
            // controlls the speed of the servo rotations
            delay(servo_time);
        }
        current_angle = min_angle;
    }

    // turns the servos to a target angle relative to their "0"
    void steady_turn(int target_angle)
    {
        if (target_angle < max_angle && target_angle > min_angle && current_angle < target_angle)
        {
            for (int angle = current_angle; angle <= target_angle; angle += 1)
            {
                // left has "0" at 270 due to being mirrored
                left_servo.write(max_angle - angle);
                right_servo.write(angle);
                // controlls the speed of the servo rotations
                delay(servo_time);
            }
            current_angle = target_angle;
        }
    }
};

class LED_indicator
{
public:
    // Constants for LED order
    const int RED = 0;
    const int GREEN = 1;
    const int BLUE = 2;

    led_pins = [ 8, 2, 11 ];
    // Red, Green, Blue

    void on(int colour)
    {
        digitalWrite(led_pins[colour], HIGH);
    }

    void off(int colour)
    {
        digitalWrite(led_pins[colour], LOW);
    }

    void blink_async(int colour)
    {
        if (millis() % 500 < 250)
        {
            led_on(colour);
        }
        else
        {
            led_off(colour);
        }
    }
};

// function to read data from the sensors
void read_line_sensors()
{
    // 0 is black, 1 is white
    fls_state = digitalRead(far_left_sensor);
    ls_state = digitalRead(left_sensor);
    rs_state = digitalRead(right_sensor);
    frs_state = digitalRead(far_right_sensor);
}

void read_magnet_sensor()
{
    float valueL, valueR;
    // const int magnet_threshold = 30;
    valueL = analogRead(A0) - 500;
    valueR = analogRead(A1) - 490;
    float sum = abs(valueL) + abs(valueR);
    if ((sum) >= 30)
    {
        is_magnet = true;
        leds.off(leds.GREEN);
        leds.on(leds.RED);
    }
    is_magnet = false;
    leds.on(leds.GREEN);
    leds.off(leds.RED);
}

int get_line_state()
{
    read_line_sensors();
    if (!fls_state && !frs_state)
    {
        // For states with no junction
        if (ls_state && rs_state)
        {
            return 1;
        } // on line
        else if (ls_state)
        {
            return 2;
        } // to the right
        else if (rs_state)
        {
            return 3;
        } // to the left
        else
        {
            return 4;
        } // off line
    }
    else
    {
        if (fls_state && frs_state)
        {
            return 7;
        } // at T junc
        else if (fls_state)
        {
            return 5;
        } // at left junc
        else if (frs_state)
        {
            return 6;
        } // at right junc
    }
}

void turn_left_until_line()
{
    main_motors.move_backward(10);
    main_motors.set_MR_speed(255);
    main_motors.set_ML_speed(0);
    main_motors.go_forward();
    delay(800);
    while (get_line_state() != 2)
    {
        leds.blue_blink_async();
        if (state)
            ;
        else
        {
            main_motors.stop();
            break;
        }
    }
    delay(80);
}

void turn_right_until_line()
{
    main_motors.move_backward(10);
    main_motors.set_ML_speed(255);
    main_motors.set_MR_speed(0);
    main_motors.go_forward();
    delay(800);
    while (get_line_state() != 3)
    { // 2
        leds.blue_blink_async();
        if (state)
            ;
        else
        {
            main_motors.stop();
            break;
        }
    }
    delay(80);
    // main_motors.set_ML_speed(230);
    // main_motors.set_MR_speed(180);
}

void line_track_forward(int follow_speed = 0)
{
    read_line_sensors();
    leds.blue_blink_async();
    switch (get_line_state())
    {
    case 1:
        // centered
        main_motors.change_speed(15);
        // main_motors.set_speed(follow_speed);
        main_motors.go_forward();
        break;
    case 2:
        // right of line
        main_motors.change_MR_speed(5);
        main_motors.change_ML_speed(-2);
        main_motors.go_forward();
        break;
    case 3:
        // left of line
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

// Less accurate distance than regular move_forward due to turning,
void move_forward_tracking(int dist, int move_speed = 240)
{
    // New function, bailen pls fix if errors exist
    main_motors.stop();
    main_motors.set_speed(move_speed);
    // I think the maths below is incorrect since the speed is nonlinear with power, good simple version
    unsigned long t = (1000.0 * dist) / ((move_speed / 255.0) * main_motors.max_wheel_speed);
    main_motors.go_forward();
    unsigned long move_forward_tracking_start = millis();
    while (millis() < move_forward_tracking_start + t)
    {
        move_forward_tracking(move_speed);
    }
    main_motors.stop();
}

void interrupt_function()
{
    volatile static unsigned long last_interrupt = 0;
    if (millis() - last_interrupt > 500UL)
    {
        state = !state;
    }
    last_interrupt = millis();
}

void IDP_setup()
{
    // LED
    pinMode(leds.led_pins[0], OUTPUT);
    pinMode(leds.led_pins[1], OUTPUT);
    pinMode(leds.led_pins[2], OUTPUT);
    leds.on(leds.BLUE);
    leds.on(leds.RED);

    while (!AFMS.begin())
        ;
    main_motors.stop();

    // waits for button press to start program
    leds.on(leds.RED);
    pinMode(3, INPUT);
    while (!digitalRead(3))
        ;
    leds.off(leds.RED);

    // sets up the servo pins and holds at "0"
    left_servo.attach(9);
    right_servo.attach(10);
    Claws.close();
    delay(300);

    // sets the interrupt pin
    attachInterrupt(digitalPinToInterrupt(3), interrupt_function, RISING);

    // sets up line sensor pins
    pinMode(left_sensor, INPUT);
    pinMode(right_sensor, INPUT);
    pinMode(far_left_sensor, INPUT);
    pinMode(far_right_sensor, INPUT);

    // tof sensor
    Wire.begin();
    tof_sensor.begin(0x50);                                       // Set I2C sub-device address
    tof_sensor.setMode(tof_sensor.eContinuous, tof_sensor.eHigh); // Set to Back-to-back mode and high precision mode
    tof_sensor.start();                                           // Laser rangefinder begins to work

    leds.off(leds.BLUE);
}

#pragma region Hardware_Variables
// Leds are setup in LED_indicator

// time of flight sensor initialisation
DFRobot_VL53L0X tof_sensor;

// Motor variable initialisations
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *main_motor_left = AFMS.getMotor(1);  // motor pin 1
Adafruit_DCMotor *main_motor_right = AFMS.getMotor(2); // motor pin 2

Servo left_servo;
Servo right_servo;

// Making instances of classes made here
MainMotors main_motors;
Servo_claws Claws;
LED_indicator leds;

// Sensor initialisation of line sensors
// set to safe pins - the name is the name of the pins i think
short int far_left_sensor = 4;
short int left_sensor = 5;
short int right_sensor = 6;
short int far_right_sensor = 7;

// initialize the states of the sensors
bool fls_state = 0;
bool ls_state = 0;
bool rs_state = 0;
bool frs_state = 0;

int tof_block_distance = 1000;
int soft_turn_rate = 10;
#pragma endregion Hardware_Variables

#pragma reigon System_Variables

// The back stop button
volatile byte state = LOW;

// This determines weather or not we send the vehicle to the magnetic recycling centre
//(the recycling centre has a chimney)
bool is_magnet = false;

#pragma endreigon System_Variables