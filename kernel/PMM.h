#pragma once

#include "int.h"
#include "display.h"




static uint32 PMM_total_pages = 0;
static uint32 PMM_free_pages = 0;

void PMM_init(uint32 kernel_end);
uint32 PMM_alloc(uint32 count);
void PMM_free(uint32 adr,uint32 count);

void MEM_print_memory_map();
void PMM_print_stats();
























































