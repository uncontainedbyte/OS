#include "keyboard.h"



uint32 make_key_event(uint8 ascii,uint8 scancode,uint8 modifiers,uint8 flags){
	return (uint32)ascii | ((uint32)scancode << 8) | ((uint32)modifiers << 16) | ((uint32)flags << 24);
}

uint32 keyboard_buffer[64];
volatile uint8 keyboard_head=0, keyboard_tail=0;
uint8 keyboard_shift=0;
uint8 keyboard_ctrl = 0;
uint8 keyboard_alt = 0;
uint8 keyboard_state=0;

uint8 key_map(uint8 sc) {
	switch (sc) {
		case 0x02: return '1';
		case 0x03: return '2';
		case 0x04: return '3';
		case 0x05: return '4';
		case 0x06: return '5';
		case 0x07: return '6';
		case 0x08: return '7';
		case 0x09: return '8';
		case 0x0A: return '9';
		case 0x0B: return '0';
		
		case 0x0C: return '-';
		case 0x0D: return '=';
		
		case 0x10: return 'q';
		case 0x11: return 'w';
		case 0x12: return 'e';
		case 0x13: return 'r';
		case 0x14: return 't';
		case 0x15: return 'y';
		case 0x16: return 'u';
		case 0x17: return 'i';
		case 0x18: return 'o';
		case 0x19: return 'p';
		
		case 0x1A: return '[';
		case 0x1B: return ']';
		
		case 0x1E: return 'a';
		case 0x1F: return 's';
		case 0x20: return 'd';
		case 0x21: return 'f';
		case 0x22: return 'g';
		case 0x23: return 'h';
		case 0x24: return 'j';
		case 0x25: return 'k';
		case 0x26: return 'l';
		
		case 0x27: return ';';
		case 0x28: return '\'';
		case 0x29: return '`';
		
		case 0x2C: return 'z';
		case 0x2D: return 'x';
		case 0x2E: return 'c';
		case 0x2F: return 'v';
		case 0x30: return 'b';
		case 0x31: return 'n';
		case 0x32: return 'm';
		
		case 0x33: return ',';
		case 0x34: return '.';
		case 0x35: return '/';
		
		case 0x39: return ' ';
		
		case 0x0E: return '\b'; // backspace
		case 0x1C: return '\n'; // enter
		case 0x0F: return '\t'; // tab
		
		default:
			return 0;
	}
}
uint8 shift_map(uint8 sc) {
	switch(sc){
		case 0x02: return '!';
		case 0x03: return '@';
		case 0x04: return '#';
		case 0x05: return '$';
		case 0x06: return '%';
		case 0x07: return '^';
		case 0x08: return '&';
		case 0x09: return '*';
		case 0x0A: return '(';
		case 0x0B: return ')';
		
		case 0x0C: return '_';
		case 0x0D: return '+';
		
		case 0x10: return 'Q';
		case 0x11: return 'W';
		case 0x12: return 'E';
		case 0x13: return 'R';
		case 0x14: return 'T';
		case 0x15: return 'Y';
		case 0x16: return 'U';
		case 0x17: return 'I';
		case 0x18: return 'O';
		case 0x19: return 'P';
		
		case 0x1A: return '{';
		case 0x1B: return '}';
		
		case 0x1E: return 'A';
		case 0x1F: return 'S';
		case 0x20: return 'D';
		case 0x21: return 'F';
		case 0x22: return 'G';
		case 0x23: return 'H';
		case 0x24: return 'J';
		case 0x25: return 'K';
		case 0x26: return 'L';
		
		case 0x27: return ':';
		case 0x28: return '"';
		case 0x29: return '~';
		
		case 0x2C: return 'Z';
		case 0x2D: return 'X';
		case 0x2E: return 'C';
		case 0x2F: return 'V';
		case 0x30: return 'B';
		case 0x31: return 'N';
		case 0x32: return 'M';
		
		case 0x33: return '<';
		case 0x34: return '>';
		case 0x35: return '?';
		
		case 0x39: return ' ';
		
		case 0x0E: return '\b';
		case 0x1C: return '\n';
		case 0x0F: return '\t';
		
		default:
			return 0;
	}
}

uint32 keyboard_interrupt_handler(registers_t *r) {
	uint8 sc = inb(0x60);
	uint8 released = sc & 0x80;
	sc &= 0x7F;
	
	if(!released && sc == 58) keyboard_state ^= KSTATE_CAPS;
	if(!released && sc == 0x45) keyboard_state ^= KSTATE_NUM;
	if(!released && sc == 0x46) keyboard_state ^= KSTATE_SCROLL;
	
	if(sc == 42 || sc == 54) keyboard_shift = !released;
	if(sc == 0x1D) keyboard_ctrl = !released;
	if(sc == 0x38) keyboard_alt = !released;
	
	uint8 ascii = 0;
	if(!released){ ascii = keyboard_shift^(keyboard_state&KSTATE_CAPS) ? shift_map(sc) : key_map(sc); }
	uint8 modifiers = 0;
	if(keyboard_shift) modifiers |= KMOD_SHIFT;
	if(keyboard_ctrl)  modifiers |= KMOD_CTRL;
	if(keyboard_alt)   modifiers |= KMOD_ALT;
	uint8 flags = released ? KFLAG_RELEASE : 0;
	
	uint32 key = make_key_event(ascii, sc, modifiers, flags);
	
	keyboard_buffer[keyboard_head] = key;
	uint8 next = (keyboard_head + 1) % 64;
	
	if(next != keyboard_tail){
		keyboard_buffer[keyboard_head] = key;
		keyboard_head = next;
	}
	
	return (uint32)r;
}
void keyboard_init(){
	registerInterrupt(33,systemFlags,keyboard_interrupt_handler);
	pic_unmask_irq(1);
}

uint32 keyboard_read(){
	disable_interrupts();
	
	uint32 key=0;
	if(keyboard_head == keyboard_tail){
		enable_interrupts();
		return key;
	}
	
	key = keyboard_buffer[keyboard_tail];
	keyboard_tail = (keyboard_tail + 1) % 64;
	
	enable_interrupts();
	
	return key;
}
uint8 keyboard_get_state(){
	return keyboard_state;
}

















































