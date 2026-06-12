#include "filesystem.h"



uint64 _convert_uint48(uint48 value){
	uint64 val = *(uint64*)(&value.data);
	return val & 0x0000FFFFFFFFFFFFULL;
}
uint64 _convert_uint40(uint40 value){
	uint64 val = *(uint64*)(&value.data);
	return val & 0x000000FFFFFFFFFFULL;
}
uint64 convert_uint48(uint48 value){
	return  ((uint64)value.data[0])        |
			((uint64)value.data[1] << 8)   |
			((uint64)value.data[2] << 16)  |
			((uint64)value.data[3] << 24)  |
			((uint64)value.data[4] << 32)  |
			((uint64)value.data[5] << 40);
}

uint64 convert_uint40(uint40 value){
	return  ((uint64)value.data[0])        |
			((uint64)value.data[1] << 8)   |
			((uint64)value.data[2] << 16)  |
			((uint64)value.data[3] << 24)  |
			((uint64)value.data[4] << 32);
}

Super_Block* superblock;
void FILESYSTEM_init(){
	uint8* buffer = (uint8*)kalloc(4096);
	
	uint8 found = 0;
	for(int s=0;s<256;s++){
		sata_read(s*8,8,buffer);
		if(((uint64*)buffer)[0] == 0x55427974655F3A44){
			superblock = (Super_Block*)buffer;
			found = 1;
			break;
		}
	}
	if(!found) kfree(buffer);
	
	
	
	
	
	
}








/// Directory_Table_Header_Index -> split_mask -> bits
// 00000000 -> [256] 1-entry
// 00001000 -> {[128][128]} 2-entry
// 00001010 -> {[128]{[64][64]}} 3-entry
// 00001011 -> {[128]{[64]{[32][32]}}} 4-entry
// 00001111 -> {[128]{{[32][32]}{[32][32]}}} 5-entry
// 00101111 -> {{[64][64]}{{[32][32]}{[32][32]}}} 6-entry
// 00111111 -> {{[64]{[32][32]}}{{[32][32]}{[32][32]}}} 7-entry
// 01111111 -> {{{[32][32]}{[32][32]}}{{[32][32]}{[32][32]}}} 8-entry
// the 8th bit is unused
// 1 is the start of a new entry
// 0 is apart of the last entry
// enteries must be a power of 2(256/128/64/32)

/// Directory_Table_Header_Index -> free_mask -> bits
// 1 bit per 32 byte entry
// 2 bits per 64 byte entry, 2nd bit ignored
// 4 bits per 128 byte entry, 2nd/3rd/4th bits ignored
// 8 bits per 256 byte entry, all bits except first bit ignored

/// uint48 -> pointers
// they are made to index 4KB blocks, not bytes
// allows indexing 1EB(1024PB) of data

/// file names
// max-size: 256 bytes
// how to get length:
//     read last byte, if null(\0), decerment index, repeat until not null
//     if last byte wasnt null, then length is full size(if 256B, then 256B length)
// length is not stored
// file names are byte strings

/// Inode_Header -> flag_mask -> bits
// -------1 -> is_compressed
// ------1- -> full_data_journaling
// -----1-- -> file_data_inlined

/// Inode_Header -> user_permissions -> bits
// ------00 -> read
// ----00-- -> write
// --00---- -> execute
// 00------ -> delete/rename

/// Inode_Header -> group_permissions
// array of 8 to allow users there own permissions by group

/// Permission levels
// 00 -> everyone/guest
// 01 -> user
// 10 -> admin/owner
// 11 -> system

/// Inode_File_Table_Entry
// file_block_ptr points to a free block
// length is the size of continues free blocks the file is using
// supports 16TB worth of blocks

/// Inode_File_Table
// stores the table to all file block entries
// supports upto 4912TB(4.7PB)

/// Block Bitmap
// pre allocated at partition time
// aproxamitly 0.0031% partition size
// 1B = 32KB(1bit = 4KB)
// example: 1GB partition == 32KB bitmap

/// journalling
// the journal is a hidden file
// it can be resized to hold more journal data
// full-data journal data is allocated into a separate file
// log action start -> write to new file -> repoint inode to new file -> mark action complete































