#include "filesystem.h"

typedef struct{
	uint8 data[6];
}__attribute__((packed)) uint48;
typedef struct{
	uint8 data[5];
}__attribute__((packed)) uint40;

typedef struct{
	uint8 free_mask;
	uint8 split_mask;
	uint48 inode_ptr1; // 256B ptr || 128B first ptr
	uint48 inode_ptr2; // 128B second ptr
}__attribute__((packed)) Directory_Table_Header_Index; //10B
typedef struct{
	uint8 type; // 'D' == directory
	Directory_Table_Header_Index indexes[15];
	uint48 nextBlock;
	uint8 reserved[39];
}__attribute__((packed)) Directory_Table_Header; //256B
typedef struct{
	char data[256];
}__attribute__((packed)) Directory_Table_Entry_256B; //256B
typedef struct{
	char data1[128];
	char data2[128];
}__attribute__((packed)) Directory_Table_Entry_128B; //256B
typedef struct{
	char data1[58];
	uint48 inode_ptr1;
	char data2[58];
	uint48 inode_ptr2;
	char data3[58];
	uint48 inode_ptr3;
	char data4[58];
	uint48 inode_ptr4;
}__attribute__((packed)) Directory_Table_Entry_64B; //256B
typedef struct{
	char data1[26];
	uint48 inode_ptr1;
	char data2[26];
	uint48 inode_ptr2;
	char data3[26];
	uint48 inode_ptr3;
	char data4[26];
	uint48 inode_ptr4;
	char data5[26];
	uint48 inode_ptr5;
	char data6[26];
	uint48 inode_ptr6;
	char data7[26];
	uint48 inode_ptr7;
	char data8[26];
	uint48 inode_ptr8;
}__attribute__((packed)) Directory_Table_Entry_32B; //256B
typedef struct{
	union{
		Directory_Table_Entry_256B A;
		Directory_Table_Entry_128B B;
		Directory_Table_Entry_64B C;
		Directory_Table_Entry_32B D;
	};
}__attribute__((packed)) Directory_Table_Entry; //256B
typedef struct{
	Directory_Table_Header header;
	Directory_Table_Entry entries[15];
}__attribute__((packed)) Directory_Table; //4096B(4KB)

typedef struct{
	uint16 group_id;
	uint8 permissions;
}__attribute__((packed)) Group_Permissions; //3B
typedef struct{
	uint8 type; // 'F' == file
	uint8 flag_mask;
	uint16 owner_id;
	uint8 user_permissions;
	Group_Permissions group_permissions[8];
	uint64 file_size;
	uint40 Created_Timestamp;//  the
	uint40 Modified_Timestamp;// ED
	uint40 Accessed_Timestamp;// group
	uint40 Moved_Timestamp;//    _
	uint8 reserved[967];
}__attribute__((packed)) Inode_Header; //1024B(1KB)
typedef struct{
	uint48 file_block_ptr;
	uint32 length;
}__attribute__((packed)) Inode_File_Table_Entry; //10B
typedef struct{
	Inode_File_Table_Entry entries[307]; //3070B
	uint8 reserved[2];
}__attribute__((packed)) Inode_File_Table; //3072B(3KB)
typedef struct{
	Inode_Header header;
	union{
		Inode_File_Table table;
		uint8 inlined_file[3072];
	};
}__attribute__((packed)) Inode; //4096B(4KB)

typedef struct{
	char   magic[8];           // 8B: "55427974655F3A44" (UByte_:D)
	uint64 total_blocks;       // 8B: Total number of 4KB blocks in this partition
	uint32 block_size;         // 4B: Hardcoded to 4096
	uint48 root_dir_ptr;       // 6B: Pointer to the Root Directory block
	uint48 bitmap_start_block; // 6B: Pointer to the first block of the allocation bitmap
	uint32 bitmap_block_count; // 4B: Size of the bitmap section in blocks (0.0031% of drive)
	uint48 journal_inode_ptr;  // 6B: Pointer to the Inode of the hidden Journal File
	uint8  filesystem_state;   // 1B: Cleanly unmounted vs Dirty/Crashed flags
	uint40 last_mounted_time;  // 5B
	uint40 last_checked_time;  // 5B: For forcing filesystem consistency checks (fsck)
	uint8  reserved[4049];
} __attribute__((packed)) Super_Block; //4096B(4KB)

