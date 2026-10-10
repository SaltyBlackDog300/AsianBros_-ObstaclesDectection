#include "modes.h"

const unsigned long COMMAND_TIMEOUT_MS = 500;

static unsigned long lastCommandAt = 0;

void manualModeStart() {
  setNeck(NECK_CENTER);
}

void manualModeCommand(char direction) {
  if (direction == 'f' && distanceCm < BRAKE_CM) direction = 's';
  drive(direction);
  lastCommandAt = millis();
}

void manualModeUpdate() {
  if (motion != 's' && millis() - lastCommandAt > COMMAND_TIMEOUT_MS) drive('s');
}
