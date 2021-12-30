#include "keymap.h"

const uint8_t keymap[KB_ROWS][KB_COLS] = {
  { KEY_9,      KEY_2,      KEY_REDIAL, KEY_5,
    KEY_MENU,   KEY_0,      KEY_DOWN,   KEY_LEFT,   },
  
  { KEY_SCAN,   KEY_STOP,   KEY_3,      KEY_7,
    0,          KEY_HASH,   KEY_SDGD,   KEY_8,      },
  
  { KEY_FAX,    KEY_1,      KEY_OK,     KEY_RIGHT,
    KEY_STAR,   KEY_BW,     KEY_COLOR,  0,          },
  
  { KEY_COPY,   KEY_UP,     KEY_6,      KEY_4,
    KEY_BACK,   KEY_WIFI,   0,          0,          },
};

const char* keynames[KEY_LAST] = {
  [KEY_COPY] = "Copy",
  [KEY_FAX] = "Fax",
  [KEY_SCAN] = "Scan",
  [KEY_OK] = "OK",
  [KEY_UP] = "Up",
  [KEY_LEFT] = "Left",
  [KEY_RIGHT] = "Right",
  [KEY_DOWN] = "Down",
  [KEY_MENU] = "Menu",
  [KEY_BACK] = "Back",
  
  [KEY_1] = "1",
  [KEY_2] = "2",
  [KEY_3] = "3",
  [KEY_4] = "4",
  [KEY_5] = "5",
  [KEY_6] = "6",
  [KEY_7] = "7",
  [KEY_8] = "8",
  [KEY_9] = "9",
  [KEY_0] = "0",

  [KEY_STAR] = "*",
  [KEY_HASH] = "#",
  [KEY_REDIAL] = "Redial/Pause",
  [KEY_SDGD] = "Speed Dial/Group Dial",
  [KEY_WIFI] = "Wi-Fi Setup",
  [KEY_STOP] = "Stop/Reset",
  [KEY_BW] = "Start B&W",
  [KEY_COLOR] = "Start Color",

};

const char* keyname(uint8_t code) {
  return keynames[code];
}

uint8_t keycode(uint8_t row, uint8_t column) {
  return keymap[row][column];
}
