#include "IDP_lib.h"

// Constants for turn directions
const int STRAIGHT_ON = 0;
const int RIGHT = 1;
const int LEFT = 2;
const int LEFT_180 = 3;
const int SPECIAL_FROM_THE_LEFT = 4;
const int SPECIAL_FROM_THE_RIGHT = 5;
const int SPECIAL_FROM_THE_MIDDLE = 6;
const int SPECIAL_DONE = 7;
const int TESTING = 14;
const int DROP_OFF_ANT = 8;                  // ANT MEANS After Next Turn
const int EXPECT_BLOCK_ANT = 9;              // These are a couple of methods that setup the system to perform things on paths
const int ROBOT_GO_WEE_WOO_ANT_FIRST = 10;   // These things are meant to be done during navigation but called after a junction
const int ROBOT_GO_WEE_WOO_ANT_SECOND = 11;  // These things are meant to be done during navigation but called after a junction
const int COMPLETED_ANT = 12;
const int COMPLETED = 13;

// Constants for LED order
const int RED = 0;
const int GREEN = 1;
const int BLUE = 2;

// Navigation variables
int progress = 0;
int special_mode = -1;
int special_progress = 0;
unsigned long last_turn_time_failsafe = 0;
bool block_expected = true;

// CONFIGURATION VARIABLES
unsigned long min_time_between_junctions = 750;                // Failsafe parameter
int distance_between_centre_junction_and_houses_FIRST = 220;   // Will likely need tuning, may vary between houses. Low reliability of move_forward() right now
int distance_between_centre_junction_and_houses_SECOND = 350;  // Will likely need tuning, may vary between houses. Low reliability of move_forward() right now
int block_approach_speed = 180;
int default_travel_speed = 400;
int amount_to_go_forward_at_the_end = 300;
int nook_depth = 250;

// Path array defining the robot's route
int path[] = {
  // Move to the first block
  // TESTING,
  LEFT,
  // Navigate to approperiate centre and end with the second block
  RIGHT, STRAIGHT_ON, RIGHT, SPECIAL_FROM_THE_LEFT,  // Whenever it goes
  // Complete this loop again ready to gather the third block
  LEFT_180, SPECIAL_FROM_THE_MIDDLE,
  // Collect the third block and place it in the recycling centre (NB Block already expected whilst turning)
  EXPECT_BLOCK_ANT, LEFT, LEFT, LEFT, SPECIAL_FROM_THE_RIGHT,
  // Collect the fourth block
  RIGHT, LEFT, LEFT, EXPECT_BLOCK_ANT, STRAIGHT_ON,
  // Recycle the fourth block
  LEFT, STRAIGHT_ON, LEFT, SPECIAL_FROM_THE_RIGHT,
  // Handle hidden block on this road - not tested this far quite yet
  ROBOT_GO_WEE_WOO_ANT_FIRST, LEFT, RIGHT, RIGHT, SPECIAL_FROM_THE_MIDDLE,
  // Go home
  RIGHT, LEFT, LEFT, RIGHT, COMPLETED_ANT, STRAIGHT_ON
};


//   // Handle final hidden block
//   RIGHT, LEFT, ROBOT_GO_WEE_WOO_ANT_SECOND, LEFT, LEFT, RIGHT, STRAIGHT_ON, RIGHT, SPECIAL_FROM_THE_LEFT,
//   // End of path - this just returns it to its starting point as a show off move.
//   LEFT, RIGHT, RIGHT, LEFT, COMPLETED_ANT, STRAIGHT_ON  // My cravings to try and make the lobster ram into the house are unparalelled, I think it would be funny.
// };

// Special paths for different modes
int special_path[][8] = {
  // Mode 0 - Going to the No Magnet, No chimney, landfill centre from the north-west
  { STRAIGHT_ON, DROP_OFF_ANT, RIGHT, RIGHT, LEFT, EXPECT_BLOCK_ANT, STRAIGHT_ON, SPECIAL_DONE },
  // Mode 1 - Going to the Magnet, recycling chimney centre from the north-west
  { RIGHT, DROP_OFF_ANT, RIGHT, EXPECT_BLOCK_ANT, LEFT, SPECIAL_DONE },
  // Mode 2 - Going to the No Magnet centre from the north-east
  { DROP_OFF_ANT, LEFT, RIGHT, LEFT, STRAIGHT_ON, SPECIAL_DONE },
  // Mode 3 - Going to the Magnet centre from the north-west
  { STRAIGHT_ON, LEFT, DROP_OFF_ANT, RIGHT, LEFT, SPECIAL_DONE },
  // Mode 4 - Special from the middle, no magnet
  { STRAIGHT_ON, RIGHT, DROP_OFF_ANT, RIGHT, RIGHT, LEFT, STRAIGHT_ON, SPECIAL_DONE },
  // Mode 5 - Special from the middle, yes magnet
  { DROP_OFF_ANT, LEFT, LEFT, SPECIAL_DONE },
  // Mode 6 - Testing routine, can be changed, is redundant. Prepared to test house wee_wooing
  { ROBOT_GO_WEE_WOO_ANT_SECOND, LEFT, LEFT, LEFT, SPECIAL_DONE }
};

