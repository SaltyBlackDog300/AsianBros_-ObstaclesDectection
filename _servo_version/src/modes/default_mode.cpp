#include "modes.h"

// Fully automatic maze solver: right-hand wall follower.
// Nothing is hand-tuned: stop distance, corridor width, cruise speed and
// turn rate are all measured by the rover itself while it drives.

int stopCm = 22;
long corridorCm = 40;
float cruiseCms = 25;
static float turnDegPerMs = 90.0f / 1200;

const int NOISE_CM = 3;
const int SCAN_STEP = 10;
const int SCAN_POINTS = 180 / SCAN_STEP + 1;
const int AIM_FROM = 50;
const int AIM_TO = 130;
const int AIM_STEP = 10;
const int AIM_POINTS = (AIM_TO - AIM_FROM) / AIM_STEP + 1;
const int AIM_OK_DEG = 8;
const int MAX_AIMS = 6;
const int SEARCH_DEG = 25;
const int RIGHT_DIAG = 45;
const int LEFT_DIAG = 135;
const int GLANCES[4] = {NECK_CENTER, RIGHT_DIAG, NECK_CENTER, LEFT_DIAG};
const float DIAG = 0.7071f;
const int CLOSE_NEEDED = 2;
const int OPEN_NEEDED = 2;
const int STUCK_CM = 3;
const unsigned long STUCK_MS = 2000;
const long MIN_BURST_MS = 60;
const float LEARN = 0.3f;

enum Step { SCANNING, TURNING, AIMING, CRUISE, STEERING, ADVANCING, BACKING, TURNING_AROUND };
static const char* const STEP_NAMES[] = {"scanning", "turning", "aiming", "cruising", "centering", "advancing", "backing up", "turning around"};

static Step step = SCANNING;
static unsigned long lastReading = 0;
static int angleReads = 0;
static long angleDist = NO_ECHO_CM;
static int pointIndex = 0;
static long scanDist[SCAN_POINTS];
static long aimDist[AIM_POINTS];
static int aims = 0;
static float errBefore = 0;
static bool errKnown = false;
static float turnCmdDeg = 0;
static long turnCmdMs = 0;
static int glance = 0;
static int closeReadings = 0;
static int openReadings = 0;
static long rightLat = -1;
static long leftLat = -1;
static unsigned long rightOpenFrom = 0;
static long prevFront = -1;
static unsigned long prevFrontAt = 0;
static long stuckRef = -1;
static unsigned long stuckSince = 0;

const char* autoPhase() {
  return STEP_NAMES[step];
}

static long openCm() {
  return constrain(corridorCm * 6 / 5, 30L, 100L);
}

static void relearn() {
  turn90Ms = (long)(90.0f / turnDegPerMs + 0.5f);
  stopCm = constrain(corridorCm * 35 / 100 + 8, 12L, 30L);
}

static bool newReading() {
  if (!freshReading() || readingId == lastReading) return false;
  lastReading = readingId;
  return true;
}

static bool isStuck(long front) {
  if (front >= NO_ECHO_CM || abs(front - stuckRef) > STUCK_CM) {
    stuckRef = front;
    stuckSince = millis();
    return false;
  }
  return millis() - stuckSince > STUCK_MS;
}

static void lookAt(int angle) {
  angleReads = 0;
  angleDist = NO_ECHO_CM;
  lastReading = readingId;
  setNeck(angle);
}

// Shortest valid echo of two pings: a missed echo (grazing wall) must not look like an opening.
static bool readAngle() {
  if (!newReading()) return false;
  if (distanceCm >= NOISE_CM && distanceCm < angleDist) angleDist = distanceCm;
  if (++angleReads < 2) return false;
  angleReads = 0;
  return true;
}

static void turnBy(float deg, Step next) {
  setNeck(NECK_CENTER);
  turnCmdDeg = deg;
  turnCmdMs = (long)(fabs(deg) / turnDegPerMs);
  if (turnCmdMs < MIN_BURST_MS) turnCmdMs = MIN_BURST_MS;
  drive(deg > 0 ? 'l' : 'r', turnCmdMs);
  step = next;
}

