#pragma once

#include "int.h"
#include "memory.h"
#include "pci.h"




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
	Directory_Table_Header_Index indexes[15];
	uint48 nextBlock;
	uint8 reserved[40];
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
	uint8 flag_mask;
	uint16 owner_id;
	uint8 user_permissions;
	Group_Permissions group_permissions[8];
	uint64 file_size;
	uint40 Created_Timestamp;//  the
	uint40 Modified_Timestamp;// ED
	uint40 Accessed_Timestamp;// group
	uint40 Moved_Timestamp;//    _
	uint8 reserved[968];
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


uint64 convert_uint40(uint40 value);
uint64 convert_uint48(uint48 value);

void FILESYSTEM_init();



































