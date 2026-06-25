#include "display.h"
#include "interrupts.h"
#include "keyboard.h"
#include "PIT.h"
#include "memory.h"
#include "pci.h"
#include "filesystem.h"
#include "processes.h"
#include "PMM.h"
#include "VMM.h"

extern uint32 kernel_start;

void ____testPrint(){
	printf("Char:     %c\n",'c');
	printf("String:   %s\n","this is a string");
	printf("Max int:  %i\n",INT32_MAX);
	printf("Min int:  %i\n",INT32_MIN);
	printf("Max uint: %u\n",UINT32_MAX);
	printf("Hex:      %x %x\n",0x01234567,0x89ABCDEF);
	printf("Binary:   %b\n",0b10001000100010001000100010001000);
	printf("Pointer:  %p\n",VIDEO_ADDRESS);
	printf("ch Color: %#1#%#2#%#3#%#4#%#5#%#6#%#7#%#8#%#9#%#a#%#b#%#c#%#d#%#e#%#f#\n");
	printf("bg color: %#01#%#02#%#03#%#04#%#05#%#06#%#07#%#08#%#09#%#0a#%#0b#%#0c#%#0d#%#0e#%#0f#\n");
}


uint8 _exe_heapdumb(){
	heap_dump();
	return 0;
}

uint8 _exe_kill(char* arg0){
	uint32 id = stoi(arg0);
	
	printf("Killing: %s\n",arg0);
	
	kill_task(id);
	
	return 0;
}
uint8 _exe_ls(char* arg0){
	char type = FS_is_dir(arg0);
	
	if(type==1||type==FS_NotDir){
		printf("Not A Directory\n");
		return FS_Fail;
	}else if(type==FS_NotFound){
		printf("Invalid Path\n");
		return FS_NotFound;
	}
	
	int count=0;
	char tmp[257];
	char buf[4096];
	while(FS_dir_get(arg0,count,tmp)==FS_Success){
		strmerge(buf,arg0,tmp);
		type = FS_is_dir(buf);
		if(type==1){ printf("%#30%s\n",tmp);
		}else{ printf("%#50%s\n",tmp); }
		count++;
	}
	return 0;
}

int32 handle=0;
uint8 _exe_openfile(char* arg0){
	uint8 r = FS_open(arg0,&handle,3);
	if(r){
		printf("Failed to Load File\n");
		printf("Error: %s\n",FS_Result_str(r));
		return 1;
	}
	printf("File Opened Successfully\n");
}
uint8 _exe_closefile(){
	uint8 r = FS_close(handle);
	if(r){
		printf("Failed to Close File\n");
		printf("Error: %s\n",FS_Result_str(r));
		return 1;
	}
	printf("File Closed Successfully\n");
}
uint8 _exe_read(char* arg0){
	int n = stoi(arg0);
	if(n==-1){
		FS_file_size(handle,&n);
	}
	char* buf = kalloc(n+1);
	if(!buf){
		printf("Not Enough Memory\n");
		return 1;
	}
	uint8 r = FS_read(handle,buf,n);
	if(r){
		printf("Failed to Read File\n");
		printf("Error: %s\n",FS_Result_str(r));
		kfree(buf);
		return 1;
	}
	buf[n] = '\0';
	printf("Reading %i Bytes\n%s\n",n,buf);
	kfree(buf);
	return 0;
}
uint8 _exe_write(char* arg0){
	int n = strlen(arg0);
	uint8 r = FS_write(handle,arg0,n);
	if(r){
		printf("Failed to Write File\n");
		printf("Error: %s\n",FS_Result_str(r));
		return 1;
	}
	printf("Wrote %i Bytes\n",n);
	return 0;
}

uint8 _exe_mkdir(char* arg0,char* arg1){
	uint8 r = FS_dir_create(arg0,arg1);
	if(r){
		printf("Failed to make Directory\n");
		printf("Error: %s\n",FS_Result_str(r));
		return 1;
	}
	
	printf("Created \"%s\" in directory %s\n",arg1,arg0);
	return 0;
}
uint8 _exe_rm(char* arg0,char* arg1){
	uint8 r = FS_delete(arg0,arg1);
	if(r){
		printf("Failed to delete \"%s\"\n",arg1);
		printf("Error: %s\n",FS_Result_str(r));
		return 1;
	}
	
	printf("Deleted \"%s\" at %s\n",arg1,arg0);
	return 0;
}
uint8 _exe_isdir(char* arg0){
	uint8 r = FS_is_dir(arg0);
	if(r>1){
		printf("Error: %s\n",FS_Result_str(r));
		return 1;
	}
	
	if(r==0) printf("Yes\n");
	if(r==1) printf("No\n");
	return 0;
}
uint8 _exe_mkfile(char* arg0,char* arg1){
	uint8 r = FS_create(arg0,arg1);
	if(r){
		printf("Failed to make File\n");
		printf("Error: %s\n",FS_Result_str(r));
		return 1;
	}
	
	printf("Created \"%s\" in directory %s\n",arg1,arg0);
	return 0;
}

