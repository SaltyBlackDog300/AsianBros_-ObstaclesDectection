#include "core.h"
#include <Servo.h>

const int STOP_US = 1500;
const int SPEED_US = 500;
const int LEFT_TRIM_US = 0;
const int RIGHT_TRIM_US = 0;
const int LEFT_FORWARD = -1;
const int RIGHT_FORWARD = 1;

int turnPercent = 100;
int drivePercent = 100;
long turn90Ms = 1200;

static Servo leftWheel;
static Servo rightWheel;
static long runMs = -1;
static unsigned long startedAt = 0;
char motion = 's';

static void setWheels(int left, int right, int speed) {
  leftWheel.writeMicroseconds(STOP_US + LEFT_TRIM_US + left * LEFT_FORWARD * speed);
  rightWheel.writeMicroseconds(STOP_US + RIGHT_TRIM_US + right * RIGHT_FORWARD * speed);
}

void motorsBegin() {
  leftWheel.attach(LEFT_WHEEL_PIN);
  rightWheel.attach(RIGHT_WHEEL_PIN);
  drive('s');
}

void drive(char direction, long ms) {
  int turnSpeed = SPEED_US * constrain(turnPercent, 10, 100) / 100;
  int driveSpeed = SPEED_US * constrain(drivePercent, 10, 100) / 100;
  bool turning = direction == 'l' || direction == 'r';
  if (turning && ms > 0) ms = ms * SPEED_US / turnSpeed;
  switch (direction) {
    case 'f': setWheels(1, 1, driveSpeed);    break;
    case 'b': setWheels(-1, -1, driveSpeed);  break;
    case 'l': setWheels(1, -1, turnSpeed);  break;
    case 'r': setWheels(-1, 1, turnSpeed);  break;
    default:  setWheels(0, 0, 0);           direction = 's'; break;
  }
  motion = direction;
  runMs = direction == 's' ? -1 : ms;
  startedAt = millis();
}

void drive(char direction) {
  drive(direction, -1);
}

void motorsUpdate() {
  if (motion != 's' && runMs > 0 && millis() - startedAt >= (unsigned long)runMs) drive('s');
}
