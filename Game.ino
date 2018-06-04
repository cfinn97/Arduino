/*
    Simon says, Pattern Game
    Arduino Esplora is the controller
    Author: Camera Finn               Created: Spring Semester 2018
*/

#include <Esplora.h>

// start of variables for Charlieplexing
byte pins[] = {14, 15, 16, 7}; // the pins that are connected to LEDs

const int pin_total = sizeof(pins) / sizeof(pins[0]); // number of pins from pins array
const int LED_total = pin_total * (pin_total - 1); // number of LEDs from number of pins

byte pairs[LED_total / 2][2] = {{0, 1}, {1, 2}, {0, 2,}, {2, 3}, {1, 3}, {0, 3}}; // map pins to LEDs
// end of variables for Charlieplexing

int pattern[100]; // array to hold pattern
int pattern_size = sizeof(pattern) / sizeof(int);
boolean play = false; // indicates start of game
int timer = 500; // timer, time between LEDs lighting up when displayng pattern
int turn = 0; // player turn #

int joyStickX, joyStickY; // store state of x & y state of joyStick
int up, down, left, right; // store state of the 4 buttons right side controller
int playButton; // Joystick button is the button to press with slider to start and quit the game
// default: high

int xAxis; // tilt the board along x-axis
int sound; // sound registered by microphone
int photocell; // photocell read/variable
int potValue; // potentiometer/slider variable
int mov; // the player's move
void setup() {

}

void loop() {
  if (!play) // game not started
    // setup game
    initialize();
  if (player_move() == 20) { // if player starts game
    play = true;
  }
  while (play) { // game play
    delay(500);
    turn++;
    for (int i = 0; i < turn; i++) {
      light_LED(pattern[i] - 1);
    }
    // light color green to let player know pattern is done
    Esplora.writeRGB(0, 255, 0);
    delay(1000);
    Esplora.writeRGB(0, 0, 0);
    // light turns off, letting player know that it's their turn to re-create the pattern

    for (int i = 0; i < turn; i++) {
      mov = player_move();
      if (mov == -20 || mov != pattern[i]) { // if player makes a mistake or quits, the game ends
        if (mov != pattern[i]) { // buzzer sound if player makes a mistake
          Esplora.tone(349);
          delay(1000);
          Esplora.noTone();
        }
        play = false;
        break;
      }
      light_LED(pattern[i]);
      delay(250);
    }
    if (turn == pattern_size) { // buzzer "song" if player completes the game with no mistakes
      play = false;
      Esplora.tone(400);
      delay(250);
      Esplora.tone(600);
      delay(250);
      Esplora.tone(800);
      delay(250);
      Esplora.tone(1000);
      delay(500);
      Esplora.tone(800);
      delay(250);
      Esplora.tone(1000);
      delay(1000);
      Esplora.noTone();
    }
    if (!play) { // end of game, then display score
      displayScore();
    }
  }
}

// display score in binary using LED on Esplora board, only when game ends
void diplayScore() {
  // color red - start and end of score
  // blue = 1
  // white = 0
  Esplora.writeRGB(100, 0, 0); // indicate start of score
  delay(1000);
  while (turn) {
    if (turn % 2) { // has remainder
      Esplora.writeRGB(100, 100, 100);
      delay(1000);
    } else {
      Esplora.writeRGB(0, 0, 100);
      delay(1000);
    }
    turn = turn / 2;
  }
  Esplora.writeRGB(100, 0, 0); // indicate end of score
  delay(1000);
  Esplora.writeRGB(0, 0, 0);
}
// set up the game
void initialize() {
  mov = 0;
  turn = 0;
  timer  = 500;
  for (int i = 0; i < pattern_size; i++) {
    pattern[i] = random(1, 13); // random number between 1 and 12
  }
}

// returns the integer representation of the player's action
int player_move() {
  while (true) {
    joyStickX = Esplora.readJoystickX();
    joyStickY = Esplora.readJoystickY();
    up = Esplora.readButton(SWITCH_UP);
    down = Esplora.readButton(SWITCH_DOWN);
    left = Esplora.readButton(SWITCH_LEFT);
    right = Esplora.readButton(SWITCH_RIGHT);
    xAxis = Esplora.readAccelerometer(X_AXIS);
    potValue = Esplora.readSlider();
    playButton = Esplora.readJoystickButton();
    sound = Esplora.readMicrophone();
    photocell = Esplora.readLightSensor();
    if (playButton == LOW) {
      if (potValue > 512)
        return 20; // start game
      return -20; // end game
    } else if (joyStickY < -200) { // joystick is moved north
      return 2;
    } else if (joyStickY > 200) { // joystick is moved south
      return 5;
    } else if (joyStickX < -200) { // joystick is moved east
      return 1;
    } else if (joyStickX > 200) { // joystick is moved west
      return 6;
    } else if (up == LOW) { // up botton is pressed
      return 9;
    } else if (down == LOW) { // bottom botton is pressed
      return 8;
    } else if (left == LOW) { // left botton is pressed
      return 7;
    } else if (right == LOW) { // right botton is pressed
      return 10;
    } else if (xAxis > 100) { // Esplora Board accelerates in positive direction of x-axis
      return 4;
    } else if (xAxis < -100) { // Esplora Board accelerates in negative direction of x-axis
      return 11;
    } else if (sound > 500) { // player speaks into the microphone
      return 12;
    } else if (photocell < 950) { // photocell is covered
      return 3;
    }
  }
}

// lightup/flash the given led - using Charlieplexing
void light_LED(int led) {
  // convert LED number to pin numbers
  int indexA = pairs[led / 2][0];
  int indexB = pairs[led / 2][1];
  int pinA = pins[indexA];
  int pinB = pins[indexB];

  // turn off all pins not connected to the given led
  for (int i = 0; i < pin_total; i++) {
    if (pins[i] != pinA && pins[i] != pinB) {
      // if pin is not mapped/connected to the given led
      pinMode(pins[i], INPUT); // set pinMode of pin to input
      digitalWrite(pins[i], LOW); // set to low to ensure pull-up is off
    }
  }
  //turn on the pins for the given led
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  if (led % 2 == 0) {
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, HIGH);
  }
  else {
    digitalWrite(pinB, LOW);
    digitalWrite(pinA, HIGH);
  }
  // blinks light, so after a dely, turns the led off
  delay(timer);
  digitalWrite(pinA, LOW);
  digitalWrite(pinB, LOW);
}

