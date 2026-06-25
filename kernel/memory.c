#include "memory.h"





#define PAGE_ALLOC 4 // number of pages to be alloceded when heap full

#define UNIT_SIZE 16
typedef struct heap_block {
	uint32 units;
	uint8 free;
	uint8 _pad;
	struct heap_block* next;
} __attribute__((packed)) heap_block_t;

heap_block_t* heap;
uint32 heap_page_count;

void MEM_init(){
	uint32 phys = PMM_alloc(PAGE_ALLOC);
	for(int s=0;s<PAGE_ALLOC;s++){
		VMM_map(KERNEL_HEAP_START+(s*4096),phys+(s*4096),3);
	}
	heap_page_count = PAGE_ALLOC;
	
	heap = (heap_block_t*)KERNEL_HEAP_START;
	heap->free=1;
	heap->next=0;
	heap->units = (PAGE_ALLOC*4096)/UNIT_SIZE;
}

uint32 bytes_to_units(uint32 payload_bytes){
	uint32 total = payload_bytes + sizeof(heap_block_t);
	return (total + UNIT_SIZE - 1) / UNIT_SIZE;
}
int heap_extend(heap_block_t* end){
	uint32 phys = PMM_alloc(PAGE_ALLOC);
	if(!phys) return 1;
	
	for(int s=0;s<PAGE_ALLOC;s++){
		VMM_map(KERNEL_HEAP_START+((heap_page_count+s)*4096),phys+(s*4096),3);
	}
	heap_page_count += PAGE_ALLOC;
	
	end->units += (PAGE_ALLOC*4096)/UNIT_SIZE;
	
	return 0;
}
void* kalloc(uint32 count){
	uint32 req_units = bytes_to_units(count);
	heap_block_t* cur = heap;
	
	while(1){
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
			return (((uint8*)cur) + sizeof(heap_block_t));
		}else if(!cur->next){
			if(heap_extend(cur)) break;
			continue;
		}
		if(!cur->next) break;
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
		printf("block: %p units: %u free: %u next: %p\n",cur,cur->units,cur->free,cur->next);
		cur = cur->next;
	}
}

































