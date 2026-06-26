/*
  ========================================================
  BLUETOOTH TANK CAR - ARDUINO CODE
  ========================================================
  4 motors (one at each corner), controlled via 2x L298N drivers.
  LEFT side motors (front+rear) move TOGETHER.
  RIGHT side motors (front+rear) move TOGETHER.

  Bluetooth (HC-05/HC-06) receives these characters from the
  "Bluetooth RC Controller" app:
gbnh
    F - forward            B - backward
    L - turn left          R - turn right
    G - forward-left       I - forward-right
    H - back-left          J - back-right
    S - stop               D - stop all

    Speed:
    0..9 -> speed 0%,10%,20%...90%
    q    -> speed 100%

  Turning works WHILE driving too: pressing L/R or G/I/H/J during
  forward/backward motion immediately re-applies the new direction
  to each side - no need to release the drive button first.
  ========================================================
*/

// ===================== PIN CONNECTIONS =====================
// L298N #1 -> LEFT side motors (front + rear wired in parallel)
const int LEFT_IN1 = 5;
const int LEFT_IN2 = 6;
const int LEFT_ENA = 9;   // PWM speed pin

// L298N #2 -> RIGHT side motors (front + rear wired in parallel)
const int RIGHT_IN1 = 7;
const int RIGHT_IN2 = 8;
const int RIGHT_ENB = 10; // PWM speed pin

// Bluetooth module (HC-05/HC-06): RX->pin2, TX->pin3 (SoftwareSerial)
#include <SoftwareSerial.h>
SoftwareSerial bluetooth(2, 3); // RX, TX

// ===================== STATE =====================
int currentSpeed = 200;       // current PWM speed (0-255), default ~78%
int leftState = 0;            // -1 backward, 0 stop, 1 forward
int rightState = 0;           // -1 backward, 0 stop, 1 forward

unsigned long lastSignalTime = 0;
const unsigned long SIGNAL_TIMEOUT = 400; // ms - auto-stop if signal lost

void setup() {
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(LEFT_ENA, OUTPUT);

  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);
  pinMode(RIGHT_ENB, OUTPUT);

  bluetooth.begin(9600);   // standard HC-05/HC-06 baud rate

  stopCar();
}

void loop() {
  if (bluetooth.available()) {
    char c = bluetooth.read();
    handleCommand(c);
    lastSignalTime = millis();
  }

  // Safety: if signal is lost, stop the car instead of driving forever
  if (millis() - lastSignalTime > SIGNAL_TIMEOUT) {
    stopCar();
  }
}

// ===================== COMMAND HANDLER =====================
void handleCommand(char c) {
  switch (c) {
    // ---- Movement ----
    case 'F': setSides(1, 1);   break; // forward
    case 'B': setSides(-1, -1); break; // backward
    case 'L': setSides(-1, 1);  break; // pivot left  (left back, right fwd)
    case 'R': setSides(1, -1);  break; // pivot right (left fwd, right back)
    case 'G': setSides(0, 1);   break; // forward-left  (left stop, right fwd)
    case 'I': setSides(1, 0);   break; // forward-right (left fwd, right stop)
    case 'H': setSides(0, -1);  break; // back-left     (left stop, right back)
    case 'J': setSides(-1, 0);  break; // back-right    (left back, right stop)
    case 'S': stopCar();        break; // stop
    case 'D': stopCar();        break; // stop all

    // ---- Speed (0-9 -> 0%-90%, q -> 100%) ----
    case '0': setSpeedPercent(0);   break;
    case '1': setSpeedPercent(10);  break;
    case '2': setSpeedPercent(20);  break;
    case '3': setSpeedPercent(30);  break;
    case '4': setSpeedPercent(40);  break;
    case '5': setSpeedPercent(50);  break;
    case '6': setSpeedPercent(60);  break;
    case '7': setSpeedPercent(70);  break;
    case '8': setSpeedPercent(80);  break;
    case '9': setSpeedPercent(90);  break;
    case 'q': setSpeedPercent(100); break;

    default: break; // unknown character - ignore
  }
}

// Change speed and immediately re-apply it to whichever side is moving
void setSpeedPercent(int percent) {
  currentSpeed = map(percent, 0, 100, 0, 255);
  applyMotors(); // re-apply current direction at the new speed instantly
}

// Set direction state for both sides and apply it right away.
// dir: -1 = backward, 0 = stop, 1 = forward
void setSides(int left, int right) {
  leftState = left;
  rightState = right;
  applyMotors();
}

// ===================== LOW-LEVEL MOTOR CONTROL =====================
void applyMotors() {
  // LEFT side
  if (leftState == 1) {
    digitalWrite(LEFT_IN1, HIGH);
    digitalWrite(LEFT_IN2, LOW);
    analogWrite(LEFT_ENA, currentSpeed);
  } else if (leftState == -1) {
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, HIGH);
    analogWrite(LEFT_ENA, currentSpeed);
  } else {
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, LOW);
    analogWrite(LEFT_ENA, 0);
  }

  // RIGHT side
  if (rightState == 1) {
    digitalWrite(RIGHT_IN1, HIGH);
    digitalWrite(RIGHT_IN2, LOW);
    analogWrite(RIGHT_ENB, currentSpeed);
  } else if (rightState == -1) {
    digitalWrite(RIGHT_IN1, LOW);
    digitalWrite(RIGHT_IN2, HIGH);
    analogWrite(RIGHT_ENB, currentSpeed);
  } else {
    digitalWrite(RIGHT_IN1, LOW);
    digitalWrite(RIGHT_IN2, LOW);
    analogWrite(RIGHT_ENB, 0);
  }
}

void stopCar() {
  leftState = 0;
  rightState = 0;
  applyMotors();
}
