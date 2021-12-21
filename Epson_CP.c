#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"

#define ARRLEN(arr) (sizeof(arr)/sizeof(arr[0]))

typedef uint pin_t;

enum {
  PIN_CLK = 0,
  PIN_SER_IN = 1,
  PIN_OE = 2,
  PIN_RCLK = 3,
};

/// Initialize a GPIO pin as an output with the given level.
static inline void init_pin(pin_t pin, uint value) {
    gpio_init(pin);
    gpio_set_function(pin, GPIO_FUNC_SIO);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, value);
}

void initialize_gpio() {
  init_pin(PIN_CLK,0);
  init_pin(PIN_SER_IN,0);
  init_pin(PIN_OE,1);
  init_pin(PIN_RCLK,0);
}

void delay_bit() {
  sleep_ms(1);
}

int main()
{
    stdio_init_all();
    initialize_gpio();
    puts("Hello, world!");
    uint8_t rowval = 0;
    while (true) {
        int c = getchar_timeout_us(2);
        if (c == PICO_ERROR_TIMEOUT) {
	  // ignore
        } else if (c >= '0' && c <= '9') {
	  rowval = (rowval << 4) + (c - '0');
        } else if (c >= 'a' && c <= 'f') {
	  rowval = (rowval << 4) + (c - 'a') + 10;
        } else if (c == 'R') {
            c = -1;
            reset_usb_boot(0,0);
	} else if (c == '\n' || c == '\r') {
            printf("Setting 595 to %x.\n",rowval);
	    gpio_put(PIN_OE,1);
	    gpio_put(PIN_RCLK,0);
            for (size_t i = 0; i < 8; i++) {
	      delay_bit();
	      gpio_put(PIN_CLK,0);
	      delay_bit();
	      gpio_put(PIN_SER_IN,(rowval & (1<< (7-i) ))?1:0);
	      delay_bit();
	      gpio_put(PIN_CLK,1);
	      delay_bit();
            }
	    gpio_put(PIN_RCLK,1);
	    gpio_put(PIN_OE,0);

	    // now read
	  
        } else {
            printf("Unrecognized hex digit.\n");
            c = -1; 
        }
    }
    return 0;
}
