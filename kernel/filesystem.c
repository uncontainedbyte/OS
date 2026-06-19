#include "filesystem.h"
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


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
uint8* filesystem_bitmap;
uint64 blockcount;

File_Handle* LoadedFiles;

void flush_to_disk(){
	write_block(_super_index,(uint8*)super);
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
	filesystem_bitmap[index/8] |= (0b1<<(index%8));
}
void mark_free(uint32 index){
	filesystem_bitmap[index/8] &= ~(0b1<<(index%8));
}




typedef struct {
	uint8 split_test_mask;  // (split&THIS)==0000
	uint8 split_test_value; // (split&0000)==THIS
	uint8 free_mask;        // free&THIS
	uint8 size_class;
	uint16 entry_size;
	uint8 claim_mask;
	uint8 split_mask;
	uint8 used_mask;
	uint8 name_offset;
	int16 inode_offset;
} DirDescriptor;
static const DirDescriptor DirDescriptor_entries[15] = {
	{0xFF,0x01,0x01,1,256,0x01,0x10,0xFF,0x00,-1},
	
	{0xF0,0x10,0x10,2,128,0x10,0x40,0xF0,0x80,-1},
	{0xC0,0x40,0x40,3,58,0x40,0x80,0xC0,0xC0,0xFA},
	{0x80,0x80,0x80,4,26,0x80,0x00,0x80,0xE0,0xFA},
	{0x40,0x40,0x40,4,26,0x40,0x00,0x40,0xC0,0xDA},
	
	{0x30,0x10,0x10,3,58,0x10,0x20,0x30,0x80,0xBA},
	{0x20,0x20,0x20,4,26,0x20,0x00,0x20,0xA0,0xBA},
	{0x10,0x10,0x10,4,26,0x10,0x00,0x10,0x80,0x9A},
	
	{0x0F,0x01,0x01,2,128,0x01,0x04,0x0F,0x00,-2},
	{0x0C,0x04,0x04,3,58,0x04,0x08,0x0C,0x40,0x7A},
	{0x08,0x08,0x08,4,26,0x08,0x00,0x08,0x60,0x7A},
	{0x04,0x04,0x04,4,26,0x04,0x00,0x04,0x40,0x5A},
	
	{0x03,0x01,0x01,3,58,0x01,0x02,0x03,0x00,0x3A},
	{0x02,0x02,0x02,4,26,0x02,0x00,0x02,0x20,0x3A},
	{0x01,0x01,0x01,4,26,0x01,0x00,0x01,0x00,0x1A},
};
uint64 get_inode_ptr(Directory_Table* dir,int slot,const DirDescriptor* d){
	if(d->inode_offset==-1){
		return convert_uint48(dir->header.indexes[slot].inode_ptr1);
	}else if(d->inode_offset==-2){
		return convert_uint48(dir->header.indexes[slot].inode_ptr2);
	}else{
		uint48* inode = (uint48*)(((char*)(&(dir->entries[slot]))) + d->inode_offset);
		return convert_uint48(*inode);
	}
}
void set_inode_ptr(Directory_Table* dir,int slot,const DirDescriptor* d,uint64 ptr){
	if(d->inode_offset==-1){
		dir->header.indexes[slot].inode_ptr1 = make_uint48(ptr);
	}else if(d->inode_offset==-2){
		dir->header.indexes[slot].inode_ptr2 = make_uint48(ptr);
	}else{
		uint48* inode = (uint48*)((char*)&dir->entries[slot] + d->inode_offset);
		*inode = make_uint48(ptr);
	}
}




