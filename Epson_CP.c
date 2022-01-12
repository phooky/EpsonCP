#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "pico/multicore.h"
#include "pico/stdio/driver.h"
#include "tusb.h"

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
  sleep_us(4);
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

/*
void video_core() {
}
*/

uint8_t hexchar(int c) {
  if (c >= '0' && c <= '9') { return c - '0'; }
  if (c >= 'a' && c <= 'f') { return (c - 'a') + 10; }
  if (c >= 'A' && c <= 'F') { return (c - 'A') + 10; }
  return 0xff;
}

int read_from_usb(uint8_t* buf, int len, uint32_t timeout_us) {
  uint32_t until = make_timeout_time_us(timeout_us);
  int total_read = 0;
  do {
    if (tud_cdc_connected() && tud_cdc_available()) {
      int read = tud_cdc_read(buf, len);
      //int read = stdio_usb.in_chars(buf, len);
      if (read > 0) {
	total_read += read;
	len -= read;
	buf += read;
	if (len == 0) break;
      }
    }
  } while (!time_reached(until));
  return total_read;
}

int main()
{
  sleep_us(100);
    stdio_init_all();
  sleep_us(2000);
    puts("Hello, world!");
    initialize_gpio();
    init_video();
    //multicore_launch_core1(video_core);
    gpio_put(SP_OE,0);
    init_lcd();
    
    while (true) {
        int c = getchar_timeout_us(100);
        if (c == PICO_ERROR_TIMEOUT) {
	  Keypress kp = scan();
	  if ((kp.type != last_kp.type ||
	       kp.row != last_kp.row ||
	       kp.col != last_kp.col)
	      && kp.type == DOWN) {
	    printf("Key down: %s (%d, %d)\n",keyname(keycode(kp.row,kp.col)),kp.row,kp.col);
	  }
	  last_kp = kp;
        } else if (c == 'R') {
            printf("*** Resetting into bootloader ***");
            c = -1;
            reset_usb_boot(0,0);
	} else if (c == 'C') {
	  // read next 6 chars as hex
	  uint8_t cmd[3] = { 0,0,0 };
	  for (uint8_t i = 0; i < 6; i++) {
	    c = getchar_timeout_us(8000000);
	    if (c == -1) { puts("command entry timeout\n"); break; }
	    uint8_t h = hexchar(c);
	    if (h >= 16) { c = -1; printf("non-hex char '%c'\n",c); break; }
	    cmd[i/2] |= h << ( (1-(i%2)) * 4 );
	  }
	  if (c != -1) {
	    printf("Sending command %x%x%x\n",cmd[0],cmd[1],cmd[2]);
	    send_command(cmd);
	    c = -1;
	  }
	} else if (c == 'P') {
	  c = -1;
	  int i = read_from_usb((uint8_t*)&palette[0],256*3,1000000);
	  printf("Got %d.\n",i);
	} else if (c == 'V') {
	  c = -1;
	  int count = read_from_usb(&framebuf[0][0],320*119,2000000);
	  printf("count %d of %d\n",count,320*119);
	  send_image();
	} else if (c == 'v') {
	  c = -1;
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
	  printf("Hello there.\n");
        } else {
	  printf("Unrecognized data.\n");
	  c = -1; 
        }
    }
    return 0;
}
