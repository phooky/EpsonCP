#include "font.h"
#include "video.h"
/*
typdef struct {
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
*/
void render_text(uint16_t x, uint16_t y, char* string, uint8_t* font) {
  font_header* fhdr = (font_header*)font;
  while (*string != '\0') {
    char c = *(string++);
    if (c < fhdr->first_ch || c > fhdr->last_ch) continue;
    uint16_t coff = fhdr->lookup_table[c - fhdr->first_ch];
    character_data* cdat = (character_data*)(font + coff);
    for (uint8_t cy = cdat->top; cy < cdat->bottom; cy++) {
      uint8_t lineoff = cdat->bw * (cy - cdat->top);
      for (uint8_t cx = 0; cx < cdat->pw; cx++) {
	uint8_t db = cdat->data[ lineoff + (cx/4) ];
	framebuf[cx + x][cy + y] |= (db >> ((cx%4)*2) & 0x03) << 6;
      }
    }
    x = x + cdat->pw + 1;
  }
}


/*
#| offset | name       | size | description |
#|--------|------------|------|-------------|
#| 0x0000 | Baseline   | 1    | Distance to baseline of top of text |
#| 0x0001 | Height     | 1    | Line height |
#| 0x0002 | First char | 1    | The ASCII code of the first character in the font |
#| 0x0003 | Last char  | 1    | The ASCII code of the last character in the font |
#| 0x0004 | Lookup     | 2*last-first | The offset  |
#| CC*2+4 | Char data  | ...  | |

# char data:
#| + 0 | top  | 1 | distance from top of line to top of bitmap
#| + 1 | bot  | 1 | distance from top of line to bottom of bitmap
#| + 2 | pw   | 1 | width in pixels of a line
#| + 3 | bw   | 1 | width in bytes of a line
#| + 4 | data | bw*(bot-top) | packed character data (4 pix / byte)
        

# test retrieving a glyph
def getglyph(font,char):
    code = char.encode('ascii')[0]
    fc, lc = font[2],font[3]
    if code < fc or code > lc:
        print("char out of range: {}".format(char))
        return
    height = font[1]
    lidx = 4 + ((code - fc)*2)
    gidx = (font[lidx+1] << 8) + font[lidx]
    width = font[gidx+2]
    # kitty output
    b = bytearray(b'\033_Gf=24,')
    b += 's={},v={},a=T;'.format(width, height*2).encode('ascii')
    top,bot = font[gidx], font[gidx+1]
    pw,bw = font[gidx+2], font[gidx+3]
    d = bytearray()
    for y in range(height):
        for _ in range(2):
            if y < top or y >= bot:
                d += b'\00\00\00' * width
            else:
                lineidx = gidx + 4 + (y-top)*bw
                for x in range(pw):
                    byoff = int(x/4)
                    bioff = (x%4) * 2
                    v = (font[lineidx + byoff] >> bioff) & 0x03
                    d.append(v * 85)
                    d.append(v * 85)
                    d.append(v * 85)
    b += b64encode(d)
    b += b'\033'
    b += b'\\'
    sys.stdout.buffer.write(bytes(b))
    sys.stdout.flush()

*/
