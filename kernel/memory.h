#pragma once

#include "int.h"
#include "display.h"



void MEM_init(uint32 kernel_end);

uint32 alloc_pages(uint32 count);
void free_pages(uint32 page, uint32 count);
void map_page(uint32 virt,uint32 phys,uint32 flags);

void* kalloc(uint32 count);
void kfree(void* ptr);
void* krealloc(void* ptr,uint32 new_size);

void MEM_printPageBitmap();
void heap_dump();