typedef struct{
	uint64 standard_inode_ptr;
	uint64 file_size;
	uint64 current_seek_pos;
	
	uint64 created_time;
	uint64 modified_time;
	uint64 accessed_time;
	uint64 moved_time;
	
	uint8 flag_mask;
} File_Handle;



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
uint48 make_uint48(uint64 val){
	uint48 out;
	out.data[0] = val & 0xFF;
	out.data[1] = (val >> 8) & 0xFF;
	out.data[2] = (val >> 16) & 0xFF;
	out.data[3] = (val >> 24) & 0xFF;
	out.data[4] = (val >> 32) & 0xFF;
	out.data[5] = (val >> 40) & 0xFF;
	return out;
}
uint40 make_uint40(uint64 val){
	uint40 out;
	out.data[0] = val & 0xFF;
	out.data[1] = (val >> 8) & 0xFF;
	out.data[2] = (val >> 16) & 0xFF;
	out.data[3] = (val >> 24) & 0xFF;
	out.data[4] = (val >> 32) & 0xFF;
	return out;
}


void read_block(uint64 index,uint8* buffer){
	sata_read(index*8,8,buffer);
}
void write_block(uint64 index,uint8* buffer){
	sata_write(index*8,8,buffer);
}


uint64 _super_index;
Super_Block* super;
Directory_Table* rootdir;
uint8* filesystem_bitmap;
uint64 blockcount;

void* BLOCK_buffer;
File_Handle* LoadedFiles;

void flush_to_disk(){
	write_block(_super_index,(uint8*)super);
	write_block(convert_uint48(super->root_dir_ptr),(uint8*)rootdir);
	for(int s=0;s<super->bitmap_block_count;s++){
		write_block(convert_uint48(super->bitmap_start_block)+s,filesystem_bitmap+(s*4096));
	}
}
uint64 get_free_block(){
	for(int s=0;s<(blockcount+7)/8;s++){
		for(int ss=0;ss<8;ss++){
			if(!((filesystem_bitmap[s]>>ss)&0b1)){
				return s*8+ss;
			}
		}
	}
	
	return 0;
}
void mark_used(uint32 index){
	filesystem_bitmap[(index+7)/8] |= (0b1<<(index%8));
}
void mark_free(uint32 index){
	filesystem_bitmap[(index+7)/8] &= ~(0b1<<(index%8));
}



int FS_len(const char* name,int l){
	for(int f=l;f>0;f--){
		if(name[f-1]!='\0'){
			return f;
		}
	}
	return 0;
}
uint64 FS_dir_get(Directory_Table* dir,const char* name){
	for(int s=0;s<15;s++){
		uint8 split = dir->header.indexes[s].split_mask;
		uint8 free = dir->header.indexes[s].free_mask;
		
		if(split==0b00000000){
			if(free&0b1){
				int size = FS_len(((char*)(dir->entries[s].A.data)),256);
				int found = 1;
				if(size!=strlen(name)) found = 0;
				for(int c=0;c<size;c++){
					if(name[c]!=dir->entries[s].A.data[c]){
						found = 0;
						break;
					}
				}
				if(!found) continue;
				return convert_uint48(dir->header.indexes[s].inode_ptr1);
			}
		}
		
		
		
	}
	
	return 0;
}
uint64 FS_parse_path(const char* path){
	
	uint16 depth = 0;
	uint16 index=0;
	char* buf = kalloc(257);
	for(int s=0;s<strlen(path);s++){
		if(path[s]=='/'){
			buf[index] = '\0';
			index = 0;
			uint64 address = 0;
			
			if(depth==0){
				address = FS_dir_get(rootdir,buf);
			}else{
				address = FS_dir_get((Directory_Table*)BLOCK_buffer,buf);
			}
			
		}
		
		buf[index] = path[s];
		
		
	}
	
	
	
	
	
	
}

