#include "IDP_lib.h"

// Constants for turn directions
const int STRAIGHT_ON                   = 0;
const int RIGHT                         = 1;
const int LEFT                          = 3;
const int SPECIAL_FROM_THE_LEFT         = 4;
const int SPECIAL_FROM_THE_RIGHT        = 5;
const int STOP                          = 7;
const int RIGHT_DROP_OFF                = 8;
const int LEFT_DROP_OFF                 = 9;
const int TESTING                       = 10;
const int LEGACY_LEFT                   = 11;
const int LEGACY_RIGHT                  = 12;
const int EXPECT_BLOCK_AFTER_NEXT       = 13;
const int ROBOT_GO_WEE_WOO_AFTER_NEXT   = 14;   //I love you Bailen, pls allow it. I was laughing so so hard here.

// Navigation variables
int progress = 0;
int special_mode = -1;
int special_progress = 0;
int distance_between_centre_junction_and_houses = 330;  //Will likely need tuning, may vary between houses
long last_turn_time = 0;
long min_time_between_junctions = 2000;     //BAILEN THE FAILSAFE IS ON, TEST HERE IF YOU WANT
bool block_expected = true;
bool has_block = false;
bool at_start = true;


// Path array defining the robot's route
int path[] = {
    // Move to the first block
    //TESTING,
    LEFT,
    // Navigate to approperiate centre and end with the second block
    RIGHT, STRAIGHT_ON, RIGHT, EXPECT_BLOCK_AFTER_NEXT, SPECIAL_FROM_THE_LEFT, //Love how the first magical demonstration of EXPECT_BLOCK_AFTER_NEXT_TURN is in a special
    // Complete this loop again ready to gather the third block
    RIGHT, RIGHT, RIGHT, SPECIAL_FROM_THE_LEFT,
    // Collect the third block and place it in the recycling centre
    EXPECT_BLOCK_AFTER_NEXT, LEFT, LEFT, LEFT, SPECIAL_FROM_THE_RIGHT,
    // Collect the fourth block
    RIGHT, LEFT, LEFT, EXPECT_BLOCK_AFTER_NEXT, STRAIGHT_ON,
    // Recycle the fourth block
    LEFT, STRAIGHT_ON, LEFT, SPECIAL_FROM_THE_RIGHT,
    // Handle hidden block on this road - not tested this far quite yet
    ROBOT_GO_WEE_WOO_AFTER_NEXT, LEFT, LEFT, LEFT, LEFT, SPECIAL_FROM_THE_RIGHT,
    // Handle final hidden block
    LEFT, RIGHT, RIGHT, ROBOT_GO_WEE_WOO_AFTER_NEXT, STRAIGHT_ON, LEFT, RIGHT, STRAIGHT_ON, RIGHT, SPECIAL_FROM_THE_RIGHT,
    // End of path - this just returns it to its starting point as a show off move.
    LEFT, RIGHT, RIGHT, LEFT, STOP  //My cravings to try and make the lobster ram into the house are unparalelled, I think it would be funny.
};

// Special paths for different modes
int special_path[][5] = {
    // Mode 0 - Going to the No Magnet centre from the north-west
    {STRAIGHT_ON, RIGHT_DROP_OFF, RIGHT, LEFT, STRAIGHT_ON},
    // Mode 1 - Going to the Magnet centre from the north-west
    {RIGHT, RIGHT_DROP_OFF, LEFT, STOP, STOP},
    // Mode 2 - Going to the No Magnet centre from the north-east
    {LEFT_DROP_OFF, RIGHT, LEFT, STRAIGHT_ON, STOP},
    // Mode 3 - Going to the Magnet centre from the north-west
    {STRAIGHT_ON, LEFT, RIGHT_DROP_OFF, LEFT, STOP},
    // Mode 4 - Testing routine, can be changed, is redundant. Prepared to test house wee_wooing
    {LEFT, LEFT_DROP_OFF, LEFT, STOP, STOP},
};

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
        leds.blue_blink();
    }
    Serial.println("Line detected, stopping and closing claws.");
    //Claws.close();
    leds.red_blink();
    main_motors.stop();
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
    leds.red_on();  //idk what red means, might be being naughty for doing this but i want red.
    pick_up_block();
    main_motors.go_backward();
    // Continue moving backward until a line is detected (at which point the line state is >= 5)
    while(get_line_state() < 5){
        leds.blue_blink();
    }
    leds.red_blink();
    read_magnet_sensor();   //LEDS must be updated
    main_motors.stop();
    main_motors.move_forward(10);
}

/**
 * @brief Handles the robot's movement at junctions based on the given turn direction.
 *
 * @param turn_direction The direction the robot should turn at the junction, obtained from get_turn_direction()
 */
