#include <Servo.h>
// left and right wheel motion direction values are reverse of eachother
Servo left_wheel; // pin 10
Servo right_wheel; // pin 9
Servo IR_motor; // pin A3
#define sound 6 // pin for rading sound to start

#define object_sensor A2 // pin for reading the Sharp IR sensor for detecting objects
/*
   Black surface absorbs more "light" than white surfaces.
   Sensor is more sensitive to white surface than black surface
   value for white is lower than for black
*/
#define left_sensor A0 // pin for reading one of the QRD1114 IR sensor devices, detecting between black and white
// analog pin
#define right_sensor A1 // pin for reading the other QRD1114 IR sensor devices

#define off_line 750 // value for if the sensor reads number for black surface
#define turn_time 900 // time it takes to do 90 degrees turn
// 100 is placeholder

unsigned long start;
unsigned long finish;

void setup() {
  Serial.begin(9600);
  //delay(2000);
  // put your setup code here, to run once:
  initialize_Robot();
  wait_for_sound();
  follow_white_line(false, 12); // until obstacle, then stop
  detour_to_white_line();
  follow_white_line(false, 12); // until wall, then stop
  follow_wall_to_white_line();
  follow_white_line(true, 12); // until obstacle, then stop
  finish_course();
}

void loop() {
} // end of loop function

/*
   Set up robot, pin readings, and everything
*/
void initialize_Robot() {
  pinMode(object_sensor, INPUT);
  pinMode(right_sensor, INPUT);
  pinMode(left_sensor, INPUT);
  pinMode(sound, INPUT);
}

/* delay until sound registered to start */
void wait_for_sound() {
  // read microphone, if reading is in a speficic range, then end function
  while (!digitalRead(sound)) { // some minimum sound
  }
  Serial.println("Sound Detected");
}

/*
  Follow the white line until an obstacle is registered or there is no more line to follow
  use IR sensors (on bottom of robot) for checking if on line and to follow line, controlling servo motor to move wheels
  Use IR senor at head/front of robot to sense for an obstacle or wall
      reading to determine if to stop or not, set to false
*/

void follow_white_line(boolean g, int dist) {
  Serial.println("Line Found");
  int reading = 100;
  int right_line_sensor = 5, left_line_sensor = 5, object_distance = 20;
  for (int i = 0; i < 1; i++) {

    while (true) {
      reading = IR_read();
      if (reading == 5) {
        Serial.println(reading);
        break;
      }

      right_line_sensor = analogRead(right_sensor); // read sensor for tacking whether or not too far over right of line
      left_line_sensor = analogRead(left_sensor); // read sensor for tacking whether or not too far over left of line
      if (right_line_sensor >= off_line && left_line_sensor >= off_line ) { // if not on line or object is detected; 0 is place holder
        break; // breaks the loop, thus also the function
      } else {
        // move: control motors
        if (left_line_sensor >= off_line) { // drift right, (b/c drifted left off line)
          move_left_wheel(0); // faster
          move_right_wheel(90); // slower
          // move left wheel faster than right
        }  else if (right_line_sensor >= off_line) { // drift left, (b/c drifted right off line)
          // slow left wheel
          // 0 is full forward
          //180 is full backward
          move_left_wheel(90);
          move_right_wheel(0);
          // move right wheel faster than left
        } else { // centered
          move_straight(0);
        }
      }
    }
    // stop moving wheels
    left_wheel.detach();
    right_wheel.detach();

    if (g) { // if at point g, waiting for flag
      g = false;
      i--;
      while (analogRead(object_sensor) >= dist) {
        /*wait until flag is nolonger infront of sensor then move*/
      }
    }
  }
} // end of follow_white_line function

/*
  Move off of line, move around obstacle and back to the correct white line
*/
void detour_to_white_line() {
  int right_line_sensor;
  int left_line_sensor;
  // rotate left
  //delay(3000);
  rotateLeft();

  // move forward
  move_straight(0);
  delay(3000); // time to travel straight

  // rotate right
  rotateRight();

  // move forward
  move_straight(0);
  delay(4500); // time to rotate, just a place holder

  rotate_robot_and_IRSensor(2, 0, 60, 0);
  move_straight(0);
  right_line_sensor = analogRead(right_sensor);
  left_line_sensor = analogRead(left_sensor);
  while (right_line_sensor > off_line && left_line_sensor > off_line) {
    right_line_sensor = analogRead(right_sensor);
    left_line_sensor = analogRead(left_sensor);
  }
  return;

} // end of detour_to_white_line