void add_to_name_dir_256B(const char* src,char* dest){
	for(int s=0;s<256;s++){ dest[s]='\0'; }
	for(int s=0;s<256;s++){
		if(src[s]=='\0') return;
		dest[s] = src[s];
	}
}
void add_to_name_dir_128B(const char* src,char* dest){
	for(int s=0;s<128;s++){ dest[s]='\0'; }
	for(int s=0;s<128;s++){
		if(src[s]=='\0') return;
		dest[s] = src[s];
	}
}
void add_to_name_dir_64B(const char* src,char* dest){
	for(int s=0;s<58;s++){ dest[s]='\0'; }
	for(int s=0;s<58;s++){
		if(src[s]=='\0') return;
		dest[s] = src[s];
	}
}
void add_to_name_dir_32B(const char* src,char* dest){
	for(int s=0;s<26;s++){ dest[s]='\0'; }
	for(int s=0;s<26;s++){
		if(src[s]=='\0') return;
		dest[s] = src[s];
	}
}
void add_to_dir(const char* name,Directory_Table* dir,uint64 index){
	uint16 length = strlen(name);
	uint8 type = 0;
	if(length<=256){ type=1; }
	if(length<=128){ type=2; }
	if(length<=58){ type=3; }
	if(length<=26){ type=4; }
	
	for(int s=0;s<15;s++){
		uint8 split = dir->header.indexes[s].split_mask;
		uint8 free = dir->header.indexes[s].free_mask;
		if(split==0b00000000){//256B
			if(!(free&0b1)){
				if(type==1){
					add_to_name_dir_256B(name,(char*)&dir->entries[s].A);
					dir->header.indexes[s].inode_ptr1 = make_uint48(index);
					dir->header.indexes[s].free_mask=0b00000001;
					return;
				}else if(type>1){
					dir->header.indexes[s].split_mask|=0b00010000;
				}
			}
		}
		if((split&0xF0)==0b00010000){//128B
			if(!(free&0b10000)){
				if(type==2){
					add_to_name_dir_128B(name,(char*)&dir->entries[s].B.data1);
					dir->header.indexes[s].inode_ptr1 = make_uint48(index);
					dir->header.indexes[s].free_mask|=0b00010000;
					return;
				}else if(type>2){
					dir->header.indexes[s].split_mask|=0b01000000;
				}
			}
		}
		if((split&0xC0)==0b01000000){//64B
			if(!(free&0b1000000)){
				if(type==3){
					add_to_name_dir_64B(name,(char*)&dir->entries[s].C.data1);
					dir->entries[s].C.inode_ptr1 = make_uint48(index);
					dir->header.indexes[s].free_mask|=0b01000000;
					return;
				}else if(type>3){
					dir->header.indexes[s].split_mask|=0b10000000;
				}
			}
		}
		if((split&0x80)==0b10000000){//32B
			if(!(free&0b10000000)){
				if(type==4){
					add_to_name_dir_32B(name,(char*)&dir->entries[s].D.data1);
					dir->entries[s].D.inode_ptr1 = make_uint48(index);
					dir->header.indexes[s].free_mask|=0b10000000;
					return;
				}
			}
		}
		if((split&0x40)==0b01000000){//32B
			if(!(free&0b1000000)){
				if(type==4){
					add_to_name_dir_32B(name,(char*)&dir->entries[s].D.data2);
					dir->entries[s].D.inode_ptr2 = make_uint48(index);
					dir->header.indexes[s].free_mask|=0b01000000;
					return;
				}
			}
		}
		if((split&0x30)==0b00010000){//64B
			if(!(free&0b10000)){
				if(type==3){
					add_to_name_dir_64B(name,(char*)&dir->entries[s].C.data2);
					dir->entries[s].C.inode_ptr2 = make_uint48(index);
					dir->header.indexes[s].free_mask|=0b00010000;
					return;
				}else if(type>3){
					dir->header.indexes[s].split_mask|=0b00100000;
				}
			}
		}
		if((split&0x20)==0b00100000){//32B
			if(!(free&0b100000)){
				if(type==4){
					add_to_name_dir_32B(name,(char*)&dir->entries[s].D.data3);
					dir->entries[s].D.inode_ptr3 = make_uint48(index);
					dir->header.indexes[s].free_mask|=0b00100000;
					return;
				}
			}
		}
		if((split&0x10)==0b00010000){//32B
			if(!(free&0b10000)){
				if(type==4){
					add_to_name_dir_32B(name,(char*)&dir->entries[s].D.data4);
					dir->entries[s].D.inode_ptr4 = make_uint48(index);
					dir->header.indexes[s].free_mask|=0b00010000;
					return;
				}
			}
		}
		if((split&0xF) ==0b00000000){//128B
			if(!(free&0b1)){
				if(type==2){
					add_to_name_dir_128B(name,(char*)&dir->entries[s].B.data2);
					dir->header.indexes[s].inode_ptr2 = make_uint48(index);
					dir->header.indexes[s].free_mask|=0b00000001;
					return;
				}else if(type>2){
					dir->header.indexes[s].split_mask|=0b00000100;
				}
			}
		}
		if((split&0xC) ==0b00000100){//64B
			if(!(free&0b100)){
				if(type==3){
					add_to_name_dir_64B(name,(char*)&dir->entries[s].C.data3);
					dir->entries[s].C.inode_ptr3 = make_uint48(index);
					dir->header.indexes[s].free_mask|=0b00000100;
					return;
				}else if(type>3){
					dir->header.indexes[s].split_mask|=0b00001000;
				}
			}
		}
		if((split&0x8) ==0b00001000){//32B
			if(!(free&0b1000)){
				if(type==4){
					add_to_name_dir_32B(name,(char*)&dir->entries[s].D.data5);
					dir->entries[s].D.inode_ptr5 = make_uint48(index);
					dir->header.indexes[s].free_mask|=0b00001000;
					return;
				}
			}
		}
		if((split&0x4) ==0b00000100){//32B
			if(!(free&0b100)){
				if(type==4){
					add_to_name_dir_32B(name,(char*)&dir->entries[s].D.data6);
					dir->entries[s].D.inode_ptr6 = make_uint48(index);
					dir->header.indexes[s].free_mask|=0b00000100;
					return;
				}
			}
		}
		if((split&0x3) ==0b00000000){//64B
			if(!(free&0b1)){
				if(type==3){
					add_to_name_dir_64B(name,(char*)&dir->entries[s].C.data4);
					dir->entries[s].C.inode_ptr4 = make_uint48(index);
					dir->header.indexes[s].free_mask|=0b00000001;
					return;
				}else if(type>3){
					dir->header.indexes[s].split_mask|=0b00000010;
				}
			}
		}
		if((split&0x2) ==0b00000010){//32B
			if(!(free&0b10)){
				if(type==4){
					add_to_name_dir_32B(name,(char*)&dir->entries[s].D.data7);
					dir->entries[s].D.inode_ptr7 = make_uint48(index);
					dir->header.indexes[s].free_mask|=0b00000010;
					return;
				}
			}
		}
		{//32B
			if(!(free&0b1)){
				if(type==4){
					add_to_name_dir_32B(name,(char*)&dir->entries[s].D.data8);
					dir->entries[s].D.inode_ptr8 = make_uint48(index);
					dir->header.indexes[s].free_mask|=0b00000001;
					return;
				}
			}
		}
	}
	
	printf("Directory Full\n");
}