void parse_args(char* buf,char* cmd,char* arg0,char* arg1, char* arg2,char* arg3,char* arg4){
	int i=0,s=0;
	while(buf[i]){
		if(buf[i]==' '){
			if(cmd[0]==0){
				i++;
				continue;
			}
			i++;
			break;
		}
		cmd[s]=buf[i];
		s++;i++;
		cmd[s]=0;
	}
	s=0;
	while(buf[i]){
		if(buf[i]==' '){
			if(arg0[0]==0){
				i++;
				continue;
			}
			i++;
			break;
		}
		arg0[s]=buf[i];
		s++;i++;
		arg0[s]=0;
	}
	s=0;
	while(buf[i]){
		if(buf[i]==' '){
			if(arg1[0]==0){
				i++;
				continue;
			}
			i++;
			break;
		}
		arg1[s]=buf[i];
		s++;i++;
		arg1[s]=0;
	}
	s=0;
	while(buf[i]){
		if(buf[i]==' '){
			if(arg2[0]==0){
				i++;
				continue;
			}
			i++;
			break;
		}
		arg2[s]=buf[i];
		s++;i++;
		arg2[s]=0;
	}
	s=0;
	while(buf[i]){
		if(buf[i]==' '){
			if(arg3[0]==0){
				i++;
				continue;
			}
			i++;
			break;
		}
		arg3[s]=buf[i];
		s++;i++;
		arg3[s]=0;
	}
	s=0;
	while(buf[i]){
		if(buf[i]==' '){
			if(arg4[0]==0){
				i++;
				continue;
			}
			i++;
			break;
		}
		arg4[s]=buf[i];
		s++;i++;
		arg4[s]=0;
	}
	
}
void execute(char* buf){
	if(buf[0]==0) return;
	char cmd[64],arg0[64],arg1[64],arg2[64],arg3[64],arg4[64];
	cmd[0]=0;arg0[0]=0;arg1[0]=0;arg2[0]=0;arg3[0]=0;arg4[0]=0;
	parse_args(buf,cmd,arg0,arg1,arg2,arg3,arg4);
	
	//printf("cmd: %s\n",cmd);
	//if(arg0[0]) printf("arg0: %s\n",arg0);
	//if(arg1[0]) printf("arg1: %s\n",arg1);
	//if(arg2[0]) printf("arg2: %s\n",arg2);
	//if(arg3[0]) printf("arg3: %s\n",arg3);
	//if(arg4[0]) printf("arg4: %s\n",arg4);
	
	uint8 e=0;
	if(cmpstr(cmd,"mkdir")){
		if(arg0==0||arg1==0){ printf("To Few Args\n"); return; }
		e = _exe_mkdir(arg0,arg1);
	}else if(cmpstr(cmd,"rm")){
		if(arg0==0||arg1==0){ printf("To Few Args\n"); return; }
		e = _exe_rm(arg0,arg1);
	}else if(cmpstr(cmd,"isdir")){
		if(arg0==0){ printf("To Few Args\n"); return; }
		e = _exe_isdir(arg0);
	}else if(cmpstr(cmd,"mkfile")){
		if(arg0==0||arg1==0){ printf("To Few Args\n"); return; }
		e = _exe_mkfile(arg0,arg1);
	}else if(cmpstr(cmd,"time")){
		uint64 t=0;
		rtc_get_seconds(&t);
		printf("time: %u\n",(uint32)t);
	}else if(cmpstr(cmd,"time-fancy")){
		RTC_Time r;
		rtc_read_time(&r);
		printf("RTC: %u-%u-%u %u:%u:%u\n",r.year,r.month,r.day,r.hour,r.minute,r.second);
	}else if(cmpstr(cmd,"openfile")){
		if(arg0==0){ printf("To Few Args\n"); return; }
		e = _exe_openfile(arg0);
	}else if(cmpstr(cmd,"closefile")){
		e = _exe_closefile();
	}else if(cmpstr(cmd,"read")){
		if(arg0==0){ printf("To Few Args\n"); return; }
		e = _exe_read(arg0);
	}else if(cmpstr(cmd,"write")){
		if(arg0==0){ printf("To Few Args\n"); return; }
		e = _exe_write(arg0);
	}else if(cmpstr(cmd,"ls")){
		if(arg0==0){ printf("To Few Args\n"); return; }
		e = _exe_ls(arg0);
	}else if(cmpstr(cmd,"kill")){
		if(arg0==0){ printf("To Few Args\n"); return; }
		e = _exe_kill(arg0);
	}else if(cmpstr(cmd,"heapdump")){
		e = _exe_heapdumb();
	}else if(cmpstr(cmd,"memmap")){
		MEM_print_memory_map();
		PMM_print_stats();
		e = 0;
	}
	if(e) printf("EXit-Code: %u\n",e);
}



void idle_task(){
	for(;;)
		asm volatile("hlt");
}
void kmain(){
	uint32 kernel_end = *((uint32*)0x7FA);
	kernel_end += ((uint32)(*((uint16*)0x7F8)))*512;
	
	clear();
	
	idt_init();
	enable_interrupts();
	install_Basic_Interrupts();
	keyboard_init();
	PIT_init();
	PMM_init(kernel_end);
	VMM_init();
	MEM_init();
	printf("good?\n");
	
	sata_init();
	
	//printf("Kernel-Start: %x4\n",*((uint32*)0x7FA));
	//printf("Kernel-End:   %x4\n",kernel_end);
	
	FILESYSTEM_init();
	
	Scheduler_init();
	start_task((uint32)idle_task);
	
	
	
	
	char cmd_buf[64];
	int i=0;
	printf(">> ");
	while(1){
		uint32 key = keyboard_read();
		char c = KEY_ASCII(key);
		if(c){
			if(c=='\n'){
				printf("\n");
				execute(cmd_buf);
				cmd_buf[0]=0;
				i=0;
				printf(">> ");
			}else if(c=='\b'){
				if(i==0) continue;
				printf("\b");
				cmd_buf[i]=0;
				i--;
			}else{
				if(i>63) continue;
				cmd_buf[i] = c;
				i++;
				cmd_buf[i]=0;
				printf("%c",c);
			}
		}
	}
	while(1);
}







































