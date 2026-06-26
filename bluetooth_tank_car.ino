#include <SoftwareSerial.h>

/* ============================================================
   PIN LIST / PINLAR RO'YXATI
   ------------------------------------------------------------
   Bluetooth module (HC-05 / HC-06):
     Bluetooth TXD  -> Arduino pin 10 (bluetooth RX)
     Bluetooth RXD  -> Arduino pin 11 (bluetooth TX) 
                       *** through a voltage divider! ***
                       (Arduino 5V -> module RXD = 3.3V max
                        Use 1k + 2k resistor divider, or a
                        logic level converter, or a ready-made
                        HC-05/06 module that already has one.)
     Bluetooth VCC  -> 5V (or 3.3V if your module needs it)
     Bluetooth GND  -> GND

   L298N Motor Driver:
     IN1 -> Arduino pin 2   (Left motor direction A)
     IN2 -> Arduino pin 3   (Left motor direction B)
     IN3 -> Arduino pin 4   (Right motor direction A)
     IN4 -> Arduino pin 5   (Right motor direction B)
     ENA -> Arduino pin 6   (Left motor speed, PWM ~)
     ENB -> Arduino pin 9   (Right motor speed, PWM ~)
     *** Remove the ENA/ENB jumpers on the L298N board,
         otherwise speed control will not work! ***

     L298N 12V   -> Battery (+)
     L298N GND   -> Battery (-) AND Arduino GND (common ground!)
     L298N 5V OUT-> Can power Arduino 5V pin IF the onboard
                    regulator jumper is in place (only for
                    small motors / low current). Safer: power
                    Arduino separately and just share GND.

   IMPORTANT: Arduino GND, L298N GND, Battery GND and
   Bluetooth GND must ALL be connected together (common ground).
   ============================================================ */

SoftwareSerial bluetooth(10, 11); // RX, TX

// Motor direction pins
const int IN1 = 2;  // Left motor forward
const int IN2 = 3;  // Left motor backward
const int IN3 = 4;  // Right motor forward
const int IN4 = 5;  // Right motor backward

// Motor speed (PWM) pins - must be PWM-capable (~)
const int ENA = 6;  // Left motor speed
const int ENB = 9;  // Right motor speed

char command = 'S';

// ---- Speed settings (0-255) ----
int normalSpeed     = 200; // forward / back
int turnSpeed       = 150; // in-place rotation
int curveFastSpeed  = 190; // diagonal: fast wheel
int curveSlowSpeed  = 80;  // diagonal: slow wheel

// ---- Connection safety ----
// If no command arrives for this many milliseconds,
// the robot stops automatically. This prevents it from
// running away if Bluetooth disconnects.
const unsigned long CONNECTION_TIMEOUT = 500; // ms
unsigned long lastCommandTime = 0;

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  bluetooth.begin(9600);
  stopRobot();
  lastCommandTime = millis();
}

void loop() {
  // ---- Read all available bytes, keep only the latest valid one ----
  while (bluetooth.available() > 0) {
    char c = bluetooth.read();
    if (isValidCommand(c)) {
      command = c;
      lastCommandTime = millis();
      executeCommand(command);
    }
  }

  // ---- Safety: if connection is silent too long, stop ----
  if (millis() - lastCommandTime > CONNECTION_TIMEOUT) {
    stopRobot();
  }
}

bool isValidCommand(char c) {
  switch (c) {
    case 'F': case 'B': case 'L': case 'R':
    case 'G': case 'I': case 'H': case 'J':
    case 'S':
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
      return true;
    default:
      return false;
  }
}

void executeCommand(char cmd) {
  switch (cmd) {
    case 'F': forward();      break;
    case 'B': back();         break;
    case 'L': turnLeft();     break;
    case 'R': turnRight();    break;
    case 'G': forwardLeft();  break;
    case 'I': forwardRight(); break;
    case 'H': backLeft();     break;
    case 'J': backRight();    break;
    case 'S': stopRobot();    break;

    // Speed level selection (1 = slowest, 9 = fastest)
    case '1': setSpeedLevel(1); break;
    case '2': setSpeedLevel(2); break;
    case '3': setSpeedLevel(3); break;
    case '4': setSpeedLevel(4); break;
    case '5': setSpeedLevel(5); break;
    case '6': setSpeedLevel(6); break;
    case '7': setSpeedLevel(7); break;
    case '8': setSpeedLevel(8); break;
    case '9': setSpeedLevel(9); break;
  }
}

// Maps level 1-9 to a speed range and re-applies it to whatever
// is currently moving, so you can change speed mid-motion.
void setSpeedLevel(int level) {
  normalSpeed    = map(level, 1, 9, 90, 255);
  turnSpeed      = map(level, 1, 9, 70, 200);
  curveFastSpeed = map(level, 1, 9, 80, 230);
  curveSlowSpeed = map(level, 1, 9, 30, 100);

  // Re-apply current movement so speed change takes effect instantly
  executeCommand(command == '1' || command == '2' || command == '3' ||
                  command == '4' || command == '5' || command == '6' ||
                  command == '7' || command == '8' || command == '9'
                  ? 'S' : command);
}

// ---------- LOW-LEVEL MOTOR HELPERS ----------

void leftMotor(bool forwardDir, int speed) {
  digitalWrite(IN1, forwardDir ? HIGH : LOW);
  digitalWrite(IN2, forwardDir ? LOW  : HIGH);
  analogWrite(ENA, speed);
}

void rightMotor(bool forwardDir, int speed) {
  digitalWrite(IN3, forwardDir ? HIGH : LOW);
  digitalWrite(IN4, forwardDir ? LOW  : HIGH);
  analogWrite(ENB, speed);
}

void leftMotorStop()  { digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); analogWrite(ENA, 0); }
void rightMotorStop() { digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); analogWrite(ENB, 0); }

// ---------- MOVEMENT FUNCTIONS ----------

void forward() {
  leftMotor(true, normalSpeed);
  rightMotor(true, normalSpeed);
}

void back() {
  leftMotor(false, normalSpeed);
  rightMotor(false, normalSpeed);
}

void turnLeft() {
  leftMotor(false, turnSpeed);
  rightMotor(true, turnSpeed);
}

void turnRight() {
  leftMotor(true, turnSpeed);
  rightMotor(false, turnSpeed);
}

void forwardLeft() {
  leftMotor(true, curveSlowSpeed);
  rightMotor(true, curveFastSpeed);
}

void forwardRight() {
  leftMotor(true, curveFastSpeed);
  rightMotor(true, curveSlowSpeed);
}

void backLeft() {
  leftMotor(false, curveSlowSpeed);
  rightMotor(false, curveFastSpeed);
}

void backRight() {
  leftMotor(false, curveFastSpeed);
  rightMotor(false, curveSlowSpeed);
}

void stopRobot() {
  leftMotorStop();
  rightMotorStop();
}
