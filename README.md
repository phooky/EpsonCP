EpsonCP
=======

Tools and information concerning the reverse engineering of a control panel from an Epson printer we found on the street.

The target controller is a Raspberry Pi Pico. You can find the pin mapping from the Pico to the control panel's flat flexible cable (FFC) in the `pins.h` file. You can run the panel off the Pico's 3.3V regulated output, but you'll need to find a way to generate 1.8V for pin 2 on the FFC if you're using the LCD panel.

The file `LCD-reversing.md` contains everything I was able to figure out about driving the LCD panel.

