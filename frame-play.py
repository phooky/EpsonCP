#!/usr/bin/python3
import struct
import sys
import serial
from PIL import Image, ImageOps


data = b'V'
if len(sys.argv) > 1:
    im = ImageOps.grayscale(Image.open(sys.argv[1]))
    for x in range(320):
        for y in range(119):
            v = 0
            try:
                v = im.getpixel((x,y*1.5))
            except:
                v = 0
            #print(v)
            data = data + struct.pack('B',v)
else:
    for x in range(320):
        for y in range(119):
            data = data + struct.pack('B',(x) % 256)

data = data + b'v'

if 0:
    s=serial.Serial("/dev/ttyACM0",baudrate=1152000,timeout=0.01,write_timeout=20)
    s.write(b'V')
    s.write(data)
    s.flush()    
    s.write(b'v')
    try:
        print(s.read(100))
    except e:
        print(e)
    s.close()
else:
    f=open("/dev/ttyACM0","wb")
    f.write(data)
    f.flush()
    f.close()
    
