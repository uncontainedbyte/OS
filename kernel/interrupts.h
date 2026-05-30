#pragma once

#include "int.h"
#include "display.h"

typedef struct {
	uint32 ds;
	uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32 int_no;
	uint32 err_code;
	uint32 eip, cs, eflags, useresp, ss;
} registers_t;
typedef enum{
	IDT_FLAG_GATE_TASK              = 0x5,
	IDT_FLAG_GATE_16BIT_INT         = 0x6,
	IDT_FLAG_GATE_16BIT_TRAP        = 0x7,
	IDT_FLAG_GATE_32BIT_INT         = 0xE,
	IDT_FLAG_GATE_32BIT_TRAP        = 0xF,
	IDT_FLAG_RING0                  = (0 << 5),
	IDT_FLAG_RING1                  = (1 << 5),
	IDT_FLAG_RING2                  = (2 << 5),
	IDT_FLAG_RING3                  = (3 << 5),
	IDT_FLAG_PRESENT                = 0x80,
} IDT_FLAGS;
#define systemFlags IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT


void idt_init();
void enable_interrupts();
void disable_interrupts();
void registerInterrupt(uint32 id,uint8 flags,uint32 handler);

void install_Basic_Interrupts();
