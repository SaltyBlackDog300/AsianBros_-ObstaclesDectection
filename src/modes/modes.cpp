#include "modes.h"

void setMode(Mode m) {
  mode = m;
  drive('s');
  switch (m) {
    case MODE_DEFAULT: defaultModeStart(); break;
    case MODE_DEFAULT2: default2ModeStart(); break;
    case MODE_DEFAULT3: default3ModeStart(); break;
    case MODE_DEFAULT4: default4ModeStart(); break;
    case MODE_MANUAL:  manualModeStart();  break;
    case MODE_FOLLOW:  followModeStart();  break;
    case MODE_RADAR:   radarModeStart();   break;
    default:           setNeck(NECK_CENTER);
  }
  displayMode(m);
  Serial.print("Mode: ");
  Serial.println(MODE_NAMES[m]);
}

bool modeFromName(const String& name, Mode& out) {
  for (int i = 0; i < MODE_COUNT; i++) {
    if (name == MODE_NAMES[i]) {
      out = (Mode)i;
      return true;
    }
  }
  return false;
}

static void safetyBrake() {
  if (motion == 'f' && abs(neckAngle - NECK_CENTER) <= 20 && freshReading() && distanceCm < BRAKE_CM) {
    drive('s');
  }
}

void modesUpdate() {
  switch (mode) {
    case MODE_DEFAULT: defaultModeUpdate(); break;
    case MODE_DEFAULT2: default2ModeUpdate(); break;
    case MODE_DEFAULT3: default3ModeUpdate(); break;
    case MODE_DEFAULT4: default4ModeUpdate(); break;
    case MODE_MANUAL:  manualModeUpdate();  break;
    case MODE_FOLLOW:  followModeUpdate();  break;
    case MODE_RADAR:   radarModeUpdate();   break;
    default:           if (motion != 's') drive('s');
  }
  safetyBrake();
}
