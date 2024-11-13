#include "IDP_lib.h"

void junction(){
    if (fls_state == 1 && frs_state == 0){
        main_motors.change_MR_speed(40);
        main_motors.go_forward();
        delay(1000);
    }
    else if (fls_state == 0 && frs_state == 1) {
        main_motors.change_ML_speed(40);
        main_motors.go_forward();
        delay(1000);
    }
    else if (fls_state == 1 && frs_state == 1) {
        main_motors.change_MR_speed(20);
        main_motors.go_forward();
        delay(2000);
    }
}


void junction2(){
    main_motors.stop();
    main_motors.set_speed(30);
    delay(200);




}



void setup() {
    Serial.begin(9600);
    AFMS.begin();
    main_motors.set_speed(0);
    pinMode(left_sensor, INPUT);
    //pinMode(center_sensor, INPUT);
    pinMode(right_sensor, INPUT);
    pinMode(far_left_sensor,INPUT);
    pinMode(far_right_sensor, INPUT);
}

void loop(){
    read_sensors();
    //test code for 4 sensor following
    if (ls_state == 1 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
        main_motors.set_speed(200);
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
        main_motors.set_speed(50);
        main_motors.go_backward();
        Serial.println("something wrong");
    }
    //junction logic
    else {
        Serial.println("at junction");
        junction();
    } 
}






