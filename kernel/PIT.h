#pragma once

#include "interrupts.h"





void PIT_init();
void PIT_register_handler(irq_handler_t handler);
uint32 get_ticks();
void set_frequency(uint32 frequency);
uint32 PIT_milliseconds_to_ticks(uint32 ms);