// Turn rate is learned from how far the opening actually moved after a timed turn.
static void learnTurn(float actualDeg) {
  if (fabs(turnCmdDeg) < 15 || fabs(actualDeg) < 8) return;
  if ((actualDeg > 0) != (turnCmdDeg > 0)) return;
  float rate = constrain(fabs(actualDeg) / turnCmdMs, 90.0f / 4000, 90.0f / 250);
  turnDegPerMs = turnDegPerMs * (1 - LEARN) + rate * LEARN;
  relearn();
}

static void learnSpeed(long front) {
  unsigned long now = millis();
  if (prevFront > 0 && front < NO_ECHO_CM && now - prevFrontAt >= 200) {
    float cms = (prevFront - front) * 1000.0f / (now - prevFrontAt) * 100 / drivePercent;
    if (cms > 3 && cms < 200) cruiseCms = cruiseCms * (1 - LEARN) + cms * LEARN;
  }
  prevFront = front < NO_ECHO_CM ? front : -1;
  prevFrontAt = now;
}

// Runs of open readings. rightmost = right-hand rule, otherwise the run closest to straight ahead.
static bool findGap(const long* dist, int count, int firstAngle, int stepDeg, int minPoints, bool rightmost, int& center, bool& edge) {
  long open = openCm();
  bool found = false;
  int bestScore = 1000;
  int i = 0;
  while (i < count) {
    if (dist[i] < open) {
      i++;
      continue;
    }
    int start = i;
    while (i < count && dist[i] >= open) i++;
    int end = i - 1;
    if (end - start + 1 < minPoints) continue;
    int c = firstAngle + (start + end) * stepDeg / 2;
    int score = rightmost ? c : abs(c - NECK_CENTER);
    if (score < bestScore) {
      bestScore = score;
      center = c;
      edge = start == 0 || end == count - 1;
      found = true;
    }
  }
  return found;
}

static void cruise(bool cooldown) {
  setNeck(NECK_CENTER);
  glance = 0;
  closeReadings = 0;
  openReadings = 0;
  rightLat = -1;
  leftLat = -1;
  prevFront = -1;
  stuckRef = -1;
  float cms = cruiseCms < 5 ? 5 : cruiseCms;
  if (cooldown) rightOpenFrom = millis() + (unsigned long)(corridorCm * 1000 / cms);
  lastReading = readingId;
  step = CRUISE;
}

static void startScan() {
  drive('s');
  pointIndex = 0;
  lookAt(0);
  step = SCANNING;
}

static void startAim() {
  aims++;
  pointIndex = 0;
  lookAt(AIM_FROM);
  step = AIMING;
}

// Right opening spotted: roll forward until the wheels sit level with its middle, then look properly.
static void startAdvance() {
  setNeck(NECK_CENTER);
  float cms = cruiseCms * drivePercent / 100;
  if (cms < 5) cms = 5;
  drive('f', (long)(corridorCm * 0.9f * 1000 / cms));
  lastReading = readingId;
  step = ADVANCING;
}

static void finishScan() {
  setNeck(NECK_CENTER);
  long r = scanDist[0];
  long l = scanDist[SCAN_POINTS - 1];
  if (r < 120 && l < 120 && r + l >= 15 && r + l <= 150) {
    corridorCm = (long)(corridorCm * (1 - LEARN) + (r + l) * LEARN + 0.5f);
    relearn();
  }
  int center;
  bool edge;
  if (!findGap(scanDist, SCAN_POINTS, 0, SCAN_STEP, 2, true, center, edge)) {
    errKnown = false;
    turnBy(180, TURNING_AROUND);
    return;
  }
  float err = center - NECK_CENTER;
  aims = 0;
  if (fabs(err) <= AIM_OK_DEG) {
    cruise(true);
    return;
  }
  errBefore = err;
  errKnown = !edge;
  turnBy(err * 0.8f, TURNING);
}

