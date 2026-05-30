#include "display.h"


int hex_to_int(char c) {
	if(c >= '0' && c <= '9'){ return c - '0';
	}else if(c >= 'a' && c <= 'f'){ return c - 'a' + 10;
	}else if(c >= 'A' && c <= 'F'){ return c - 'A' + 10; }
	return -1;
}
int is_hex_digit(char c) {
	if(c >= '0' && c <= '9'){ return 1; }
	if(c >= 'a' && c <= 'f'){ return 1; }
	if(c >= 'A' && c <= 'F'){ return 1; }
	return 0;
}

void clear(){
	for(uint16 i = 0; i < MAX_COLS * MAX_ROWS; ++i){
		uint8 *vidmem = (uint8 *)VIDEO_ADDRESS;
		vidmem[i*2] = ' ';
		vidmem[i*2 + 1] = DEFAULT_COLOR;
	}
	set_cursor(0);
}
void set_cursor(int offset){
	if(offset!=0) offset /= 2;
	outb(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
	outb(VGA_DATA_REGISTER, (unsigned char) (offset >> 8));
	outb(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
	outb(VGA_DATA_REGISTER, (unsigned char) (offset & 0xff));
}
int get_cursor(){
	outb(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
	int offset = inb(VGA_DATA_REGISTER) << 8;
	outb(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
	offset += inb(VGA_DATA_REGISTER);
	return offset * 2;
}



void setChar(char c, int offset, int color){
	if(offset%2!=0){return;}
	uint8 *vidmem = (uint8 *)VIDEO_ADDRESS;
	vidmem[offset] = c;
	vidmem[offset + 1] = (color<0)? DEFAULT_COLOR:color;
}
void shiftUp(){
	uint8* vidmem = (uint8*)VIDEO_ADDRESS;
	for(int s=0;s<MAX_ROWS*MAX_COLS*2-MAX_COLS*2;s++){
		vidmem[s]=vidmem[s+160];
	}
	for(int s=0;s<MAX_COLS;s++){
		vidmem[s*2+MAX_ROWS*MAX_COLS*2-MAX_COLS*2]=' ';
		vidmem[s*2+1+MAX_ROWS*MAX_COLS*2-MAX_COLS*2]=DEFAULT_COLOR;
	}
}

void printChar(char ch,int color){
	int offset = get_cursor();
	
	if(offset >= MAX_ROWS * MAX_COLS * 2){
		offset = offset - 2 * MAX_COLS;
		shiftUp();
	}
	if(ch == '\n'){
		offset += MAX_COLS * 2;
		offset -= offset%(2*MAX_COLS);
	}else if(ch == '\b'){
		if(offset>=2){
			offset -= 2;
			setChar(' ', offset,color);
		}
	}else{
		setChar(ch, offset,color);
		offset += 2;
	}
	set_cursor(offset);
}
void printStr(char* str,int color){
	int index=0;
	while(str[index]!='\0'){
		printChar(str[index],color);
		index++;
	}
}


void printf(const char* _format,...){
	va_list args;
	va_start(args,_format);
	uint16 index=0;
	int color = DEFAULT_COLOR;
	
	//color: 0=black 1=blue 2=green 3=cyan 4=red 5=magenta 6=brown 7=L-grey 8=D-grey
	//       9=L-blue A=L-green B=L-cyan C=L-red D=L-magenta E=L-brown F=white
	
	while(_format[index]!=0){
		if(_format[index]=='%'){
			index++;
			switch(_format[index]){
				case 'c':{
					printChar((char)va_arg(args,int),color);
				}break;
				case 's':{
					printStr((char*)va_arg(args,char*),color);
				}break;
				case 'd':
				case 'i':{
					int32 num = va_arg(args,int32);
					char buf[20];
					itos(num,buf);
					printStr(buf,color);
				}break;
				case 'u':{
					uint32 num = va_arg(args,uint32);
					char buf[20];
					utos(num,buf);
					printStr(buf,color);
				}break;
				case 'x':{
					uint32 num = va_arg(args,uint32);
					int8 bytes=0;
					if(_format[index+1]>='0'&&'9'>=_format[index+1]){
						index++;
						bytes = _format[index]-'0';
					}
					char buf[10];
					utox(num,buf,bytes);
					printStr(buf,color);
				}break;
				case 'b':{
					uint32 num = va_arg(args,uint32);
					int8 bytes=0;
					if(_format[index+1]>='0'&&'9'>=_format[index+1]){
						index++;
						bytes = _format[index]-'0';
					}
					char buf[33];
					utob(num,buf,bytes);
					printStr(buf,color);
				}break;
				case 'p':{
					uint32 num = va_arg(args,uint32);
					char buf[10];
					utox(num,buf,4);
					printChar('0',color);
					printChar('x',color);
					printStr(buf,color);
				}break;
				case '#':{
					index++;
					if(_format[index]=='$'){
						color = DEFAULT_COLOR;
					}else if(is_hex_digit(_format[index])){
						char txtColor = hex_to_int(_format[index++]);
						if(is_hex_digit(_format[index])){
							char bgColor = hex_to_int(_format[index++]);
							color = ((bgColor<<4)&0xF0) | (txtColor&0xF);
						}else{
							color = (DEFAULT_COLOR&0xF0) | (txtColor&0xF);
						}
					}
					continue;
				}break;
				case '%':{
					printChar('%',color);
				}break;
				default:
					printChar('%',color);
					continue;
			}
		}else{
			printChar(_format[index],color);
		}
		index++;
	}
	
	va_end(args);
}