/*
  Rotate the robot, so moving parallel to wall, until a white line is detected
*/
void follow_wall_to_white_line() {
  Serial.println("running follow to white line");
  // first need to rotate, left
  rotate_robot_and_IRSensor(1, 2, 100, 100);
  while (true) {
    int d = IR_read();
    //Serial.println(d);
    move_straight(0);
    if (d > 10) {
      move_left_wheel(0);
      move_right_wheel(90);
    }
    if (d < 10) {
      move_left_wheel(90);
      move_right_wheel(0);
    }
    int right_line_sensor = analogRead(right_sensor); // read sensor for right side of line
    int left_line_sensor = analogRead(left_sensor); // read sensor for left side of line
    if (right_line_sensor < off_line || left_line_sensor < off_line) { // found line

      Serial.println("Found Line");
      left_wheel.detach();
      right_wheel.detach();
      break;

    }
  }
  move_straight(0);
  delay(1050);
  rotate_robot_and_IRSensor(1, 1, 100, 100);

} // end of follow_wall_to_white_line


/* Return to start function, to move a ball and finish through arena */
void finish_course() {
  follow_white_line(false, 5);

  rotate_robot_and_IRSensor(1, 2, 45, 0);

  while (true) {
    move_straight(0);
  }
  left_wheel.detach();
  right_wheel.detach();
}





/* move left wheel with velocity */
void move_left_wheel(int velocity) {
  int s = 180 - velocity; // right servo motor is in backwards in robot
  left_wheel.attach(10);
  left_wheel.write(s);
}

/* move right with velocity */
void move_right_wheel(int velocity) {
  right_wheel.attach(9);
  right_wheel.write(velocity);
}
/* move in a straight line with velocity v */
void move_straight(int v) {
  move_left_wheel(v);
  move_right_wheel(v);
}

/* Rotate robot 90 degrees left (counterclockwise)*/
void rotateLeft() {
  move_left_wheel(180);
  move_right_wheel(0);
  delay(turn_time);
}

/* Rotate robot 90 degrees right (clockwise)*/
void rotateRight() {
  move_left_wheel(0);
  move_right_wheel(180);
  delay(turn_time);
}

/*
  Rotates all the motors simultaneously, wheels and IR sensor motor.
  1 = move counterclockwise; otherwise move clockwise
*/
void rotate_robot_and_IRSensor(int direct_wheels, int direct_IRsensor, int angle_wheels, int angle_IRsensor) {
  IR_motor.attach(A3);
  // mapping of angle in degrees to time it takes for motor to turn that angle
  int a_wheels = map(angle_wheels, 0, 360, 0, 3600);
  int a_sensor = map(angle_IRsensor, 0, 360, 0, 1100);
  if (direct_IRsensor == 1) { // turn counter clockwise
    IR_motor.write(180);
    Serial.println("counter clockwise");
  } else { // turn clockwise
    IR_motor.write(0);
    Serial.println("turning clockwise");
  }
  if (direct_wheels == 1) { // turn left
    move_left_wheel(180);
    move_right_wheel(0);
  } else { // turn right
    move_left_wheel(0);
    move_right_wheel(180);
  }
  start = millis();
  finish = millis();
  int maxAngle = max(a_wheels, a_sensor);
  int minAngle = min(a_wheels, a_sensor);
  unsigned long t = finish - start;
  while (t < maxAngle) {
    finish = millis();
    t = finish - start;
    if (t >= minAngle) {
      if (minAngle == a_wheels) {
        left_wheel.detach();
        right_wheel.detach();
      }
      else
        IR_motor.detach();
    }
  }
  if (maxAngle == a_sensor) {
    IR_motor.detach();
  } else {
    left_wheel.detach();
    right_wheel.detach();
  }
}

/*
  Returns the distance from an object/surface in centimeters
*/
int IR_read() {
  float volts = analogRead(object_sensor) * 0.0048828125; // value from sensor * (5/1024)
  int distance = 13 * pow(volts, -1); // worked out from datasheet graph
  return distance;
}
/*
  Rotate the Sharp sensor for detecting objects
*/
void rotate_IR_motor(int direct, int angle) {
  IR_motor.attach(A3);
  int t = map(angle, 0, 360, 0, 1100);
  if (direct == 1) { // turn 90 degrees counter clockwise
    IR_motor.write(180);
  } else { // turn 90 degrees clockwise
    IR_motor.write(0);
  }
  delay(t);
  IR_motor.detach();
}
