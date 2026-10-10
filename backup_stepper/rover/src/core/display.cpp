#include "core.h"
#include "Arduino_LED_Matrix.h"

static ArduinoLEDMatrix matrix;

static const uint8_t GLYPHS[MODE_COUNT][5] = {
  {0b111, 0b100, 0b111, 0b001, 0b111},
  {0b110, 0b101, 0b101, 0b101, 0b110},
  {0b101, 0b111, 0b111, 0b101, 0b101},
  {0b111, 0b100, 0b110, 0b100, 0b100},
  {0b110, 0b101, 0b110, 0b101, 0b101},
};

void displayBegin() {
  matrix.begin();
}

void displayMode(Mode m) {
  uint8_t frame[8][12] = {0};
  for (int row = 0; row < 5; row++) {
    for (int col = 0; col < 3; col++) {
      if (GLYPHS[m][row] & (4 >> col)) {
        frame[row + 1][3 + col * 2] = 1;
        frame[row + 1][4 + col * 2] = 1;
      }
    }
  }
  matrix.renderBitmap(frame, 8, 12);
}
