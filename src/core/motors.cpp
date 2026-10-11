#include "core.h"

const unsigned long STEP_US = 800;
const int LEFT_FORWARD = 1;
const int RIGHT_FORWARD = 1;

const uint8_t SEQUENCE[8][4] = {
  {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 1, 0},
  {0, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 0, 1}, {1, 0, 0, 1},
};

struct Stepper {
  const uint8_t* pins;
  int phase;

  void begin() {
    for (int i = 0; i < 4; i++) pinMode(pins[i], OUTPUT);
  }

  void step(int dir) {
    phase = (phase + dir + 8) % 8;
    for (int i = 0; i < 4; i++) digitalWrite(pins[i], SEQUENCE[phase][i]);
  }

  void release() {
    for (int i = 0; i < 4; i++) digitalWrite(pins[i], LOW);
  }
};

static Stepper leftMotor = {LEFT_MOTOR_PINS, 0};
static Stepper rightMotor = {RIGHT_MOTOR_PINS, 0};
static int leftDir = 0;
static int rightDir = 0;
static long stepsLeft = -1;
static unsigned long lastStepAt = 0;
char motion = 's';

void motorsBegin() {
  leftMotor.begin();
  rightMotor.begin();
  drive('s');
}

void drive(char direction, long steps) {
  switch (direction) {
    case 'f': leftDir = 1;  rightDir = 1;  break;
    case 'b': leftDir = -1; rightDir = -1; break;
    case 'l': leftDir = -1; rightDir = 1;  break;
    case 'r': leftDir = 1;  rightDir = -1; break;
    default:  leftDir = 0;  rightDir = 0;  direction = 's'; break;
  }
  motion = direction;
  stepsLeft = direction == 's' ? -1 : steps;
  if (direction == 's') {
    leftMotor.release();
    rightMotor.release();
  }
}

void drive(char direction) {
  drive(direction, -1);
}

void motorsUpdate() {
  if (motion == 's' || micros() - lastStepAt < STEP_US) return;
  lastStepAt = micros();
  leftMotor.step(leftDir * LEFT_FORWARD);
  rightMotor.step(rightDir * RIGHT_FORWARD);
  if (stepsLeft > 0 && --stepsLeft == 0) drive('s');
}
