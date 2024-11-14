#include "IDP_lib.h"
#include <vector>
#include <map>
#include <string>

volatile byte state = LOW;

int NORTH = 0;
int EAST = 1;
int SOUTH = 2;
int WEST = 3;

bool junction_present = false;


std::map<int, std::map<int, int>> adjacent[MAX_NODES] = {
    { 0, { {NORTH, 1} } },                         // Node 0
    { 1, { {EAST, 3}, {SOUTH, 0}, {WEST, 11} } },  // Node 1
    { 2, { {NORTH, 11} } },                        // Node 2
    { 3, { {NORTH, 5}, {SOUTH, 4}, {WEST, 1} } },  // Node 3
    { 4, { {NORTH, 3} } },                         // Node 4
    { 5, { {NORTH, 7}, {SOUTH, 3}, {WEST, 6} } },  // Node 5
    { 6, { {NORTH, 14}, {EAST, 5}, {WEST, 10} } }, // Node 6
    { 7, { {SOUTH, 5}, {WEST, 12} } },             // Node 7
    { 8, { {EAST, 12}, {SOUTH, 14}, {WEST, 9} } }, // Node 8
    { 9, { {EAST, 8}, {SOUTH, 10} } },             // Node 9
    { 10, { {NORTH, 9}, {EAST, 6}, {SOUTH, 11} } }, // Node 10
    { 11, { {NORTH, 10}, {EAST, 1}, {SOUTH, 2} } }, // Node 11
    { 12, { {EAST, 7}, {SOUTH, 13}, {WEST, 8} } },  // Node 12
    { 13, { {NORTH, 12} } },                        // Node 13
    { 14, { {NORTH, 8}, {SOUTH, 6}, {WEST, 15} } }, // Node 14
    { 15, { {EAST, 14} } }                          // Node 15
};

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

int get_direction(int currentNode, int targetNode) {
    for (const auto& direction_pair : adjacent[currentNode]) {
        if (direction_pair.second == targetNode) {
            return direction_pair.first; // Returns direction constant (NORTH, EAST, etc.)
        }
    }
    return -1; // Direction not found
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


std::vector<int> generateNavigationPath(int current_node, int target_node)
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






