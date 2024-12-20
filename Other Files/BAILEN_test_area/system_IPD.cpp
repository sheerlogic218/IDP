// testing_area.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "test.h"
using namespace std;

// Constants for turn directions
const int STRAIGHT_ON = 0;
const int RIGHT = 1;
const int FULL_AROUND = 2;
const int LEFT = 3;
const int SPECIAL_FROM_THE_LEFT = 4;
const int SPECIAL_FROM_THE_RIGHT = 5;
const int STOP = 7;
const int RIGHT_DIP = 8;
const int LEFT_DIP = 9;

// constants for move directions
const int FORWARD_MOVE = 1;
const int FORWARD_UNTIL_END_THEN_START_REVERSE = 2;
const int REVERSE_MOVE = 3;

// Path array defining the robot's route
// int path[24] = {
//     SPECIAL_FROM_THE_LEFT, LEFT,                        // BLOCK
//     RIGHT, STRAIGHT_ON, RIGHT, SPECIAL_FROM_THE_LEFT,   // END SPECIAL AT MIDDLE BLOCK
//     RIGHT, RIGHT, RIGHT, SPECIAL_FROM_THE_LEFT,         // END SPECIAL AT MIDDLE BLOCK
//     LEFT, LEFT, LEFT, SPECIAL_FROM_THE_RIGHT,           // END SPECIAL AT MIDDLE BLOCK
//     RIGHT, LEFT, LEFT, STRAIGHT_ON,                     // BLOCK
//     LEFT, STRAIGHT_ON, LEFT, SPECIAL_FROM_THE_RIGHT,    // END SPECIAL AT MIDDLE BLOCK
//     LEFT,                                               // There's a hidden block on this road
//     STOP                                                // Use comments to explain each part of the route.
// };
int path[1] = {
  LEFT
};
int path_length = sizeof(path) / sizeof(path[0]);


int progress = 0;
int special_mode = -1;
int special_progress = 0;
int direction = 0;
int is_magnet = 0; // Magnetic is recyclable
int move_mode = 1;

// Special paths for different modes
int special_path[5][6] = {
    {STRAIGHT_ON, RIGHT_DIP, RIGHT, LEFT, STRAIGHT_ON},
    {RIGHT, RIGHT_DIP, LEFT, STOP, STOP}, // RLEFT goes forward after
    {LEFT_DIP, RIGHT, LEFT, STRAIGHT_ON, STOP},
    {STRAIGHT_ON, LEFT, RIGHT_DIP, LEFT, STOP},
    {LEFT, LEFT_DIP, STOP, STOP, STOP},
};


int get_turn_direction() {
    if (special_mode == -1 && progress <= path_length) {
        int turn_direction = path[progress];
        //leds.blue_on();
        progress++;
    } else {
        direction = special_path[special_mode][special_progress];
        //leds.red_on();
        special_progress++;
        if (direction == STOP) {
            special_mode = -1;
            special_progress = 0;
        }
    }
    return direction;
}


// Junction function to handle the robot's movements at junctions
void turn_junction(int turn_direction) {
    cout << "Junction turn_direction: ";
    cout << turn_direction << endl;
    switch (turn_direction) {
    case STRAIGHT_ON:
        // Code to go straight
        //leds.blue_blink();
        main_motors.move_forward(30);
        break;
    case RIGHT:
        //leds.red_blink();
        // Code to turn right
        main_motors.turn_90_right();
        break;
    case LEFT:
        //leds.blue_blink();
        //leds.red_blink();
        // Code to turn left
        main_motors.turn_90_left();
        break;
    case RIGHT_DIP:
        //leds.red_blink();
        //leds.red_blink();
        // Code to turn right
        main_motors.turn_90_right();
        move_mode = FORWARD_UNTIL_END_THEN_START_REVERSE;
        break;
    case LEFT_DIP:
        //leds.blue_blink();
        //leds.red_blink();
        // Code to turn left
        main_motors.turn_90_left();
        move_mode = FORWARD_UNTIL_END_THEN_START_REVERSE;
        break;
    case SPECIAL_FROM_THE_LEFT:
        // Code for special action to the left
        special_mode = 0 + is_magnet;
        turn_junction(get_turn_direction());
        break;
    case SPECIAL_FROM_THE_RIGHT:
        // Code for special action to the left
        special_mode = 2 + is_magnet;
        turn_junction(get_turn_direction());
        break;
    default:
        // Default action for invalid direction
        main_motors.stop();
        break;
    }
}


// Function for line tracking forward
void line_track_forward() {
    read_sensors();
    // Test code for 4 sensor following
    if (ls_state == 1 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
        main_motors.set_speed(230);
        main_motors.go_forward();
        cout << "Line tracking forward: On line." << endl;
    } else if (ls_state == 1 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        main_motors.change_MR_speed(12);
        main_motors.go_forward();
        cout << "Line tracking forward: Right of line." << endl;
    } else if (ls_state == 0 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
        main_motors.change_ML_speed(12);
        main_motors.go_forward();
        cout << "Line tracking forward: Left of line." << endl;
    } else if (ls_state == 0 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        // Something went wrong
        //main_motors.stop();
        main_motors.move_backward(10);
        cout << "Line tracking forward: Lost line, stopping." << endl;
    }
}

// Function for line tracking backward
void line_track_backward() {
    read_sensors();
    // Test code for 4 sensor following
    if (ls_state == 1 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
        main_motors.set_speed(230);
        main_motors.go_backward();
        cout << "Line tracking backward: On line." << endl;
    } else if (ls_state == 0 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
        main_motors.change_MR_speed(-10);
        main_motors.go_backward();
        cout << "Line tracking backward: Right of line." << endl;
    } else if (ls_state == 1 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        main_motors.change_ML_speed(-10);
        main_motors.go_backward();
        cout << "Line tracking backward: Left of line." << endl;
    } else if (ls_state == 0 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        // Something went wrong
        // main_motors.set_speed(130);
        // main_motors.go_backward();
        main_motors.move_forward(30);
        cout << "Line tracking backward: Lost line, adjusting speed." << endl;
    }
}

void do_a_move() {
    if (move_mode == FORWARD_MOVE) {
        line_track_forward();
    } if (move_mode == FORWARD_UNTIL_END_THEN_START_REVERSE) {
        line_track_backward();
        if (ls_state == 0 && rs_state == 0) {
            move_mode == REVERSE_MOVE;
        }
    } if (move_mode == REVERSE_MOVE) {
        line_track_backward();
        if (fls_state == 1 || frs_state == 1) {
            move_mode = FORWARD_MOVE;
        }
    }
}

// Function for making system decisions based on sensor readings
void system_decisions() {
    do_a_move();
    cout << "Executing system decisions." << endl;
    // Junction logic
    if (fls_state == 1 || frs_state == 1) {
        cout << "At junction." << endl;
        turn_junction(get_turn_direction());
    }
}

void loop() {
        system_decisions();
}



int main() {
    while (1) {
        sleep_for(seconds(1));
        loop();
    }
}
