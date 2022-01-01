

It appears we have four lines transmitting data: 
* CLK
* DAT0
* DAT1
* DAT2

Some of the data lines are shared with the button-scanning hardware, but as the clock line is kept
low during button scans and LED updates, this doesn't impact the LCD interface.

The LCD appears to receive commands on the DAT0 line, and only uses DAT1 and DAT2 for additional
color channels during screen updates.

Before a screen is sent, there is a three-byte sequence. A screen is sent as a series of lines,
each terminated by a single byte-- unlikely parity, almost always the same? Maybe it's an attribute.

Configuration transaction:
```
00000000  0b 05 05 30 00 01 65 00  08 50 00 09 50 00 0a 58  |...0..e..P..P..X|
00000010  00 05 37 00 01 37 00 00  0d 00 01 0d 00 00 36 00  |..7..7........6.|
00000020  01 7f 00 00 20 0f ef 32  01 40 33 00 77 34 00 00  |.... ..2.@3.w4..|
00000030  35 00 7c 38 03 ff 39 03  ff 00 00 63 01 00 24 02  |5.|8..9....c..$.|
00000040  01 df 03 01 3f 04 00 07  05 02 68 06 00 0c 07 00  |....?.....h.....|
00000050  08 08 00 ef 09 00 00 0a  00 08 0e 00 00 10 00 00  |................|
00000060  12 00 31 13 00 00 70 00  01 0c 00 03 22 00 0d 23  |..1...p....."..#|
00000070  00 00 22 00 05 23 00 00  22 00 0d 23 00 00 22 60  |.."..#.."..#.."`|
00000080  00 23 00 00 22 40 00 23  00 00 22 00 0f 23 00 00  |.#.."@.#.."..#..|
```
screen starts at 0x00 and 0x967b in out_trans2-ch0:

0x31 0x00 0x01 (49 0 1)

... followed by the data.

commands appear to be three bytes?
* 32 01 40 -- set X resolution (320?)
* 33 00 77 -- set Y resolution (119?) -- this appears to be X+1, Y+1, oh my

* 0x00 first image end of line
* 0x59 second image end of line

Second sending of shutting down image is prefaced with:
```
00000000  0b 05 05 30 00 01 65 00  08 50 00 0a 58 00 05 37  |...0..e..P..X..7| // missing 50 00 09
00000010  00 01 37 00 00 0d 00 00  36 00 01 7f 00 00 20 0f  |..7.....6..... .| // missing 0d 00 01
00000020  ef 32 01 40 33 00 77 34  00 00 35 00 7c 38 03 ff  |.2.@3.w4..5.|8..|
00000030  39 03 ff 00 00 63 01 00  24 02 01 df 03 01 3f 04  |9....c..$.....?.|
00000040  00 07 05 02 68 06 00 0c  07 00 08 08 00 ef 09 00  |....h...........|
00000050  00 0a 00 08 0e 00 00 10  00 00 12 00 31 13 00 00  |............1...|
00000060  70 00 01 0c 00 03 22 60  00 23 00 00 22 40 00 23  |p....."`.#.."@.#| // missing below
00000070  00 00 22 00 0f 23 00 00  60 00 01// 31 00 01 00 00  |.."..#..`..1....|
// missing 22 00 0d 23 00 00 22 00 05 23 00 00 22 00 0d 23 00 00
```

this is repeated on frame 2. My guess: unnecessary repeats

Shutdown message:
```
00000000  60 00 00 22 00 0b 23 00  00                       |`.."..#..|
```

Could 60 XX XX be backlight?


HERE'S THE PLAN:
* replay configuration block
* replay first image

if that works

* write custom image

finally

* attempt shutdown sequence