void cpy_name_to_dir(const char* src,char* dest,int l){
	for(int s=0;s<l;s++){ dest[s]='\0'; }
	for(int s=0;s<l;s++){
		if(src[s]=='\0') return;
		dest[s] = src[s];
	}
}
uint8 add_to_dir(const char* name,Directory_Table* dir,uint64 index){
	uint16 length = strlen(name);
	uint8 type = 0;
	if(length<=26 ){ type=4; }else
	if(length<=58 ){ type=3; }else
	if(length<=128){ type=2; }else
	if(length<=256){ type=1; }
	
	const DirDescriptor* dde = DirDescriptor_entries;
	
	for(int s=0;s<15;s++){
		uint8 split = dir->header.indexes[s].split_mask;
		uint8 free = dir->header.indexes[s].free_mask;
		
		for(int ss=0;ss<15;ss++){
			if((split&dde[ss].split_test_mask)==dde[ss].split_test_value){
				if(!(free&dde[ss].free_mask)){
					if(dde[ss].size_class==type){
						cpy_name_to_dir(name,(char*)&dir->entries[s]+dde[ss].name_offset,dde[ss].entry_size);
						set_inode_ptr(dir,s,&(dde[ss]),index);
						dir->header.indexes[s].free_mask|=dde[ss].claim_mask;
						return FS_Success;
					}else if(type>dde[ss].size_class&&dde[ss].size_class!=4){
						dir->header.indexes[s].split_mask |= dde[ss].split_mask;
						split |= dde[ss].split_mask;
	}}}}}
	
	printf("Directory Full\n");
	return FS_Fail;
}

uint8 FS_namecmp(const char* a,const char* b,uint16 B){
	if(B<strlen(a)) return 0;
	
	for(int s=0;s<B;s++){
		if(a[s]!=b[s]) return 0;
		if(a[s]=='\0') return 1;
	}
}
uint8 find_in_dir(const char* name,Directory_Table* dir,int* index,uint8* index_mask,uint64* ptr){
	int ____NULL_1;
	uint8 ____NULL_2;
	uint64 ____NULL_3;
	if(index==0) index=&____NULL_1;
	if(index_mask==0) index_mask=&____NULL_2;
	if(ptr==0) ptr=&____NULL_3;
	
	uint16 length = strlen(name);
	uint8 type = 0;
	if(length<=26 ){ type=4; }else
	if(length<=58 ){ type=3; }else
	if(length<=128){ type=2; }else
	if(length<=256){ type=1; }
	
	const DirDescriptor* dde = DirDescriptor_entries;
	
	for(int s=0;s<15;s++){
		uint8 split = dir->header.indexes[s].split_mask;
		uint8 free = dir->header.indexes[s].free_mask;
		
		for(int ss=0;ss<15;ss++){
			if((split&dde[ss].split_test_mask)==dde[ss].split_test_value){
				if(free&dde[ss].free_mask){
					if(dde[ss].size_class==type){
						if(FS_namecmp(name,(char*)&dir->entries[s]+dde[ss].name_offset,dde[ss].entry_size)){
							*index = s; *index_mask = dde[ss].used_mask;
							*ptr = get_inode_ptr(dir,s,&(dde[ss]));
							return FS_Success;
		}}}}}
		
	}
	
	*ptr = 0;
	*index = -1;
	*index_mask = 0;
	return FS_Fail;
}

int FS_len(const char* name,int l){
	for(int f=l;f>0;f--){
		if(name[f-1]!='\0'){
			return f;
		}
	}
	return 0;
}
uint8 format_path(char* path){
	int len = 0;
	while(path[len]!='\0'){
		if(path[len]=='/'&&path[len+1]=='/') return FS_InvalidPath;
		len++;
	}
	if(len==0) return FS_Success;
	if(path[0]=='/'){
		for(int s=0;s<len;s++){
			path[s] = path[s+1];
		}
		len--;
	}
	if(len>0&&path[len-1]=='/') path[len-1]='\0';
	return FS_Success;
}
uint8 FS_itoritive_parse_path(const char* path,Directory_Table* dir,uint16* depth,uint64* index){
	if(path[0]=='\0') return FS_Success;
	
	uint64 ptr = 0;
	int s=0;
	{
		char buffer[257];
		memset(buffer,0,257);
		
		while(1){
			if(path[s]=='/')  break;
			if(path[s]=='\0') break;
			if(s>256) return FS_NameToLong;
			buffer[s] = path[s];
			s++;
		}
		buffer[s] = '\0';
		
		find_in_dir(buffer,dir,0,0,&ptr);
		
		if(ptr==0){ return FS_NotFound; }
		*index = ptr;
		
		if(path[s]=='\0') return FS_Success;
	}
	
	*depth += 1;
	uint8 block_buffer[4096];
	read_block(ptr,block_buffer);
	if(((char*)block_buffer)[0] != 'D') return FS_NotDir;
	return FS_itoritive_parse_path(path+s+1,(Directory_Table*)block_buffer,depth,index);
}
uint8 FS_parse_path(const char* path,Directory_Table* dir,uint16* depth,uint64* index){
	if(index==0||path==0){
		printf("<FS_parse_path> Requires index/path ptr(s)");
		asm volatile("hlt");
		while(1);
	}
	char p[MAX_PATH_LENGTH];
	memcpy(p,path,strlen(path)+1);
	uint8 E = format_path(p);
	
	if(E) return E;
	if(dir==0){
		uint8 block_buffer[4096];
		read_block(convert_uint48(super->root_dir_ptr),block_buffer);
		*index = convert_uint48(super->root_dir_ptr);
		Directory_Table* dr = (Directory_Table*)block_buffer;
		if(depth==0){
			uint16 d;
			return FS_itoritive_parse_path(p,dr,&d,index);
		}else{
			*depth = 0;
			return FS_itoritive_parse_path(p,dr,depth,index);
	}}else{
		if(depth==0){
			uint16 d;
			return FS_itoritive_parse_path(p,dir,&d,index);
		}else{
			*depth = 0;
			return FS_itoritive_parse_path(p,dir,depth,index);
	}}
}

