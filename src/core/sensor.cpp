#include "core.h"
#include <Servo.h>

static Servo neck;
static unsigned long lastPingAt = 0;
static unsigned long neckSettleAt = 0;

long distanceCm = NO_ECHO_CM;
int neckAngle = NECK_CENTER;
int radar[19];
unsigned long readingId = 0;
float speedCms = -1;
float peakSpeedCms = 0;

const int SPEED_SAMPLES = 12;
const unsigned long SPEED_WINDOW_MS = 600;
const int SPEED_MIN_SAMPLES = 4;

static unsigned long sampleAt[SPEED_SAMPLES];
static long sampleCm[SPEED_SAMPLES];
static int sampleCount = 0;
static int sampleHead = 0;

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

static void clearSpeed() {
  sampleCount = 0;
  speedCms = -1;
}

static void updateSpeed() {
  bool usable = motion == 'f' && neckAngle == NECK_CENTER && freshReading() && distanceCm < NO_ECHO_CM;
  if (!usable) {
    clearSpeed();
    return;
  }
  sampleAt[sampleHead] = lastPingAt;
  sampleCm[sampleHead] = distanceCm;
  sampleHead = (sampleHead + 1) % SPEED_SAMPLES;
  if (sampleCount < SPEED_SAMPLES) sampleCount++;

  float n = 0, sumT = 0, sumD = 0, sumTT = 0, sumTD = 0;
  for (int i = 0; i < sampleCount; i++) {
    unsigned long age = lastPingAt - sampleAt[i];
    if (age > SPEED_WINDOW_MS) continue;
    float t = -(float)age / 1000.0f;
    float d = sampleCm[i];
    n++;
    sumT += t;
    sumD += d;
    sumTT += t * t;
    sumTD += t * d;
  }
  float spread = n * sumTT - sumT * sumT;
  if (n < SPEED_MIN_SAMPLES || spread <= 0) {
    speedCms = -1;
    return;
  }
  float slope = (n * sumTD - sumT * sumD) / spread;
  speedCms = slope < 0 ? -slope : 0;
  if (speedCms > peakSpeedCms) peakSpeedCms = speedCms;
}

void sensorUpdate() {
  if (millis() - lastPingAt < 60) return;
  distanceCm = readCm();
  lastPingAt = millis();
  readingId++;
  updateSpeed();
  if (freshReading() && neckAngle % 10 == 0) {
    radar[neckAngle / 10] = distanceCm > 250 ? 250 : distanceCm;
  }
}
