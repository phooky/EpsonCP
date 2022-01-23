#!/usr/bin/python3

import freetype
import sys
import subprocess
import struct
from base64 import b64encode

fn = 'hack'
if len(sys.argv) > 1:
    fn = sys.argv[1]

lookup_result = subprocess.run("fc-match -f %{{file}} {}".format(fn).split(),
                               capture_output=True,
                               text=True)
if lookup_result.returncode != 0:
    print("Couldn't find font, exiting")
    sys.exit(1)

class Glyph:
    def __init__(self,w,h,left,top,bitmap):
        self.w = w
        self.h = h
        self.left = left
        self.top = top
        self.bitmap = bitmap
        self.aspect = 1

    def even_h(self):
        if (self.h%2) == 1:
            self.h = self.h + 1
            self.top = self.top + 1
            self.bitmap = ([0] * self.w) + self.bitmap

    def squash(self):
        self.h = int(self.h/2)
        self.top = int(self.top/2)
        bm = [0] * int(self.w * self.h)
        for y in range(self.h):
            for x in range(self.w):
                bm[x+y*self.w] = 0
                bm[x+y*self.w] = int( (self.bitmap[x+(y*2*self.w)] +
                                       self.bitmap[x+((y*2+1)*self.w)])/2)
        self.aspect = 2
        self.bitmap = bm

    def reduce_colors(self):
        self.bitmap = [ min((x + 0x1f),0xff) >> 6 for x in self.bitmap ]
    
    def print(self):
        print("len {} x {} y {} left {} top {}".format(len(self.bitmap),self.w,self.h,
                                                       self.left, self.top))
        for y in range(self.h):
            for x in range(self.w):
                print("{:4}".format(self.bitmap[x + self.w*y]),end='')
            if y == self.top - 1:
                print(" -- baseline")
            else:
                print()

    def pad_top(self,h):
        if (h <= self.top):
            return
        delta = h - self.top
        self.bitmap = ([0] * (self.w * delta)) + self.bitmap
        self.h = self.h + delta
        self.top = h
        
    def kitty(self):
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
        
class EpsonFont:
    def __init__(self, face, ptsize):
        self.ascii_map = [None] * 128
        self.face = face
        self.face.set_char_size( ptsize * 64)
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
        if char == ' ':
            g.w = self.face.glyph.metrics.horiAdvance >> 6
        g.even_h()
        g.squash()
        g.reduce_colors()
        self.ascii_map[cs[0]] = g
        #sys.stdout.write("Inserting {} ({}/{}) :".format(char,g.top,g.h))
        #sys.stdout.flush()
        #g.kitty()
        #print()

    def write_font(self):
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
        #print("First {}, last {}, baseline {}, height {}".format(min_ch,max_ch,
        #                                                         baseline, height))
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


import string
font_path = lookup_result.stdout;
face = freetype.Face(font_path)
ef = EpsonFont(face,24)
for c in string.ascii_lowercase + string.ascii_uppercase + " .'!":
    ef.add_glyph(c)
font = ef.write_font()

fontstr = ''
while font:
    fontstr += '    '
    line = font[:12]
    font = font[12:]
    ldat = ', '.join(map(lambda x:'0x{:02x}'.format(x), line))
    fontstr += ldat + ',\n'
    
print("""
#ifndef __FONT_{GUARD}_H__
#define __FONT_{GUARD}_H__

#include <stdint.h>

uint8_t font_{FN}[] = {{
{DATA}
}}

#endif // __FONT_{GUARD}_H__
""".format(GUARD=fn.upper(),FN=fn,DATA=fontstr))
