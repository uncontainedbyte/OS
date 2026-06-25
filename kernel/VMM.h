#pragma once

#include "int.h"
#include "PMM.h"



#define KERNEL_HEAP_START 0x01000000  // 16MB
#define KERNEL_HEAP_END   0x07FFFFFF  // 127MB~
#define KERNEL_MMIO_START 0x08000000
static uint32 ____next_mmio = KERNEL_MMIO_START;


typedef struct {
	uint32 cr3;
} address_space_t;

void VMM_init();

int VMM_map(uint32 virt,uint32 phys,uint32 flags);
void VMM_unmap(uint32 virt);

uint32 VMM_get_phys_address(uint32 virt);

address_space_t VMM_create_directory();
void VMM_switch_directory(address_space_t adr_spc);
void VMM_destroy_directory(address_space_t adr_spc);
int VMM_map_in_directory(address_space_t space,uint32 virt,uint32 phys,uint32 flags);

void* VMM_temp_map_0(uint32 phys);
void* VMM_temp_map_1(uint32 phys);
void* VMM_temp_map_2(uint32 phys);
void* VMM_temp_map_3(uint32 phys);
void VMM_temp_unmap_0();
void VMM_temp_unmap_1();
void VMM_temp_unmap_2();
void VMM_temp_unmap_3();






























































