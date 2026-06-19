#pragma once

#include "int.h"



typedef struct{
	uint8 second;
	uint8 minute;
	uint8 hour;
	uint8 day;
	uint8 month;
	uint16 year;
} RTC_Time;

uint8  rtc_read_time(RTC_Time* out);
uint8  rtc_get_seconds(uint64* out);
uint64 rtc_time_to_seconds(const RTC_Time* out);
uint8  is_leap_year(const uint16 year);




