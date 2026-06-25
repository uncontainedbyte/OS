#pragma once

#include "int.h"
#include "display.h"
#include "VMM.h"



void MEM_init();

void* kalloc(uint32 count);
void kfree(void* ptr);
void* krealloc(void* ptr,uint32 new_size);

void heap_dump();
