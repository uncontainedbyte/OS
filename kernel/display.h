#pragma once
#include "int.h"

#define VGA_CTRL_REGISTER 0x3d4
#define VGA_DATA_REGISTER 0x3d5
#define VGA_OFFSET_LOW 0x0f
#define VGA_OFFSET_HIGH 0x0e

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

#define DEFAULT_COLOR 0x02

//color: 0=black 1=blue 2=green 3=cyan 4=red 5=magenta 6=brown 7=L-grey 8=D-grey
//       9=L-blue A=L-green B=L-cyan C=L-red D=L-magenta E=L-brown F=white

void clear();
void set_cursor(int offset);
int get_cursor();


void setChar(char c, int offset, int color);
void shiftUp();

void printChar(char ch,int color);
void printStr(char* str,int color);

void printf(const char* _format,...);








































