#include "IDP_lib.h"

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
// int path[] = {
//     SPECIAL_FROM_THE_LEFT, LEFT,                        // BLOCK
//     RIGHT, STRAIGHT_ON, RIGHT, SPECIAL_FROM_THE_LEFT,   // END SPECIAL AT MIDDLE BLOCK
//     RIGHT, RIGHT, RIGHT, SPECIAL_FROM_THE_LEFT,         // END SPECIAL AT MIDDLE BLOCK
//     LEFT, LEFT, LEFT, SPECIAL_FROM_THE_RIGHT,           // END SPECIAL AT MIDDLE BLOCK
//     RIGHT, LEFT, LEFT, STRAIGHT_ON,                     // BLOCK
//     LEFT, STRAIGHT_ON, LEFT, SPECIAL_FROM_THE_RIGHT,    // END SPECIAL AT MIDDLE BLOCK
//     LEFT,                                               // There's a hidden block on this road
//     STOP                                                // Use comments to explain each part of the route.
// };
int path[] = {
  LEFT,RIGHT,STRAIGHT_ON,RIGHT,RIGHT,RIGHT_DIP
};
int path_length = 6;//sizeof(path)/sizeof(path[0]);


int progress = 0;
int special_mode = -1;
int special_progress = 0;
int direction = 0;
int is_magnet = 0; // Magnetic is recyclable
int move_mode = 1;
int turn_direction;

// Special paths for different modes
int special_path[5][6] = {
    {STRAIGHT_ON, RIGHT_DIP, RIGHT, LEFT, STRAIGHT_ON},
    {RIGHT, RIGHT_DIP, LEFT, STOP, STOP}, // RLEFT goes forward after
    {LEFT_DIP, RIGHT, LEFT, STRAIGHT_ON, STOP},
    {STRAIGHT_ON, LEFT, RIGHT_DIP, LEFT, STOP},
    {LEFT, LEFT_DIP, STOP, STOP, STOP},
};
int special_path_lengths = 5;


int get_turn_direction() {
    if (special_mode == -1  &&  progress <= (path_length-1) ) {
        turn_direction = path[progress];
        leds.red_off();
        leds.blue_on();
        progress++;
    } else if (special_progress <= special_path_lengths){
        direction = special_path[special_mode][special_progress];
        leds.blue_off();
        leds.red_on();
        special_progress++;
        if (direction == STOP) {
            special_mode = -1;
            special_progress = 0;
        }
    } else {
      main_motors.stop();
      leds.blue_blink();
      leds.red_blink();
    }
}


// Junction function to handle the robot's movements at junctions
void turn_junction() {
    Serial.print("Junction turn_direction: ");
    Serial.println(turn_direction);
    switch (turn_direction) {
        case STRAIGHT_ON:
            // Code to go straight
            main_motors.move_forward(30);
            break;
        case RIGHT:
            // Code to turn right
            main_motors.turn_90_right();
            break;
        case LEFT:
            // Code to turn left
            main_motors.turn_90_left();
            leds.blue_blink();
            break;
        case RIGHT_DIP:
            leds.red_blink();
            leds.red_blink();
            // Code to turn right
            main_motors.turn_90_right();
            move_mode = FORWARD_UNTIL_END_THEN_START_REVERSE;
            break;
        case LEFT_DIP:
            leds.blue_blink();
            leds.red_blink();
            // Code to turn left
            main_motors.turn_90_left();
            move_mode = FORWARD_UNTIL_END_THEN_START_REVERSE;
            break;
        case SPECIAL_FROM_THE_LEFT:
            // Code for special action to the left
            special_mode = 0 + is_magnet;
            get_turn_direction();
            turn_junction();
            break;
        case SPECIAL_FROM_THE_RIGHT:
            // Code for special action to the left
            special_mode = 2 + is_magnet;
            get_turn_direction();
            turn_junction();
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
        Serial.println("Line tracking forward: On line.");
    } else if (ls_state == 1 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        main_motors.change_MR_speed(12);
        main_motors.go_forward();
        Serial.println("Line tracking forward: Right of line.");
    } else if (ls_state == 0 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
        main_motors.change_ML_speed(12);
        main_motors.go_forward();
        Serial.println("Line tracking forward: Left of line.");
    } else if (ls_state == 0 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        // Something went wrong
        //main_motors.stop();
        main_motors.move_backward(10);
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
        // main_motors.set_speed(130);
        // main_motors.go_backward();
        main_motors.move_forward(30);
        Serial.println("Line tracking backward: Lost line, adjusting speed.");
    }
}

void do_a_move() {
    if(move_mode == FORWARD_MOVE){
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
    Serial.println("Executing system decisions.");
    // Junction logic
    if (fls_state == 1 || frs_state == 1) {
        Serial.println("At junction.");
        get_turn_direction();
        turn_junction();
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