#include "IDP_lib.h"

// volatile byte state = LOW;

const int STRAIGHT_ON = 0;
const int RIGHT = 1;
const int FULL_AROUND = 2;
const int LEFT = 3;
const int SPECIALl = 4;
const int SPECIALr = 5;
const int SPECIAL = 6;
const int FORWARD_THEN_BACKWARD = 11;
const int REVERSE = 8;
const int RLEFT = 9;
const int RRIGHT = 10;
const int STOP = 7;

int path[24] = {SPECIALl, LEFT, //BLICK!
RIGHT, STRAIGHT_ON, RIGHT, SPECIALl, //END SPECIAL AT MIDDLE BLOCK
RIGHT, RIGHT, RIGHT, SPECIALl,       //END SPECIAL AT MIDDLE BLOCK
LEFT, LEFT, LEFT, SPECIALr,          //END SPECIAL AT MIDDLE BLOCK
RIGHT, LEFT, LEFT, STRAIGHT_ON,         //BLICK
LEFT, STRAIGHT_ON, LEFT, SPECIALr,    //END SPECIAL AT MIDDLE BLOCK
LEFT,                               //Theres a hidden block on this road

STOP //use comments to explain each part of the route.
};
int progress = 0;

int special_mode = -1;
int special_progress = 0;
int special_direction = 0;
bool is_magnet = false; //Magnetic is recyclable
int special_path[5][6] = {
    {STRAIGHT_ON, RIGHT, FORWARD_THEN_BACKWARD, RRIGHT, LEFT, STRAIGHT_ON},
    {RIGHT, RIGHT, FORWARD_THEN_BACKWARD, RLEFT, STOP, STOP}, //rleft goes forward after
    {LEFT, FORWARD_THEN_BACKWARD, RRIGHT, LEFT, STRAIGHT_ON, STOP},
    {STRAIGHT_ON, LEFT, RIGHT, FORWARD_THEN_BACKWARD, RLEFT, STOP},
    {LEFT, LEFT, FORWARD_THEN_BACKWARD, STOP, STOP, STOP},
};

void junction(){
    int turn_direction = path[progress];
    progress++;
    switch(turn_direction) {
        case STRAIGHT_ON:
            // Code to go straight
            //main_motors.go_forward(); literally no effort is needed here
            delay(1000);
            break;
        case RIGHT:
            // Code to turn right
            main_motors.turn_90_right();
            break;
        case FULL_AROUND:
            // Code to turn around
            //main_motors.turn_180();  A REAL FUNCTION WILL BE DESIRABLE HERE
                main_motors.turn_90_left();            
                main_motors.turn_90_left();
            break;
        case LEFT:
            // Code to turn left
            main_motors.turn_90_left();
            break;
        case SPECIALr:
            // Code for special action
            // main_motors.special_action();
            if(is_magnet)
            {
                special_mode = 3;
            }
            else
            {
                special_mode = 2;
            }
            special_junction();
            break;
        case SPECIALl:
            // Code for special action
            // main_motors.special_action();
            if(is_magnet)
            {
                special_mode = 1;
            }
            else
            {
                special_mode = 4;
            }
            special_junction();
            break;
        case STOP:
            // Default action
            main_motors.stop();
            break;
        default:
            // Default action
            main_motors.stop();
            break;
    }
}

void special_junction()
{
        special_direction = special_path[special_mode][special_progress];
        switch(special_direction) {
            case STRAIGHT_ON:
                // Code to go straight
                //main_motors.go_forward(); literally no effort is needed here
                break;
            case RIGHT:
                // Code to turn right
                main_motors.turn_90_right();
                break;
            case FULL_AROUND:
                // Code to turn around
                //main_motors.turn_180();  A REAL FUNCTION WILL BE DESIRABLE HERE
                    main_motors.turn_90_left();            
                    main_motors.turn_90_left();
                break;
            case LEFT:
                // Code to turn left
                main_motors.turn_90_left();
                break;
            case REVERSE:
                main_motors.go_backward();
                while(fls_state == 0 && frs_state ==0)     //add alignment
                {
                    delay(50);
                }
                break;
            case RLEFT:
                main_motors.turn_90_left_back();
                break;
            case RRIGHT:
                main_motors.turn_90_right_back();
                break;
            case FORWARD_THEN_BACKWARD:
                main_motors.go_backward();
                read_sensors();
                while (ls_state == 1 || rs_state == 1)
                {
                    line_track_forward();
                    read_sensors();
                }

                while (fls_state == 0 && frs_state == 0)
                {
                    line_track_backward();
                    read_sensors();
                }
                main_motors.turn_90_left_back();
                break;
            default:
                special_mode = -1;
                special_progress = 0;
                break;
        }
}

void line_track_forward()
{
    read_sensors();
    //test code for 4 sensor following
    if (ls_state == 1 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
        main_motors.set_speed(230);
        main_motors.go_forward();
        Serial.println("on line");
    }
    else if (ls_state == 1 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        main_motors.change_MR_speed(10);
        main_motors.go_forward();
        Serial.println("right of line");
    }
    
    else if (ls_state == 0 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
        main_motors.change_ML_speed(10);
        main_motors.go_forward();
        Serial.println("left of line");
    }
    //something gone wrong
    else if (ls_state == 0 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        main_motors.stop();
        Serial.println("something wrong");
    }
}
void line_track_backward()
{
    read_sensors();
    //test code for 4 sensor following
    if (ls_state == 1 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
        main_motors.set_speed(230);
        main_motors.go_backward();
        Serial.println("on line");
    }
    else if (ls_state == 0 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
        main_motors.change_MR_speed(10);
        main_motors.go_backward();
        Serial.println("right of line");
    }
    
    else if (ls_state == 1 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        main_motors.change_ML_speed(10);
        main_motors.go_backward();
        Serial.println("left of line");
    }
    //something gone wrong
    else if (ls_state == 0 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        main_motors.stop();
        Serial.println("something wrong");
    }
}


void loop(){
    //Serial.println(state);
    if (state) {
        system_decisions();
    }
    else {
        Serial.println("we are not running");
        main_motors.stop();
    }
}

void system_decisions()
{
    Serial.println("we are running");
    line_track_forward();
    //junction logic
    if(fls_state == 1 || frs_state == 1){
        Serial.println("at junction");
        //main_motors.move_forward(20);
        if(special_mode == -1)
        {
            junction();
        }
        else
        {
            special_junction();
        }
    }
}





