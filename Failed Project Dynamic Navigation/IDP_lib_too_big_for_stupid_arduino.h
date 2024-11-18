#include <Adafruit_MotorShield.h>
#include <Arduino_LSM6DS3.h>
#include <StandardCplusplus.h>

//required for navigation code
#include <vector>
#include <map>

// Ensure using the std namespace or prefix with std::
using std::vector;
// using std::map;


Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *main_motor_left  = AFMS.getMotor(1); //motor pin 1
Adafruit_DCMotor *main_motor_right = AFMS.getMotor(2); //motor pin 2

volatile byte state = LOW;

// Define direction constants
const int NORTH = 0;
const int EAST = 1;
const int SOUTH = 2;
const int WEST = 3;


//create class to control main motors
class MainMotors {
    public:
        int max_speed = 255;
        int min_speed = 0;
        int speed = 0;
        int l_speed = 0;
        int r_speed = 0;

        //FORWARD = 1
        //BACKWARD = 2
        float Pi = 3.14159;

        //test values
        float wheel_radius = 32.5; //mm
        int wheel_base = 170; //mm
        float max_wheel_angular_speed = 1.95*Pi;//(5*360)/(5*2*Pi); //max angular speed of wheel, radians per second
        //double wheel_angular_speed_100 = 0;
        float max_wheel_speed = max_wheel_angular_speed*wheel_radius; //max linear speed of wheel mm/s
        int sensor_wheel_dist = 110;//mm



    //simple interface for using built in functions -- verified
    void set_ML_speed(int l_speed) {
        if (l_speed > max_speed) {
          l_speed = max_speed;
        }
        else if (l_speed < min_speed) {
          l_speed = min_speed;
        }
        main_motor_left->setSpeed(l_speed);
    }
    void set_MR_speed(int r_speed) {
        if (r_speed > max_speed) {
          r_speed = max_speed;
        }
        else if (r_speed < min_speed) {
          r_speed = min_speed;
        }
        main_motor_right->setSpeed(r_speed);
    }

    //joint function to set speed of both motors -- verified -- test for values out of range, i would assume they would be clipped to the max/min values by default
    void set_speed(int speed) {
        if (speed > max_speed) {
          speed = max_speed;
        }
        else if (speed < min_speed) {
          speed = min_speed;
        }
        l_speed = r_speed = speed;
        set_ML_speed(l_speed);
        set_MR_speed(r_speed);
    }

    //function to change speed of both motors -- verified
    void change_speed(int delta) {
        l_speed = r_speed = speed += delta;
        if (speed > max_speed) {
            speed = max_speed;
        } else if (speed < min_speed) {
            speed = min_speed;
        }
        set_speed(speed);
    }
    void change_ML_speed(int delta) {
        l_speed += delta;
        set_ML_speed(l_speed);
    }
    void change_MR_speed(int delta) {
        r_speed += delta;
        set_MR_speed(r_speed);
    }

    //due to the way the motors are wired they may have to be reversed, -- verified for current arrangement, ie forward is forward and turns are correct
    void ML_run(int direction) {
        //motor left run
        //swap if direction is reversed
        if (direction == BACKWARD) {
            main_motor_left->run(FORWARD);
        } else if (direction == FORWARD) {
            main_motor_left->run(BACKWARD);
        }
    }

    void MR_run(int direction) {
        //motor right run
        //swap if direction is reversed
        if (direction == BACKWARD) {
            main_motor_right->run(FORWARD);
        } else if (direction == FORWARD) {
            main_motor_right->run(BACKWARD);
        }
    }
    // sends both motors forward -- verified
    void go_forward() {
        //both motors forward
        ML_run(FORWARD);
        MR_run(FORWARD);
    }
    // sends both motors backward -- verified
    void go_backward() {
        //both motors backward
        ML_run(BACKWARD);
        MR_run(BACKWARD);
    }
    //cuts power and motors come to a stop -- verified
    void stop() {
        //cuts power to both motors
        set_speed(0);
        main_motor_left->run(RELEASE);
        main_motor_right->run(RELEASE);
    }
    //test function to allow for faster stops -- unverified
    void hard_stop() {
        //cuts forward power and brakes both motors
        set_speed(0);
        main_motor_left->run(BRAKE);
        main_motor_right->run(BRAKE);
    }
    //simple turn functions -- verified
    void turn_left(int speed) {
        set_speed(speed);
        ML_run(BACKWARD);
        MR_run(FORWARD);
    }
    void turn_right(int speed) {
        set_speed(speed);
        ML_run(FORWARD);
        MR_run(BACKWARD);
    }

