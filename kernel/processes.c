#include "processes.h"





enum{
	TASK_RUNNING,
	TASK_READY,
	TASK_SLEEPING,
	TASK_DEAD
};

typedef struct{
	LinkedList_Node linkedlist_tasks;
	LinkedList_Node linkedlist_sleeping;
	
	uint32 pid;
	uint32 esp;
	
	uint32 stack_base;
	uint32 stack_pages;
	
	uint32 wake_tick;
	
	uint8 state;
} Task;



uint32 next_pid;
LinkedList tasks;
LinkedList sleeping;
Task* current_task;
Task kernel_task;


uint32 Scheduler_tick(registers_t* r){
	if(tasks.count<1)
		return (uint32)r;
	
	if(!current_task)
		return (uint32)r;
	
	current_task->esp = (uint32)r;
	
	if(current_task->state == TASK_RUNNING)
		current_task->state = TASK_READY;
	
	uint32 now = get_ticks();
	{
		LinkedList_Node* nd = sleeping.head;
		while(nd){
			LinkedList_Node* next = nd->next;
			
			Task* tmp = container_of(nd,Task,linkedlist_sleeping);
			
			if(tmp->state == TASK_SLEEPING && (int32)(now - tmp->wake_tick) >= 0){
				tmp->state = TASK_READY;
				LIST_remove(&sleeping,&tmp->linkedlist_sleeping);
			}
			
			nd = next;
		}
	}
	
	Task* cur = current_task;
	Task* cleanup = 0;
	do{
		if(cur->linkedlist_tasks.next){
			cur = (Task*)cur->linkedlist_tasks.next;
		}else{
			cur = (Task*)tasks.head;
		}
		
		if(cur->state == TASK_READY || cur->state == TASK_RUNNING){
			cur->state = TASK_RUNNING;
			break;
		}else if(cur->state == TASK_DEAD){
			cleanup = cur;
			if(cur->linkedlist_tasks.next){
				cur = (Task*)cur->linkedlist_tasks.next;
			}else{
				cur = (Task*)tasks.head;
			}
			if((int32)(now - cleanup->wake_tick) >= 0)
				LIST_remove(&sleeping,&cleanup->linkedlist_sleeping);
			LIST_remove(&tasks,(LinkedList_Node*)cleanup);
			kfree((void*)cleanup->stack_base);
			kfree(cleanup);
		}
		
	}while(cur != current_task);
	
	current_task = cur;
	
	return current_task->esp;
}

void task_yield(){
	asm volatile("int $32");
}
void Scheduler_init(){
	LIST_init(&tasks);
	LIST_init(&sleeping);
	
	next_pid = 1;
	
	current_task = &kernel_task;
	kernel_task.pid = 0;
	kernel_task.state = TASK_RUNNING;
	kernel_task.esp = 0;
	
	LIST_push_back(&tasks,&kernel_task.linkedlist_tasks);
	
	PIT_register_handler(Scheduler_tick);
}



typedef struct{
	uint32 ds;
	
	uint32 edi;
	uint32 esi;
	uint32 ebp;
	uint32 esp_dummy;
	uint32 ebx;
	uint32 edx;
	uint32 ecx;
	uint32 eax;
	
	uint32 int_no;
	uint32 err_code;
	
	uint32 eip;
	uint32 cs;
	uint32 eflags;
} InitialFrame;
uint32 start_task(uint32 task_start){
	const static uint32 PAGE_COUNT = 1;
	
	uint32 stack_phys = (uint32)kalloc(PAGE_COUNT*4096);
	uint32 stack_top = stack_phys + 4096;
	InitialFrame* frame = (InitialFrame*)(stack_top - sizeof(InitialFrame));
	
	memset(frame,0,sizeof(InitialFrame));
	
	frame->ds = 0x10;
	frame->eip = task_start;
	frame->cs = 0x08;
	frame->eflags = 0x202;
	
	Task* task = kalloc(sizeof(Task));
	task->pid = next_pid++;
	task->esp = (uint32)frame;
	task->state = TASK_READY;
	task->stack_base = stack_phys;
	task->stack_pages = PAGE_COUNT;
	
	LIST_push_back(&tasks,&task->linkedlist_tasks);
	
	return task->pid;
}
void kill_task(uint32 pid){
	if(!pid) return;
	LinkedList_Node* cur = tasks.head;
	for(;cur;cur=cur->next){
		Task* p = (Task*)cur;
		if(p->pid != pid) continue;
		p->state = TASK_DEAD;
		p->wake_tick = UINT32_MAX;
		break;
	}
	task_yield();
}
void sleep_task(uint32 ms){
	current_task->wake_tick = get_ticks() + PIT_milliseconds_to_ticks(ms);
	
	current_task->state = TASK_SLEEPING;
	
	LIST_push_back(&sleeping,&current_task->linkedlist_sleeping);
	
	task_yield();
}





void mutex_init(Mutex* mtx){
	*mtx = 0;
}
void mutex_lock(Mutex* mtx){
	while(*mtx) task_yield();
	*mtx = 1;
}
void mutex_unlock(Mutex* mtx){
	*mtx = 0;
}

















































