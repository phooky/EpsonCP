

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
  KEY_COLOR
};

const size_t KB_ROWS = 4;
const size_t KB_COLS = 8;

const uint8_t keymap[4][8] = {
  { KEY_9,      KEY_2,      KEY_REDIAL, KEY_5,
    KEY_MENU,   KEY_0,      KEY_DOWN,   KEY_LEFT,   },
  
  { KEY_SCAN,   KEY_STOP,   KEY_3,      KEY_7,
    0,          KEY_HASH,   KEY_SDGD,   KEY_8,      },
  
  { KEY_FAX,    KEY_1,      KEY_OK,     KEY_RIGHT,
    KEY_STAR,   KEY_BW,     KEY_COLOR,  0,          },
  
  { KEY_COPY,   KEY_UP,     KEY_6,      KEY_4,
    KEY_BACK,   KEY_WIFI,   0,          0,          },
};
