#pragma once
#include "../core/core.h"

const int BRAKE_CM = 6;
extern int stopCm;

void setMode(Mode m);
bool modeFromName(const String& name, Mode& out);
void modesUpdate();

void defaultModeStart();
void defaultModeUpdate();

const int MAP_MAX_POINTS = 181;
extern int mapStep;
extern int mapReads;
extern int mapScanStep;
extern long mapDist[MAP_MAX_POINTS];
extern int mapPoints;
extern int mapRoute;
extern unsigned long mapId;
void default2ModeStart();
void default2ModeUpdate();
extern int mapAim;
void default3ModeStart();
void default3ModeUpdate();
void default4ModeStart();
void default4ModeUpdate();

void manualModeStart();
void manualModeUpdate();
void manualModeCommand(char direction);

void followModeStart();
void followModeUpdate();

void radarModeStart();
void radarModeUpdate();
