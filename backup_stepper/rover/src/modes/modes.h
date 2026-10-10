#pragma once
#include "../core/core.h"

const int BRAKE_CM = 12;

void setMode(Mode m);
bool modeFromName(const String& name, Mode& out);
void modesUpdate();

void defaultModeStart();
void defaultModeUpdate();

void manualModeStart();
void manualModeUpdate();
void manualModeCommand(char direction);

void followModeStart();
void followModeUpdate();

void radarModeStart();
void radarModeUpdate();
