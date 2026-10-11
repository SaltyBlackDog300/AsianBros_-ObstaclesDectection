#include "core.h"

const unsigned long STEP_US = 800;
const int LEFT_FORWARD = 1;
const int RIGHT_FORWARD = 1;

int turnPercent = 100;
int drivePercent = 100;
long turn90Ms = 1640;

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
static unsigned long stepUs = STEP_US;
static long runMs = -1;
static unsigned long startedAt = 0;
static unsigned long lastStepAt = 0;
char motion = 's';

void motorsBegin() {
  leftMotor.begin();
  rightMotor.begin();
  drive('s');
}

void drive(char direction, long ms) {
  bool turning = direction == 'l' || direction == 'r';
  int percent = constrain(turning ? turnPercent : drivePercent, 10, 100);
  stepUs = STEP_US * 100 / percent;
  if (turning && ms > 0) ms = ms * 100 / percent;
  switch (direction) {
    case 'f': leftDir = 1;  rightDir = 1;  break;
    case 'b': leftDir = -1; rightDir = -1; break;
    case 'l': leftDir = -1; rightDir = 1;  break;
    case 'r': leftDir = 1;  rightDir = -1; break;
    default:  leftDir = 0;  rightDir = 0;  direction = 's'; break;
  }
  motion = direction;
  runMs = direction == 's' ? -1 : ms;
  startedAt = millis();
  if (direction == 's') {
    leftMotor.release();
    rightMotor.release();
  }
}

void drive(char direction) {
  drive(direction, -1);
}

void motorsUpdate() {
  if (motion == 's') return;
  if (runMs > 0 && millis() - startedAt >= (unsigned long)runMs) {
    drive('s');
    return;
  }
  if (micros() - lastStepAt < stepUs) return;
  lastStepAt = micros();
  leftMotor.step(leftDir * LEFT_FORWARD);
  rightMotor.step(rightDir * RIGHT_FORWARD);
}
