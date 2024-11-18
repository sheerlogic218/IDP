#include "IDP_lib.h"

// volatile byte state = LOW;

const int STRAIGHT_ON = 0;
const int RIGHT = 1;
const int FULL_AROUND = 2;
const int LEFT = 3;
const int SPECIALl = 4;
const int SPECIALr = 5;
const int SPECIAL = 6;
const int REVERSE = 8;
const int RLEFT = 9;
const int RRIGHT = 10;
const int STOP = 7;

int path[23] = {LEFT, //BLICK!
RIGHT, STRAIGHT_ON, RIGHT, SPECIALl, //END SPECIAL AT MIDDLE BLOCK
RIGHT, RIGHT, RIGHT, SPECIALl,       //END SPECIAL AT MIDDLE BLOCK
LEFT, LEFT, LEFT, SPECIALr,          //END SPECIAL AT MIDDLE BLOCK
RIGHT, LEFT, LEFT, STRAIGHT_ON,         //BLICK
LEFT, STRAIGHT_ON, LEFT, SPECIALr,    //END SPECIAL AT MIDDLE BLOCK
LEFT,                               //Theres a hidden block on this road

STOP //use comments to explain each part of the route.
};
int progress = 0;

int special_mode = 0;
int special_progress = 0;
int special_direction = 0;
bool is_magnet = false; //Magnetic is recyclable
int special_path[4][5] = {
    {STRAIGHT_ON, RIGHT, REVERSE, RRIGHT},
    {RIGHT, RIGHT, REVERSE, RLEFT},
    {LEFT, STOP, STOP, STOP},
    {STRAIGHT_ON, LEFT, RIGHT, STOP},
};

void junction(){
    int turn_direction = path[progress];
    progress ++;
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
                special_mode = 4;
            }
            else
            {
                special_mode = 3;
            }
            break;
        case SPECIALl:
            // Code for special action
            // main_motors.special_action();
            if(is_magnet)
            {
                special_mode = 2;
            }
            else
            {
                special_mode = 1;
            }
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

    if(special_mode != 0)
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
            default:
                special_mode = 0;
                special_progress = -1;
                break;
        }
    }
    // if (fls_state == 1 && frs_state == 0){
    //     // main_motors.change_MR_speed(40);
    //     // main_motors.go_forward();
    //     // delay(1000);
    //     main_motors.turn_90_left();
    // }
    // else if (fls_state == 0 && frs_state == 1) {
    //     // main_motors.change_ML_speed(40);
    //     // main_motors.go_forward();
    //     // delay(1000);
    //     main_motors.turn_90_right();
    // }
    // else if (fls_state == 1 && frs_state == 1) {
    //     // main_motors.change_MR_speed(20);
    //     // main_motors.go_forward();
    //     // delay(2000);
    //     main_motors.turn_90_left();
    // }
}

void loop(){
  //Serial.println(state);
  if (state) {
    Serial.println("we are running");
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
    //junction logic
    else if(fls_state == 1 || frs_state == 1){
        Serial.println("at junction");
        //main_motors.move_forward(20);
        junction();
    }
    //delay(100);
  }

  else {
    Serial.println("we are not running");
    main_motors.stop();
  }
}






