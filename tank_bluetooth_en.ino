/*
  ========================================================
  BLUETOOTH TANK CAR - ARDUINO CODE
  ========================================================
  4 motors (one at each corner), controlled via 2x L298N drivers.
  LEFT side motors (front+rear) move TOGETHER.
  RIGHT side motors (front+rear) move TOGETHER.

  Bluetooth (HC-05/HC-06) receives these letters from the
  "Bluetooth RC Car" app:
    F - forward
    B - backward
    L - turn left
    R - turn right
    S - stop (sent automatically when button is released)

  TURN MODE: choose below with TURN_MODE.
    1 = PIVOT TURN (spin in place: one side forward, other side backward)
    2 = SMOOTH TURN (one side stops, other side moves)
  ========================================================
*/

// ----------- SETTING: choose turn mode here -----------
#define TURN_MODE 1   // 1 = pivot turn | 2 = smooth turn
// --------------------------------------------------------

// ===================== PIN CONNECTIONS =====================
// L298N #1 -> LEFT side motors (front + rear wired in parallel)
const int LEFT_IN1 = 5;
const int LEFT_IN2 = 6;
const int LEFT_ENA = 9;   // PWM speed pin

// L298N #2 -> RIGHT side motors (front + rear wired in parallel)
const int RIGHT_IN1 = 7;
const int RIGHT_IN2 = 8;
const int RIGHT_ENB = 10; // PWM speed pin

// Speed (range 0 - 255)
const int SPEED = 200;

// Bluetooth module (HC-05/HC-06): RX->pin2, TX->pin3 (SoftwareSerial)
#include <SoftwareSerial.h>
SoftwareSerial bluetooth(2, 3); // RX, TX

// Safety: auto-stop if no signal is received for a while
unsigned long lastSignalTime = 0;
const unsigned long SIGNAL_TIMEOUT = 300; // ms

void setup() {
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(LEFT_ENA, OUTPUT);

  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);
  pinMode(RIGHT_ENB, OUTPUT);

  Serial.begin(9600);
  bluetooth.begin(9600);   // standard HC-05/HC-06 baud rate

  stopCar();
}

void loop() {
  if (bluetooth.available()) {
    char command = bluetooth.read();
    runCommand(command);
    lastSignalTime = millis();
  }

  // Safety: if signal is lost (out of range, interference, etc.)
  // the car stops by itself instead of driving forever
  if (millis() - lastSignalTime > SIGNAL_TIMEOUT) {
    stopCar();
  }
}

// ===================== RUN COMMAND =====================
void runCommand(char c) {
  switch (c) {
    case 'F': moveForward(); break;
    case 'B': moveBackward(); break;
    case 'L': turnLeft(); break;
    case 'R': turnRight(); break;
    case 'S': stopCar(); break;
    default: break; // unknown character - do nothing
  }
}

// ===================== MOTOR FUNCTIONS =====================

// Left side motors
void leftForward() {
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(LEFT_IN2, LOW);
  analogWrite(LEFT_ENA, SPEED);
}
void leftBackward() {
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, HIGH);
  analogWrite(LEFT_ENA, SPEED);
}
void leftStop() {
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, LOW);
  analogWrite(LEFT_ENA, 0);
}

// Right side motors
void rightForward() {
  digitalWrite(RIGHT_IN1, HIGH);
  digitalWrite(RIGHT_IN2, LOW);
  analogWrite(RIGHT_ENB, SPEED);
}
void rightBackward() {
  digitalWrite(RIGHT_IN1, LOW);
  digitalWrite(RIGHT_IN2, HIGH);
  analogWrite(RIGHT_ENB, SPEED);
}
void rightStop() {
  digitalWrite(RIGHT_IN1, LOW);
  digitalWrite(RIGHT_IN2, LOW);
  analogWrite(RIGHT_ENB, 0);
}

// ===================== MOVEMENT COMMANDS =====================

void moveForward() {
  leftForward();
  rightForward();
}

void moveBackward() {
  leftBackward();
  rightBackward();
}

void turnLeft() {
#if TURN_MODE == 1
  // Pivot turn: left side backward, right side forward
  leftBackward();
  rightForward();
#else
  // Smooth turn: left side stops, right side moves forward
  leftStop();
  rightForward();
#endif
}

void turnRight() {
#if TURN_MODE == 1
  // Pivot turn: left side forward, right side backward
  leftForward();
  rightBackward();
#else
  // Smooth turn: right side stops, left side moves forward
  leftForward();
  rightStop();
#endif
}

void stopCar() {
  leftStop();
  rightStop();
}
