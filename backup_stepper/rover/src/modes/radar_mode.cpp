#include "modes.h"

static int angle = 0;
static int stepDeg = 10;

void radarModeStart() {
  angle = 0;
  stepDeg = 10;
  setNeck(0);
}

void radarModeUpdate() {
  if (motion != 's') drive('s');
  if (!freshReading()) return;
  angle += stepDeg;
  if (angle >= 180 || angle <= 0) {
    angle = constrain(angle, 0, 180);
    stepDeg = -stepDeg;
  }
  setNeck(angle);
}
