#include "PIT.h"



#define PIT_CHANNEL0_DATA 0x40
#define PIT_COMMAND       0x43
#define PIT_FREQUENCY     1193182  // PIT input clock in Hz

volatile uint32 PIT_ticks;
irq_handler_t PIT_tick_handler;
uint32 PIT_freq_hz;

uint32 PIT_milliseconds_to_ticks(uint32 ms){
	return (PIT_freq_hz * ms) / 1000;
}

uint32 PIT_interrupt_handler(registers_t *r){
	PIT_ticks++;
	
	if(PIT_tick_handler) return PIT_tick_handler(r);
	
	return (uint32)r;
}
void PIT_init(){
	uint32 frequency = 100;
	
	PIT_ticks = 0;
	PIT_tick_handler = 0;
	PIT_freq_hz = frequency;
	
	uint16 divisor = (uint16)(PIT_FREQUENCY / frequency);
	// Command: channel 0, lo/hi byte, mode 3 (square wave)
	outb(PIT_COMMAND, 0x36);
	outb(PIT_CHANNEL0_DATA, divisor & 0xFF);        // low byte
	outb(PIT_CHANNEL0_DATA, (divisor >> 8) & 0xFF); // high byte
	
	registerInterrupt(32,systemFlags,PIT_interrupt_handler);
	pic_unmask_irq(0);
}

/*void sleep(uint32 ms){
	uint32 start = PIT_ticks;
	uint32 delta = PIT_milliseconds_to_ticks(ms);
	
	if(!interrupts_enabled()){
		printf("\n\n\n\n\n\n\n\n\n\n\n");
		printf("                          Eternal slumber has begun...                          ");
		printf("\n\n\n\n\n\n\n\n\n\n\n\n");
		printf("because future me is an idiot that called sleep with interrupts disabled");
	}
	
	while((uint32)(PIT_ticks - start) < delta)
		; // busy wait
}*/
void PIT_register_handler(irq_handler_t handler){
	PIT_tick_handler = handler;
}
uint32 get_ticks(){
	return PIT_ticks;
}
void set_frequency(uint32 frequency){
	if(!frequency) return;
	if(frequency > PIT_FREQUENCY) frequency = PIT_FREQUENCY;
	PIT_freq_hz = frequency;
	uint16 divisor = (uint16)(PIT_FREQUENCY / frequency);
	
	outb(PIT_COMMAND, 0x36);
	outb(PIT_CHANNEL0_DATA, divisor & 0xFF);
	outb(PIT_CHANNEL0_DATA, (divisor >> 8) & 0xFF);
}































































