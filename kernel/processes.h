#pragma once

#include "int.h"
#include "interrupts.h"
#include "PIT.h"
#include "memory.h"
#include "util/linked-list.h"





void Scheduler_init();
uint32 start_task(uint32 task_start);
void kill_task(uint32 pid);
void sleep_task(uint32 ms);



typedef uint8 Mutex;
void mutex_init(Mutex* mtx);
void mutex_lock(Mutex* mtx);
void mutex_unlock(Mutex* mtx);









































