#include "PMM.h"



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
	printf("--e820-Map--\n");
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
void PMM_print_stats(){
	printf("--PMM-pages--\n");
	printf("Free:  %p : %u\n",PMM_free_pages,PMM_free_pages);
	printf("Used:  %p : %u\n",PMM_total_pages-PMM_free_pages,PMM_total_pages-PMM_free_pages);
	printf("Total: %p : %u\n",PMM_total_pages,PMM_total_pages);
}

uint8* bitmap;
void bitmap_set(uint32 page){
	bitmap[page >> 3] |= (1 << (page & 7));
}
void bitmap_clear(uint32 page){
	bitmap[page >> 3] &= ~(1 << (page & 7));
}
int bitmap_test(uint32 page){
	return bitmap[page >> 3] & (1 << (page & 7));
}
void PMM_init(uint32 kernel_end){
	e820_entry_t* map = MEMORY_MAP;
	uint16 count = MEMORY_MAP_COUNT;
	
	uint32 highest = 0;
	
	for(uint32 i = 0; i < count; i++){
		if(map[i].type != 1) continue;
		
		uint32 end = (uint32)(map[i].base + map[i].length);
		
		if(end > highest)
			highest = end;
	}
	
	PMM_total_pages = (highest + PAGE_SIZE - 1) / PAGE_SIZE;
	
	uint32 bitmap_bytes = ALIGN_UP(PMM_total_pages,8) / 8;
	
	bitmap = (uint8*)ALIGN_UP(kernel_end, PAGE_SIZE);
	
	memset(bitmap, 0xFF, bitmap_bytes);
	
	for(int s=0;s<count;s++){
		if(map[s].type!=1) continue;
		
		uint32 first_page = map[s].base / PAGE_SIZE;
		uint32 last_page  = (map[s].base + map[s].length) / PAGE_SIZE;
		
		for(uint32 page = first_page; page < last_page; page++){
			bitmap_clear(page);
			PMM_free_pages++;
		}
	}
	uint32 reserved_end = ((uint32)bitmap) + ALIGN_UP(bitmap_bytes,PAGE_SIZE);
	
	for(uint32 page = 0;page < (reserved_end / PAGE_SIZE);page++){
		bitmap_set(page);
		PMM_free_pages--;
	}
}
uint32 PMM_alloc(uint32 count){
	uint32 run = 0;
	uint32 start = 0;
	
	for(uint32 page = 0;page < PMM_total_pages;page++){
		if(!bitmap_test(page)){
			if(run == 0)
				start = page;
			
			run++;
			
			if(run == count){
				for(uint32 i=0;i<count;i++)
					bitmap_set(start+i);
				
				return start * PAGE_SIZE;
			}
		}else{
			run = 0;
		}
	}
	
	return 0;
}
void PMM_free(uint32 adr,uint32 count){
	uint32 page = adr / PAGE_SIZE;
	
	for(uint32 i=0;i<count;i++)
		bitmap_clear(page+i);
}





























































