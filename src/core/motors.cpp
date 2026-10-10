#include "core.h"
#include <Servo.h>

const int STOP_US = 1500;
const int SPEED_US = 500;
const int LEFT_TRIM_US = 0;
const int RIGHT_TRIM_US = 0;
const int LEFT_FORWARD = 1;
const int RIGHT_FORWARD = -1;

static Servo leftWheel;
static Servo rightWheel;
static long runMs = -1;
static unsigned long startedAt = 0;
char motion = 's';

static void setWheels(int left, int right) {
  leftWheel.writeMicroseconds(STOP_US + LEFT_TRIM_US + left * LEFT_FORWARD * SPEED_US);
  rightWheel.writeMicroseconds(STOP_US + RIGHT_TRIM_US + right * RIGHT_FORWARD * SPEED_US);
}

void motorsBegin() {
  leftWheel.attach(LEFT_WHEEL_PIN);
  rightWheel.attach(RIGHT_WHEEL_PIN);
  drive('s');
}

void drive(char direction, long ms) {
  switch (direction) {
    case 'f': setWheels(1, 1);   break;
    case 'b': setWheels(-1, -1); break;
    case 'l': setWheels(-1, 1);  break;
    case 'r': setWheels(1, -1);  break;
    default:  setWheels(0, 0);   direction = 's'; break;
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