uint8 FS_namecmp(const char* a,const char* b,uint16 B){
	if(B<strlen(a)) return 0;
	
	for(int s=0;s<B;s++){
		if(a[s]!=b[s]) return 0;
		if(a[s]=='\0') return 1;
	}
}
void remove_from_dir(const char* name,Directory_Table* dir){
	
	uint16 length = strlen(name);
	uint8 type = 0;
	if(length<=256){ type=1; }
	if(length<=128){ type=2; }
	if(length<=58){ type=3; }
	if(length<=26){ type=4; }
	
	for(int s=0;s<15;s++){
		uint8 split = dir->header.indexes[s].split_mask;
		uint8 free = dir->header.indexes[s].free_mask;
		if(split==0b00000000){//256B
			if(free&0b1){
				if(type==1){
					if(FS_namecmp(name,(char*)&dir->entries[s].A,256)){
						dir->header.indexes[s].free_mask=0;
						return;
					}
				}
			}
		}
		if((split&0xF0)==0b00010000){//128B
			if(free&0b10000){
				if(type==2){
					if(FS_namecmp(name,(char*)&dir->entries[s].B.data1,128)){
						dir->header.indexes[s].free_mask&=0b00001111;
						return;
					}
				}
			}
		}
		if((split&0xC0)==0b01000000){//64B
			if(free&0b1000000){
				if(type==3){
					if(FS_namecmp(name,(char*)&dir->entries[s].C.data1,58)){
						dir->header.indexes[s].free_mask&=0b00111111;
						return;
					}
				}
			}
		}
		if((split&0x80)==0b10000000){//32B
			if(free&0b10000000){
				if(type==4){
					if(FS_namecmp(name,(char*)&dir->entries[s].D.data1,26)){
						dir->header.indexes[s].free_mask&=0b01111111;
						return;
					}
				}
			}
		}
		if((split&0x40)==0b01000000){//32B
			if(free&0b1000000){
				if(type==4){
					if(FS_namecmp(name,(char*)&dir->entries[s].D.data2,26)){
						dir->header.indexes[s].free_mask&=0b10111111;
						return;
					}
				}
			}
		}
		if((split&0x30)==0b00010000){//64B
			if(free&0b10000){
				if(type==3){
					if(FS_namecmp(name,(char*)&dir->entries[s].C.data2,58)){
						dir->header.indexes[s].free_mask&=0b11001111;
						return;
		}}}}
		if((split&0x20)==0b00100000){//32B
			if(free&0b100000){
				if(type==4){
					if(FS_namecmp(name,(char*)&dir->entries[s].D.data3,26)){
						dir->header.indexes[s].free_mask&=0b11011111;
						return;
		}}}}
		if((split&0x10)==0b00010000){//32B
			if(free&0b10000){
				if(type==4){
					if(FS_namecmp(name,(char*)&dir->entries[s].D.data4,26)){
						dir->header.indexes[s].free_mask&=0b11101111;
						return;
		}}}}
		if((split&0xF) ==0b00000000){//128B
			if(free&0b1){
				if(type==2){
					if(FS_namecmp(name,(char*)&dir->entries[s].B.data2,128)){
						dir->header.indexes[s].free_mask&=0b11110000;
						return;
		}}}}
		if((split&0xC) ==0b00000100){//64B
			if(free&0b100){
				if(type==3){
					if(FS_namecmp(name,(char*)&dir->entries[s].C.data3,58)){
						dir->header.indexes[s].free_mask&=0b11110011;
						return;
		}}}}
		if((split&0x8) ==0b00001000){//32B
			if(free&0b1000){
				if(type==4){
					if(FS_namecmp(name,(char*)&dir->entries[s].D.data5,26)){
						dir->header.indexes[s].free_mask&=0b11110111;
						return;
		}}}}
		if((split&0x4) ==0b00000100){//32B
			if(free&0b100){
				if(type==4){
					if(FS_namecmp(name,(char*)&dir->entries[s].D.data6,26)){
						dir->header.indexes[s].free_mask&=0b11111011;
						return;
		}}}}
		if((split&0x3) ==0b00000000){//64B
			if(free&0b1){
				if(type==3){
					if(FS_namecmp(name,(char*)&dir->entries[s].C.data4,58)){
						dir->header.indexes[s].free_mask&=0b11111100;
						return;
		}}}}
		if((split&0x2) ==0b00000010){//32B
			if(free&0b10){
				if(type==4){
					if(FS_namecmp(name,(char*)&dir->entries[s].D.data7,26)){
						dir->header.indexes[s].free_mask&=0b11111101;
						return;
		}}}}
		{//32B
			if(free&0b1){
				if(type==4){
					if(FS_namecmp(name,(char*)&dir->entries[s].D.data8,26)){
						dir->header.indexes[s].free_mask&=0b11111110;
						return;
		}}}}
	}
	
	
}