void turn_junction(int turn_direction) {
    Serial.print("Junction turn_direction: ");
    Serial.println(turn_direction);
    switch (turn_direction) {
        case STRAIGHT_ON:
            // Move forward through the junction, ideally this wouldn't be so blind.
            move_forward_tracking(30);
            break;
        case RIGHT:
            // Turn right at the junction
            turn_right_until_line();
            break;
        case LEFT:
            // Turn left at the junction
            turn_left_until_line();
            break;
        case RIGHT_DROP_OFF:
            // Deposit block into the centre after turning right
            turn_right_until_line();
            drop_off();
            break;
        case LEFT_DROP_OFF:
            // Deposit block into the centre after turning left
            turn_left_until_line();
            drop_off();
            break;
        case SPECIAL_FROM_THE_LEFT:
            // Navigate to approperiate centre from the north-west-most corner of the map headed to the middle.
            read_magnet_sensor();
            special_mode = 0 + is_magnet;
            turn_junction(get_turn_direction());
            break;
        case SPECIAL_FROM_THE_RIGHT:
            // Navigate to approperiate centre from the north-east-most corner of the map headed to the middle.
            read_magnet_sensor();
            special_mode = 2 + is_magnet;
            turn_junction(get_turn_direction());
            break;
        case TESTING:
            // Enter testing mode, this is redundant and will be removed soon.
            special_mode = 4;
            turn_junction(get_turn_direction());
            break;
        case LEGACY_LEFT:
            // Legacy turn left function
            main_motors.turn_90_left();
            break;
        case LEGACY_RIGHT:
            // Legacy turn right function
            main_motors.turn_90_right();
            break;
        case EXPECT_BLOCK_AFTER_NEXT:
            //This should go before the turn command for the next junction
            block_expected = true;
            last_turn_time = 0;
            turn_junction(get_turn_direction());
            break;
        case ROBOT_GO_WEE_WOO_AFTER_NEXT:
            //This is the finding the block in the nooks
            turn_junction(get_turn_direction());
            move_forward_tracking(distance_between_centre_junction_and_houses);
            main_motors.turn_90_right();    //Means it always needs to go in a specific way.
            grab_from_nook();
            break;
        default:
            // Stop the robot for invalid direction
            main_motors.stop();
            break;
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
    Serial.println("Getting turn direction...");
    Serial.print("Last turn time: ");
    Serial.println(last_turn_time);

    // Failsafe for if we are trying to turn the same junction twice.
    if(last_turn_time + min_time_between_junctions > millis())
    {
        Serial.println("Failsafe: Trying to turn the same junction twice.");
        if(special_mode == -1)
        {
            progress--;
        }
        else
        {
            special_progress--;
        }
    }

    // Failsafe for if we are past the end of the path.
    if(progress > (sizeof(path)/sizeof(path[0])))
    {
        Serial.println("End of path reached. Stopping.");
        state = HIGH;
        return STOP;
    }

    if(special_mode == -1)
    {
        // Use the next direction from the main path
        direction = path[progress];
        Serial.print("Main path direction: ");
        Serial.println(direction);
        progress++;
        last_turn_time = millis();
    }
    else
    {
        // Use the next direction from the special path
        direction = special_path[special_mode][special_progress];
        Serial.print("Special path direction: ");
        Serial.println(direction);
        special_progress++;
        last_turn_time = millis();

        // Reset special mode if end of special path is reached
        if(direction == STOP)
        {
            Serial.println("End of special path. Resetting special mode.");
            special_mode = -1;
            special_progress = 0;
            last_turn_time = 0;
            turn_junction(get_turn_direction());
        }
    }
    return direction;
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
    if (at_start){
        line_track_forward(150);
    } else{
    line_track_forward();
    }
    //Block logic
    tof_block_distance = tof_sensor.getDistance();//-20;//30
    //Pick up the block if it is sensed within threshold
    if (tof_block_distance <= 60 && has_block == false && block_expected)//40 //not sure how i feel about has_block, i suppose it prevents walls?
    { 
        pick_up_block();
    }

    // Check for junctions and handle them
    if (get_line_state() >= 5) {
       Serial.println("At junction.");
       turn_junction(get_turn_direction());
    }
}

void setup()
{
    IDP_setup();
    //leds.blue_blink();
    Serial.println("moving forward");
    main_motors.move_forward(200);
    Serial.println("done");
}

//executes the pick_up_block() routine
void pick_up_block(){
    main_motors.move_backward(10);
    Claws.open();
    main_motors.move_forward(100);
    Claws.close();
    //This needs to be tested for lower numbers
    main_motors.move_backward(50);  
    has_block = true;
    at_start = false;
    block_expected = false;
    read_magnet_sensor(); //updates is_magnet
}