/**
 * @brief I am trying to make special switching automatic to reduce bugs.
 *
 * @param amount Usually either to indicate weather to go up or down.
 */
void iterate_respective_progress(int amount = 1) {
  if (special_mode == -1) {
    progress = progress + amount;
  } else {
    special_progress = special_progress + amount;
  }
}

/**
 * @brief Determines the next turn direction for the robot based on the predetermined navigation paths.
 *
 * @return The value corresponding to the turn the robot will now perform.
 */
int get_turn_direction() {
  int direction;
  // Failsafe for if we are trying to turn the same junction twice.
  if (last_turn_time_failsafe + min_time_between_junctions > millis()) {
    iterate_respective_progress(-1);
  }

  // Failsafe for if we are past the end of the path.
  if (progress > (sizeof(path) / sizeof(path[0]))) {
    state = HIGH;  //
    return COMPLETED;
  }

  if (special_mode == -1) {
    // Use the next direction from the main path
    direction = path[progress];
  } else {
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
  iterate_respective_progress(1);  // This is because we will now complete this turn (or find out and undo this assumption)
  switch (turn_direction) {
    case STRAIGHT_ON:
      break;
    case RIGHT:
      // Serial.println("Turn direction: RIGHT");
      turn_right_until_line();  // Turn right at the junction
      break;
    case LEFT:
      Serial.println("Turn direction: LEFT");
      turn_left_until_line();
      break;
    case LEFT_180:
      Serial.println("Turn direction: LEFT");
      turn_left_until_line();  // Turn left at the junction
      main_motors.set_speed(180);
      main_motors.go_backward();
      // Continue moving backward until a line is detected (at which point the line state is >= 5)
      while (get_line_state() < 5) {
        leds.blue_blink_async();
      }
      turn_left_until_line();  // Turn left at the junction
      break;
    case TESTING:
      // Serial.println("Turn direction: TESTING");
      // Enter testing mode, this is redundant and will be removed soon.
      special_mode = 6;
      turn_junction(get_turn_direction());
      break;
    case SPECIAL_FROM_THE_LEFT:
      // Serial.println("Turn direction: SPECIAL_FROM_THE_LEFT");
      // Navigate to approperiate centre from the north-west-most corner of the map headed to the middle.
      special_mode = 0 + is_magnet;
      turn_junction(get_turn_direction());
      // Leaves the code with the next main direction as the next one and has just set off first special mode
      break;
    case SPECIAL_FROM_THE_RIGHT:
      // Serial.println("Turn direction: SPECIAL_FROM_THE_RIGHT");
      // Navigate to approperiate centre from the north-east-most corner of the map headed to the middle.
      special_mode = 2 + is_magnet;
      turn_junction(get_turn_direction());
      // Leaves the code with the next main direction as the next one and has just set off first special mode
      break;
    case SPECIAL_FROM_THE_MIDDLE:
      // Serial.println("Turn direction: SPECIAL_FROM_THE_RIGHT");
      // Navigate to approperiate centre from the north-east-most corner of the map headed to the middle.
      special_mode = 4 + is_magnet;
      turn_junction(get_turn_direction());
      // Leaves the code with the next main direction as the next one and has just set off first special mode
      break;
    case DROP_OFF_ANT:
      // Serial.println("Turn direction: DROP_OFF_ANT");
      // Deposit block into the centre after turning right
      turn_junction(get_turn_direction());  // Drop off needs to do this turn now
      drop_off();
      turn_junction(get_turn_direction());
      break;
    case EXPECT_BLOCK_ANT:
      // Serial.println("Turn direction: EXPECT_BLOCK_ANT");
      // This should go before the turn command for the next junction
      turn_junction(get_turn_direction());
      block_expected = true;
      break;
    case ROBOT_GO_WEE_WOO_ANT_FIRST:
      // Serial.println("Turn direction: ROBOT_GO_WEE_WOO_ANT");
      // This is the finding the block in the nooks
      turn_junction(get_turn_direction());
      move_forward_tracking(distance_between_centre_junction_and_houses_FIRST);
      // Serial.println("turning right");
      main_motors.turn_90_right();  // Means it always needs to go in a specific way.
      // Serial.println("turned right");
      grab_from_nook();
      turn_junction(get_turn_direction());
      break;
    case ROBOT_GO_WEE_WOO_ANT_SECOND:
      // Serial.println("Turn direction: ROBOT_GO_WEE_WOO_ANT");
      // This is the finding the block in the nooks
      turn_junction(get_turn_direction());
      move_forward_tracking(distance_between_centre_junction_and_houses_SECOND);
      main_motors.turn_90_left();  // Means it always needs to go in a specific way.
      grab_from_nook();
      turn_junction(get_turn_direction());
      break;
    case SPECIAL_DONE:
      // Serial.println("Turn direction: SPECIAL_DONE");
      // Serial.println("End of special path. Resetting special mode.");
      special_mode = -1;
      special_progress = 0;
      turn_junction(get_turn_direction());
      break;
    case COMPLETED_ANT:
      // Serial.println("Turn direction: COMPLETED_ANT");
      turn_junction(get_turn_direction());
      main_motors.move_forward(amount_to_go_forward_at_the_end);
      main_motors.stop();
      while (true)
        ;
      break;
    default:
      // Serial.println("Turn direction: INVALID");
      // Stop the robot for invalid direction
      main_motors.stop();
      break;
  }
}

/**
 * @brief Performs a dropping off maneuver to deliver and deposit a block into a recycling center.
 *        The robot moves forward, releases the block, then reverses back to the line.
 */
void drop_off() {
  leds.blue_blink();
  unsigned long drop_start = millis();
  Serial.println(drop_start);
  while (millis() < drop_start + 1100UL) {
    Serial.println(millis());
    line_track_forward();
  }
  //main_motors.move_forward(70);
  // main_motors.go_forward();
  // delay(300);
  // main_motors.stop();
  Claws.power_on();
  main_motors.stop();
  Claws.open();
  delay(100);
  main_motors.move_backward(150);
  Claws.close();
  // Can't track backwards due to sensor positioning.
  main_motors.set_speed(180);
  Serial.println("Continuing to move backward until line is detected.");
  main_motors.go_backward();
  // Continue moving backward until a line is detected (at which point the line state is >= 5)
  while (get_line_state() < 5) {
    leds.blue_blink_async();
  }
  Claws.power_off();
  Serial.println("Line detected, stopping and closing claws.");
  // Claws.close();
  leds.red_blink();
  main_motors.stop();
  is_magnet = false;
  //main_motors.move_forward(10);
  Serial.println("Drop off complete.");
}

/**
 * @brief Performs a grabbing maneuver to grab an off course block.
 *        The robot opens wide, moves forward, hopes and reverses.
 *        The system may benefit from further going forward.
 */
void grab_from_nook() {
  Serial.println("Move mode: Praying grab from the nook works");
  // leds.red_on();  //idk what red means, might be being naughty for doing this but i want red.
  Claws.power_on();
  main_motors.move_backward(75);
  Claws.open();
  main_motors.move_forward(nook_depth);
  Claws.close();
  // This needs to be tested for lower numbers
  main_motors.move_backward(40);
  Claws.power_off();
  block_expected = false;
  // assume no magnet
  is_magnet = false;
  leds.red_off();
  leds.green_on();

  // check if we are wrong
  read_magnet_sensor();  // updates is_magnet
  // ZAC ILL NEED TO TEST HERE FOR WHY IT ISN'T REVERSING
  main_motors.set_speed(200);
  main_motors.go_backward();
  // Continue moving backward until a line is detected (at which point the line state is >= 5)
  while (get_line_state() < 5) {
    main_motors.go_backward();
    leds.blue_blink_async();  // If there is a bug here its async
  }
  read_magnet_sensor();  // LEDS must be updated
  main_motors.stop();
  main_motors.move_forward(10);
}

// executes the pick_up_block() routine
void pick_up_block() {
  Claws.power_on();
  main_motors.move_backward(40);
  Claws.open();
  main_motors.move_forward(110);
  Claws.close();
  Claws.power_off();
  // This needs to be tested for lower numbers
  main_motors.move_backward(60);
  block_expected = false;
  // assume no magnet
  //  is_magnet = false;
  //  leds.red_off();
  //  leds.green_on();

  // check if we are wrong

  read_magnet_sensor();  // updates is_magnet
  Serial.print("Magnet detected: ");
  Serial.println(is_magnet);
  Claws.power_off();
}

void setup()
{
    IDP_setup();
    // leds.blue_blink();
    Serial.println("moving forward");
    main_motors.move_forward(150);
    Serial.println("done");
    // while(get_line_state() != 1)
    // {
    //   turn_left_until_line();
    // }
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
  if (block_expected) {
    line_track_forward(block_approach_speed);
  } else {
    line_track_forward(default_travel_speed);
  }

  // Block logic
  tof_block_distance = tof_sensor.getDistance();  //-20;//30
  // Pick up the block if it is sensed within threshold
  //   has_block == false && block_expected SHOULD always == block_expected
  if (tof_block_distance <= 70 && block_expected)  // 40 //not sure how i feel about has_block, i suppose it prevents walls?
  {
    pick_up_block();
  }

  // Check for junctions and handle them
  if (get_line_state() >= 5) {
    turn_junction(get_turn_direction());
    // Once the turn junction command has completely finished we restart the timer,
    //  so if it thinks its on the line quickly after it'll get picked up on.
    last_turn_time_failsafe = millis();
  }
}