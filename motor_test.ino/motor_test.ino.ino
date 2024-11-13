#include "IDP_lib.h"

volatile byte state = LOW;


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



int calc_dist(int speed, int time) {
    main_motors.set_speed(speed);
    main_motors.go_forward();
    delay(time);

}

void junction2(){
    main_motors.stop();
    main_motors.set_speed(30);
    delay(200);
}

void interrupt_function(){
    volatile static unsigned long last_interrupt = 0;
    unsigned long t = millis();
    if (t - last_interrupt > 500UL){
      state = !state;
    }
    last_interrupt = t;
    //Serial.println(state);
  }

void setup() {
    pinMode(3,INPUT);
    while ( !digitalRead(3) );
    Serial.begin(9600);
    Serial.println("a");
    attachInterrupt(digitalPinToInterrupt(3), interrupt_function, RISING);
    //while(1);
    // AFMS.begin();
    // main_motors.set_speed(0);
    // pinMode(left_sensor, INPUT);
    // //pinMode(center_sensor, INPUT);
    // pinMode(right_sensor, INPUT);
    // pinMode(far_left_sensor,INPUT);
    // pinMode(far_right_sensor, INPUT);
    
}

void loop(){
  //Serial.println(state);
  if (state) {
    Serial.println("we are running");
    }
  else {
    Serial.println("we are not running");
  }
    //read_sensors();
    //test code for 4 sensor following
    // if (ls_state == 1 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
    //     main_motors.set_speed(200);
    //     main_motors.go_forward();
    //     Serial.println("on line");
    // }
    // else if (ls_state == 1 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
    //     main_motors.change_MR_speed(10);
    //     main_motors.go_forward();
    //     Serial.println("right of line");
    // }
    
    // else if (ls_state == 0 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
    //     main_motors.change_ML_speed(10);
    //     main_motors.go_forward();
    //     Serial.println("left of line");
    // }
    // //something gone wrong
    // else if (ls_state == 0 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
    //     main_motors.set_speed(50);
    //     main_motors.go_backward();
    //     Serial.println("something wrong");
    // }
    // //junction logic
    // else {
    //     Serial.println("at junction");
    //     junction();
    // }

}






