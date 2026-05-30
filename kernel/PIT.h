#pragma once

#include "interrupts.h"


void PIT_init();
void sleep(uint32 ms);
void PIT_register_handler(void (*handler)(void));
uint32 get_ticks();
void set_frequency(uint32 frequency);