void cpy_dir_name_to_buf(const char* src,char* dest,int l){
	for(int s=0;s<l;s++){
		dest[s] = src[s];
	}
	dest[l] = '\0';
}
uint8 index_dir(Directory_Table* dir,uint32 index,char* name,uint64* ptr){
	const DirDescriptor* dde = DirDescriptor_entries;
	int i=0;
	for(int s=0;s<15;s++){
		uint8 split = dir->header.indexes[s].split_mask;
		uint8 free = dir->header.indexes[s].free_mask;
		
		for(int ss=0;ss<15;ss++){
			if((split&dde[ss].split_test_mask)==dde[ss].split_test_value){
				if(free&dde[ss].free_mask){
					if(index==i){
						cpy_dir_name_to_buf((char*)&dir->entries[s]+dde[ss].name_offset,name,dde[ss].entry_size);
						*ptr = get_inode_ptr(dir,s,&(dde[ss]));
						return FS_Success;
					}else{ i++; }
	}}}}
	
	*ptr = 0;
	return FS_Fail;
}

void itoritive_free_dir(Directory_Table* dir){
	if(dir->header.type != 'D') return;
	
	uint32 s=0;
	uint64 ptr=0;
	char null[257];
	while(!index_dir(dir,s,null,&ptr)){
		if(ptr==0) return;
		uint8 block_buffer[4096];
		read_block(ptr,block_buffer);
		Directory_Table* current = (Directory_Table*)block_buffer;
		itoritive_free_dir(current);
		mark_free(ptr);
		s++;
	}
	
}
uint8 remove_from_dir(const char* name,Directory_Table* dir){
	
	uint8 index_mask = 0;
	int index = 0;
	uint64 ptr=0;
	uint8 E = find_in_dir(name,dir,&index,&index_mask,&ptr);
	if(E) return E;
	
	uint8 block_buffer[4096];
	read_block(ptr,block_buffer);
	
	Directory_Table* current = (Directory_Table*)block_buffer;
	itoritive_free_dir(current);
	mark_free(ptr);
	
	dir->header.indexes[index].free_mask &= (~index_mask)&0xFE;
	return FS_Success;
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
				dir->header.indexes[s].split_mask = 1;
			}
		}
	}
	
}