    void move_forward(int dist) {
        stop();
        int move_speed = 200;
        set_speed(move_speed);
        unsigned long t = ( 1000.0*dist )/( (move_speed/255.0)*max_wheel_speed );
        go_forward();
        delay(t);
        stop();
    }

    void turn_90_left(bool move = true) {
      if (move) {
        move_forward(20);
      }
      stop();
      int turn_speed = 200;
      set_MR_speed(turn_speed);
      set_ML_speed(0);
      double factor = (turn_speed/255.0)*max_wheel_angular_speed*(wheel_radius/wheel_base);
      unsigned long t = 1000.0*Pi/(2*factor);
      go_forward();
      delay(t);
      stop();
    }

    void turn_90_right(bool move = true) {
      if (move) {
        move_forward(20);
      }
      stop();
      int turn_speed = 200;
      set_ML_speed(turn_speed);
      set_MR_speed(0);
      double factor = (turn_speed/255.0)*max_wheel_angular_speed*(wheel_radius/wheel_base);
      unsigned long t = 1000.0*Pi/(2*factor);
      go_forward();
      delay(t);
      stop();
    }



};

class Navigator 
{
    // Initialize the adjacency map with proper nested map initialization
    public: std::map<int, std::map<int, int> > adjacent;

        
    // Populates the adjacency map with node connections and directions
    void populate_node_map()
    {
        // Populate the adjacency list with proper std::map initializations
        adjacent[0] = std::map<int, int>();
        adjacent[0].insert(std::make_pair(NORTH, 1));
        Serial.println("Added first values to adjacent");

        adjacent[1] = std::map<int, int>();
        adjacent[1].insert(std::make_pair(EAST, 3));
        adjacent[1].insert(std::make_pair(SOUTH, 0));
        adjacent[1].insert(std::make_pair(WEST, 11));

        adjacent[2] = std::map<int, int>();
        adjacent[2].insert(std::make_pair(NORTH, 11));

        adjacent[3] = std::map<int, int>();
        adjacent[3].insert(std::make_pair(NORTH, 5));
        adjacent[3].insert(std::make_pair(SOUTH, 4));
        adjacent[3].insert(std::make_pair(WEST, 1));

        adjacent[4] = std::map<int, int>();
        adjacent[4].insert(std::make_pair(NORTH, 3));

        adjacent[5] = std::map<int, int>();
        adjacent[5].insert(std::make_pair(NORTH, 7));
        adjacent[5].insert(std::make_pair(SOUTH, 3));
        adjacent[5].insert(std::make_pair(WEST, 6));

        adjacent[6] = std::map<int, int>();
        adjacent[6].insert(std::make_pair(NORTH, 14));
        adjacent[6].insert(std::make_pair(EAST, 5));
        adjacent[6].insert(std::make_pair(WEST, 10));
        Serial.println("Added 6th values to adjacent");

        adjacent[7] = std::map<int, int>();
        adjacent[7].insert(std::make_pair(SOUTH, 5));
        adjacent[7].insert(std::make_pair(WEST, 12));

        adjacent[8] = std::map<int, int>();
        adjacent[8].insert(std::make_pair(EAST, 12));
        adjacent[8].insert(std::make_pair(SOUTH, 14));
        adjacent[8].insert(std::make_pair(WEST, 9));

        adjacent[9] = std::map<int, int>();
        adjacent[9].insert(std::make_pair(EAST, 8));
        adjacent[9].insert(std::make_pair(SOUTH, 10));

        adjacent[10] = std::map<int, int>();
        adjacent[10].insert(std::make_pair(NORTH, 9));
        adjacent[10].insert(std::make_pair(EAST, 6));
        adjacent[10].insert(std::make_pair(SOUTH, 11));

        adjacent[11] = std::map<int, int>();
        adjacent[11].insert(std::make_pair(NORTH, 10));
        adjacent[11].insert(std::make_pair(EAST, 1));
        adjacent[11].insert(std::make_pair(SOUTH, 2));

        adjacent[12] = std::map<int, int>();
        adjacent[12].insert(std::make_pair(EAST, 7));
        adjacent[12].insert(std::make_pair(SOUTH, 13));
        adjacent[12].insert(std::make_pair(WEST, 8));

        adjacent[13] = std::map<int, int>();
        adjacent[13].insert(std::make_pair(NORTH, 12));

        adjacent[14] = std::map<int, int>();
        adjacent[14].insert(std::make_pair(NORTH, 8));
        adjacent[14].insert(std::make_pair(SOUTH, 6));
        adjacent[14].insert(std::make_pair(WEST, 15));

        adjacent[15] = std::map<int, int>();
        adjacent[15].insert(std::make_pair(EAST, 14));
        Serial.println("Added 15th values to adjacent");
    }



