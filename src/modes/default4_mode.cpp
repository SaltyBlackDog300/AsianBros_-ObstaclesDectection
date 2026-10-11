#include "modes.h"

const int WALL_ANGLE = 10;
const long TARGET_CM = 23;
const long BAND_CM = 5;
const long LOST_CM = 50;
const int LOST_NEEDED = 3;
const long FRONT_CM = 25;
const long FRONT_CLEAR_CM = 35;
const unsigned long FRONT_EVERY_MS = 800;
const unsigned long SETTLE_MS = 300;
const int NOISE_CM = 3;
const int STUCK_CM = 3;
const unsigned long STUCK_MS = 2000;
const long STEER_MIN_MS = 150;
const long STEER_MAX_MS = 600;
const long STEER_MS_PER_CM = 20;
const long FRONT_TURN_MS = TURN_90_MS / 6;
const long ROVER_CMS = 20;
const long CORNER_FORWARD_CM = 40;
const long CORNER_FORWARD_MS = CORNER_FORWARD_CM * 1000 / ROVER_CMS;
const long CORNER_TURN_MS = TURN_90_MS * 3 / 4;

enum Step { FOLLOW, STEERING, GLANCE, FRONT_TURN, FRONT_LOOK, BACKING, CORNER, CORNER_TURN };

static Step step = FOLLOW;
static unsigned long lastReading = 0;
static unsigned long lastGlance = 0;
static unsigned long followSince = 0;
static int lostReadings = 0;
static long stuckRef = -1;
static unsigned long stuckSince = 0;

static bool newReading() {
  if (!freshReading() || readingId == lastReading) return false;
  lastReading = readingId;
  return distanceCm >= NOISE_CM;
}

static void waitReading(Step next) {
  lastReading = readingId;
  step = next;
}

static bool isStuck(long front) {
  if (front >= NO_ECHO_CM || abs(front - stuckRef) > STUCK_CM) {
    stuckRef = front;
    stuckSince = millis();
    return false;
  }
  return millis() - stuckSince > STUCK_MS;
}

static void follow() {
  setNeck(WALL_ANGLE);
  lostReadings = 0;
  followSince = millis();
  drive('f');
  waitReading(FOLLOW);
}

static void glance() {
  setNeck(NECK_CENTER);
  waitReading(GLANCE);
}

static void steer(char dir, long ms) {
  drive(dir, constrain(ms, STEER_MIN_MS, STEER_MAX_MS));
  step = STEERING;
}

static void frontTurn() {
  stuckRef = -1;
  setNeck(NECK_CENTER);
  drive('l', FRONT_TURN_MS);
  step = FRONT_TURN;
}

static void startCorner() {
  stuckRef = -1;
  drive('f', CORNER_FORWARD_MS);
  step = CORNER;
}

void default4ModeStart() {
  lostReadings = 0;
  stuckRef = -1;
  glance();
}

void default4ModeUpdate() {
  switch (step) {
    case FOLLOW: {
      if (motion == 's') drive('f');
      if (millis() - lastGlance >= FRONT_EVERY_MS) {
        glance();
        return;
      }
      if (!newReading()) return;
      long d = distanceCm;
      if (d >= LOST_CM) {
        if (++lostReadings >= LOST_NEEDED) startCorner();
        return;
      }
      lostReadings = 0;
      if (millis() - followSince < SETTLE_MS) return;
      long error = d - TARGET_CM;
      if (abs(error) > BAND_CM) steer(error > 0 ? 'r' : 'l', abs(error) * STEER_MS_PER_CM);
      break;
    }

    case STEERING:
      if (motion == 's') follow();
      break;

    case GLANCE: {
      if (!newReading()) return;
      long front = distanceCm;
      lastGlance = millis();
      if (front <= FRONT_CM) {
        drive('s');
        frontTurn();
        return;
      }
      if (motion == 'f' && isStuck(front)) {
        drive('b', BACK_MS);
        step = BACKING;
        return;
      }
      follow();
      break;
    }

    case FRONT_TURN:
      if (motion == 's') waitReading(FRONT_LOOK);
      break;

    case FRONT_LOOK:
      if (!newReading()) return;
      if (distanceCm <= FRONT_CLEAR_CM) {
        frontTurn();
        return;
      }
      lastGlance = millis();
      follow();
      break;

    case BACKING:
      if (motion == 's') frontTurn();
      break;

    case CORNER:
      if (motion == 's') {
        drive('r', CORNER_TURN_MS);
        step = CORNER_TURN;
      }
      break;

    case CORNER_TURN:
      if (motion == 's') glance();
      break;
  }
}
