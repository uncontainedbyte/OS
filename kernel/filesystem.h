#pragma once

#include "int.h"
#include "memory.h"
#include "pci.h"




enum FS_Result : uint8{
	FS_Success       = 0,
	FS_GenericError  = 1,
	FS_AlreadyExists = 2,  // file/directory already exists
	FS_NotFound      = 3,  // no file/directory found
	FS_AccessDenied  = 4,  // permissions check fails
	FS_IsADirectory  = 5,  // tried to open a folder as a file
	FS_NoSpace       = 6,  // drive is full
	FS_InvalidHandle = 7,  // invalid handle_id used to read/write
	FS_OverRead      = 8   // tried to read more bytes then avalable
};
static inline const char* FS_Result_str(uint8 r){
	switch(r){
		case 0:  return "FS_Success";
		case 1:  return "FS_GenericError";
		case 2:  return "FS_AlreadyExists";
		case 3:  return "FS_NotFound";
		case 4:  return "FS_AccessDenied";
		case 5:  return "FS_IsADirectory";
		case 6:  return "FS_NoSpace";
		case 7:  return "FS_InvalidHandle";
		case 8:  return "FS_OverRead";
		default: return "FS_Unkown";
	};
}



// TODO
uint8 FS_exists(const char* path);
int32 FS_open(const char* path, uint8 mode);
uint8 FS_read(int32 handle_id,void* buffer,uint64 bytes_to_read);
uint8 FS_write(int32 handle_id, const void* buffer, uint64 bytes_to_write);
uint8 FS_seek(int32 handle_id, int32 pos,uint8 mode);//modes: 0=set,1=add
uint8 FS_close(int32 handle_id);
uint8 FS_create(const char* dir,const char* name);
uint8 FS_delete(const char* path);
uint8 FS_find(const char* name); // searches entire filesystem for name/directory
uint8 FS_dir_count(const char* path,uint64* value); // counts files/directorys in directory
uint8 FS_dir_list(const char* path,uint32 index,char* buffer); // returns 256 byte name
uint8 FS_dir_find(const char* path, const char* name); // searches in specific directory

// HALF-DONE (function, but has edge cases)
uint8 FS_dir_create(const char* dir,const char* name);
uint8 FS_dir_delete(const char* path,const char* name);

// DONE

// CMD's Added
mkdir path name
rmdir path name


void FILESYSTEM_init();



































