#include "IDP_lib.h"

// Constants for movement directions
const int STRAIGHT_ON = 0;
const int RIGHT = 1;
const int FULL_AROUND = 2;
const int LEFT = 3;
const int SPECIAL_FROM_THE_LEFT = 4;
const int SPECIAL_FROM_THE_RIGHT = 5;
const int FORWARD_THEN_BACKWARD = 11;
const int REVERSE = 8;
const int STOP = 7;

// Path array defining the robot's route
int path[24] = {
    SPECIAL_FROM_THE_LEFT, LEFT, // BLICK!
    RIGHT, STRAIGHT_ON, RIGHT, SPECIAL_FROM_THE_LEFT,   // END SPECIAL AT MIDDLE BLOCK
    RIGHT, RIGHT, RIGHT, SPECIAL_FROM_THE_LEFT,         // END SPECIAL AT MIDDLE BLOCK
    LEFT, LEFT, LEFT, SPECIAL_FROM_THE_RIGHT,           // END SPECIAL AT MIDDLE BLOCK
    RIGHT, LEFT, LEFT, STRAIGHT_ON,                     // BLICK
    LEFT, STRAIGHT_ON, LEFT, SPECIAL_FROM_THE_RIGHT,    // END SPECIAL AT MIDDLE BLOCK
    LEFT,                                // There's a hidden block on this road
    STOP // Use comments to explain each part of the route.
};

int progress = 0;
int special_mode = -1;
int special_progress = 0;
int direction = 0;
int is_magnet = 0; // Magnetic is recyclable

// Special paths for different modes
int special_path[5][6] = {
    {STRAIGHT_ON, RIGHT, FORWARD_THEN_BACKWARD, RIGHT, LEFT, STRAIGHT_ON},
    {RIGHT, RIGHT, FORWARD_THEN_BACKWARD, LEFT, STOP, STOP}, // RLEFT goes forward after
    {LEFT, FORWARD_THEN_BACKWARD, RIGHT, LEFT, STRAIGHT_ON, STOP},
    {STRAIGHT_ON, LEFT, RIGHT, FORWARD_THEN_BACKWARD, LEFT, STOP},
    {LEFT, LEFT, FORWARD_THEN_BACKWARD, STOP, STOP, STOP},
};

// Junction function to handle the robot's movements at junctions
void turn_junction() {
    Serial.print("Junction turn_direction: ");
    Serial.println(turn_direction);
    switch (turn_direction) {
        case STRAIGHT_ON:
            // Code to go straight
            delay(1000);
            break;
        case RIGHT:
            // Code to turn right
            main_motors.turn_90_right();
            break;
        case LEFT:
            // Code to turn left
            main_motors.turn_90_left();
            break;
        case SPECIAL_FROM_THE_LEFT:
            // Code for special action to the left
            special_mode = 0 + is_magnet;
            break;
        case SPECIAL_FROM_THE_RIGHT:
            // Code for special action to the left
            special_mode = 2 + is_magnet;
            break;
        default:
            // Default action for invalid direction
            main_motors.stop();
            break;
    }
}

// Function to handle special junction movements
void special_junction() {
    direction = special_path[special_mode][special_progress];
    special_progress++;
    switch (direction) {
        case STRAIGHT_ON:
            // Code to go straight
            break;
        case RIGHT:
            // Code to turn right
            main_motors.turn_90_right();
            break;
        case LEFT:
            // Code to turn left
            Serial.println("Special junction: Turning left.");
            main_motors.turn_90_left();
            break;
        case REVERSE:
            Serial.println("Special junction: Reversing.");
            main_motors.go_backward();
            while (fls_state == 0 && frs_state == 0) {    // Add alignment
                Serial.println("Reversing until front sensors detect line.");
                delay(50);
            }
            break;
        case STOP:
            Serial.println("Special junction: Stopping special mode.");
            special_mode = -1;
            special_progress = 0;
            break;
        default:
            Serial.println("Special junction: Invalid direction.");
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
        Serial.println("Line tracking forward: On line.");
    } else if (ls_state == 1 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        main_motors.change_MR_speed(10);
        main_motors.go_forward();
        Serial.println("Line tracking forward: Right of line.");
    } else if (ls_state == 0 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
        main_motors.change_ML_speed(10);
        main_motors.go_forward();
        Serial.println("Line tracking forward: Left of line.");
    } else if (ls_state == 0 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        // Something went wrong
        main_motors.stop();
        Serial.println("Line tracking forward: Lost line, stopping.");
    }
}

// Function for line tracking backward
void line_track_backward() {
    read_sensors();
    // Test code for 4 sensor following
    if (ls_state == 1 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
        main_motors.set_speed(230);
        main_motors.go_backward();
        Serial.println("Line tracking backward: On line.");
    } else if (ls_state == 0 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
        main_motors.change_MR_speed(-10);
        main_motors.go_backward();
        Serial.println("Line tracking backward: Right of line.");
    } else if (ls_state == 1 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        main_motors.change_ML_speed(-10);
        main_motors.go_backward();
        Serial.println("Line tracking backward: Left of line.");
    } else if (ls_state == 0 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        // Something went wrong
        main_motors.set_speed(130);
        main_motors.go_backward();
        Serial.println("Line tracking backward: Lost line, adjusting speed.");
    }
}

void loop() {
    if (state) {
        Serial.println("System is running.");
        system_decisions();
    } else {
        Serial.println("System is not running.");
        main_motors.stop();
    }
}

// Function for making system decisions based on sensor readings
void system_decisions() {
    line_track_forward();
    Serial.println("Executing system decisions.");
    // Junction logic
    if (fls_state == 1 || frs_state == 1) {
        Serial.println("At junction.");
        if (special_mode == -1) {
            junction();
        } else {
            special_junction();
        }
    }
}

int path_switching()
{
    if(special_mode == -1)
    {
        int turn_direction = path[progress];
        progress++;
    }
    else
    {

    }
}


