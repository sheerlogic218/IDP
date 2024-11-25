#include "IDP_lib.h"
#include <vector>
#include <map>
#include <string>

using namespace std;
volatile byte state = LOW;

// Define direction constants as const to prevent modification
const int NORTH = 0;
const int EAST = 1;
const int SOUTH = 2;
const int WEST = 3;

// Initialize the adjacency map with proper nested map initialization
map<int, std::map<int, int> > adjacent;

void populate_node_map()
{
    // Populate the adjacency list
    adjacent[0] = { {1, NORTH} };
    adjacent[1] = { {3, EAST}, {0, SOUTH}, {11, WEST} };
    adjacent[2] = { {11, NORTH} };
    adjacent[3] = { {5, NORTH}, {4, SOUTH}, {1, WEST} };
    adjacent[4] = { {3, NORTH} };
    adjacent[5] = { {7, NORTH}, {3, SOUTH}, {6, WEST} };
    adjacent[6] = { {14, NORTH}, {5, EAST}, {10, WEST} };
    adjacent[7] = { {5, SOUTH}, {12, WEST} };
    adjacent[8] = { {12, EAST}, {14, SOUTH}, {9, WEST} };
    adjacent[9] = { {8, EAST}, {10, SOUTH} };
    adjacent[10] = { {9, NORTH}, {6, EAST}, {11, SOUTH} };
    adjacent[11] = { {10, NORTH}, {1, EAST}, {2, SOUTH} };
    adjacent[12] = { {7, EAST}, {13, SOUTH}, {8, WEST} };
    adjacent[13] = { {12, NORTH} };
    adjacent[14] = { {8, NORTH}, {6, SOUTH}, {15, WEST} };
    adjacent[15] = { {14, EAST} };
}

std::vector<int[2]> traversed;

void traverseToNode(int goalNode, int currentNode)      //the goal of this function is to be a high level way to control where the car travels towards
{
    int ahead = 1;
    int behind = 0;

    int route_progress = 0;
    while(!junction_present) 
    {
        //Serial.println(state);
        if (state) {
            Serial.println("we are running");
            read_sensors();
            test code for 4 sensor following
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
                //work out which junction we are on and which way to steer (it should be forward junction)
                int direction = get_direction(behind, ahead);
                int progress++;


                //add in junction verification ///THIS MAY CAUSE FALSE ERRORS IF SENSORS REACT TOO QUICKLY
                

                //update forward and backward junctions

            }
            }
        else {
            Serial.println("we are not running");
        }

    }
}

void turn_left()
{
        main_motors.change_MR_speed(40);
        main_motors.go_forward();
        delay(1000);
}

void turn_right()
{
        main_motors.change_ML_speed(40);
        main_motors.go_forward();
        delay(1000);
}

void turn_forward()
{
        delay(2000);
}


vector<int> generateNavigationPath(int current_node, int target_node)
{
    int search_node = current_node;
    int depth = 0;
    int new_node;
    std::vector<int> current_route = {search_node};
    std::vector<std::vector<int>> possible_routes = {search_node};
    while(depth < 10)                                           //there are two ways to make this run faster, eliminate routes that end at useless nodes (dead ends and already found routes) and eliminate routes that get to the same non target node in the same speed.
    {
        for(int j = 0, j < possible_routes, j++)                //continue extending all destination nodes
        {
            for(int i = 0, i < 4, i++)    //add next nodes to possible_routes, increasing possibilites
            {
                new_node = adjacent{search_node}{i};
                if(new_node != -1)
                {
                    current_route.push_back(new_node);
                    if(new_node = target_node)
                    {
                        return current_route;
                    }
                    possible_routes.push_back{current_route};
                }
            }
        }
        depth++;
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






