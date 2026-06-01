#include "memory.h"





typedef struct {
	uint64 base;
	uint64 length;
	uint32 type;
	uint32 attributes;
} __attribute__((packed)) e820_entry_t;

#define MEMORY_MAP_COUNT (*(uint16*)0x7FE)
#define MEMORY_MAP       ((e820_entry_t*)0x800)

#define PAGE_SIZE 4096

const char* e820_type_name(uint32 type){
	switch(type){
		case 1: return "Usable    ";
		case 2: return "Reserved  ";
		case 3: return "ACPI claim";
		case 4: return "ACPI NVS  ";
		case 5: return "Bad Memory";
		default: return "Unknown   ";
	}
}
void MEM_print_memory_map(){
	e820_entry_t* map = MEMORY_MAP;
	uint16 count = MEMORY_MAP_COUNT;
	for(int s=0;s<count;s++){
		uint32 base = map[s].base&0xFFFFFFFF;
		uint32 length = map[s].length&0xFFFFFFFF;
		uint32 type = map[s].type;
		
		printf("Start: %x4 End: %x4 Type: %s Size: ",base,base+length,e820_type_name(type));
		printf("%uB ",length);
		if(length/1024>0)
			printf("%uKB ",length/1024);
		if((length/1024)/1024>0)
			printf("%uMB ",(length/1024)/1024);
		printf("\n");
	}
}



uint32* page_directory;
uint32* first_page_table;
uint8 page_used[1024];

void load_page_directory(uint32* dir){
	asm volatile("mov %0, %%cr3":: "r"(dir));
}
void enable_paging(){
	uint32 cr0;
	
	asm volatile("mov %%cr0, %0": "=r"(cr0));
	
	cr0 |= 0x80000000;
	
	asm volatile("mov %0, %%cr0":: "r"(cr0));
}
void MEM_printPageBitmap(){
	printf("Pages Bitmap:\n");
	for(int ss=0;ss<1024;ss++){
		printf("%i",page_used[ss]);
	}printf("\n");
}
void MEM_init(uint32 kernel_end){
	uint32 paging_base = (kernel_end + 0xFFF) & ~0xFFF;
	
	page_directory  = (uint32*)paging_base;
	first_page_table = (uint32*)(paging_base + 0x1000);
	
	for(int s=0;s<4096;s++){
		((uint8*)page_directory)[s] = 0;
		((uint8*)first_page_table)[s] = 0;
	}
	
	for(uint32 i = 0; i < 1024; i++){
		first_page_table[i] = (i * 0x1000) | 0b11;
	}
	
	page_directory[0] = ((uint32)first_page_table) | 0b11;
	
	load_page_directory(page_directory);
	enable_paging();
	
	printf("paging enabled\n");
	
	for(uint32 i = 0; i < 1024; i++){
		page_used[i] = 1;
	}
	
	e820_entry_t* map = MEMORY_MAP;
	uint16 count = MEMORY_MAP_COUNT;
	for(int s=0;s<count;s++){
		uint32 base = map[s].base&0xFFFFFFFF;
		uint32 length = map[s].length&0xFFFFFFFF;
		uint8 usable = map[s].type==1;
		
		uint32 first_page = base / 4096;
		uint32 last_page  = (base + length) / 4096;
		
		if(first_page>=1024) continue;
		if(!usable) continue;
		for(int ss=0;ss<last_page-first_page;ss++){
			if(first_page+ss>=1024) continue;
			page_used[first_page+ss] = 0;
		}
	}
}




























