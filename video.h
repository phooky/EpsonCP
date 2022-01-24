#pragma once
#include <stdint.h>

void init_video();

void init_lcd();
void send_image();
void clear_buffer();

void send_command(uint8_t* data);

typedef struct {
  uint8_t r; uint8_t g; uint8_t b;
} __attribute__((packed)) color_t;

extern uint8_t framebuf[320][119];
extern color_t palette[256];
