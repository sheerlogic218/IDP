#include "IDP_lib.h"

// Constants for turn directions
const int STRAIGHT_ON                   = 0;
const int RIGHT                         = 1;
const int LEFT                          = 2;
const int SPECIAL_FROM_THE_LEFT         = 3;
const int SPECIAL_FROM_THE_RIGHT        = 4;
const int SPECIAL_DONE                  = 5;
const int TESTING                       = 6;
const int DROP_OFF_ANT                  = 7;   //ANT MEANS After Next Turn
const int EXPECT_BLOCK_ANT              = 8;   //These are a couple of methods that setup the system to perform things on paths
const int ROBOT_GO_WEE_WOO_ANT          = 9;   //These things are meant to be done during navigation but called after a junction
const int COMPLETED_ANT                 = 10;
const int COMPLETED                     = 11;

// Navigation variables
int progress = 0;
int special_mode = -1;
int special_progress = 0;
long last_turn_time_failsafe = 0;
bool block_expected = true;                 
bool has_block = false;

//CONFIGURATION VARIABLES
long min_time_between_junctions = 750;                 //Failsafe parameter
int distance_between_centre_junction_and_houses = 150;  //Will likely need tuning, may vary between houses. Low reliability of move_forward() right now
int block_approach_speed = 130;
int default_travel_speed = 220;

// Path array defining the robot's route
int path[] = {
    // Move to the first block
    //TESTING,
    LEFT,
    // Navigate to approperiate centre and end with the second block
    RIGHT, STRAIGHT_ON, RIGHT, SPECIAL_FROM_THE_LEFT, //Whenever it goes
    // Complete this loop again ready to gather the third block
    RIGHT, RIGHT, RIGHT, SPECIAL_FROM_THE_LEFT,
    // Collect the third block and place it in the recycling centre (NB Block already expected whilst turning)
    EXPECT_BLOCK_ANT, LEFT, LEFT, LEFT, SPECIAL_FROM_THE_RIGHT,
    // Collect the fourth block
    RIGHT, LEFT, LEFT, EXPECT_BLOCK_ANT, STRAIGHT_ON,
    // Recycle the fourth block
    LEFT, STRAIGHT_ON, LEFT, SPECIAL_FROM_THE_RIGHT,
    // Handle hidden block on this road - not tested this far quite yet
    ROBOT_GO_WEE_WOO_ANT, LEFT, LEFT, LEFT, LEFT, SPECIAL_FROM_THE_RIGHT,
    // Handle final hidden block
    LEFT, RIGHT, RIGHT, ROBOT_GO_WEE_WOO_ANT, STRAIGHT_ON, LEFT, RIGHT, STRAIGHT_ON, RIGHT, SPECIAL_FROM_THE_LEFT,
    // End of path - this just returns it to its starting point as a show off move.
    LEFT, RIGHT, RIGHT, LEFT, COMPLETED_ANT ,STRAIGHT_ON  //My cravings to try and make the lobster ram into the house are unparalelled, I think it would be funny.
};

// Special paths for different modes
int special_path[][8] = {
    // Mode 0 - Going to the No Magnet, No chimney, landfill centre from the north-west
    {STRAIGHT_ON, DROP_OFF_ANT ,RIGHT, RIGHT, LEFT, EXPECT_BLOCK_ANT, STRAIGHT_ON, SPECIAL_DONE},
    // Mode 1 - Going to the Magnet, recycling chimney centre from the north-west
    {RIGHT, DROP_OFF_ANT, RIGHT , LEFT, SPECIAL_DONE},
    // Mode 2 - Going to the No Magnet centre from the north-east
    {DROP_OFF_ANT, LEFT, RIGHT, LEFT, STRAIGHT_ON, SPECIAL_DONE},
    // Mode 3 - Going to the Magnet centre from the north-west
    {STRAIGHT_ON, LEFT, DROP_OFF_ANT, RIGHT, LEFT, SPECIAL_DONE},
    // Mode 4 - Testing routine, can be changed, is redundant. Prepared to test house wee_wooing
    {LEFT, DROP_OFF_ANT, LEFT, LEFT, SPECIAL_DONE}
};

/**
 * @brief I am trying to make special switching automatic to reduce bugs.
 *
 * @param amount Usually either to indicate weather to go up or down.
 */
