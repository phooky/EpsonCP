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
            if y == face.glyph.bitmap_top - 1:
                print(" -- baseline")
            else:
                print()

    def print_c(self):
        pass
        



font_path = lookup_result.stdout;
face = freetype.Face(font_path)
face.set_char_size( 18*64 )
for c in 'S':
    face.load_char(c)
    bitmap = face.glyph.bitmap
    g = Glyph(bitmap.width,bitmap.rows,
              face.glyph.bitmap_left,
              face.glyph.bitmap_top,
              bitmap.buffer)
    g.print()
    g.even_h()
    g.print()
    print()
    g.squash()
    g.reduce_colors()
    g.print()
    print()
