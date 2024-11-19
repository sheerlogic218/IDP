#include "IDP_lib.h"
// #include <vector>

// int target_node = 1;
// int behind_node = 0;
// int ahead_node = 1;
// int direction = 0;
// std::vector<int> path;
// int path_progress;

// void junction(int behind_node, int junction_node, int target_node){
//     Serial.println("Entering junction function");
//     Serial.print("behind_node: ");
//     Serial.println(behind_node);
//     Serial.print("junction_node: ");
//     Serial.println(junction_node);
//     Serial.print("target_node: ");
//     Serial.println(target_node);
    
//     int turn_direction = main_navigator.generateTurnDirection(behind_node, junction_node, target_node);
//     Serial.print("Turn direction: ");
//     Serial.println(turn_direction);
    
//     if (turn_direction == -1){
//         main_motors.turn_90_left();
//         Serial.println("Turning 90 degrees left");
//     }
//     else if (turn_direction == 1) {
//         main_motors.turn_90_right();
//         Serial.println("Turning 90 degrees right");
//     }
//     else if (turn_direction == 0) {
//         main_motors.turn_90_left();
//         Serial.println("No turn, proceeding straight");
//     }
// }


// void loop(){
//     Serial.println("Loop started");
//     if (state) {
//         Serial.println("State is active");
//         if(ahead_node == target_node)
//         {
//             Serial.println("Ahead node matches target node");
//             target_node = millis() % 16;
//             Serial.print("New target_node: ");
//             Serial.println(target_node);
//             path_progress = 0;
//             path = main_navigator.generateNavigationPath(ahead_node, target_node);
//             Serial.println("Generated new navigation path");
//         }
//         Serial.println("We are running");
//         read_sensors();
//         Serial.println("Sensors read");
        
//         // Test code for 4 sensor following
//         Serial.print("Sensor states - ls: ");
//         Serial.print(ls_state);
//         Serial.print(", rs: ");
//         Serial.print(rs_state);
//         Serial.print(", fls: ");
//         Serial.print(fls_state);
//         Serial.print(", frs: ");
//         Serial.println(frs_state);
        
//         if (ls_state == 1 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
//             main_motors.set_speed(230);
//             main_motors.go_forward();
//             Serial.println("On line: Moving forward at speed 230");
//         }
//         else if (ls_state == 1 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
//             main_motors.change_MR_speed(10);
//             main_motors.go_forward();
//             Serial.println("Right of line: Adjusting MR speed by 10");
//         }
//         else if (ls_state == 0 && rs_state == 1 && fls_state == 0 && frs_state == 0) {
//             main_motors.change_ML_speed(10);
//             main_motors.go_forward();
//             Serial.println("Left of line: Adjusting ML speed by 10");
//         }
//         // Something gone wrong
//         else if (ls_state == 0 && rs_state == 0 && fls_state == 0 && frs_state == 0) {
//             main_motors.stop();
//             Serial.println("Error: No sensors active, stopping motors");
//         }
//         // Junction logic
//         else if(fls_state == 1 || frs_state == 1){
//             Serial.println("At junction");
//             path_progress++;
//             Serial.print("Path progress: ");
//             Serial.println(path_progress);
//             if(path_progress < path.size()){
//                 junction(behind_node, ahead_node, path[path_progress]);
//             }
//             else{
//                 Serial.println("Path completed");
//             }
//         }
//         //delay(100);
//     }
//     else {
//         Serial.println("State is inactive, stopping motors");
//         main_motors.stop();
//     }
//     Serial.println("Loop ended\n");
// }

// void setup() {
//     Serial.begin(9600);
//     Serial.println("a");
//     delay(1000);
//     Serial.println("b");
// }

void loop ()
{
      Serial.println("b");
          delay(1000);
}
