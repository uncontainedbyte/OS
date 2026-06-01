#include "display.h"
#include "interrupts.h"
#include "keyboard.h"
#include "PIT.h"
#include "memory.h"

extern uint32 kernel_start;

void ____testPrint(){
	printf("Char:     %c\n",'c');
	printf("String:   %s\n","this is a string");
	printf("Max int:  %i\n",INT32_MAX);
	printf("Min int:  %i\n",INT32_MIN);
	printf("Max uint: %u\n",UINT32_MAX);
	printf("Hex:      %x %x\n",0x01234567,0x89ABCDEF);
	printf("Binary:   %b\n",0b10001000100010001000100010001000);
	printf("Pointer:  %p\n",VIDEO_ADDRESS);
	printf("ch Color: %#1#%#2#%#3#%#4#%#5#%#6#%#7#%#8#%#9#%#a#%#b#%#c#%#d#%#e#%#f#\n");
	printf("bg color: %#01#%#02#%#03#%#04#%#05#%#06#%#07#%#08#%#09#%#0a#%#0b#%#0c#%#0d#%#0e#%#0f#\n");
}
















void kmain(){
	uint32 kernel_end = *((uint32*)0x7FA);
	kernel_end += ((uint32)(*((uint16*)0x7F8)))*512;
	
	clear();
	
	idt_init();
	enable_interrupts();
	install_Basic_Interrupts();
	keyboard_init();
	PIT_init();
	MEM_init(kernel_end);
	
	printf("Kernel-Start: %x4\n",*((uint32*)0x7FA));
	printf("Kernel-End:   %x4\n",kernel_end);
	MEM_print_memory_map();
	MEM_printPageBitmap();
	
	while(1){
		uint32 c = keyboard_read();
		if(KEY_ASCII(c)=='-'){ disable_interrupts(); sleep(100); }
		if(KEY_ASCII(c)) printf("%c",c);
	}
	while(1);
}







