void desplit_dir(Directory_Table* dir){
	
	for(int s=0;s<15;s++){
		uint8 split = dir->header.indexes[s].split_mask;
		uint8 free = dir->header.indexes[s].free_mask;
		
		if((split&0b10000000)&&(split&0b01000000)){ // Merge s1-32B & s2-32B -> s1-64B
			if(!(free&0b11000000)){
				dir->header.indexes[s].split_mask &= 0b01111111;
				split &= 0b01111111;
			}
		}
		if((split&0b100000)&&(split&0b010000)){     // Merge s3-32B & s4-32B -> s2-64B
			if(!(free&0b110000)){
				dir->header.indexes[s].split_mask &= 0b11011111;
				split &= 0b11011111;
			}
		}
		if((split&0b1000)&&(split&0b0100)){         // Merge s5-32B & s6-32B -> s3-64B
			if(!(free&0b1100)){
				dir->header.indexes[s].split_mask &= 0b11110111;
				split &= 0b11110111;
			}
		}
		if((split&0b10)){                           // Merge s7-32B & s8-32B -> s4-64B
			if(!(free&0b11)){
				dir->header.indexes[s].split_mask &= 0b11111101;
				split &= 0b11111101;
			}
		}
		if((split&0b01000000)&&(split&0b00010000)){ // Merge s1-64B & s2-64B -> s1-128B
			if(!(free&0b11110000)){
				dir->header.indexes[s].split_mask &= 0b00011111;
				split &= 0b00011111;
			}
		}
		if((split&0b0100)){                         // Merge s3-64B & s4-64B -> s2-128B
			if(!(free&0b1111)){
				dir->header.indexes[s].split_mask &= 0b11110001;
				split &= 0b11110001;
			}
		}
		if((split&0b010000)){                       // Merge s3-128B & s4-128B -> 256B
			if(!(free&0b11111111)){
				dir->header.indexes[s].split_mask = 0;
			}
		}
	}
	
	
}

