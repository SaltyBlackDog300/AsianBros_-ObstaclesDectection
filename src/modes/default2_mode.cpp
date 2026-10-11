#include "modes.h"

const int STOP_CM = 20;
const int NOISE_CM = 3;
const int CLOSE_NEEDED = 3;
const int STUCK_CM = 3;
const unsigned long STUCK_MS = 2000;
const int ROUTES[3] = {0, 90, 180};
const int ROUTE_WIDTH = 15;
const int ALIGN_STEP = 10;
const int ALIGN_SPAN = 60;
const int ALIGN_TOLERANCE = 10;
const long MIN_BURST_MS = 250;
const int MAX_BURSTS = 12;
const long WALL_SEEN_CM = 25;
const long OPEN_CM = 80;

int mapStep = 5;
int mapReads = 2;
int mapScanStep = 5;
long mapDist[MAP_MAX_POINTS];
int mapPoints = 0;
int mapRoute = -1;
unsigned long mapId = 0;
int mapAim = -1;

enum Step { CRUISE, SCANNING, TURNING, ALIGNING, TURNING_AROUND, BACKING };

static Step step = CRUISE;
static unsigned long lastReading = 0;
static int closeReadings = 0;
static int angleReads = 0;
static long angleDist = 0;
static int scanAngle = 0;
static int readsNeeded = 2;
static long oldFront = NO_ECHO_CM;
static char turnDir = 'r';
static int bursts = 0;
static int alignOffset = 0;
static long wallMin = NO_ECHO_CM;
static int wallMinOffset = 0;
static long stuckRef = 0;
static unsigned long stuckSince = 0;

static void resetStuck() {
  stuckRef = distanceCm;
  stuckSince = millis();
}

static bool isStuck() {
  if (distanceCm >= NO_ECHO_CM || abs(distanceCm - stuckRef) > STUCK_CM) {
    resetStuck();
    return false;
  }
  return millis() - stuckSince > STUCK_MS;
}

static bool readAngle() {
  if (!freshReading() || readingId == lastReading) return false;
  lastReading = readingId;
  if (distanceCm >= NOISE_CM && distanceCm > angleDist) angleDist = distanceCm;
  if (++angleReads < readsNeeded) return false;
  angleReads = 0;
  return true;
}

static void lookAt(int angle) {
  angleReads = 0;
  angleDist = 0;
  lastReading = readingId;
  setNeck(angle);
}

static void cruise() {
  setNeck(NECK_CENTER);
  closeReadings = 0;
  resetStuck();
  step = CRUISE;
}

static void startScan() {
  drive('s');
  mapScanStep = constrain(mapStep, 1, 30);
  readsNeeded = constrain(mapReads, 1, 5);
  mapPoints = 180 / mapScanStep + (180 % mapScanStep ? 2 : 1);
  for (int i = 0; i < mapPoints; i++) mapDist[i] = -1;
  mapRoute = -1;
  mapId++;
  scanAngle = 0;
  lookAt(scanAngle);
  step = SCANNING;
}

static int pointAngle(int i) {
  int angle = i * mapScanStep;
  return angle > 180 ? 180 : angle;
}

static long routeClear(int route) {
  long clear = NO_ECHO_CM;
  for (int i = 0; i < mapPoints; i++) {
    if (abs(pointAngle(i) - route) <= ROUTE_WIDTH && mapDist[i] < clear) clear = mapDist[i];
  }
  return clear;
}

static long pickRoute() {
  long bestDist = 0;
  mapRoute = NECK_CENTER;
  for (int r = 0; r < 3; r++) {
    long clear = routeClear(ROUTES[r]);
    if (ROUTES[r] == NECK_CENTER) oldFront = clear;
    if (clear > bestDist || (clear == bestDist && ROUTES[r] == NECK_CENTER)) {
      bestDist = clear;
      mapRoute = ROUTES[r];
    }
  }
  return bestDist;
}

static int wallNeck(int offset) {
  return turnDir == 'l' ? offset : 180 - offset;
}

static void turnFor(long ms) {
  if (ms < MIN_BURST_MS) ms = MIN_BURST_MS;
  setNeck(NECK_CENTER);
  drive(turnDir, ms);
  bursts++;
  step = TURNING;
}

static void startTurn(char dir) {
  turnDir = dir;
  bursts = 0;
  turnFor(TURN_90_MS / 2);
}

static void startAlign() {
  readsNeeded = 2;
  alignOffset = 0;
  wallMin = NO_ECHO_CM;
  wallMinOffset = 0;
  lookAt(wallNeck(alignOffset));
  step = ALIGNING;
}

static void finishAlign() {
  setNeck(NECK_CENTER);
  bool wallSeen = oldFront < OPEN_CM && wallMin <= oldFront + WALL_SEEN_CM;
  if (!wallSeen) {
    if (bursts >= 2) cruise();
    else turnFor(TURN_90_MS / 2);
    return;
  }
  if (wallMinOffset <= ALIGN_TOLERANCE) {
    cruise();
    return;
  }
  if (bursts >= MAX_BURSTS) {
    startScan();
    return;
  }
  turnFor(TURN_90_MS * wallMinOffset / 90 * 2 / 3);
}

void default2ModeStart() {
  closeReadings = 0;
  resetStuck();
  startScan();
}

void default2ModeUpdate() {
  switch (step) {
    case CRUISE:
      setNeck(NECK_CENTER);
      if (!freshReading()) {
        drive('s');
        return;
      }
      if (readingId == lastReading) return;
      lastReading = readingId;
      if (distanceCm < NOISE_CM) return;
      if (motion == 'f' && isStuck()) {
        drive('b', BACK_MS);
        step = BACKING;
        return;
      }
      closeReadings = distanceCm <= STOP_CM ? closeReadings + 1 : 0;
      if (closeReadings >= CLOSE_NEEDED) startScan();
      else if (motion != 'f') drive('f');
      break;

    case SCANNING: {
      if (!readAngle()) return;
      int index = scanAngle / mapScanStep;
      if (scanAngle == 180) index = mapPoints - 1;
      mapDist[index] = angleDist;
      if (scanAngle < 180) {
        scanAngle = scanAngle + mapScanStep > 180 ? 180 : scanAngle + mapScanStep;
        lookAt(scanAngle);
        return;
      }
      setNeck(NECK_CENTER);
      long best = pickRoute();
      if (best <= STOP_CM) {
        drive('r', TURN_90_MS * 2);
        step = TURNING_AROUND;
      } else if (mapRoute == NECK_CENTER) {
        cruise();
      } else {
        startTurn(mapRoute < NECK_CENTER ? 'r' : 'l');
      }
      break;
    }

    case TURNING:
      if (motion == 's') startAlign();
      break;

    case ALIGNING:
      if (!readAngle()) return;
      if (angleDist >= NOISE_CM && angleDist < wallMin) {
        wallMin = angleDist;
        wallMinOffset = alignOffset;
      }
      if (alignOffset < ALIGN_SPAN) {
        alignOffset += ALIGN_STEP;
        lookAt(wallNeck(alignOffset));
        return;
      }
      finishAlign();
      break;

    case BACKING:
    case TURNING_AROUND:
      if (motion == 's') startScan();
      break;
  }
}
