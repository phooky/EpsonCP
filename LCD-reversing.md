Description
===========

This is the control panel from an Epson WF-2540 inkjet printer.

The panel has a 14-pin FFC that runs to the CPU, and a 10-pin FFC that goes to the LCD
subassembly. Most of the pins on the LCD FFC are connected to pins on the CPU FFC.

Several of the LCD data pins are also used by the button scanning logic; LCD communications
are paused and the video clock line is held low while the buttons are checked and resume once
the button scan is finished.

The controller IC for the LCD is labelled "EPSON E02A46EA". No documentation is available.

Pinout
======

The FFC to the LCD has the following pinout:
| Pin | Purpose |
|-----|---------|
| 1   | Clock (~6MHz) | 
| 2   | GND |
| 3   | Command (active low) | 
| 4   | Data 2 | 
| 5   | 1.8V |
| 6   | Enable (active high) | 
| 7   | GND |
| 8   | Data 1 | 
| 9   | Data 0 | 
| 10  | 3.3V |

The FFC from the CPU to the control board has this pinout:
| Pin | Purpose |
|-----|---------|
| 1   | Piezo buzzer |
| 2   | 1.8V supply |
| 3   | GND |
| 4   | 3.3V supply |
| 5   | Shift register clock / Data 2 |
| 6   | CPU to shift register / Data 0 | 
| 7   | Video clock (~6MHz) |
| 8   | Command (active low) |
| 9   | Data 1 |
| 10  | ON button |
| 11  | Shift/load |
| 12  | SR Output enable |
| 13  | Shift register to CPU |
| 14  | GND |


Protocol
========

We have one clock line (FFC 1), three data lines (FFC9, FFC8, FFC4), one chip
enable line (FFC 6), and one command line (FFC 3). There appear to be two modes
of communication: sending a command, and sending frame data.

All data is clocked in on rising pulses.

Command transaction
-------------------
The command transactions consist of three bytes of data. 
1. The command line is asserted LOW. (t=0)
2. The data lines are set to the correct bit values. In a command transaction,
   DAT1 and DAT2 are set low; only DAT0 contains data.
3. The first clock starts at t=375nS.
4. Three bytes are clocked in, MSB first, at 6MHz.
5. The command line goes high at 625nS after the last bit.
6. The command line stays high for about 200nS before the next command.

Probable commands
-----------------

| 1st byte | 2nd byte | 3rd byte | Interpretation |
|----------|----------|----------|----------------|
| 0x30     | 0x00     | M        | Set data mode (std 0x01) |
| 0x31     | 0x00     | 0x01     | Start sending video frame data |
| 0x32     | X        | X        | Set horizontal resolution to X |
| 0x33     | Y        | Y        | Set vertical resolution to Y |
| 0x60     | 0x00     | V        | Turn on backlight (V=1 on) (takes effect at next frame)| 


Data transaction
----------------
A data transaction always immediately follows a command transaction of the form `0x31 0x00 0x01`.
It begins approximately 1uS after the command line goes high.

The entire frame is clocked in at 6MHz on three lines (DAT0, DAT1, DAT2). It appears that 
each represents one color channel. There are 321 bytes per line, and 120 lines.

Configuration sequence
======================

Most of this is going to be secret sauce; we can always try removing/altering/inserting commands
but it doesn't seem very valuable to do so.

```
pos    data
0000   0b 05 05   30 00 01   65 00 08   50 00 09 
000c   50 00 0a   58 00 05   37 00 01   37 00 00
0018   0d 00 01   0d 00 00   36 00 01   7f 00 00
0024   20 0f ef   32 01 40   33 00 77   34 00 00
0030   35 00 7c   38 03 ff   39 03 ff   00 00 63
003c   01 00 24   02 01 df   03 01 3f   04 00 07
0048   05 02 68   06 00 0c   07 00 08   08 00 ef
0054   09 00 00   0a 00 08   0e 00 00   10 00 00
0060   12 00 31   13 00 00   70 00 01   0c 00 03
006c   22 00 0d   23 00 00   22 00 05   23 00 00
0078   22 00 0d   23 00 00   22 60 00   23 00 00
0084   22 40 00   23 00 00   22 00 0f   23 00 00
```

The backlight on command `60 00 01` should be sent to make the screen visible.

Every frame needs to be proceeded by a `31 00 01` command.


Other
=====

Some of the data lines are shared with the button-scanning hardware, but as the clock line is kept
low during button scans and LED updates, this doesn't impact the LCD interface.

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
* Check other lines, make sure we're not missing a reset (we still have two mystery pins)
* replay configuration block
* replay first image

if that works

* write custom image

finally

* attempt shutdown sequence
