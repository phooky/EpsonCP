#!/usr/bin/python3

import freetype
import sys
import subprocess
import struct
from base64 import b64encode

class Glyph:
    def __init__(self,w,h,left,top,bitmap):
        self.w = w
        self.h = h
        self.left = left
        self.top = top
        self.bitmap = bitmap

    def even_h(self):
        "Modify glyph to support an even number of rows."
        if (self.h%2) == 1:
            self.h = self.h + 1
            self.top = self.top + 1
            self.bitmap = ([0] * self.w) + self.bitmap

    def squash(self):
        "Squash font on vertical axis by a factor of 2."
        self.h = int(self.h/2)
        self.top = int(self.top/2)
        bm = [0] * int(self.w * self.h)
        for y in range(self.h):
            for x in range(self.w):
                bm[x+y*self.w] = 0
                bm[x+y*self.w] = int( (self.bitmap[x+(y*2*self.w)] +
                                       self.bitmap[x+((y*2+1)*self.w)])/2)
        self.bitmap = bm

    def reduce_colors(self):
        "Reduce bitmap to 2bpp."
        self.bitmap = [ min((x + 0x1f),0xff) >> 6 for x in self.bitmap ]
    
        
    def kitty(self):
        "Write modified glyph to terminal via Kitty graphical codes."
        b = b'' #b'\00\00\00' * self.left
        for y in range(self.h):
            st = self.w * y
            en = st + self.w
            for _ in range(2):
                b = b + bytes([int(x*255/4) for x in self.bitmap[st:en] for _ in range(3)])
                
        esc = b'\033'
        body ='_Gf=24,s={},v={},a=T;'.format(self.w, self.h*2).encode('ascii')
        sys.stdout.buffer.write(esc+body+b64encode(b)+esc+b'\\')
        sys.stdout.flush()

    def pack_2bpp(self):
        "Return bitmap data in 2bpp representation."
        bytew = int( (self.w+3)/4 )
        packed = bytearray()
        for y in range(self.h):
            row = bytearray(bytew)
            for x in range(self.w):
                src = self.bitmap[(y*self.w) + x]
                byteidx = int(x / 4)
                pixidx = x % 4
                row[byteidx] |= src << (pixidx*2)
            packed += row
        return packed


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
    
class EpsonFont:
    def __init__(self, face, pxsize):
        self.ascii_map = [None] * 128
        self.face = face
        self.face.set_char_size( pxsize * 64 ) # adjust to 16.6 fixed-point format
        
    def add_glyph(self, char):
        cs = char.encode('ascii')
        if len(cs) > 1 or cs[0] > 127:
            print("Character {} is out of ASCII range".format(char))
            return
        self.face.load_char(char)
        bitmap = self.face.glyph.bitmap
        g = Glyph(bitmap.width, bitmap.rows,
                  self.face.glyph.bitmap_left,
                  self.face.glyph.bitmap_top,
                  bitmap.buffer.copy())
        if char == ' ': # spaces have empty bitmaps, just advance the cursor
            g.w = self.face.glyph.metrics.horiAdvance >> 6 # it's in 16.6 fixed-point format
        g.even_h()
        g.squash()
        g.reduce_colors()
        self.ascii_map[cs[0]] = g

    def write_font(self):
        "Return total font as a bytearray"
        min_ch = len(self.ascii_map)
        max_ch = -1
        ascender = 0
        descender = 0
        for (i,g) in enumerate(self.ascii_map):
            if not g:
                continue
            min_ch,max_ch = min(min_ch,i),max(max_ch,i)
            ascender = max(ascender,g.top)
            descender = max(descender,g.h-g.top)
        baseline = ascender
        height = ascender+descender
        if min_ch > max_ch:
            print("Font is empty")
            return None
        data = bytearray([baseline,height,min_ch,max_ch])
        data += bytes([0,0] * (1 + max_ch - min_ch))
        for i in range(min_ch, max_ch+1):
            lookup_off = 4 + (i - min_ch)*2
            data[lookup_off:lookup_off+2] = struct.pack("<H",len(data))
            g = self.ascii_map[i]
            if g:
                top = height - g.top
                bot = g.h + top
                pw = g.w
                bw = int( (g.w+3)/4 )
                data += bytearray([top,bot,pw,bw])
                data += g.pack_2bpp()
        return data

import argparse
import string

def font_as_C_header(font):
    fontstr = ''
    while font:
        fontstr += '    '
        line = font[:12]
        font = font[12:]
        ldat = ', '.join(map(lambda x:'0x{:02x}'.format(x), line))
        fontstr += ldat + ',\n'
    return """
#ifndef __FONT_{GUARD}_H__
#define __FONT_{GUARD}_H__

#include <stdint.h>

uint8_t font_{FN}[] = {{
{DATA}
}}

#endif // __FONT_{GUARD}_H__
""".format(GUARD=args.font.upper(),FN=args.font,DATA=fontstr)
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate EpsonCP fonts from freetype fonts.')
    parser.add_argument('font', default='hack', help='Name of the font to process')
    parser.add_argument('size', type=int, default=24, nargs='?', help='Size in pixels of font')
    args = parser.parse_args()
    lookup_result = subprocess.run(['fc-match', '-f', '%{file}', args.font ],
                                   capture_output=True,
                                   text=True)
    if lookup_result.returncode != 0:
        print("Couldn't find font, exiting")
        sys.exit(1)

    font_path = lookup_result.stdout;
    face = freetype.Face(font_path)
    ef = EpsonFont(face,args.size)
    for c in string.ascii_lowercase + string.ascii_uppercase + string.digits + " .'!":
        ef.add_glyph(c)
    font = ef.write_font()
    sys.stderr.write("Font {}, binary size {}\n".format(args.font,len(font)))
    print(font_as_C_header(font))
