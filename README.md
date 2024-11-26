# IDP Project Documentation

## Introduction

This project involves programming a lobster-shaped autonomous vehicle designed to navigate a model village, pick up blocks, and deposit them in the appropriate recycling centers based on their recyclability. The vehicle uses sensors and actuators to perform line following, object detection, block manipulation, and decision-making.

## Overview

The system is implemented using two main files:

- **IDP_lib.h**: Contains class definitions, global variables, and function declarations for controlling the robot's hardware components and sensors.
- **system.ino**: Contains the main logic and control flow for the robot's navigation and block handling tasks.

## File Structure

### IDP_lib.h

This file includes necessary libraries and defines key components:

- **Libraries**:
    - Motor control (Adafruit Motor Shield)
    - IMU sensors (LSM6DS3)
    - Servo control
    - Distance sensors (DFRobot_VL53L0X)

- **Sensors and Actuators**:
    - Hall effect sensors for magnet detection
    - Line sensors for navigation
    - Time-of-Flight distance sensor for block detection
    - DC motors for movement
    - Servos for claw control
    - LEDs for status indication

- **Classes**:
    - `MainMotors`: Controls the main drive motors, including speed and directional control.
    - `Servo_claws`: Manages the servo-operated claws for picking up and releasing blocks.
    - `LED_indicator`: Controls the status LEDs.

- **Functions**:
    - Sensor reading functions
    - Line tracking functions
    - Movement and turning functions

### system.ino

This file contains the main program logic:

- **Constants**:
    - Turn direction definitions for navigation
    - Path arrays defining the robot's route

- **Variables**:
    - Navigation progress indicators
    - Special mode flags for handling different scenarios

- **Functions**:
    - `drop_off()`: Handles the block dropping mechanism at recycling centers.
    - `grab_from_nook()`: Manages block collection from specific positions.
    - `turn_junction()`: Determines robot movement at junctions.
    - `get_turn_direction()`: Retrieves the next movement direction based on the path.
    - `system_decisions()`: Core decision-making process based on sensor inputs.
    - `loop()`: Main program loop that calls decision functions.
    - `setup()`: Initializes hardware components and configurations.
    - `pick_up_block()`: Executes the block pickup routine.

## Dependencies

Ensure the following libraries are included in the project:

- Adafruit Motor Shield library
- Arduino_LSM6DS3 library
- Servo library
- Wire library
- DFRobot_VL53L0X library

## Setup and Installation

1. Install the required libraries.
2. Connect the hardware components according to their respective pins defined in `IDP_lib.h`.
3. Upload `system.ino` to the microcontroller, ensuring `IDP_lib.h` is included in the project directory.

## Usage

- The robot follows a predefined path to navigate the model village.
- It uses line sensors to stay on track and detect junctions.
- Magnetic sensors determine if a block is recyclable.
- The robot picks up blocks using servo-operated claws.
- Blocks are deposited in the appropriate recycling centers based on sensor readings.

## Code Structure

- **Navigation**: Implemented using line tracking and a path array defining the sequence of moves.
- **Block Handling**: The robot picks up blocks when detected and determines recyclability.
- **Decision Making**: Sensor inputs inform movement decisions.
- **Actuator Control**: Motor speeds and directions are controlled based on navigation needs.

## Detailed System Descriptions

### Navigation System

The navigation system enables the robot to traverse the model village accurately. It relies on line sensors to detect the path and junctions.

#### Line Sensors

- **Configuration**: Four line sensors connected to digital pins 4, 5, 6, and 7.
    - `far_left_sensor` (pin 4)
    - `left_sensor` (pin 5)
    - `right_sensor` (pin 6)
    - `far_right_sensor` (pin 7)
- **Functionality**: The sensors detect black lines on a white surface, enabling the robot to follow the path and recognize junctions.
- **Implementation**:
    - `read_line_sensors()`: Reads the state of each sensor and updates the corresponding variables.
    - `get_line_state()`: Interprets the sensor readings to determine the robot's position relative to the line and identify junctions.

#### Line Tracking Algorithm

- **Algorithm**: The robot adjusts its motor speeds based on sensor inputs to stay centered on the line.
- **Functions**:
    - `line_track_forward(int forward_speed)`: Controls the motors to maintain alignment with the line while moving forward.
    - `move_forward_tracking(int dist)`: Moves the robot forward while tracking the line for a specified distance.

#### Junction Handling

- **Detection**: Junctions are identified when specific patterns are detected by the line sensors.
- **Decision Making**: The robot uses the `path[]` array to decide whether to turn left, right, or continue straight.
- **Implementation**:
    - `turn_junction(int turn_direction)`: Executes the appropriate maneuver at a junction based on the turn direction.

### Block Handling System

The block handling system allows the robot to pick up blocks and deposit them at the correct recycling centers.

#### Block Detection

- **Sensor**: A Time-of-Flight (ToF) distance sensor (`DFRobot_VL53L0X`) measures the distance to potential blocks.
- **Functionality**: The robot detects blocks within a set distance (typically 40mm) and initiates the pickup sequence.
- **Implementation**:
    - Regularly checks `tof_block_distance` to determine if a block is nearby.

#### Magnet Detection

- **Sensors**: Two Hall effect sensors (`hallPinLeft` on A0 and `hallPinRight` on A1) detect magnetic fields.
- **Purpose**: Determines if the block is magnetic (recyclable) or non-magnetic.
- **Implementation**:
    - `read_magnet_sensor()`: Reads analog values from the Hall effect sensors and sets `is_magnet` based on a threshold.

