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

font_path = lookup_result.stdout;
face = freetype.Face(font_path)
face.set_char_size( 16*64 )
for c in 'Sg':
    face.load_char(c)
    bitmap = face.glyph.bitmap
    print("len {} x {} y {} left {} top {}".format(len(bitmap.buffer),bitmap.width,bitmap.rows,
                                                   face.glyph.bitmap_left,face.glyph.bitmap_top))
    for y in range(bitmap.rows):
        for x in range(bitmap.width):
            print("{:4}".format(bitmap.buffer[x + bitmap.width*y]),end='')
        if y == face.glyph.bitmap_top - 1:
            print(" -- baseline")
        else:
            print()
