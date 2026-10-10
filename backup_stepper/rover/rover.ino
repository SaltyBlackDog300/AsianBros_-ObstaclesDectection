#include "src/core/core.h"
#include "src/modes/modes.h"
#include "src/web/web_server.h"

void setup() {
  Serial.begin(9600);
  motorsBegin();
  sensorBegin();
  displayBegin();
  webBegin();
  setMode(MODE_STOP);
}

void loop() {
  motorsUpdate();
  webUpdate();
  motorsUpdate();
  sensorUpdate();
  motorsUpdate();
  modesUpdate();
}