void iterate_respective_progress(int amount = 1)
{
    if(special_mode == -1)
    {
        Serial.print("Current navigation progress (main): ");
        Serial.println(progress);
        progress = progress + amount;
    }
    else
    {
        Serial.print("Current special progress: ");
        Serial.println(special_progress);
        Serial.print("Current special mode: ");
        Serial.println(special_mode);
        special_progress = special_progress + amount;
    }
}

/**
 * @brief Determines the next turn direction for the robot based on the predetermined navigation paths.
 *
 * @return The value corresponding to the turn the robot will now perform.
 */
int get_turn_direction()
{
    int direction;
    // Failsafe for if we are trying to turn the same junction twice.
    if(last_turn_time_failsafe + min_time_between_junctions > millis())
    {
        Serial.println("ERROR: Rapid re-junctioning detected, repeating last junction.");
        Serial.print("Last turn time: ");
        Serial.println(last_turn_time_failsafe);
        iterate_respective_progress(-1);
    }

    // Failsafe for if we are past the end of the path.
    if(progress > (sizeof(path)/sizeof(path[0])))
    {
        Serial.println("End of path reached. Stopping.");
        state = HIGH;
        return COMPLETED;
    }

    if(special_mode == -1)
    {
        // Use the next direction from the main path
        direction = path[progress];   
    }
    else
    {
        // Use the next direction from the special path
        direction = special_path[special_mode][special_progress];
        
    }
    return direction;
}

/**
 * @brief Handles the robot's movement at junctions based on the given turn direction.
 *
 * @param turn_direction The direction the robot should turn at the junction, obtained from get_turn_direction()
 */
void turn_junction(int turn_direction) {
    iterate_respective_progress(1);     //This is because we will now complete this turn (or find out and undo this assumption)
    Serial.print("Block expected: ");
    Serial.println(block_expected);
    switch (turn_direction) {
        case STRAIGHT_ON:
            Serial.println("Turn direction: STRAIGHT_ON");
            move_forward_tracking(30);            // Move forward through the junction.
            break;
        case RIGHT:
            Serial.println("Turn direction: RIGHT");
            turn_right_until_line();              // Turn right at the junction
            break;
        case LEFT:
            Serial.println("Turn direction: LEFT");
            turn_left_until_line();               // Turn left at the junction
            break;
        case TESTING:
            Serial.println("Turn direction: TESTING");
            // Enter testing mode, this is redundant and will be removed soon.
            special_mode = 4;
            turn_junction(get_turn_direction());
            break;
        case SPECIAL_FROM_THE_LEFT:
            Serial.println("Turn direction: SPECIAL_FROM_THE_LEFT");
            // Navigate to approperiate centre from the north-west-most corner of the map headed to the middle.
            read_magnet_sensor();
            special_mode = 0 + is_magnet;
            turn_junction(get_turn_direction());
            //Leaves the code with the next main direction as the next one and has just set off first special mode
            break;
        case SPECIAL_FROM_THE_RIGHT:
            Serial.println("Turn direction: SPECIAL_FROM_THE_RIGHT");
            // Navigate to approperiate centre from the north-east-most corner of the map headed to the middle.
            read_magnet_sensor();
            special_mode = 2 + is_magnet;
            turn_junction(get_turn_direction());
            //Leaves the code with the next main direction as the next one and has just set off first special mode
            break;
        case DROP_OFF_ANT:
            Serial.println("Turn direction: DROP_OFF_ANT");
            // Deposit block into the centre after turning right
            turn_junction(get_turn_direction());    //Drop off needs to do this turn now
            drop_off();
            turn_junction(get_turn_direction());
            break;
        case EXPECT_BLOCK_ANT:
            Serial.println("Turn direction: EXPECT_BLOCK_ANT");
            //This should go before the turn command for the next junction
            turn_junction(get_turn_direction());
            block_expected = true;
            break;
        case ROBOT_GO_WEE_WOO_ANT:
            Serial.println("Turn direction: ROBOT_GO_WEE_WOO_ANT");
            //This is the finding the block in the nooks
            turn_junction(get_turn_direction());
            move_forward_tracking(distance_between_centre_junction_and_houses);
            main_motors.turn_90_right();    //Means it always needs to go in a specific way.
            grab_from_nook();
            turn_junction(get_turn_direction());
            break;
        case SPECIAL_DONE:
            Serial.println("Turn direction: SPECIAL_DONE");
            Serial.println("End of special path. Resetting special mode.");
            special_mode = -1;
            special_progress = 0;
            turn_junction(get_turn_direction());
            break;
        case COMPLETED_ANT:
            Serial.println("Turn direction: COMPLETED_ANT");
            turn_junction(get_turn_direction());
            main_motors.stop();
            while(true);
            break;
        default:
            Serial.println("Turn direction: INVALID");
            // Stop the robot for invalid direction
            main_motors.stop();
            break;
    }
}

