#include "lobster.h"

// Constants for turn directions
const int STRAIGHT_ON = 0;
const int RIGHT = 1;
const int LEFT = 2;
const int SPECIAL_FROM_THE_LEFT = 3;
const int SPECIAL_FROM_THE_RIGHT = 4;
const int SPECIAL_DONE = 5;
const int TESTING = 6;
const int DROP_OFF_ANT = 7;                 // ANT MEANS After Next Turn
const int EXPECT_BLOCK_ANT = 8;             // These are a couple of methods that setup the system to perform things on paths
const int ROBOT_GO_WEE_WOO_ANT_FIRST = 9;   // These things are meant to be done during navigation but called after a junction
const int ROBOT_GO_WEE_WOO_ANT_SECOND = 10; // These things are meant to be done during navigation but called after a junction
const int COMPLETED_ANT = 11;
const int COMPLETED = 12;

// Navigation variables
int progress = 0;
int special_mode = -1;
int special_progress = 0;
long last_turn_time_failsafe = 0;
bool block_expected = true;

// CONFIGURATION VARIABLES
long min_time_between_junctions = 750;                        // Failsafe parameter
int distance_between_centre_junction_and_houses_FIRST = 150;  // Will likely need tuning, may vary between houses. Low reliability of move_forward() right now
int distance_between_centre_junction_and_houses_SECOND = 300; // Will likely need tuning, may vary between houses. Low reliability of move_forward() right now
int block_approach_speed = 180;
int default_travel_speed = 240;
int amount_to_go_forward_at_the_end = 300;
int nook_depth = 350;

// Path array defining the robot's route
int path[] = {
    // Move to the first block
    // TESTING,
    LEFT,
    // Navigate to approperiate centre and end with the second block
    RIGHT, STRAIGHT_ON, RIGHT, SPECIAL_FROM_THE_LEFT, // Whenever it goes
    // Complete this loop again ready to gather the third block
    RIGHT, RIGHT, RIGHT, SPECIAL_FROM_THE_LEFT,
    // Collect the third block and place it in the recycling centre (NB Block already expected whilst turning)
    EXPECT_BLOCK_ANT, LEFT, LEFT, LEFT, SPECIAL_FROM_THE_RIGHT,
    // Collect the fourth block
    RIGHT, LEFT, LEFT, EXPECT_BLOCK_ANT, STRAIGHT_ON,
    // Recycle the fourth block
    LEFT, STRAIGHT_ON, LEFT, SPECIAL_FROM_THE_RIGHT,
    // Handle hidden block on this road - not tested this far quite yet
    ROBOT_GO_WEE_WOO_ANT_FIRST, LEFT, LEFT, LEFT, LEFT, SPECIAL_FROM_THE_RIGHT,
    // Handle final hidden block
    LEFT, RIGHT, RIGHT, ROBOT_GO_WEE_WOO_ANT_SECOND, STRAIGHT_ON, LEFT, RIGHT, STRAIGHT_ON, RIGHT, SPECIAL_FROM_THE_LEFT,
    // End of path - this just returns it to its starting point as a show off move.
    LEFT, RIGHT, RIGHT, LEFT, COMPLETED_ANT, STRAIGHT_ON // My cravings to try and make the lobster ram into the house are unparalelled, I think it would be funny.
};

// Special paths for different modes
int special_path[][8] = {
    // Mode 0 - Going to the No Magnet, No chimney, landfill centre from the north-west
    {STRAIGHT_ON, DROP_OFF_ANT, RIGHT, RIGHT, LEFT, EXPECT_BLOCK_ANT, STRAIGHT_ON, SPECIAL_DONE},
    // Mode 1 - Going to the Magnet, recycling chimney centre from the north-west
    {RIGHT, DROP_OFF_ANT, RIGHT, EXPECT_BLOCK_ANT, LEFT, SPECIAL_DONE},
    // Mode 2 - Going to the No Magnet centre from the north-east
    {DROP_OFF_ANT, LEFT, RIGHT, LEFT, STRAIGHT_ON, SPECIAL_DONE},
    // Mode 3 - Going to the Magnet centre from the north-west
    {STRAIGHT_ON, LEFT, DROP_OFF_ANT, RIGHT, LEFT, SPECIAL_DONE}};

/**
 * @brief I am trying to make special switching automatic to reduce bugs.
 *
 * @param amount Usually either to indicate weather to go up or down.
 */
