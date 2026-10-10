#include "core.h"
#include <Servo.h>

static Servo neck;
static unsigned long lastPingAt = 0;
static unsigned long neckSettleAt = 0;

long distanceCm = NO_ECHO_CM;
int neckAngle = NECK_CENTER;
int radar[19];
unsigned long readingId = 0;

static long readCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long us = pulseIn(ECHO_PIN, HIGH, 7000UL);
  return us == 0 ? NO_ECHO_CM : us / 58;
}

void sensorBegin() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  for (int i = 0; i < 19; i++) radar[i] = 250;
  neck.attach(SERVO_PIN);
  neck.write(NECK_CENTER);
}

bool freshReading() {
  return lastPingAt >= neckSettleAt;
}

void setNeck(int angle) {
  if (angle == neckAngle) return;
  unsigned long settle = 80 + (unsigned long)abs(angle - neckAngle) * 3;
  neckAngle = angle;
  neck.write(angle);
  neckSettleAt = millis() + settle;
}

void sensorUpdate() {
  if (millis() - lastPingAt < 60) return;
  distanceCm = readCm();
  lastPingAt = millis();
  readingId++;
  if (freshReading() && neckAngle % 10 == 0) {
    radar[neckAngle / 10] = distanceCm > 250 ? 250 : distanceCm;
  }
}
