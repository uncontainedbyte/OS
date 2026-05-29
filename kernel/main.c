#include "display.h"

extern char kernel_start;
extern char kernel_end;


void kmain(){
	clear();
	
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
	while(1);
	
	
}
