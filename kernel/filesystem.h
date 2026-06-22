#pragma once

#include "int.h"
#include "memory.h"
#include "pci.h"
#include "rtc.h"
#include "util/linked-list.h"






#define MAX_PATH_LENGTH 4096

enum FS_Result : uint8{
	FS_Success       = 0,
	FS_Fail          = 1,
	FS_AlreadyExists = 2,  // file/directory already exists
	FS_NotFound      = 3,  // no file/directory found
	FS_AccessDenied  = 4,  // permissions check fails
	FS_IsADirectory  = 5,  // tried to open a folder as a file
	FS_NoSpace       = 6,  // drive is full
	FS_InvalidHandle = 7,  // invalid handle_id used to read/write
	FS_OverRead      = 8,  // tried to read more then avalable
	FS_NameToLong    = 9,
	FS_InvalidPath   = 10,
	FS_NotDir        = 11,
	FS_MemError      = 12,
};
static inline const char* FS_Result_str(uint8 r){
	switch(r){
		case 0:  return "FS_Success";
		case 1:  return "FS_Fail";
		case 2:  return "FS_AlreadyExists";
		case 3:  return "FS_NotFound";
		case 4:  return "FS_AccessDenied";
		case 5:  return "FS_IsADirectory";
		case 6:  return "FS_NoSpace";
		case 7:  return "FS_InvalidHandle";
		case 8:  return "FS_OverRead";
		case 9:  return "FS_NameToLong";
		case 10: return "FS_InvalidPath";
		case 11: return "FS_NotDir";
		case 12: return "FS_MemError";
		default: return "FS_Unknown";
	};
}

enum FILE_Flags : uint8{
	F_Read   = 0b00000001,
	F_Write  = 0b00000010,
};

/// TODO

/// HALF-DONE (some-what functional)

/// DONE
uint8 FS_dir_create(const char* dir,const char* name);
uint8 FS_delete(const char* path,const char* name);
uint8 FS_exists(const char* path);
uint8 FS_is_dir(const char* path);
uint8 FS_create(const char* path,const char* name);
uint8 FS_open(const char* path, int32* handle_id, uint8 mode);
uint8 FS_read(int32 handle_id,void* buffer,uint64 bytes_to_read);
uint8 FS_write(int32 handle_id, const void* buffer, uint64 bytes_to_write);
uint8 FS_seek(int32 handle_id, int32 pos,uint8 mode);//modes: 0=set,1=add
uint8 FS_tell(int32 handle_id, int32* out);
uint8 FS_close(int32 handle_id);
uint8 FS_file_size(int32 handle_id,uint32* out);
uint8 FS_dir_get(const char* path,uint32 index,char* name); // returns 256 byte name + \0

/// CMD's
// mkdir  path name
// rm     path name      ; deletes a file/directory (including children)
// isdir  path
// mkfile path name      ; creates a file
// time                  ; returns seconds since 2000
// time-fancy
// openfile    name
// read N
// write msg
// closefile
// ls path

void FILESYSTEM_init();



































