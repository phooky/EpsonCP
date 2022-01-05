#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "keymap.h"
#include "pins.h"
#include "video.h"

// RCLK shifts to registers on rising edge;
// SH/LD' loads during LOW but needs to be high
// during shift/read. So strategy is:
//
// 
// * shift in next row pattern to 595/shift out prev. column pattern
// * RCLK low to load columns
// * RCLK high to latch pattern
//
//

/// Initialize a GPIO pin as an output with the given level.
static inline void init_pin(pin_t pin, uint value) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, value);
}

void initialize_gpio() {
  gpio_init(SP_SER_IN);
  init_pin(SP_CLK,0);
  init_pin(SP_SER_OUT,0);
  init_pin(SP_OE,1);
  init_pin(SP_RCLK,0);
}

void delay_bit() {
  sleep_us(80);
}

uint8_t scan_cols(uint8_t next_rows) {
  uint8_t scan;
  gpio_put(SP_RCLK,1); // Sample columns for T
  delay_bit();
  for (size_t i = 0; i < 8; i++) {
    delay_bit();
    gpio_put(SP_CLK,0);
    delay_bit();
    scan = (scan << 1) | (gpio_get(SP_SER_IN)?0:1);
    gpio_put(SP_SER_OUT,(next_rows & (1 << (7-i) ))?1:0);
    delay_bit();
    gpio_put(SP_CLK,1);
    delay_bit();
  }
  gpio_put(SP_RCLK,0);
  delay_bit();
  gpio_put(SP_RCLK,1); // Latch rowval for T+1
  delay_bit();
  gpio_put(SP_RCLK,0); // Load columns for T+1
  return scan;
}

const size_t ROW_COUNT = 8;

typedef enum {
  NONE,
  DOWN,
  UP,
} KeypressType;
  
typedef struct {
  uint8_t row;
  uint8_t col;
  KeypressType type;
} Keypress;

Keypress last_kp;


Keypress scan() {
  Keypress cur_kp = { 0, 0, NONE };
  // do scan
  scan_cols(0x01);
  for (uint8_t i = 1; i <= 8; i++) {
    uint8_t cols = scan_cols(1<<(i%8));
    if (cols) {
      cur_kp = (Keypress){ i-1, 31-__builtin_clz(cols), DOWN };
    }
  }
  return cur_kp;
}

int main()
{
    stdio_init_all();
    initialize_gpio();
    init_video();
    init_lcd();
    send_image();
    puts("Hello, world!");
    gpio_put(SP_OE,0);
    uint8_t rowval = 0;
    while (true) {
        int c = getchar_timeout_us(2);
        if (c == PICO_ERROR_TIMEOUT) {
	  Keypress kp = scan();
	  if ((kp.type != last_kp.type ||
	       kp.row != last_kp.row ||
	       kp.col != last_kp.col)
	      && kp.type == DOWN) {
	    printf("Key down: %s (%d, %d)\n",keyname(keycode(kp.row,kp.col)),kp.row,kp.col);
	  }
	  last_kp = kp;
        } else if (c >= '0' && c <= '9') {
	  rowval = (rowval << 4) + (c - '0');
        } else if (c >= 'a' && c <= 'f') {
	  rowval = (rowval << 4) + (c - 'a') + 10;
        } else if (c == 'R') {
            c = -1;
            reset_usb_boot(0,0);
	} else if (c == 'v') {
	  c = -1;
	  init_video();
	  init_lcd();
	  send_image();
	} else if (c == 's') {
	  // do scan
	  scan_cols(0x01);
	  printf("*** Full matrix scan ***\n");
	  for (uint8_t i = 1; i <= 8; i++) {
	    printf("(scan %x) ",1<<(i%8));
	    printf("Row %x: %x\n",i-1,scan_cols(1<<(i%8)));
	  }
	  printf("--- scan done --- \n");
	} else if (c == '\n' || c == '\r') {
            printf("Setting 595 to %x.\n",rowval);
	    printf("Received %x from last scan.\n",scan_cols(rowval));
	  
        } else {
            printf("Unrecognized hex digit.\n");
            c = -1; 
        }
    }
    return 0;
}