#### Claw Mechanism

- **Hardware**: Two servos control the left and right claws for gripping blocks.
- **Class**: `Servo_claws` manages the servo positions.
- **Functions**:
    - `open()`: Opens the claws to grab or release a block.
    - `close()`: Closes the claws to hold a block securely.
    - `straight_ahead()`: Positions the claws forward during normal movement.

#### Block Pickup Routine

- **Process**:
    1. Detect block using the distance sensor.
    2. Stop the robot and open the claws.
    3. Move forward to position the claws around the block.
    4. Close the claws to secure the block.
    5. Update `has_block` to `true`.
- **Implementation**:
    - `pick_up_block()`: Handles the entire pickup sequence.

#### Block Dropping Routine

- **Process**:
    1. Navigate to the recycling center.
    2. Move forward into the deposit area.
    3. Open the claws to release the block.
    4. Reverse to exit the deposit area.
    5. Close the claws and reset `has_block`.
- **Implementation**:
    - `drop_off()`: Manages the drop-off sequence.

### Decision-Making Process

The robot's behavior is determined by sensor inputs and a predefined path.

#### Navigation Path

- **Path Array**: `path[]` defines a sequence of directions the robot should follow.
- **Special Modes**: `special_mode` and `special_path` handle specific scenarios, such as reaching recycling centers.

#### System Decisions

- **Function**: `system_decisions()`.
- **Logic**:
    - Checks for blocks ahead.
    - Determines if a junction is present.
    - Decides on actions based on the current state and path.

#### Turn Direction Retrieval

- **Function**: `get_turn_direction()`.
- **Purpose**: Determines the next movement direction from the path array.
- **Implementation**:
    - Considers special modes and progress counters.

### Sensors

Detailed descriptions of the sensors used in the robot.

#### Line Sensors

- **Type**: Infrared reflective sensors.
- **Pins**: Digital pins 4, 5, 6, 7.
- **Reading Values**: Outputs binary values indicating line presence.

#### Hall Effect Sensors

- **Type**: Analog sensors detecting magnetic fields.
- **Pins**: Analog pins A0, A1.
- **Calibration**:
    - `offsetL` and `offsetR` adjust for sensor zero points.
    - `magnet_threshold` defines sensitivity.

#### Distance Sensor

- **Type**: DFRobot_VL53L0X Time-of-Flight sensor.
- **Functionality**: Provides accurate distance measurements for block detection.
- **Usage**: Readings are stored in `tof_block_distance`.

### Actuators

Details about the actuators controlling robot movement and interaction.

#### DC Motors

- **Hardware**: Two DC motors controlled via the Adafruit Motor Shield.
- **Control**:
    - `MainMotors` class provides methods for speed and direction control.
    - Implements functions for moving forward, backward, and turning.
- **Calibration**:
    - `max_speed` and `min_speed` set speed limits.
    - Wheel specifications used for precise movement calculations.

#### Servo Motors

- **Hardware**: Two standard servos for the claws.
- **Control**:
    - `Servo_claws` class manages servo angles.
    - Angles are calibrated for open and closed positions.
- **Functions**:
    - Methods to open, close, and reset claws.

#### LEDs

- **Hardware**: Two status LEDs connected to pins 8 and 11.
- **Control**:
    - `LED_indicator` class provides methods to control LEDs.
    - Used for signaling robot states (e.g., block detected, error).

### Software Design

The software architecture separates hardware control from logic.

#### Modular Design

- **IDP_lib.h**: Contains hardware abstractions and utility functions.
- **system.ino**: Implements the control logic using abstractions from `IDP_lib.h`.

#### Event-Driven Programming

- **Interrupts**: Used for handling specific events (e.g., button presses).
- **State Management**: Variables track the robot's current state and progress.

## Testing and Calibration

Proper testing and calibration ensure reliable robot performance.

### Calibration Steps

- **Line Sensors**: Adjust sensor positions and sensitivity.
- **Hall Effect Sensors**: Fine-tune `offsetL`, `offsetR`, and `magnet_threshold`.
- **Motors**: Adjust `max_speed` and ensure balanced movement.
- **Servos**: Calibrate minimum and maximum angles for claw operation.

### Testing Procedures

- **Unit Tests**: Test individual functions and classes.
- **Integration Tests**: Test combined systems (e.g., navigation with block handling).
- **Field Tests**: Run the robot in the model village to observe behavior.

## Troubleshooting

Common issues and solutions. These are out of date currently. Refer to code itself for possible causes of errors.
Most errors will come from mistuning.

### Problem: Robot Veers Off Line

- **Causes**:
    - Misaligned line sensors.
    - Uneven motor speeds.
- **Solutions**:
    - Re-calibrate sensors.
    - Check motor configurations.

### Problem: Blocks Not Detected

- **Causes**:
    - Faulty distance sensor.
    - Incorrect threshold settings.
- **Solutions**:
    - Test the distance sensor separately.
    - Adjust detection thresholds.

### Problem: Claws Not Operating Correctly

- **Causes**:
    - Servos not receiving signals.
    - Incorrect servo angles.
- **Solutions**:
    - Verify servo connections.
    - Calibrate servo angles.

## Contributing

Contributions are welcome in the form of verbal feedback. Please email [lh848@cam.ac.uk](mailto:lh848@cam.ac.uk).