/**
 * @brief Performs a dropping off maneuver to deliver and deposit a block into a recycling center.
 *        The robot moves forward, releases the block, then reverses back to the line.
 */
void drop_off()
{
    Serial.println("Move mode: Drop Off Block");
    leds.blue_blink();
    Serial.println("Moving forward to drop off block.");
    move_forward_tracking(50);
    delay(50);
    Serial.println("Opening claws to release block.");
    Claws.open();

    Serial.println("Moving backward after releasing block.");
    main_motors.move_backward(100);
    Claws.close();
    // Can't track backwards due to sensor positioning.
    main_motors.set_speed(180);
    Serial.println("Continuing to move backward until line is detected.");
    main_motors.go_backward();
    // Continue moving backward until a line is detected (at which point the line state is >= 5)
    while(get_line_state() < 5){
        //leds.blue_blink();
    }
    Serial.println("Line detected, stopping and closing claws.");
    //Claws.close();
    leds.red_blink();
    main_motors.stop();
    main_motors.move_forward(10);
    Serial.println("Drop off complete.");
}

/**
 * @brief Performs a grabbing maneuver to grab an off course block.
 *        The robot opens wide, moves forward, hopes and reverses.
 *        The system may benefit from further going forward.
 */
void grab_from_nook()
{
    Serial.println("Move mode: Praying grab from the nook works");
    //leds.red_on();  //idk what red means, might be being naughty for doing this but i want red.
    pick_up_block();
    main_motors.go_backward();
    // Continue moving backward until a line is detected (at which point the line state is >= 5)
    while(get_line_state() < 5){
        leds.blue_blink_async();    //If there is a bug here its async
    }
    //leds.red_blink();
    read_magnet_sensor();   //LEDS must be updated
    main_motors.stop();
    main_motors.move_forward(10);
}

//executes the pick_up_block() routine
void pick_up_block(){
    main_motors.move_backward(10);
    Claws.open();
    main_motors.move_forward(200);
    Claws.close();
    //This needs to be tested for lower numbers
    main_motors.move_backward(50);  
    has_block = true;
    block_expected = false;
    //assume no magnet
    is_magnet = false;
    leds.red_on();
    leds.green_off();
    is_magnet = false;

    //check if we are wrong
    read_magnet_sensor(); //updates is_magnet
}

void setup()
{
    IDP_setup();
    //leds.blue_blink();
    Serial.println("moving forward");
    // main_motors.move_forward(300);
    Serial.println("done");
}

/**
 * @brief Main program loop, checks the system state and executes decisions accordingly.
 */
void loop() {
    if (state) {
        system_decisions();
    } else {
        main_motors.stop();
    }
}

/**
 * @brief Executes the system's decision-making process based on sensor readings.
 *        Manages line following and junction handling.
 */
void system_decisions() {
    // Follow the line forward
    if (block_expected){
        line_track_forward(block_approach_speed);
    } else{
        line_track_forward(default_travel_speed);
    }

    //Block logic
    tof_block_distance = tof_sensor.getDistance();//-20;//30
    //Pick up the block if it is sensed within threshold
    //  has_block == false && block_expected SHOULD always == block_expected
    if (tof_block_distance <= 60 && block_expected)//40 //not sure how i feel about has_block, i suppose it prevents walls?
    { 
        pick_up_block();
    }

    // Check for junctions and handle them
    if (get_line_state() >= 5) {
        turn_junction(get_turn_direction());
        //Once the turn junction command has completely finished we restart the timer,
        // so if it thinks its on the line quickly after it'll get picked up on.
        last_turn_time_failsafe = millis();
    }
}