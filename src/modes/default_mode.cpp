#include "modes.h"

const int STOP_CM = 20;
const int SCAN_FROM = 15;
const int SCAN_TO = 165;
const int SCAN_STEP = 15;
const int STRAIGHT_ZONE = 10;

enum Step { CRUISE, SCANNING, TURNING, TURNING_AROUND };

static Step step = CRUISE;
static int scanAngle = SCAN_FROM;
static int bestAngle = NECK_CENTER;
static long bestDist = 0;
static unsigned long lastReading = 0;
static int closeReadings = 0;

static void startScan() {
  drive('s');
  scanAngle = SCAN_FROM;
  bestDist = 0;
  bestAngle = NECK_CENTER;
  lastReading = readingId;
  setNeck(scanAngle);
  step = SCANNING;
}

static void turnToward(int angle) {
  int offset = angle - NECK_CENTER;
  long ms = TURN_90_MS * (long)abs(offset) / 90;
  if (abs(offset) <= STRAIGHT_ZONE) {
    step = CRUISE;
    return;
  }
  drive(offset < 0 ? 'r' : 'l', ms);
  step = TURNING;
}

void defaultModeStart() {
  step = CRUISE;
  closeReadings = 0;
  lastReading = readingId;
  setNeck(NECK_CENTER);
}

void defaultModeUpdate() {
  switch (step) {
    case CRUISE:
      setNeck(NECK_CENTER);
      if (!freshReading()) {
        drive('s');
        return;
      }
      if (readingId == lastReading) return;
      lastReading = readingId;
      closeReadings = distanceCm <= STOP_CM ? closeReadings + 1 : 0;
      if (closeReadings >= 2) {
        closeReadings = 0;
        startScan();
      } else if (motion != 'f') {
        drive('f');
      }
      break;

    case SCANNING:
      if (!freshReading() || readingId == lastReading) return;
      lastReading = readingId;
      if (distanceCm > bestDist ||
          (distanceCm == bestDist && abs(neckAngle - NECK_CENTER) < abs(bestAngle - NECK_CENTER))) {
        bestDist = distanceCm;
        bestAngle = neckAngle;
      }
      if (scanAngle < SCAN_TO) {
        scanAngle += SCAN_STEP;
        setNeck(scanAngle);
        return;
      }
      setNeck(NECK_CENTER);
      if (bestDist <= STOP_CM) {
        drive('r', TURN_90_MS * 2);
        step = TURNING_AROUND;
      } else {
        turnToward(bestAngle);
      }
      break;

    case TURNING:
      if (motion == 's') step = CRUISE;
      break;

    case TURNING_AROUND:
      if (motion == 's') startScan();
      break;
  }
}