void iterate_respective_progress(int amount = 1)
{
    if (special_mode == -1)
    {
        progress = progress + amount;
    }
    else
    {
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
    if (last_turn_time_failsafe + min_time_between_junctions > millis())
    {
        iterate_respective_progress(-1);
    }

    // Failsafe for if we are past the end of the path.
    if (progress > (sizeof(path) / sizeof(path[0])))
    {
        state = HIGH;
        return COMPLETED;
    }

    if (special_mode == -1)
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
void turn_junction(int turn_direction)
{
    iterate_respective_progress(1); // This is because we will now complete this turn (or find out and undo this assumption)
    switch (turn_direction)
    {
    case STRAIGHT_ON:
        Serial.println("direction: STRAIGHT_ON");
        main_motors.move_forward(30); // Move forward through the junction.
        break;
    case RIGHT:
        Serial.println("direction: RIGHT");
        turn_right_until_line(); // Turn right at the junction
        break;
    case LEFT:
        Serial.println("direction: LEFT");
        turn_left_until_line(); // Turn left at the junction
        break;
    case TESTING:
        Serial.println("direction: TESTING");
        // Enter testing mode, this is redundant and will be removed soon.
        special_mode = 4;
        turn_junction(get_turn_direction());
        break;
    case SPECIAL_FROM_THE_LEFT:
        Serial.println("direction: SPECIAL_FTL");
        // Navigate to approperiate centre from the north-west-most corner of the map headed to the middle.
        read_magnet_sensor();
        special_mode = 0 + is_magnet;
        turn_junction(get_turn_direction());
        // Leaves the code with the next main direction as the next one and has just set off first special mode
        break;
    case SPECIAL_FROM_THE_RIGHT:
        Serial.println("direction: SPECIAL_FTR");
        // Navigate to approperiate centre from the north-east-most corner of the map headed to the middle.
        read_magnet_sensor();
        special_mode = 2 + is_magnet;
        turn_junction(get_turn_direction());
        // Leaves the code with the next main direction as the next one and has just set off first special mode
        break;
    case DROP_OFF_ANT:
        Serial.println("direction: DROP_OFF_ANT");
        // Deposit block into the centre after turning right
        turn_junction(get_turn_direction()); // Drop off needs to do this turn now
        drop_off();
        turn_junction(get_turn_direction());
        break;
    case EXPECT_BLOCK_ANT:
        Serial.println("direction: EXPECT_BLOCK_ANT");
        // This should go before the turn command for the next junction
        turn_junction(get_turn_direction());
        block_expected = true;
        break;
    case ROBOT_GO_WEE_WOO_ANT_FIRST:
        Serial.println("direction: WEE_WOO_FIRST");
        // This is the finding the block in the nooks
        turn_junction(get_turn_direction());
        move_forward_tracking(distance_between_centre_junction_and_houses_FIRST);
        Serial.println("turning right");
        main_motors.turn_90_right(); // Means it always needs to go in a specific way.
        Serial.println("turned right");
        grab_from_nook();
        turn_junction(get_turn_direction());
        break;
    case ROBOT_GO_WEE_WOO_ANT_SECOND:
        Serial.println("direction: WEE_WOO_SECOND");
        // This is the finding the block in the nooks
        turn_junction(get_turn_direction());
        move_forward_tracking(distance_between_centre_junction_and_houses_SECOND);
        main_motors.turn_90_right(); // Means it always needs to go in a specific way.
        grab_from_nook();
        turn_junction(get_turn_direction());
        break;
    case SPECIAL_DONE:
        Serial.println("direction: SPECIAL_DONE");
        Serial.println("End of special path. Resetting special mode.");
        special_mode = -1;
        special_progress = 0;
        turn_junction(get_turn_direction());
        break;
    case COMPLETED_ANT:
        Serial.println("direction: COMPLETED_ANT");
        turn_junction(get_turn_direction());
        main_motors.move_forward(amount_to_go_forward_at_the_end);
        main_motors.stop();
        while (true)
            ;
        break;
    default:
        Serial.println("direction: INVALID");
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
    line_track_forward();
    delay(75);
    Claws.open();
    main_motors.move_backward(100);
    Claws.close();
    main_motors.set_speed(180);
    main_motors.go_backward();
    // Continue moving backward until a line is detected (at which point the line state is >= 5)
    while (get_line_state() < 5)
    {
        leds.blink_async(leds.BLUE);
    }
    leds.off(leds.RED);
    leds.off(leds.GREEN);
    main_motors.stop();
    main_motors.move_forward(10);
}

/**
 * @brief Performs a grabbing maneuver to grab an off course block.
 *        The robot opens wide, moves forward, hopes and reverses.
 *        The system may benefit from further going forward.
 */
void grab_from_nook()
{
    main_motors.move_backward(75);
    Claws.open();
    main_motors.move_forward(nook_depth);
    Claws.close();
    // This needs to be tested for lower numbers
    main_motors.move_backward(50);
    block_expected = false;

    // check if we are wrong
    read_magnet_sensor(); // updates is_magnet
    main_motors.set_speed(200);
    main_motors.go_backward();
    // Continue moving backward until a line is detected (at which point the line state is >= 5)
    while (get_line_state() < 5)
    {
        leds.blink_async(leds.BLUE);
    }
    read_magnet_sensor();
    main_motors.stop();
    main_motors.move_forward(10);
}

// executes the pick_up_block() routine
void pick_up_block()
{
    main_motors.move_backward(30);
    Claws.open();
    main_motors.move_forward(70);
    Claws.close();
    main_motors.move_backward(50);
    block_expected = false;
    read_magnet_sensor();
}

void setup()
{
    IDP_setup();
    main_motors.move_forward(200);

    //sets up serial communication
    Serial.begin(9600);
    Serial.println("Startup complete");
}

/**
 * @brief Main program loop, checks the system state and executes decisions accordingly.
 */
void loop()
{
    if (state)
    {
        system_decisions();
    }
    else
    {
        main_motors.stop();
    }
}

/**
 * @brief Executes the system's decision-making process based on sensor readings.
 *        Manages line following and junction handling.
 */
void system_decisions()
{
    // Follow the line forward
    line_track_forward();

    // Block logic
    tof_block_distance = tof_sensor.getDistance(); //-20;//30
    // Pick up the block if it is sensed within threshold
    if (tof_block_distance <= 60 && block_expected)
    {
        pick_up_block();
    }

    // Check for junctions and handle them
    if (get_line_state() >= 5)
    {
        block_expected = false;
        turn_junction(get_turn_direction());
        // Once the turn junction command has completely finished we restart the timer,
        //  so if it thinks its on the line quickly after it'll get picked up on.
        last_turn_time_failsafe = millis();
    }
}