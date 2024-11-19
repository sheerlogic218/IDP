#include "IDP_lib.h"

// volatile byte state = LOW;


void junction(){
    if (fls_state == 1 && frs_state == 0){
        // main_motors.change_MR_speed(40);
        // main_motors.go_forward();
        // delay(1000);
        main_motors.turn_90_left();
    }
    else if (fls_state == 0 && frs_state == 1) {
        // main_motors.change_ML_speed(40);
        // main_motors.go_forward();
        // delay(1000);
        main_motors.turn_90_right();
    }
    else if (fls_state == 1 && frs_state == 1) {
        // main_motors.change_MR_speed(20);
        // main_motors.go_forward();
        // delay(2000);
        main_motors.turn_90_left();
    }
}



void junction2(){
    main_motors.stop();
    main_motors.set_speed(30);
    delay(200);
}

// void interrupt_function(){
//     volatile static unsigned long last_interrupt = 0;
//     unsigned long t = millis();
//     if (t - last_interrupt > 500UL){
//       state = !state;
//       main_motors.stop();
//     }
//     last_interrupt = t;
//     //Serial.println(state);
//   }

// void setup() {
//     pinMode(3,INPUT);
//     while ( !digitalRead(3) );
//     Serial.begin(9600);
//     Serial.println("a");
//     attachInterrupt(digitalPinToInterrupt(3), interrupt_function, RISING);
//     AFMS.begin();
//     main_motors.set_speed(0);
//     pinMode(left_sensor, INPUT);
//     //pinMode(center_sensor, INPUT);
//     pinMode(right_sensor, INPUT);
//     pinMode(far_left_sensor,INPUT);
//     pinMode(far_right_sensor, INPUT);
    
// }

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
        main_motors.change_MR_speed(12);
        main_motors.go_forward();
        Serial.println("right of line");
    }
    
    else if (ls_state == 0 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
        main_motors.change_ML_speed(12);
        main_motors.go_forward();
        Serial.println("left of line");
    }
    //something gone wrong
    else if (ls_state == 0 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
        main_motors.stop();
        Serial.println("something wrong");
        main_motors.move_backward(10);
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






