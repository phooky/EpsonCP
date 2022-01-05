#pragma once
#include <stdint.h>

void init_video();

void init_lcd();
void send_image();

void send_command(uint8_t* data);