static void finishAim() {
  setNeck(NECK_CENTER);
  int center;
  bool edge;
  if (!findGap(aimDist, AIM_POINTS, AIM_FROM, AIM_STEP, 1, false, center, edge)) {
    errKnown = false;
    if (aims >= MAX_AIMS) startScan();
    else turnBy(turnCmdDeg >= 0 ? SEARCH_DEG : -SEARCH_DEG, TURNING);
    return;
  }
  float err = center - NECK_CENTER;
  if (errKnown && !edge) learnTurn(errBefore - err);
  if (fabs(err) <= AIM_OK_DEG || aims >= MAX_AIMS) {
    cruise(true);
    return;
  }
  errBefore = err;
  errKnown = !edge;
  turnBy(err, TURNING);
}

static void nextGlance() {
  glance = (glance + 1) % 4;
  setNeck(GLANCES[glance]);
}

static void frontReading(long d) {
  if (motion == 'f' && isStuck(d)) {
    drive('b', BACK_MS);
    step = BACKING;
    return;
  }
  if (d <= stopCm) {
    if (++closeReadings >= CLOSE_NEEDED) startScan();
    return;
  }
  closeReadings = 0;
  learnSpeed(d);
  int pct = d >= NO_ECHO_CM ? 100 : constrain(40 + (int)(d - stopCm) * 2, 40, 100);
  if (pct != drivePercent) {
    drivePercent = pct;
    drive('f');
  }
  nextGlance();
}

static void sideReading(long d) {
  if (d < NOISE_CM) return;
  long wall = corridorCm * 9 / 10;
  long lat = d >= NO_ECHO_CM ? -1 : (long)(d * DIAG);
  bool open = lat < 0 || lat >= wall;
  if (neckAngle == RIGHT_DIAG) {
    if (open && millis() >= rightOpenFrom) {
      if (++openReadings >= OPEN_NEEDED) startAdvance();
      return;
    }
    openReadings = 0;
    rightLat = open ? -1 : lat;
  } else {
    leftLat = open ? -1 : lat;
  }
  if (rightLat > 0 && leftLat > 0) {
    long err = rightLat - leftLat;
    long band = corridorCm * 12 / 100 > 5 ? corridorCm * 12 / 100 : 5;
    rightLat = -1;
    leftLat = -1;
    if (abs(err) > band) {
      float deg = constrain(abs(err) * 0.6f, 4.0f, 15.0f);
      turnBy(err > 0 ? -deg : deg, STEERING);
      return;
    }
  }
  nextGlance();
}

void defaultModeStart() {
  drivePercent = 100;
  relearn();
  startScan();
}

void defaultModeUpdate() {
  switch (step) {
    case SCANNING:
      if (!readAngle()) return;
      scanDist[pointIndex] = angleDist;
      if (++pointIndex < SCAN_POINTS) {
        lookAt(pointIndex * SCAN_STEP);
        return;
      }
      finishScan();
      break;

    case TURNING:
      if (motion == 's') startAim();
      break;

    case AIMING:
      if (!readAngle()) return;
      aimDist[pointIndex] = angleDist;
      if (++pointIndex < AIM_POINTS) {
        lookAt(AIM_FROM + pointIndex * AIM_STEP);
        return;
      }
      finishAim();
      break;

    case CRUISE:
      if (motion != 'f') drive('f');
      if (!newReading()) return;
      if (neckAngle == NECK_CENTER) frontReading(distanceCm);
      else sideReading(distanceCm);
      break;

    case STEERING:
      if (motion == 's') cruise(false);
      break;

    case ADVANCING:
      if (motion == 's') {
        startScan();
        return;
      }
      if (newReading() && distanceCm <= stopCm) startScan();
      break;

    case BACKING:
    case TURNING_AROUND:
      if (motion == 's') startScan();
      break;
  }
}
