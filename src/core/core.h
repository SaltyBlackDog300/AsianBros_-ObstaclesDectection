#pragma once
#include <Arduino.h>

const uint8_t LEFT_MOTOR_PINS[4] = {2, 3, 4, 5};
const uint8_t RIGHT_MOTOR_PINS[4] = {6, 7, 8, 9};
const int SERVO_PIN = 10;
const int TRIG_PIN = 11;
const int ECHO_PIN = 12;

enum Mode { MODE_STOP, MODE_DEFAULT, MODE_MANUAL, MODE_FOLLOW, MODE_RADAR, MODE_DEFAULT2, MODE_DEFAULT3, MODE_DEFAULT4, MODE_COUNT };
extern Mode mode;
extern const char* const MODE_NAMES[MODE_COUNT];

const long BACK_MS = 600;
extern long turn90Ms;
#define TURN_90_MS turn90Ms
extern char motion;
extern int turnPercent;
extern int drivePercent;
void motorsBegin();
void motorsUpdate();
void drive(char direction);
void drive(char direction, long ms);

const int NECK_CENTER = 90;
const int NECK_RIGHT = 10;
const int NECK_LEFT = 170;
const long NO_ECHO_CM = 999;
extern long distanceCm;
extern int neckAngle;
extern int radar[19];
extern unsigned long readingId;
extern float speedCms;
extern float peakSpeedCms;
void sensorBegin();
void sensorUpdate();
bool freshReading();
void setNeck(int angle);

void displayBegin();
void displayMode(Mode m);
