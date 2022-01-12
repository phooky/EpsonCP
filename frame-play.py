#!/usr/bin/python3
import struct
import sys
import serial
from PIL import Image

s=serial.Serial("/dev/ttyACM0",timeout=0.01)

data = b''
if len(sys.argv) > 1:
    im = Image.open(sys.argv[1])
    for x in range(320):
        for y in range(119):
            v = 0
            try:
                v = im.getpixel((x,y))
            except:
                v = 0
            #print(v)
            data = data + struct.pack('B',v)
else:
    for x in range(320):
        for y in range(119):
            data = data + struct.pack('B',(x) % 256)

s.write(b'V')
s.write(data)
s.flush()
    
s.close()
