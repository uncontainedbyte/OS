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

#define UNIT_SIZE 16
typedef struct heap_block {
	uint16 units;
	uint8 free;
	uint8 _pad;
	struct heap_block* next;
} __attribute__((packed)) heap_block_t;

uint8 _heap_size = 16; // size in pages
heap_block_t* heap;

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
		uint32 last_page  = ((base + length)-1) / 4096;
		
		if(first_page>=1024) continue;
		if(!usable) continue;
		for(int ss=0;ss<last_page-first_page;ss++){
			if(first_page+ss>=1024) continue;
			page_used[first_page+ss] = 0;
		}
	}
	
	for(int ss=0;ss<((uint32)first_page_table/4096+1);ss++){
		page_used[ss] = 1;
	}
	page_used[0] = 1;
	
	heap = (heap_block_t*)alloc_pages(_heap_size);
	heap->free=1;
	heap->next=0;
	heap->units = (_heap_size*4096)/UNIT_SIZE;
}

uint32 maybe_use_later___get_free_page(){
	for(int i = 0; i < 1024; i++){
		if(!page_used[i]){
			page_used[i] = 1;
			return i * 4096;
		}
	}
	
	return 0;
}
void maybe_use_later___free_page(uint32 addr){
	page_used[addr / 4096] = 0;
}
uint32 alloc_pages(uint32 count){
	uint32 c = 0;
	uint32 ptr = 0;
	for(uint32 i = 0; i < 1024; i++){
		if(!page_used[i]){
			if(c==0) ptr=i;
			c++;
			if(c==count) break;
		}else{ c=0; }
	}
	if(c != count) return 0;
	for(uint32 i = 0; i < c; i++){
		page_used[i+ptr]=1;
	}
	
	return ptr*4096;
}
void free_pages(uint32 page, uint32 count){
	for(uint32 i = 0; i < count; i++){
		page_used[i+(page/4096)]=0;
	}
}

void map_page(uint32 virt,uint32 phys,uint32 flags){
	
	uint32 pt_index = (virt >> 12) & 0x3FF;
	
	first_page_table[pt_index] = (phys & 0xFFFFF000) | flags | 1;
	
	asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

uint16 bytes_to_units(uint32 payload_bytes){
	uint32 total = payload_bytes + sizeof(heap_block_t);
	return (total + UNIT_SIZE - 1) / UNIT_SIZE;
}
void* kalloc(uint32 count){
	uint16 req_units = bytes_to_units(count);
	heap_block_t* cur = heap;
	
	while(cur){
		if(cur->free && cur->units >= req_units){
			if(cur->units > req_units){//split block into 2
				heap_block_t* split = (heap_block_t*)((uint8*)cur + req_units * UNIT_SIZE);
				
				split->units = cur->units - req_units;
				split->free = 1;
				split->next = cur->next;
				
				cur->units = req_units;
				cur->next = split;
			}
			
			cur->free = 0;
			return ((uint8*)cur + sizeof(heap_block_t));
		}
		cur = cur->next;
	}
	printf("%#40\n! Heap Out Of Space !\n");
	asm volatile("hlt");
	while(1);
	return 0;
}
void kfree(void* ptr){
	if(!ptr) return;
	
	heap_block_t* block = (heap_block_t*)((uint8*)ptr - sizeof(heap_block_t));
	
	if(block->free){
		return;
	}
	
	block->free = 1;
	
	if(block->next && block->next->free){
		block->units += block->next->units;
		block->next = block->next->next;
	}
	
	heap_block_t* last=0;
	heap_block_t* cur = heap;
	while(cur!=block){
		last = cur;
		cur = cur->next;
	}
	if(last!=0 && last->free){
		last->units += block->units;
		last->next = block->next;
	}
}
void* krealloc(void* ptr, uint32 new_size){
	if(ptr == 0) return kalloc(new_size);
	
	if(new_size == 0){
		kfree(ptr);
		return 0;
	}
	
	heap_block_t* block = (heap_block_t*)((char*)ptr - sizeof(heap_block_t));
	
	uint32 old_size = block->units * UNIT_SIZE;
	
	void* new_ptr = kalloc(new_size);
	if(new_ptr == 0) return 0;
	
	uint32 size = (old_size<new_size)? old_size : new_size;
	memcpy(new_ptr,ptr,size);
	
	kfree(ptr);
	
	return new_ptr;
}

void heap_dump(){
	heap_block_t* cur = heap;
	
	while(cur){
		printf(
			"block: %p units: %u free: %u next: %p\n",
			cur,
			cur->units,
			cur->free,
			cur->next
		);
		
		cur = cur->next;
	}
}

































