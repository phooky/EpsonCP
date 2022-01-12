#include <stdint.h>
#include <stddef.h>
#include "pins.h"
#include "pico/stdlib.h"
#include "video.pio.h"
#include "video.h"

size_t msg_init_lcd_len = 144;
uint8_t msg_init_lcd[] = {
  0x0b, 0x05, 0x05, 0x30, 0x00, 0x01, 0x65, 0x00, 0x08, 0x50, 0x00, 0x09,
  0x50, 0x00, 0x0a, 0x58, 0x00, 0x05, 0x37, 0x00, 0x01, 0x37, 0x00, 0x00,
  0x0d, 0x00, 0x01, 0x0d, 0x00, 0x00, 0x36, 0x00, 0x01, 0x7f, 0x00, 0x00,
  0x20, 0x0f, 0xef, 0x32, 0x01, 0x40, 0x33, 0x00, 0x77, 0x34, 0x00, 0x00,
  0x35, 0x00, 0x7c, 0x38, 0x03, 0xff, 0x39, 0x03, 0xff, 0x00, 0x00, 0x63,
  0x01, 0x00, 0x24, 0x02, 0x01, 0xdf, 0x03, 0x01, 0x3f, 0x04, 0x00, 0x07,
  0x05, 0x02, 0x68, 0x06, 0x00, 0x0c, 0x07, 0x00, 0x08, 0x08, 0x00, 0xef,
  0x09, 0x00, 0x00, 0x0a, 0x00, 0x08, 0x0e, 0x00, 0x00, 0x10, 0x00, 0x00,
  0x12, 0x00, 0x31, 0x13, 0x00, 0x00, 0x70, 0x00, 0x01, 0x0c, 0x00, 0x03,
  0x22, 0x00, 0x0d, 0x23, 0x00, 0x00, 0x22, 0x00, 0x05, 0x23, 0x00, 0x00,
  0x22, 0x00, 0x0d, 0x23, 0x00, 0x00, 0x22, 0x60, 0x00, 0x23, 0x00, 0x00,
  0x22, 0x40, 0x00, 0x23, 0x00, 0x00, 0x22, 0x00, 0x0f, 0x23, 0x00, 0x00,
};

size_t msg_start_frame_len = 3;
uint8_t msg_start_frame[] = {
  0x31, 0x00, 0x01,
};

uint8_t framebuf[320][119];
color_t palette[256];

static inline void init_pin2(pin_t pin, uint value) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, value);
}

PIO pio = pio0;
uint data_sm = 0;
uint data_offset;

void pins_to_sio() {
  init_pin2(VP_CLK,1);
  init_pin2(VP_DAT0,0);
  init_pin2(VP_DAT1,0);
  init_pin2(VP_DAT2,0);
  init_pin2(VP_CMD,1);
}

void pins_to_pio() {
  for (uint i = VP_CLK; i <= VP_DAT2; i++) {
    pio_gpio_init(pio, i);
  }
}

static inline bool pio_sm_check_stalled(PIO pio, uint sm) {
  return !!(pio->fdebug & (1 << (PIO_FDEBUG_TXSTALL_LSB+sm)));
}

void wait_for_stall(PIO pio, uint sm) {
  while (!pio_sm_check_stalled(pio,sm)) ;
}

void init_video() {
  for (uint i = 0; i < 256; i++) {
    palette[i] = (color_t){ r : i, g : i/2, b : i };
  }
  for (int y = 0; y < 120; y++) {
    for (int x = 0; x < 320; x++) {
      framebuf[x][y] = ((x%20) == 1 || (y%20) == 1)?255:((x+y)%160);
    }
  }
  pins_to_sio();
  data_offset = pio_add_program(pio, &data_send_program);
  pio_sm_config data_conf = data_send_program_get_default_config(data_offset);
  // Configuration for data send program
  sm_config_set_out_pins(&data_conf, 2, 3);
  sm_config_set_out_shift(&data_conf, false, true, 24);
  sm_config_set_sideset_pins(&data_conf, 1);
  sm_config_set_clkdiv(&data_conf, 7.0);
  pio_sm_set_pindirs_with_mask(pio, data_sm, 0x1e, 0x1e);
  pio_sm_init(pio, data_sm, data_offset, &data_conf);
  pio_sm_set_enabled(pio, data_sm, true);
}

uint32_t interleave_zero(uint32_t v) {
  v = (v | (v<<8)) & 0xf00f;
  v = (v | (v<<4)) & 0xc30c3;
  v = (v | (v<<2)) & 0xc249249;
  return v;
}

uint32_t interleave(uint8_t r, uint8_t g, uint8_t b) {
  // high 24 bits should be r7g7b7r6g6b6 .. r0g0b0 00000000
  return (interleave_zero(r) << 10) |
    (interleave_zero(g) << 9) |
    (interleave_zero(b) << 8);
}

void clock_byte(uint8_t b) {
  int i = 8;
  uint32_t b32;
  do {
    i--;
    gpio_put(VP_CLK,0);
    gpio_put(VP_DAT0, (b >> i) & 0x01);
    gpio_put(VP_CLK,1);
  } while (i > 0);
}

void send_command(uint8_t* data) {
  sleep_us(2);
  gpio_put(VP_DAT1,0);
  gpio_put(VP_DAT2,0);
  gpio_put(VP_CMD,0);
  sleep_us(2);
  clock_byte(*(data++));
  clock_byte(*(data++));
  clock_byte(*(data++));
  sleep_us(2);
  gpio_put(VP_CMD,1);
  sleep_us(2);
}

void send_image() {
  sleep_us(20);
  send_command(msg_start_frame);
  pins_to_pio();
  for (int y = 0; y < 120; y++) {
    for (int x = 0; x < 321; x++) {
      if (x >= 320 || y >= 119) pio_sm_put_blocking(pio,data_sm,0);
      else {
	color_t c = palette[framebuf[x][y]];
	pio_sm_put_blocking(pio,data_sm,interleave(c.b,c.g,c.r));
      }
    }
  }
  pins_to_sio();
}

void init_lcd() {
  pins_to_sio();
  size_t start = 0;
  while (start < msg_init_lcd_len) {
    send_command(msg_init_lcd + start);
    start += 3;
  }
  uint8_t bl[] = {0x60, 0x00, 0x01};
  sleep_us(20);
  send_command(bl);
}
