#include "IDP_lib.h"

// Constants for turn directions
const int STRAIGHT_ON             = 0;
const int RIGHT                   = 1;
const int FULL_AROUND             = 2;
const int LEFT                    = 3;
const int SPECIAL_FROM_THE_LEFT   = 4;
const int SPECIAL_FROM_THE_RIGHT  = 5;
const int STOP                    = 7;
const int RIGHT_DIP               = 8;
const int LEFT_DIP                = 9;
const int TESTING                 = 10;
const int LEGACY_LEFT             = 11;
const int LEGACY_RIGHT            = 12;

// Path array defining the robot's route
int path[] = {
    // Move to the first block
    LEFT,
    // Navigate to approperiate centre and end with the second block
    RIGHT, STRAIGHT_ON, RIGHT, SPECIAL_FROM_THE_LEFT,
    // Complete this loop again ready to gather the third block
    RIGHT, RIGHT, RIGHT, SPECIAL_FROM_THE_LEFT,
    // Collect the third block and place it in the recycling centre
    LEFT, LEFT, LEFT, SPECIAL_FROM_THE_RIGHT,
    // Collect the fourth block
    RIGHT, LEFT, LEFT, STRAIGHT_ON,
    // Recycle the fourth block
    LEFT, STRAIGHT_ON, LEFT, SPECIAL_FROM_THE_RIGHT,
    // Handle hidden block on this road - not got this far quite yet
    LEFT,
    // End of path
    STOP
};

// Special paths for different modes
int special_path[][5] = {
    // Mode 0 - Going to the No Magnet centre from the north-west
    {STRAIGHT_ON, RIGHT_DIP, RIGHT, LEFT, STRAIGHT_ON},
    // Mode 1 - Going to the Magnet centre from the north-west
    {RIGHT, RIGHT_DIP, LEFT, STOP, STOP},
    // Mode 2 - Going to the No Magnet centre from the north-east
    {LEFT_DIP, RIGHT, LEFT, STRAIGHT_ON, STOP},
    // Mode 3 - Going to the Magnet centre from the north-west
    {STRAIGHT_ON, LEFT, RIGHT_DIP, LEFT, STOP},
    // Mode 4 - Testing routine, can be changed, is redundant.
    {LEFT, LEFT_DIP, STOP, STOP, STOP},
};

// Navigation variables
int progress = 0;
int special_mode = -1;
int special_progress = 0;
long last_turn_time = 0;
long min_time_between_junctions = 100;

/**
 * @brief Performs a dipping maneuver to deliver a block into a recycling center.
 *        The robot moves forward, releases the block, then reverses back to the line.
 */
void dip()
{
    Serial.println("Move mode: DIP");
    leds.blue_blink();
    main_motors.move_forward(50);
    main_motors.move_backward(50);
    main_motors.set_speed(150);
    main_motors.go_backward();
    // Continue moving backward until a line is detected (at which point the line state is >= 5)
    while(get_line_state() < 5){
        leds.blue_blink();
    }
    leds.red_blink();
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
        case RIGHT_DIP:
            // Deposit block into the centre after turning right
            turn_right_until_line();
            dip();
            break;
        case LEFT_DIP:
            // Deposit block into the centre after turning left
            turn_left_until_line();
            dip();
            break;
        case SPECIAL_FROM_THE_LEFT:
            // Navigate to approperiate centre from the north-west-most corner of the map headed to the middle.
            special_mode = 0 + is_magnet;
            turn_junction(get_turn_direction());
            break;
        case SPECIAL_FROM_THE_RIGHT:
            // Navigate to approperiate centre from the north-east-most corner of the map headed to the middle.
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
        default:
            // Stop the robot for invalid direction
            main_motors.stop();
            break;
    }
}

/**
 * @brief Determines the next turn direction for the robot based on the predetermined navigaton paths.
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
        progress--;
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
        progress++;
        last_turn_time = millis();
    }
    else
    {
        // Use the next direction from the special path
        direction = special_path[special_mode][special_progress];
        special_progress++;
        last_turn_time = millis();

        // Reset special mode if end of special path is reached
        if(direction == STOP)
        {
            Serial.println("End of special path. Resetting special mode.");
            special_mode = -1;
            special_progress = 0;
        }
    }
    return direction;
}

/**
 * @brief Main program loop, checks the system state and executes decisions accordingly.
 */
void loop() {
    if (state) {
        Serial.println("System is running.");
        system_decisions();
    } else {
        Serial.println("System is not running.");
        main_motors.stop();
    }
}

/**
 * @brief Executes the system's decision-making process based on sensor readings.
 *        Manages line following and junction handling.
 */
void system_decisions() {
    // Follow the line forward
    line_track_forward();
    Serial.println("Executing system decisions.");

    // Block logic
    tof_block_distance = tof_sensor.getDistance()-30;
    //Pick up the block if it is sensed within threshold
    if (tof_block_distance <= 40 && has_block == false){ 
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
    main_motors.move_forward(50);
}

//executes the pick_up_block() routine
void pick_up_block(){
    main_motors.stop();
    Claws.open();
    main_motors.move_forward(80);
    Claws.close();
    //This needs to be tested for lower numbers
    main_motors.move_backward(80);  
    has_block = true;
    read_magnet_sensor();
}