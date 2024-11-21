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
const int TESTING = 10;

// constants for move directions
const int FORWARD_MOVE = 1;
const int FORWARD_UNTIL_END_THEN_START_REVERSE = 2;
const int REVERSE_MOVE = 3;
const int KEEP_GOING_UNTIL_END = 4;

// Path array defining the robot's route
int path[23] = {
    LEFT,                        // BLOCK
    RIGHT, STRAIGHT_ON, RIGHT, SPECIAL_FROM_THE_LEFT,   // END SPECIAL AT MIDDLE BLOCK
    RIGHT, RIGHT, RIGHT, SPECIAL_FROM_THE_LEFT,         // END SPECIAL AT MIDDLE BLOCK
    LEFT, LEFT, LEFT, SPECIAL_FROM_THE_RIGHT,           // END SPECIAL AT MIDDLE BLOCK
    RIGHT, LEFT, LEFT, STRAIGHT_ON,                     // BLOCK
    LEFT, STRAIGHT_ON, LEFT, SPECIAL_FROM_THE_RIGHT,    // END SPECIAL AT MIDDLE BLOCK
    LEFT,                                               // There's a hidden block on this road
    STOP                                                // Use comments to explain each part of the route.
};

int progress = 0;
int special_mode = -1;
int special_progress = 0;
int direction = 0;
int is_magnet = 0; // Magnetic is recyclable
int move_mode = FORWARD_MOVE;

// Special paths for different modes
int special_path[5][6] = {
    {STRAIGHT_ON, RIGHT_DIP, RIGHT, LEFT, STRAIGHT_ON},
    {RIGHT, RIGHT_DIP, LEFT, STOP, STOP}, // RLEFT goes forward after
    {LEFT_DIP, RIGHT, LEFT, STRAIGHT_ON, STOP},
    {STRAIGHT_ON, LEFT, RIGHT_DIP, LEFT, STOP},
    {LEFT, LEFT_DIP, STOP, STOP, STOP},
};

// Junction function to handle the robot's movements at junctions
void turn_junction(int turn_direction) {
    Serial.print("Junction turn_direction: ");
    Serial.println(turn_direction);
    switch (turn_direction) {
        case STRAIGHT_ON:
            // Code to go straight
            main_motors.move_forward(30);
            break;
        case RIGHT:
            // Code to turn right
            turn_right_until_line():
            break;
        case LEFT:
            // Code to turn left
            turn_left_until_line();
            break;
        case RIGHT_DIP:
            // Code to turn right
            turn_right_until_line();
            move_mode = FORWARD_UNTIL_END_THEN_START_REVERSE;
            break;
        case LEFT_DIP:
            // Code to turn left
            turn_left_until_line();
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
        case TESTING:
            special_mode = 4;
            turn_junction(get_turn_direction());
            break;
        default:
            // Default action for invalid direction
            main_motors.stop();
            break;
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
    do_a_move();
    Serial.println("Executing system decisions.");
    //Junction logic
    if (fls_state == 1 || frs_state == 1) {
       Serial.println("At junction.");
       turn_junction(get_turn_direction());
    }
}

int get_turn_direction()
{
    if(special_mode == -1)
    {
        direction = path[progress];
        progress++;
    }
    else
    {
        direction = special_path[special_mode][special_progress];
        special_progress++;
        if(direction == STOP)
        {
            special_mode = -1;
            special_progress = 0;
        }
    }
    return direction;
}

void do_a_move()
{
    if(move_mode == FORWARD_MOVE)
    {
        Serial.println("Move mode: FORWARD_MOVE");
        line_track_forward();
    }
    if(move_mode == FORWARD_UNTIL_END_THEN_START_REVERSE)
    {
        Serial.println("Move mode: FORWARD_UNTIL_END_THEN_START_REVERSE");
        line_track_forward();
        if(ls_state == 1 && rs_state == 1)
        {
            move_mode = KEEP_GOING_UNTIL_END;
        }
    }
    if(move_mode == REVERSE_MOVE)
    {
        Serial.println("Move mode: REVERSE_MOVE");
        reverse_backward();
        if(fls_state == 1 || frs_state == 1)
        {
            move_mode = FORWARD_MOVE;
        }
    }
    if(move_mode == KEEP_GOING_UNTIL_END)
    {
        Serial.println("Move mode: KEEP_GOING_UNTIL_END");
        line_track_forward();
        if(ls_state == 0 && rs_state == 0)
        {
            move_mode = REVERSE_MOVE;
        }
    }
}