uint8 FS_dir_create(const char* path,const char* name){
	BLOCK_buffer = (void*)rootdir;
	
	// after parsing path
	
	Directory_Table* dir = (Directory_Table*)BLOCK_buffer;
	
	uint64 index = get_free_block();
	if(index==0) return FS_NoSpace;
	
	Directory_Table new_dir;
	new_dir.header.type = 'D';
	
	// clear new directory
	for(int s=0;s<39;s++){ new_dir.header.reserved[s] = 0; }
	new_dir.header.nextBlock = make_uint48(0);
	for(int s=0;s<15;s++){
		new_dir.header.indexes[s].free_mask = 0;
		new_dir.header.indexes[s].split_mask = 0;
		new_dir.header.indexes[s].inode_ptr1 = make_uint48(0);
		new_dir.header.indexes[s].inode_ptr2 = make_uint48(0);
	}
	
	write_block(index,(uint8*)&new_dir);
	mark_used(index);
	
	// add new directory to existing directory
	add_to_dir(name,dir,index);
	
	flush_to_disk();
	return FS_Success;
}
uint8 FS_dir_delete(const char* path,const char* name){
	BLOCK_buffer = (void*)rootdir;
	
	// after parsing path
	
	Directory_Table* dir = (Directory_Table*)BLOCK_buffer;
	
	remove_from_dir(name,dir);
	
	desplit_dir(dir);
	
	flush_to_disk();
	return FS_Success;
}












void FILESYSTEM_init(){
	uint8* buffer = (uint8*)alloc_pages(1); // superblock
	
	uint8 found = 0;
	for(int s=0;s<256;s++){
		read_block(s,buffer);
		if(((uint64*)buffer)[0] == 0x443A5F6574794255){
			super = (Super_Block*)buffer;
			found = 1;
			_super_index = s;
			break;
		}
	}
	if(!found){
		free_pages((uint32)buffer,1);
		printf("no file system found");
		printf("halting");
		asm volatile("hlt");
		while(1);
	}
	
	blockcount = super->bitmap_block_count;
	
	buffer = (uint8*)alloc_pages(blockcount); // filesystem_bitmap
	for(int s=0;s<blockcount;s++){
		read_block(s+convert_uint48(super->bitmap_start_block),buffer+(s*4096));
	}
	filesystem_bitmap = buffer;
	
	buffer = (uint8*)alloc_pages(1); // rootdir
	read_block(convert_uint48(super->root_dir_ptr),buffer);
	rootdir = (Directory_Table*)buffer;
	
	BLOCK_buffer = (void*)alloc_pages(1);
	
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