    // Generates a navigation path from the current node to the target node
    vector<int> generateNavigationPath(int current_node, int target_node)
    {
        int search_node = current_node; // Node currently being searched
        int depth = 0; // Current depth of the search
        int new_node; // Newly discovered node
        vector<int> current_route = {search_node}; // Current navigation route
        vector<vector<int>> possible_routes = vector<vector<int>>({current_route}); // List of possible routes to explore
        vector<int> found_nodes = {search_node}; // List of nodes that have been visited

        Serial.println("Starting navigation from node " + String(current_node) + " to node " + String(target_node));

        while(depth < 10) // Limit the search depth to prevent infinite loops
        {
            Serial.println("Depth: " + String(depth) + ", Possible routes: " + String(possible_routes.size()));

            for(int j = 0; j < possible_routes.size(); j++) // Iterate through all possible routes
            {
                current_route = possible_routes[j];
                search_node = current_route.back(); // Get the last node in the current route
                Serial.print("Extending route: ");
                for (int node : current_route) {
                    Serial.print(String(node) + " ");
                }
                Serial.println();

                // Iterate over all adjacent nodes from the current search node
                for (std::pair<int, int> direction_pair : adjacent[search_node])
                {
                    new_node = direction_pair.second; // Get the connected node
                    vector<int> new_route = current_route;
                    // Check if the new node has not been visited yet
                    if(std::find(found_nodes.begin(), found_nodes.end(), new_node) == found_nodes.end())
                    {
                        found_nodes.push_back(new_node); // Mark the new node as visited
                        new_route.push_back(new_node); // Add the new node to the current route
                        Serial.println("Found new node " + String(new_node) + " from node " + String(search_node));
                        if(new_node == target_node) // Check if the target node is reached
                        {
                            Serial.println("Target node " + String(target_node) + " reached!");
                            return new_route; // Return the completed route
                        }
                        possible_routes.push_back(new_route); // Add the new route to the list of possible routes
                    }  
                }
            }
            depth++; // Increment the search depth
        }
        Serial.println("No path found to target node " + String(target_node));
        return vector<int>(); // Return empty vector if no path is found
    }

    int generateTurnDirection(int behind_node, int junction_node, int target_node)
    {
      int current_direction = adjacent[behind_node][junction_node];
      int target_direction = adjacent[junction_node][target_node];
      return (target_direction - current_direction);
    }
    
    Navigator()
    {
        populate_node_map();
    }
};

MainMotors main_motors; //create main motors object
Navigator main_navigator;

//set to safe pins
int far_left_sensor = 4;
int left_sensor = 5; 
//int center_sensor = 5;
int right_sensor = 6;
int far_right_sensor = 7;

//initialize the states of the sensors
int fls_state = 0;
int ls_state = 0;
//int cs_state = 0;
int rs_state = 0;
int frs_state = 0;


int soft_turn_rate = 10;

//function to read data from the sensors
void read_sensors(){
    //0 is black, 1 is white
    fls_state = digitalRead(far_left_sensor);
    ls_state = digitalRead(left_sensor);
    //cs_state = 0;   // digitalRead(center_sensor);
    rs_state = digitalRead(right_sensor);
    frs_state = digitalRead(far_right_sensor);
}



void interrupt_function(){
    volatile static unsigned long last_interrupt = 0;
    unsigned long t = millis();
    if (t - last_interrupt > 500UL){
      state = !state;
    }
    last_interrupt = t;
  }

void setup() {
    Serial.begin(9600);
    Serial.println("a");
    delay(1000);
    Serial.println("b");
    pinMode(3,INPUT);
    while ( !digitalRead(3) );
    attachInterrupt(digitalPinToInterrupt(3), interrupt_function, RISING);
    if (AFMS.begin()){
      Serial.println("AFMS connected");
    }
    else{
      Serial.println("AFMS not connected");
      while(1);
    }
    main_motors.stop();
    pinMode(left_sensor, INPUT);
    //pinMode(center_sensor, INPUT);
    pinMode(right_sensor, INPUT);
    pinMode(far_left_sensor,INPUT);
    pinMode(far_right_sensor, INPUT);

    //Navigation code
    main_navigator = Navigator();
    Serial.println("Setup complete");
}




