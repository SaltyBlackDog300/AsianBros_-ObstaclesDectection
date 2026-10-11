#pragma once
#include "../core/core.h"

const int BRAKE_CM = 6;
extern int stopCm;

void setMode(Mode m);
bool modeFromName(const String& name, Mode& out);
void modesUpdate();

void defaultModeStart();
void defaultModeUpdate();

extern long corridorCm;
extern float cruiseCms;
const char* autoPhase();

void manualModeStart();
void manualModeUpdate();
void manualModeCommand(char direction);

void followModeStart();
void followModeUpdate();

void radarModeStart();
void radarModeUpdate();
