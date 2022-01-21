#!/usr/bin/python3

import freetype
import sys
import subprocess

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
        bm = [0] * int(self.w * self.h)
        for y in range(self.h):
            for x in range(self.w):
                bm[x+y*self.w] = 0
                bm[x+y*self.w] = int( (self.bitmap[x+(y*2*self.w)] +
                                       self.bitmap[x+((y*2+1)*self.w)])/2)
        self.aspect = 2
        self.bitmap = bm

    def reduce_colors(self):
        self.bitmap = [ (x + 0x1f) >> 6 for x in self.bitmap ]
    
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
        from base64 import b64encode
        b = b'\00\00\00' * self.left
        for y in range(self.h):
            st = self.w * y
            en = st + self.w
            for _ in range(2):
                b = b + bytes([int(x*255/4) for x in self.bitmap[st:en] for _ in range(3)])
                
        esc = b'\033'
        body ='_Gf=24,s={},v={},a=T;'.format(self.w, self.h*2).encode('ascii')
        sys.stdout.buffer.write(esc+body+b64encode(b)+esc+b'\\')
        sys.stdout.flush()

        
        
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
        g.even_h()
        g.squash()
        g.reduce_colors()
        self.ascii_map[cs[0]] = g
        sys.stdout.write("Inserting {} :".format(char))
        sys.stdout.flush()
        g.kitty()
        print()



font_path = lookup_result.stdout;
face = freetype.Face(font_path)
ef = EpsonFont(face,18)
for c in 'abcdefg':
    ef.add_glyph(c)

