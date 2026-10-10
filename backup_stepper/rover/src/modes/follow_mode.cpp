#include "modes.h"

const int TOO_CLOSE_CM = 5;
const int LOCK_CM = 50;
const int LOST_CM = 70;
const int NEAR_CM = 20;
const int FAR_CM = 40;
const int STEADY_CM = 4;
const int MOVE_MARGIN_CM = 3;
const int MIN_ANGLE = 45;
const int MAX_ANGLE = 135;
const int SEARCH_STEP = 15;
const int TRACK_STEP = 6;
const int CENTER_ZONE = 15;
const int LOST_LIMIT = 3;
const int OFFSETS[3] = {-TRACK_STEP, 0, TRACK_STEP};

enum State { SEARCHING, HOLDING, CHECKING };

static State state = SEARCHING;
static int searchAngle = NECK_CENTER;
static int searchDir = 1;
static int sweepBestAngle = NECK_CENTER;
static long sweepBestDist = NO_ECHO_CM;
static int target = NECK_CENTER;
static long targetDist = NO_ECHO_CM;
static int probe = 0;
static long probeDist[3];
static int misses = 0;
static unsigned long lastReading = 0;

static long validReading() {
  return distanceCm >= TOO_CLOSE_CM ? distanceCm : NO_ECHO_CM;
}

static int probeAngle() {
  return constrain(target + OFFSETS[probe], MIN_ANGLE, MAX_ANGLE);
}

static void steerBody() {
  if (motion != 's') return;
  if (target < NECK_CENTER - CENTER_ZONE)      drive('r', TURN_90_STEPS / 6);
  else if (target > NECK_CENTER + CENTER_ZONE) drive('l', TURN_90_STEPS / 6);
  else if (targetDist > FAR_CM)                drive('f', 400);
  else if (targetDist < NEAR_CM)               drive('b', 300);
}

static void startSearching(int fromAngle) {
  state = SEARCHING;
  sweepBestDist = NO_ECHO_CM;
  searchAngle = constrain(fromAngle, MIN_ANGLE, MAX_ANGLE);
  drive('s');
  setNeck(searchAngle);
}

static void startChecking() {
  state = CHECKING;
  probe = 0;
  setNeck(probeAngle());
}

static void hold(int angle, long dist) {
  state = HOLDING;
  target = angle;
  targetDist = dist;
  misses = 0;
  setNeck(target);
  steerBody();
}

static void updateSearching() {
  long d = validReading();
  if (d < sweepBestDist) {
    sweepBestDist = d;
    sweepBestAngle = neckAngle;
  }
  searchAngle += searchDir * SEARCH_STEP;
  if (searchAngle >= MAX_ANGLE || searchAngle <= MIN_ANGLE) {
    searchAngle = constrain(searchAngle, MIN_ANGLE, MAX_ANGLE);
    searchDir = -searchDir;
    if (sweepBestDist <= LOCK_CM) {
      hold(sweepBestAngle, sweepBestDist);
      return;
    }
    sweepBestDist = NO_ECHO_CM;
  }
  setNeck(searchAngle);
}

static void updateHolding() {
  long d = validReading();
  if (d <= LOST_CM && abs(d - targetDist) <= STEADY_CM) {
    targetDist = d;
    steerBody();
    return;
  }
  startChecking();
}

static void updateChecking() {
  probeDist[probe] = validReading();
  probe++;
  if (probe < 3) {
    setNeck(probeAngle());
    return;
  }

  int best = 1;
  for (int i = 0; i < 3; i++) {
    if (probeDist[i] + MOVE_MARGIN_CM < probeDist[best]) best = i;
  }

  if (probeDist[best] <= LOST_CM) {
    hold(constrain(target + OFFSETS[best], MIN_ANGLE, MAX_ANGLE), probeDist[best]);
  } else if (++misses >= LOST_LIMIT) {
    startSearching(target);
  } else {
    startChecking();
  }
}

void followModeStart() {
  searchDir = 1;
  lastReading = readingId;
  startSearching(NECK_CENTER);
}

void followModeUpdate() {
  if (!freshReading() || readingId == lastReading) return;
  lastReading = readingId;

  switch (state) {
    case SEARCHING: updateSearching(); break;
    case HOLDING:   updateHolding();   break;
    case CHECKING:  updateChecking();  break;
  }
}
