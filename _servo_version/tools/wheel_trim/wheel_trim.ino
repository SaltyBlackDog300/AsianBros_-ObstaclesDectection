// Finds the pulse at which each 360 wheel servo really stops.
// Open Serial Monitor (9600, "Newline"), lift the wheels, then type e.g.
//   l1500   -> left wheel gets 1500 us
//   r1520   -> right wheel gets 1520 us
// Change the number in steps of 5 until that wheel stands still.
// Then in src/core/motors.cpp set LEFT_TRIM_US = left - 1500, RIGHT_TRIM_US = right - 1500.
#include <Servo.h>

const int LEFT_WHEEL_PIN = 5;
const int RIGHT_WHEEL_PIN = 6;

Servo leftWheel;
Servo rightWheel;
int leftUs = 1500;
int rightUs = 1500;

void setup() {
  Serial.begin(9600);
  leftWheel.attach(LEFT_WHEEL_PIN);
  rightWheel.attach(RIGHT_WHEEL_PIN);
  leftWheel.writeMicroseconds(leftUs);
  rightWheel.writeMicroseconds(rightUs);
  Serial.println("Type l1500 or r1500 (wheel + pulse in us).");
}

void loop() {
  if (!Serial.available()) return;
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() < 2) return;
  int us = constrain(line.substring(1).toInt(), 1300, 1700);
  if (line[0] == 'l') leftUs = us;
  else if (line[0] == 'r') rightUs = us;
  else return;
  leftWheel.writeMicroseconds(leftUs);
  rightWheel.writeMicroseconds(rightUs);
  Serial.print("left ");
  Serial.print(leftUs);
  Serial.print("  right ");
  Serial.println(rightUs);
}
