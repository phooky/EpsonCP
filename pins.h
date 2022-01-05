#include <stdint.h>

#define ARRLEN(arr) (sizeof(arr)/sizeof(arr[0]))

typedef uint8_t pin_t;

/* The FFC from the CPU to the control board has this pinout: */
/* | Pin | Purpose | */
/* |-----|---------| */
/* | 1   | Piezo buzzer | */
/* | 2   | 1.8V supply | */
/* | 3   | GND | */
/* | 4   | 3.3V supply | */
/* | 5   | Shift register clock / Data 2 | */
/* | 6   | CPU to shift register / Data 0 |  */
/* | 7   | Video clock (~6MHz) | */
/* | 8   | Command (active low) | */
/* | 9   | Data 1 | */
/* | 10  | ON button | */
/* | 11  | Shift/load | */
/* | 12  | SR Output enable | */
/* | 13  | Shift register to CPU | */
/* | 14  | GND | */

// D0, D1, D2 should be contiguous OUT on PIO
// Command, Video clock should be contiguous SET on PIO

enum {
  VP_CMD       = 0,  // FFC 8
  VP_CLK       = 1,  // FFC 7
  VP_DAT0      = 2,  // FFC 6
  VP_DAT1      = 3,  // FFC 9
  VP_DAT2      = 4,  // FFC 5

  SP_SER_OUT   = 2,  // FFC 6
  SP_CLK       = 4,  // FFC 5
  SP_SER_IN    = 5,  // FFC 13
  SP_OE        = 6,  // FFC 12
  SP_RCLK      = 7,  // FFC 11
};
