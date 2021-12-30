#include <stdint.h>

enum {
  KEY_COPY = 1,
  KEY_FAX,
  KEY_SCAN,
  KEY_OK,
  KEY_UP,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_DOWN,
  KEY_MENU,
  KEY_BACK,
  KEY_0, KEY_1, KEY_2,
  KEY_3, KEY_4, KEY_5,
  KEY_6, KEY_7, KEY_8,
  KEY_9,
  KEY_STAR,
  KEY_HASH,
  KEY_REDIAL,
  KEY_SDGD,
  KEY_WIFI,
  KEY_STOP,
  KEY_BW,
  KEY_COLOR,

  KEY_LAST
};

enum {
  KB_ROWS = 4,
  KB_COLS = 8,
};

const char* keyname(uint8_t code);

uint8_t keycode(uint8_t row, uint8_t column);

