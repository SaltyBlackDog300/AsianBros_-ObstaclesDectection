#include "modes.h"

const int STOP_CM = 20;
const int NOISE_CM = 3;
const int CLOSE_NEEDED = 3;
const int STUCK_CM = 3;
const unsigned long STUCK_MS = 2000;
const int ROUTES[3] = {0, 90, 180};
const int ROUTE_WIDTH = 15;
const long OPEN_CM = 45;
const int AIM_FROM = 50;
const int AIM_TO = 130;
const int AIM_STEP = 5;
const int AIM_POINTS = (AIM_TO - AIM_FROM) / AIM_STEP + 1;
const int AIM_TOLERANCE = 6;
const long MIN_BURST_MS = 200;
const int MAX_CHECKS = 14;

enum Step { CRUISE, SCANNING, TURNING, AIMING, TURNING_AROUND, BACKING };

static Step step = CRUISE;
static unsigned long lastReading = 0;
static int closeReadings = 0;
static int angleReads = 0;
static long angleDist = 0;
static int scanAngle = 0;
static int readsNeeded = 2;
static char turnDir = 'l';
static int checks = 0;
static int aimIndex = 0;
static long aimDist[AIM_POINTS];
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
  mapAim = -1;
  step = CRUISE;
}

static void startScan() {
  drive('s');
  mapScanStep = constrain(mapStep, 1, 30);
  readsNeeded = constrain(mapReads, 1, 5);
  mapPoints = 180 / mapScanStep + (180 % mapScanStep ? 2 : 1);
  for (int i = 0; i < mapPoints; i++) mapDist[i] = -1;
  mapRoute = -1;
  mapAim = -1;
  mapId++;
  scanAngle = 0;
  lookAt(scanAngle);
  step = SCANNING;
}

static int pointAngle(int i) {
  int angle = i * mapScanStep;
  return angle > 180 ? 180 : angle;
}

static long pickRoute() {
  long bestDist = 0;
  mapRoute = NECK_CENTER;
  for (int r = 0; r < 3; r++) {
    long clear = NO_ECHO_CM;
    for (int i = 0; i < mapPoints; i++) {
      if (abs(pointAngle(i) - ROUTES[r]) <= ROUTE_WIDTH && mapDist[i] < clear) clear = mapDist[i];
    }
    if (clear > bestDist || (clear == bestDist && ROUTES[r] == NECK_CENTER)) {
      bestDist = clear;
      mapRoute = ROUTES[r];
    }
  }
  return bestDist;
}

static int openingCenter(const long* dist, int count, int firstAngle, int stepDeg, int nearAngle, bool& touchesLow, bool& touchesHigh) {
  int bestStart = -1, bestEnd = -1, bestGap = 1000;
  int i = 0;
  while (i < count) {
    if (dist[i] < OPEN_CM) {
      i++;
      continue;
    }
    int start = i;
    while (i < count && dist[i] >= OPEN_CM) i++;
    int end = i - 1;
    int a = firstAngle + start * stepDeg;
    int b = firstAngle + end * stepDeg;
    int gap = nearAngle < a ? a - nearAngle : nearAngle > b ? nearAngle - b : 0;
    if (gap < bestGap) {
      bestGap = gap;
      bestStart = start;
      bestEnd = end;
    }
  }
  if (bestStart < 0) return -1;
  touchesLow = bestStart == 0;
  touchesHigh = bestEnd == count - 1;
  return firstAngle + (bestStart + bestEnd) * stepDeg / 2;
}

static void turnFor(char dir, long ms) {
  if (ms < MIN_BURST_MS) ms = MIN_BURST_MS;
  setNeck(NECK_CENTER);
  drive(dir, ms);
  checks++;
  step = TURNING;
}

static void startTurn() {
  turnDir = mapRoute < NECK_CENTER ? 'r' : 'l';
  bool low = false, high = false;
  int center = openingCenter(mapDist, mapPoints, 0, mapScanStep, mapRoute, low, high);
  if (center > 180) center = 180;
  mapAim = center >= 0 ? center : mapRoute;
  checks = 0;
  turnFor(turnDir, TURN_90_MS * abs(mapAim - NECK_CENTER) / 90 / 2);
}

static void startAim() {
  readsNeeded = 2;
  aimIndex = 0;
  lookAt(AIM_FROM);
  step = AIMING;
}

static void finishAim() {
  setNeck(NECK_CENTER);
  bool low = false, high = false;
  int center = openingCenter(aimDist, AIM_POINTS, AIM_FROM, AIM_STEP, NECK_CENTER, low, high);
  if (checks >= MAX_CHECKS) {
    startScan();
    return;
  }
  if (center < 0) {
    turnFor(turnDir, TURN_90_MS / 6);
    return;
  }
  if (low && high) {
    cruise();
    return;
  }
  int error = center - NECK_CENTER;
  if (abs(error) <= AIM_TOLERANCE) {
    cruise();
    return;
  }
  turnFor(error > 0 ? 'l' : 'r', TURN_90_MS * abs(error) / 90 * 2 / 3);
}

void default3ModeStart() {
  closeReadings = 0;
  resetStuck();
  startScan();
}

void default3ModeUpdate() {
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
      int index = scanAngle >= 180 ? mapPoints - 1 : scanAngle / mapScanStep;
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
        startTurn();
      }
      break;
    }

    case TURNING:
      if (motion == 's') startAim();
      break;

    case AIMING:
      if (!readAngle()) return;
      aimDist[aimIndex] = angleDist;
      if (++aimIndex < AIM_POINTS) {
        lookAt(AIM_FROM + aimIndex * AIM_STEP);
        return;
      }
      finishAim();
      break;

    case BACKING:
    case TURNING_AROUND:
      if (motion == 's') startScan();
      break;
  }
}
