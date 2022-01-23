#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

typedef struct {
  uint8_t baseline;
  uint8_t height;
  uint8_t first_ch;
  uint8_t last_ch;
  uint16_t lookup_table[];
} font_header;

typedef struct {
  uint8_t top;     // from top of line to top of char
  uint8_t bottom;  // from top of line to bottom of char
  uint8_t pw;      // width of char in pixels
  uint8_t bw;      // width of char in bytes
  uint8_t data[];  // character data packed in 2bpp
} character_data;

void render_text(uint16_t x, uint16_t y, char* string, uint8_t* font);


#endif // __FONT_H__
