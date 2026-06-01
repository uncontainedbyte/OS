#pragma once

#include "int.h"
#include "display.h"



void MEM_print_memory_map();
void MEM_init(uint32 kernel_end);
void MEM_printPageBitmap();
