#include "motor_control.h"

void MainMotors::set_ML_speed(int speed) {
    main_motor_left->setSpeed(speed);
}

void MainMotors::set_MR_speed(int speed) {
    main_motor_right->setSpeed(speed);
}

void MainMotors::set_speed(int speed) {
    this->speed = speed;
    set_ML_speed(speed);
    set_MR_speed(speed);
}

void MainMotors::change_speed(int delta) {
    speed += delta;
    if (speed > max_speed) {
        speed = max_speed;
    }
    if (speed < min_speed) {
        speed = min_speed;
    }
    set_speed(speed);
}