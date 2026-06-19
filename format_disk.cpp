// # 2 cpp START
//args = "format_disk.cpp"
//mk-exe args "format_disk"
//END
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>



typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t  uint8;

typedef struct{
	uint8 bytes[6];
} __attribute__((packed)) uint48;
typedef struct{
	uint8 bytes[5];
} __attribute__((packed)) uint40;

typedef struct{
	uint8 free_mask;
	uint8 split_mask;
	uint48 inode_ptr1; // 256B ptr || 128B first ptr
	uint48 inode_ptr2; // 128B second ptr
}__attribute__((packed)) Directory_Table_Header_Index; //10B
typedef struct{
	uint8 type;
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


uint48 make_uint48(uint64_t val){
	uint48 out;
	out.bytes[0] = val & 0xFF;
	out.bytes[1] = (val >> 8) & 0xFF;
	out.bytes[2] = (val >> 16) & 0xFF;
	out.bytes[3] = (val >> 24) & 0xFF;
	out.bytes[4] = (val >> 32) & 0xFF;
	out.bytes[5] = (val >> 40) & 0xFF;
	return out;
}
uint40 make_uint40(uint64_t val){
	uint40 out;
	out.bytes[0] = val & 0xFF;
	out.bytes[1] = (val >> 8) & 0xFF;
	out.bytes[2] = (val >> 16) & 0xFF;
	out.bytes[3] = (val >> 24) & 0xFF;
	out.bytes[4] = (val >> 32) & 0xFF;
	return out;
}
uint64 convert_uint48(uint48 value){
	return  ((uint64)value.bytes[0])        |
			((uint64)value.bytes[1] << 8)   |
			((uint64)value.bytes[2] << 16)  |
			((uint64)value.bytes[3] << 24)  |
			((uint64)value.bytes[4] << 32)  |
			((uint64)value.bytes[5] << 40);
}
uint64 convert_uint40(uint40 value){
	return  ((uint64)value.bytes[0])        |
			((uint64)value.bytes[1] << 8)   |
			((uint64)value.bytes[2] << 16)  |
			((uint64)value.bytes[3] << 24)  |
			((uint64)value.bytes[4] << 32);
}



typedef struct{
	uint8 bytes[4096];
} __attribute__((packed)) Block; //4096B(4KB)

void consume_to(std::vector<Block>& bitmap,uint64 index){
	for(uint64 s=0;s<bitmap.size();s++){
		for(uint64 ss=0;ss<4096;ss++){
			for(uint64 sss=0;sss<8;sss++){
				if((s*32768+ss*8+sss) > index) return;
				bitmap[s].bytes[ss] |= 1<<sss;
			}
		}
	}
}
int main(int argc, char* argv[]){
	
	if(argc<2){
		std::cout<<"Error: no Path passed"<<std::endl;
		return 1;
	}
	
	std::string path = argv[1];
	uint8 index = 1,SuperIndex=1;
	
	if(argc<3){
		std::cout<<"Warning: no Block specified"<<std::endl;
		std::cout<<"Defaulting to Block 1"<<std::endl;
	}else{
		index = std::stoi(argv[2]);
		SuperIndex = index;
	}
	
	std::fstream disk(path,std::ios::in | std::ios::out | std::ios::binary);
	
	if(!disk.is_open()){
		std::cout<<"Error: Disk not found <"<<path<<">"<<std::endl;
		return 1;
	}
	
	std::cout<<"<"<<path<<">"<<std::endl;
	std::cout<<"Format This Disk? <y/n> ";
	
	char answer=0;
	std::cin>>answer;
	
	if(!(answer=='y'||answer=='Y')){
		std::cout<<"Canceling Formating"<<std::endl;
		return 0;
	}
	std::cout<<"Starting Formating..."<<std::endl;
	
	std::cout<<"Generating SuperBlock..."<<std::endl;
	Super_Block super;
	super.magic[0]='U';
	super.magic[1]='B';
	super.magic[2]='y';
	super.magic[3]='t';
	super.magic[4]='e';
	super.magic[5]='_';
	super.magic[6]=':';
	super.magic[7]='D';
	
	for(int s=0;s<4049;s++){
		super.reserved[s] = 0;
	}
	
	disk.seekg(0, std::ios::end);
	uint64 diskSize = disk.tellg();
	uint64 blockCount = diskSize/4096;
	if(blockCount<255){
		std::cout<<"Error: Disk Size to Small"<<std::endl;
		std::cout<<"--1MB disk or larger required"<<std::endl;
		std::cout<<std::endl;
		std::cout<<"Canceling Formating..."<<std::endl;
		return 1;
	}
	
	super.block_size = 4096;
	super.filesystem_state = 0;
	super.total_blocks = blockCount;
	
	std::cout<<"Generating Memory Bitmap..."<<std::endl;
	std::vector<Block> bitmap((((blockCount+7)/8)+4095)/4096,Block());
	for(uint s=0;s<bitmap.size();s++){
		for(uint ss=0;ss<4096;ss++){
			bitmap[s].bytes[ss] = 0;
		}
	}
	
	super.bitmap_block_count = bitmap.size();
	super.bitmap_start_block = make_uint48(index+1);
	index += bitmap.size();
	
	super.last_mounted_time = make_uint40(0);
	super.last_checked_time = make_uint40(0);
	
	super.journal_inode_ptr = make_uint48(0);
	
	super.root_dir_ptr = make_uint48(index+1);
	
	std::cout<<"Generating Root Directory..."<<std::endl;
	Directory_Table root;
	root.header.type = 'D';
	index+=1;
	
	for(int s=0;s<39;s++){
		root.header.reserved[s] = 0;
	}
	
	root.header.nextBlock = make_uint48(0);
	
	for(int s=0;s<15;s++){
		root.header.indexes[s].free_mask = 0;
		root.header.indexes[s].split_mask = 1;
		root.header.indexes[s].inode_ptr1 = make_uint48(0);
		root.header.indexes[s].inode_ptr2 = make_uint48(0);
	}
	
	std::cout<<"Used Blocks: "<<index+1<<std::endl;
	std::cout<<"Free Blocks: "<<blockCount - (index+1)<<std::endl;
	
	consume_to(bitmap,index);
	
	std::cout<<"Writing to Disk..."<<std::endl;
	
	disk.seekp(SuperIndex*4096);
	disk.write((char*)&super,4096);
	
	for(uint s=0;s<super.bitmap_block_count;s++){
		disk.seekp((convert_uint48(super.bitmap_start_block)+s)*4096);
		disk.write((char*)&(bitmap[s]),4096);
	}
	
	disk.seekp(convert_uint48(super.root_dir_ptr)*4096);
	disk.write((char*)&root,4096);
	
	disk.close();
	std::cout<<"Formating Complete..."<<std::endl;
}





































