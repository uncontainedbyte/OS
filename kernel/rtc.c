#include "rtc.h"


static uint8 bcd_to_bin(uint8 value){
	return (value & 0x0F) + ((value >> 4) * 10);
}
uint8 rtc_updating(){
	outb(0x70,0x0A);
	return inb(0x71)&0x80;
}
uint8 rtc_read(uint8 reg){
	outb(0x70,reg | 0x80);
	return inb(0x71);
}
uint8 rtc_read_time(RTC_Time* time){
	if(time==0){
		return 1;
	}
	
	while(rtc_updating());
	
	uint8 second = rtc_read(0x00);
	uint8 minute = rtc_read(0x02);
	uint8 hour   = rtc_read(0x04);
	
	uint8 day    = rtc_read(0x07);
	uint8 month  = rtc_read(0x08);
	uint8 year   = rtc_read(0x09);
	uint8 century= rtc_read(0x32);
	
	uint8 regB   = rtc_read(0x0B);
	
	uint8 is_pm = 0;
	if(!(regB & 0x02)){
		is_pm = hour & 0x80;
		hour &= 0x7F;
	}
	
	if(!(regB & 0x04)){
		second = bcd_to_bin(second);
		minute = bcd_to_bin(minute);
		hour   = bcd_to_bin(hour);
		
		day    = bcd_to_bin(day);
		month  = bcd_to_bin(month);
		year   = bcd_to_bin(year);
		century= bcd_to_bin(century);
	}
	
	if(!(regB & 0x02)){
		if(is_pm){
			if(hour != 12){
				hour += 12;
			}
		}else{
			if(hour == 12){
				hour = 0;
			}
		}
	}
	
	time->second = second;
	time->minute = minute;
	time->hour   = hour;
	
	time->day    = day;
	time->month  = month;
	
	if(century == 0){ century = 20; }
	time->year   = (century * 100) + year;
	
	return 0;
}
uint8 is_leap_year(uint16 year){
	if(year % 400 == 0) return 1;
	if(year % 100 == 0) return 0;
	if(year % 4   == 0) return 1;
	return 0;
}
uint64 rtc_time_to_seconds(const RTC_Time* time){
	static const uint8 days_per_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	
	uint64 days = 0;
	
	for(uint16 year=2000;year<time->year;year++){
		days += is_leap_year(year)?366:365;
	}
	
	for(uint8 month=1;month<time->month;month++){
		days += days_per_month[month-1];
		if(month==2 && is_leap_year(time->year)){
			days++;
		}
	}
	days += time->day-1;
	return
		days * 86400ULL +
		time->hour * 3600ULL +
		time->minute * 60ULL +
		time->second;
}
uint8 rtc_get_seconds(uint64* seconds){
	if(seconds==0) return 1;
	
	RTC_Time time;
	if(rtc_read_time(&time)) return 1;
	*seconds = rtc_time_to_seconds(&time);
	
	return 0;
}















































