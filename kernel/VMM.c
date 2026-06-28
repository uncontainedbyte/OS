#include "VMM.h"



#define RECURSIVE_PD ((uint32*)0xFFFFF000)
#define RECURSIVE_PT(i) ((uint32*)(0xFFC00000 + ((i) << 12)))

uint32 current_cr3;

void load_page_directory(uint32 dir){
	asm volatile("mov %0, %%cr3":: "r"(dir));
}
void VMM_init(){
	current_cr3 = PMM_alloc(1);
	memset((uint8*)current_cr3,0,4096);
	
	uint32 first_pt = PMM_alloc(1);
	memset((void*)first_pt, 0, 4096);
	
	for(uint32 i=0;i<1024;i++){
		((uint32*)first_pt)[i] = (i * 0x1000) | 0x3;
	}
	
	((uint32*)current_cr3)[0] = first_pt | 0x3;
	((uint32*)current_cr3)[1023] = current_cr3 | 0x3;
	
	load_page_directory(current_cr3);
	
	// enable paging
	
	uint32 cr0;
	asm volatile("mov %%cr0, %0": "=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "r"(cr0));
}

int VMM_map(uint32 virt,uint32 phys,uint32 flags){
	phys &= 0xFFFFF000;
	virt &= 0xFFFFF000;
	
	uint32 pd_index = virt >> 22;
	uint32 pt_index = (virt >> 12) & 0x3FF;
	
	if(!(RECURSIVE_PD[pd_index] & 1)){
		uint32 table_phys = PMM_alloc(1);
		
		if(!table_phys) return 1;
		
		RECURSIVE_PD[pd_index] = table_phys | 0x3;
		
		load_page_directory(current_cr3);
		
		memset(RECURSIVE_PT(pd_index),0,4096);
	}
	
	if(RECURSIVE_PT(pd_index)[pt_index] & 1)
		return 1;
	
	RECURSIVE_PT(pd_index)[pt_index] = (phys & 0xFFFFF000) | flags | 0x1;
	
	asm volatile("invlpg (%0)" ::"r" (virt) : "memory");
	return 0;
}
void VMM_unmap(uint32 virt){
	virt &= 0xFFFFF000;
	
	uint32 pd_index = virt >> 22;
	uint32 pt_index = (virt >> 12) & 0x3FF;
	
	if(!(RECURSIVE_PD[pd_index] & 1))
		return;
	
	RECURSIVE_PT(pd_index)[pt_index] = 0;
	
	asm volatile("invlpg (%0)" ::"r" (virt) : "memory");
	
	int empty = 1;
	for(int s=0;s<1024;s++){
		if(RECURSIVE_PT(pd_index)[s] & 1){
			empty = 0;
			break;
	}}
	if(empty){
		PMM_free(RECURSIVE_PD[pd_index]&0xFFFFF000,1);
		RECURSIVE_PD[pd_index] = 0;
		load_page_directory(current_cr3);
	}
}

uint32 VMM_get_phys_address(uint32 virt){
	uint32 page_offset = virt & 0xFFF;
	uint32 pd_index = virt >> 22;
	uint32 pt_index = (virt >> 12) & 0x3FF;
	
	if(!(RECURSIVE_PD[pd_index] & 1))
		return 0;
		
	if(!(RECURSIVE_PT(pd_index)[pt_index] & 1))
		return 0;
	
	return (RECURSIVE_PT(pd_index)[pt_index] & 0xFFFFF000) | page_offset;
}

address_space_t VMM_create_directory(){
	address_space_t adr_spc = {0};
	adr_spc.cr3 = PMM_alloc(1);
	
	if(!adr_spc.cr3){
		return adr_spc;
	}
	
	void* pd = VMM_temp_map_0(adr_spc.cr3);
	memset(pd,0,4096);
	((uint32*)pd)[0] = RECURSIVE_PD[0];
	((uint32*)pd)[1023] = adr_spc.cr3 | 0x3;
	VMM_temp_unmap_0();
	
	return adr_spc;
}
void VMM_switch_directory(address_space_t adr_spc){
	current_cr3 = adr_spc.cr3;
	load_page_directory(adr_spc.cr3);
}
void VMM_destroy_directory(address_space_t adr_spc){
	uint32* pd = (uint32*)VMM_temp_map_0(adr_spc.cr3);
	for(int s=1;s<1023;s++){
		if(pd[s] & 1){
			PMM_free(pd[s]&0xFFFFF000,1);
	}}
	VMM_temp_unmap_0();
	
	PMM_free(adr_spc.cr3,1);
}
int VMM_map_in_directory(address_space_t space,uint32 virt,uint32 phys,uint32 flags){
	uint32* pd = VMM_temp_map_0(space.cr3);
	
	uint32 pd_index = virt >> 22;
	uint32 pt_index = (virt >> 12) & 0x3FF;
	
	if(!(pd[pd_index] & 1)){
		uint32 table_phys = PMM_alloc(1);
		
		if(!table_phys){
			VMM_temp_unmap_0();
			return 1;
		}
		
		pd[pd_index] = table_phys | 0x3;
		
		uint32* pt = VMM_temp_map_1(table_phys);
		
		memset(pt,0,4096);
		
		VMM_temp_unmap_1();
	}
	
	uint32* pt = VMM_temp_map_1(pd[pd_index] & 0xFFFFF000);
	pt[pt_index] = (phys & 0xFFFFF000) | flags | 1;
	
	VMM_temp_unmap_1();
	VMM_temp_unmap_0();
	
	return 0;
}


#define TEMP_PAGE_0 0x003FF000
#define TEMP_PAGE_1 0x003FE000
#define TEMP_PAGE_2 0x003FD000
#define TEMP_PAGE_3 0x003FC000
uint32 original_TEMP_PAGE_0;
uint32 original_TEMP_PAGE_1;
uint32 original_TEMP_PAGE_2;
uint32 original_TEMP_PAGE_3;

void* VMM_temp_map_0(uint32 phys){
	if(phys == 0)
		return 0;
	
	original_TEMP_PAGE_0 = RECURSIVE_PT(0)[1023];
	RECURSIVE_PT(0)[1023] = (phys & 0xFFFFF000) | 0x3;
	
	asm volatile("invlpg (%0)"::"r"(TEMP_PAGE_0):"memory");
	
	return (void*)TEMP_PAGE_0;
}
void* VMM_temp_map_1(uint32 phys){
	if(phys == 0)
		return 0;
	
	original_TEMP_PAGE_1 = RECURSIVE_PT(0)[1022];
	RECURSIVE_PT(0)[1022] = (phys & 0xFFFFF000) | 0x3;
	
	asm volatile("invlpg (%0)"::"r"(TEMP_PAGE_1):"memory");
	
	return (void*)TEMP_PAGE_1;
}
void* VMM_temp_map_2(uint32 phys){
	if(phys == 0)
		return 0;
	
	original_TEMP_PAGE_2 = RECURSIVE_PT(0)[1021];
	RECURSIVE_PT(0)[1021] = (phys & 0xFFFFF000) | 0x3;
	
	asm volatile("invlpg (%0)"::"r"(TEMP_PAGE_2):"memory");
	
	return (void*)TEMP_PAGE_2;
}
void* VMM_temp_map_3(uint32 phys){
	if(phys == 0)
		return 0;
	
	original_TEMP_PAGE_3 = RECURSIVE_PT(0)[1020];
	RECURSIVE_PT(0)[1020] = (phys & 0xFFFFF000) | 0x3;
	
	asm volatile("invlpg (%0)"::"r"(TEMP_PAGE_3):"memory");
	
	return (void*)TEMP_PAGE_3;
}
void VMM_temp_unmap_0(){
	RECURSIVE_PT(0)[1023] = original_TEMP_PAGE_0;
	
	asm volatile("invlpg (%0)"::"r"(TEMP_PAGE_0):"memory");
}
void VMM_temp_unmap_1(){
	RECURSIVE_PT(0)[1022] = original_TEMP_PAGE_1;
	
	asm volatile("invlpg (%0)"::"r"(TEMP_PAGE_1):"memory");
}
void VMM_temp_unmap_2(){
	RECURSIVE_PT(0)[1021] = original_TEMP_PAGE_2;
	
	asm volatile("invlpg (%0)"::"r"(TEMP_PAGE_2):"memory");
}
void VMM_temp_unmap_3(){
	RECURSIVE_PT(0)[1020] = original_TEMP_PAGE_3;
	
	asm volatile("invlpg (%0)"::"r"(TEMP_PAGE_3):"memory");
}

























































