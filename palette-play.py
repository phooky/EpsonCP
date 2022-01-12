#!/usr/bin/python3
import struct
import sys
import serial

r=float(sys.argv[1])
g=float(sys.argv[2])
b=float(sys.argv[3])

s=serial.Serial("/dev/ttyACM0",timeout=0.01)
s.write(b'P')
for i in range(256):
    s.write(struct.pack('BBB',int(b*i)%256,int(g*i)%256,int(r*i)%256))
    s.flush()
s.write(b'v')
try:
    print(s.read(100))
except e:
    print(e)
    
s.close()
