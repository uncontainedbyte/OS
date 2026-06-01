#pragma once

typedef unsigned char          uint8;
typedef unsigned short         uint16;
typedef unsigned int           uint32;
typedef unsigned long long     uint64;

typedef signed char            int8;
typedef signed short           int16;
typedef signed int             int32;
typedef signed long long       int64;

typedef char* va_list;
#define va_start(ap, parmn) (ap = (char*)&(parmn) + sizeof(parmn))
#define va_arg(ap, type) (*(type*)((ap += sizeof(type)) - sizeof(type)))
#define va_end(ap) (ap = 0)

#define UINT8_MAX   0xFFU
#define UINT16_MAX  0xFFFFU
#define UINT32_MAX  0xFFFFFFFFU
#define UINT64_MAX  0xFFFFFFFFFFFFFFFFULL

#define INT8_MIN    (-128)
#define INT16_MIN   (-32768)
#define INT32_MIN   (-2147483647 - 1)
#define INT64_MIN   (-9223372036854775807LL - 1)

#define INT8_MAX    127
#define INT16_MAX   32767
#define INT32_MAX   2147483647
#define INT64_MAX   9223372036854775807LL



static inline unsigned char inb(uint16 port){
	unsigned char result;
	asm("in %%dx, %%al" : "=a" (result) : "d" (port));
	return result;
}
static inline void outb(uint16 port, uint8 data){
	asm("out %%al, %%dx" : : "a" (data), "d" (port));
}
static inline unsigned short inw(uint16 port){
	unsigned short result;
	asm("in %%dx, %%ax" : "=a" (result) : "d" (port));
	return result;
}
static inline void outw(uint16 port, uint16 data){
	asm("out %%ax, %%dx" : : "a" (data), "d" (port));
}



static inline int32 stoi(const char* string){
	int32 out=0;
	uint8 i=0,neg=0;
	while(string[i] == ' '){
		i++;
	}
	if(string[i]=='-'){
		neg=1;
		i++;
	}
	while(string[i]>='0'&&'9'<=string[i]){
		out*=10;
		out+= '0'+string[i];
		i++;
	}
	if(neg){
		out=-out;
	}
	return out;
}
static inline uint32 stou(const char* string){
	uint32 out=0;
	uint8 i=0;
	while(string[i] == ' '){
		i++;
	}
	while(string[i]>='0'&&'9'<=string[i]){
		out*=10;
		out+= '0'+string[i];
		i++;
	}
	return out;
}
static inline uint32 xtou(const char* string){
	uint32 out=0;
	uint8 i=0;
	while(string[i] == ' '){
		i++;
	}
	while((string[i]>='0'&&'9'<=string[i])
			||(string[i]>='a'&&'f'<=string[i])
			||(string[i]>='A'&&'F'<=string[i])){
		out*=16;
		if(string[i]>='0'&&'9'<=string[i]){
			out+= '0'+string[i];
			i++;
		}else if(string[i]>='a'&&'f'<=string[i]){
			out+= 'a'+string[i];
			i++;
		}else{
			out+= 'A'+string[i];
			i++;
		}
	}
	return out;
}

static inline void itos(int32 num,char* buf){
	int8 i=0,neg=0;
	uint32 unum;
	if(num<0){
		neg=1;
		unum=-num;
	}else{ unum=num; }
	char tmp[20];
	do{
		tmp[i]=(unum%10)+'0';
		unum /= 10;
		i++;
	}while(unum>0);
	int8 s=0;
	if(neg) s=1;
	while(i>0){
		buf[s] = tmp[i-1];
		i--;s++;
	}
	buf[s] = 0;
	if(neg) buf[0] = '-';
}
static inline void utos(uint32 num,char* buf){
	int8 i=0;
	char tmp[20];
	do{
		tmp[i]=(num%10)+'0';
		num /= 10;
		i++;
	}while(num>0);
	int8 s=0;
	while(i>0){
		buf[s] = tmp[i-1];
		i--;s++;
	}
	buf[s] = 0;
}
static inline void utox(uint32 num, char* buf, int8 bytes) {
	int8 i = 0;
	char tmp[20];
	
	switch(bytes){
		case 1: num &= 0xFF;     break;
		case 2: num &= 0xFFFF;   break;
		case 3: num &= 0xFFFFFF; break;
		default:                 break; // 0 or 4 uses all 32 bits
	}
	
	do{
		tmp[i] = (num % 16) + '0';
		if(tmp[i] > '9'){
			tmp[i] += 7;
		}
		num /= 16;
		i++;
	}while(num > 0);
	
	int8 target_len = (bytes > 0) ? (bytes * 2) : i;
	
	while(i < target_len){
		tmp[i] = '0';
		i++;
	}
	
	int8 s = 0;
	while(i > 0){
		buf[s] = tmp[i - 1];
		i--; 
		s++;
	}
	buf[s] = 0;
}
static inline void utob(uint32 num, char* buf, int8 bytes) {
	switch(bytes){
		case 1: num &= 0xFF;     break;
		case 2: num &= 0xFFFF;   break;
		case 3: num &= 0xFFFFFF; break;
		default:                 break; // 0 or 4 uses all 32 bits
	}
	
	int8 i = 0;
	char tmp[33];
	
	do{
		tmp[i] = (num % 2) + '0';
		num /= 2;
		i++;
	}while (num > 0);
	
	int8 target_len = (bytes > 0) ? (bytes * 8) : i;
	
	while(i < target_len){
		tmp[i] = '0';
		i++;
	}
	
	int8 s = 0;
	while(i > 0){
		buf[s] = tmp[i - 1];
		i--; 
		s++;
	}
	buf[s] = 0;
}
static inline void utobo(uint32 num,char* buf){
	int8 i=0;
	char tmp[33];
	for(int s=0;s<32;s++){buf[s]='0';};
	while(num>0){
		tmp[i]=(num%2)+'0';
		num /= 2;
		i++;
	}
	int8 s=32-i;
	while(i>0){
		buf[s] = tmp[i-1];
		i--;s++;
	}
	buf[s] = 0;
}

static inline int string_length(char s[]) {
	int i = 0;
	while (s[i] != '\0') ++i;
	return i;
}