uint8 FS_dir_create(const char* path,const char* name){
	uint64 path_end;
	uint8 E = FS_parse_path(path,0,0,&path_end);
	if(E) return E;
	
	uint8 block_buffer[4096];
	read_block(path_end,block_buffer);
	
	// after parsing path
	Directory_Table* dir = (Directory_Table*)block_buffer;
	
	uint64 index = get_free_block();
	if(index==0) return FS_NoSpace;
	
	Directory_Table new_dir;
	new_dir.header.type = 'D';
	
	// clear new directory
	for(int s=0;s<ARRAY_SIZE(new_dir.header.reserved);s++){ new_dir.header.reserved[s] = 0; }
	new_dir.header.nextBlock = make_uint48(0);
	for(int s=0;s<15;s++){
		new_dir.header.indexes[s].free_mask = 0;
		new_dir.header.indexes[s].split_mask = 1;
		new_dir.header.indexes[s].inode_ptr1 = make_uint48(0);
		new_dir.header.indexes[s].inode_ptr2 = make_uint48(0);
	}
	
	write_block(index,(uint8*)&new_dir);
	
	// add new directory to existing directory
	E = add_to_dir(name,dir,index);
	if(E) return E;
	
	write_block(path_end,block_buffer);
	
	mark_used(index);
	
	flush_to_disk();
	return FS_Success;
}
uint8 FS_dir_delete(const char* path,const char* name){
	uint64 path_end;
	uint8 E = FS_parse_path(path,0,0,&path_end);
	if(E) return E;
	
	uint8 block_buffer[4096];
	read_block(path_end,block_buffer);
	
	// after parsing path
	
	Directory_Table* dir = (Directory_Table*)block_buffer;
	
	E = remove_from_dir(name,dir);
	if(E) return E;
	
	desplit_dir(dir);
	
	write_block(path_end,block_buffer);
	
	flush_to_disk();
	return FS_Success;
}
uint8 FS_exists(const char* path){
	uint64 path_end;
	uint8 E = FS_parse_path(path,0,0,&path_end);
	return E;
}
uint8 FS_is_dir(const char* path){
	uint64 path_end;
	uint8 E = FS_parse_path(path,0,0,&path_end);
	if(E) return E;
	
	uint8 block_buffer[4096];
	read_block(path_end,block_buffer);
	if(((char*)block_buffer)[0] == 'D') return FS_Success;
	return FS_Fail;
}
uint8 FS_create(const char* path,const char* name){
	uint64 path_end;
	uint8 E = FS_parse_path(path,0,0,&path_end);
	if(E) return E;
	
	uint8 block_buffer[4096];
	read_block(path_end,block_buffer);
	
	Directory_Table* dir = (Directory_Table*)block_buffer;
	
	uint64 index = get_free_block();
	if(index==0) return FS_NoSpace;
	
	Inode file;
	file.header.type = 'F';
	file.header.flag_mask = 0x04;
	file.header.owner_id = 0;
	file.header.user_permissions = 0b01010101;
	for(int s=0;s<8;s++){
		file.header.group_permissions[s].group_id = 0;
		file.header.group_permissions[s].permissions = 0;
	}
	file.header.file_size = 0;
	uint64 time=0;
	E = rtc_get_seconds(&time);
	if(E) time=0;
	
	file.header.Created_Timestamp = make_uint40(time);
	file.header.Modified_Timestamp = make_uint40(time);
	file.header.Accessed_Timestamp = make_uint40(time);
	file.header.Moved_Timestamp = make_uint40(time);
	
	for(int s=0;s<ARRAY_SIZE(file.header.reserved);s++){ file.header.reserved[s] = 0; }
	for(int s=0;s<3072;s++){ file.inlined_file[s] = 0; }
	
	write_block(index,(uint8*)&file);
	
	E = add_to_dir(name,dir,index);
	if(E) return E;
	
	write_block(path_end,block_buffer);
	
	mark_used(index);
	
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
		printf("no file system found\n");
		printf("halting");
		asm volatile("hlt");
		while(1);
	}
	
	blockcount = super->total_blocks;
	
	buffer = (uint8*)alloc_pages(super->bitmap_block_count); // filesystem_bitmap
	for(int s=0;s<super->bitmap_block_count;s++){
		read_block(s+convert_uint48(super->bitmap_start_block),buffer+(s*4096));
	}
	filesystem_bitmap = buffer;
	
}



































/// Directory_Table_Header_Index -> split_mask -> bits
// 10000000 -> [256] 1-entry
// 10001000 -> {[128][128]} 2-entry
// 10001010 -> {[128]{[64][64]}} 3-entry
// 10001011 -> {[128]{[64]{[32][32]}}} 4-entry
// 10001111 -> {[128]{{[32][32]}{[32][32]}}} 5-entry
// 10101111 -> {{[64][64]}{{[32][32]}{[32][32]}}} 6-entry
// 10111111 -> {{[64]{[32][32]}}{{[32][32]}{[32][32]}}} 7-entry
// 11111111 -> {{{[32][32]}{[32][32]}}{{[32][32]}{[32][32]}}} 8-entry
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































