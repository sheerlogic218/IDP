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
const int NO_MOVE = 0;
const int FORWARD_MOVE = 1;
const int DIP = 2;
const int REVERSE_MOVE = 3;
const int KEEP_GOING_UNTIL_END = 4;

// Path array defining the robot's route
int path[] = {
    LEFT,                        // BLOCK
    RIGHT, STRAIGHT_ON, RIGHT, SPECIAL_FROM_THE_LEFT,   // END SPECIAL AT MIDDLE BLOCK
    RIGHT, RIGHT, RIGHT, SPECIAL_FROM_THE_LEFT,         // END SPECIAL AT MIDDLE BLOCK
    LEFT, LEFT, LEFT, SPECIAL_FROM_THE_RIGHT,           // END SPECIAL AT MIDDLE BLOCK
    RIGHT, LEFT, LEFT, STRAIGHT_ON,                     // BLOCK
    LEFT, STRAIGHT_ON, LEFT, SPECIAL_FROM_THE_RIGHT,    // END SPECIAL AT MIDDLE BLOCK
    LEFT,                                               // There's a hidden block on this road
    STOP                                                // Use comments to explain each part of the route.
};

// Special paths for different modes
int special_path[][5] = {
    {STRAIGHT_ON, RIGHT_DIP, RIGHT, LEFT, STRAIGHT_ON}, //
    {RIGHT, RIGHT_DIP, LEFT, STOP, STOP}, // RLEFT goes forward after
    {LEFT_DIP, RIGHT, LEFT, STRAIGHT_ON, STOP},
    {STRAIGHT_ON, LEFT, RIGHT_DIP, LEFT, STOP},
    {LEFT, LEFT_DIP, STOP, STOP, STOP},
};

//Navigation variables
int progress = 0;
int special_mode = -1;
int special_progress = 0;
int move_mode = FORWARD_MOVE;   //start life by moving forward
long last_turn_time = 0;
long min_time_between_junctions = 2000; //SHOULDNT NEED TO EXIST, its a failsafe

//CLAW Variables
int claw_range = 100;   //Not used yet
int is_magnet = 0; // Magnetic is recyclable

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
            main_motors.turn_90_right();
            break;
        case LEFT:
            // Code to turn left
            main_motors.turn_90_left();
            break;
        case RIGHT_DIP:
            // Code to turn right
            main_motors.turn_90_right();
            move_mode = DIP;
            break;
        case LEFT_DIP:
            // Code to turn left
            main_motors.turn_90_left();
            move_mode = DIP;
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
    //Block logic
    // if (get_block_distance() < claw_range)
    // {
    //     //GRAB THE BLOCKKKK
    // }
    //Junction logic
    if (get_line_state() >= 5) {        //If it misinterprets things as junctions look here.
       Serial.println("At junction.");
       turn_junction(get_turn_direction());
    }
}

int get_turn_direction()
{
    int direction;
    Serial.println("Getting turn direction...");
    Serial.print("Last turn time: ");
    Serial.println(last_turn_time);
    move_mode = FORWARD_MOVE;
    //not a huge fan of this, this function just shouldnt even be able to be called if the last turn was too recent.
    //but if it works it works.
    if(last_turn_time + min_time_between_junctions > millis())        //NEW feature to prevent multi junction attitude
    {
        Serial.println("Failsafe triggered: Cancelling last turn.");
        progress--;
    }
    if(progress > (sizeof(path)/sizeof(path[0]))) //sizeof(path) is the byte size not index size, if this works its a fluke
    {
        Serial.println("End of path reached. Stopping.");
        move_mode = 0;
        return STOP;
    }
    if(special_mode == -1)
    {
        direction = path[progress];
        progress++;
        last_turn_time = millis();
    }
    else
    {
        direction = special_path[special_mode][special_progress];
        special_progress++;
        last_turn_time = millis();
        if(direction == STOP)
        {
            Serial.println("End of special path. Resetting special mode.");
            special_mode = -1;
            special_progress = 0;
        }
    }
    return direction;
}

void do_a_move()
{
    if(move_mode == NO_MOVE)
    {
        Serial.println("Move mode: NO_MOVE");
    }
    if(move_mode == FORWARD_MOVE)
    {
        Serial.println("Move mode: FORWARD_MOVE");
        line_track_forward();
    }
    if(move_mode == DIP)
    {
        Serial.println("Move mode: DIP");
        //looks like its gonna do a forward then backward
        main_motors.move_forward(50);
        main_motors.move_backward(45);  //My gut tells me the code wont work properly here, consider adding print lines
        move_mode = REVERSE_MOVE;
    }
    if(move_mode == REVERSE_MOVE)
    {
        Serial.println("Move mode: REVERSE_MOVE");
        reverse_backward();
    }
}



