/*
 * Sched.c
 *
 *  Created on: Dec 24, 2022
 *      Author: lephu
 */

#include "Sched.h"

char str[50];

typedef struct {
  void (*pTask)();
  uint32_t Delay;
  uint32_t Period;
  uint32_t RunMe;
  uint32_t TaskID;
  uint32_t next;
  uint32_t prev;
  uint32_t busy;
}sTask;

#define SCH_MAX_TASKS 10
#define RAM_SIZE 30
#define TICK 10;
sTask SCH_tasks_G[RAM_SIZE];
uint32_t head = -1;
uint32_t tail = -1;
uint32_t size = 0;

uint32_t head_ready_queue = -1;
uint32_t tail_ready_queue = -1;

uint32_t time = 0;

/* HELP FUNC ***************************************************************/
uint32_t alloc_task(void){
	for(uint32_t i = 0; i < RAM_SIZE; i++)
	{
		if(SCH_tasks_G[i].busy == 0)
			return i;
	}
	return -1;
}

void free_task(uint32_t index){
	if(index >= 0 && index < RAM_SIZE)
		SCH_tasks_G[index].busy = 0;
}

void print_Array(void){
	HAL_UART_Transmit(&huart2, (void *)str, sprintf(str, "Number of tasks: %li\r", size), 1000);
	for(int i = 0; i < SCH_MAX_TASKS; i++){
		HAL_UART_Transmit(&huart2, (void *)str, sprintf(str, "%i: %li\r", i, SCH_tasks_G[i].busy), 1000);
	}
}
void print_Task(void){
	if(head != -1){
		uint32_t ptr = head;
		for(int i = 0; i < size; i++)
		{
			HAL_UART_Transmit(&huart2, (void *)str, sprintf(str, "%li: %li\r", ptr, SCH_tasks_G[ptr].Delay), 1000);
			ptr = SCH_tasks_G[ptr].next;
		}
	}
}
void Add_Task(uint32_t task){
	if(head == -1){
		head = task;
		tail = task;
		size++;
		return;
	}
//	SCH_tasks_G[tail].next = index;
//	SCH_tasks_G[index].prev = tail;
//	tail = index;
	if(SCH_tasks_G[task].Delay >= SCH_tasks_G[head].Delay){
		SCH_tasks_G[task].Delay -= SCH_tasks_G[head].Delay;
		if(SCH_tasks_G[task].Delay == 0)
			SCH_tasks_G[task].RunMe++;
		uint32_t curr = SCH_tasks_G[head].next;
		for(int i = 1; i < size; i++){
			if(SCH_tasks_G[task].Delay < SCH_tasks_G[curr].Delay)
				break;
			curr = SCH_tasks_G[curr].next;
		}
		//HAL_UART_Transmit(&huart2, (void *)str, sprintf(str, "curr = %li\r", curr), 1000);
		if(curr == -1){//add after tail
			SCH_tasks_G[task].prev = tail;
			SCH_tasks_G[tail].next = task;
			tail = task;
			size++;
			return;
		}
		//add in middle
		SCH_tasks_G[task].prev = SCH_tasks_G[curr].prev;
		SCH_tasks_G[task].next = curr;
		SCH_tasks_G[SCH_tasks_G[task].prev].next = task;
		SCH_tasks_G[curr].prev = task;
		size++;
	}
	else{
		uint32_t time_interval = SCH_tasks_G[head].Delay;
		SCH_tasks_G[head].Delay -= SCH_tasks_G[task].Delay;
		if(SCH_tasks_G[head].Delay == 0)
			SCH_tasks_G[head].RunMe++;
		uint32_t curr = SCH_tasks_G[head].next;
		for(int i = 1; i < size; i++){
			SCH_tasks_G[curr].Delay = SCH_tasks_G[curr].Delay + time_interval - SCH_tasks_G[task].Delay;
			if(SCH_tasks_G[curr].Delay == 0)
				SCH_tasks_G[curr].RunMe++;
			curr = SCH_tasks_G[curr].next;
		}
		SCH_tasks_G[head].prev = task;
		SCH_tasks_G[task].next = head;
		head = task;
		size++;
	}
}
uint32_t Get_Task(void){
	uint32_t tmp_task = head;
	SCH_tasks_G[SCH_tasks_G[head].next].prev = -1;
	head = SCH_tasks_G[head].next;
	SCH_tasks_G[tmp_task].next = -1;
	size--;
	return tmp_task;
}

void Push_queue(uint32_t task){
	if(head_ready_queue == -1){
		head_ready_queue = task;
	}
	else{
		SCH_tasks_G[tail_ready_queue].next = task;
	}
	tail_ready_queue = task;
}

uint32_t Pop_queue(void){
	if(head == -1)
		return -1;
	uint32_t tmp_task = head_ready_queue;
	head_ready_queue = SCH_tasks_G[tmp_task].next;
	SCH_tasks_G[tmp_task].next = -1;
	return tmp_task;
}
/* MAIN FUNC ***************************************************************/
void SCH_Init(void){
	head = -1;
	tail = -1;
	size = 0;
	for(int i = 0; i < RAM_SIZE; i++){
		SCH_tasks_G[i].pTask = NULL;
		SCH_tasks_G[i].Delay = -1;
		SCH_tasks_G[i].Period = -1;
		SCH_tasks_G[i].RunMe = -1;
		SCH_tasks_G[i].TaskID = -1;
		SCH_tasks_G[i].next = -1;
		SCH_tasks_G[i].prev = -1;
		SCH_tasks_G[i].busy = 0;
	}
}

void SCH_Add_Task(void (*pFunction)(), uint32_t Delay, uint32_t Period){
	uint32_t index = 0;
	index = alloc_task();
	if(size < SCH_MAX_TASKS && index != -1){
		index = alloc_task();
		SCH_tasks_G[index].pTask = pFunction;
		SCH_tasks_G[index].Delay = Delay;
		SCH_tasks_G[index].Period = Period;
		SCH_tasks_G[index].RunMe = 0;
		SCH_tasks_G[index].TaskID = index;
		SCH_tasks_G[index].next = -1;
		SCH_tasks_G[index].prev = -1;
		SCH_tasks_G[index].busy = 1;
		Add_Task(index);
//		HAL_UART_Transmit(&huart2, (void *)str, sprintf(str, "\rTask List:\r"), 1000);
//		print_Task();
	}
}

void SCH_Update(void){
	time += TICK;
	if(head != -1){
		if(SCH_tasks_G[head].Delay > 0){
			SCH_tasks_G[head].Delay--;
		}
		else{
			SCH_tasks_G[head].RunMe = 1;
			if(SCH_tasks_G[head].Period > 0){
				SCH_tasks_G[head].Delay = SCH_tasks_G[head].Period;
			}
			uint32_t run_Task = Get_Task();
			Push_queue(run_Task);
		}
	}
}

void SCH_Dispatch_Tasks(void){
	uint32_t running = Pop_queue();
	if(running != -1){
		HAL_UART_Transmit(&huart2, (void *)str, sprintf(str, "Task running: %ld\r", running), 1000);
		HAL_UART_Transmit(&huart2, (void *)str, sprintf(str, "Start: %ld\r", time), 1000);
		(*SCH_tasks_G[running].pTask)();
		HAL_UART_Transmit(&huart2, (void *)str, sprintf(str, "End: %ld\r", time), 1000);
		if(SCH_tasks_G[running].Period != 0){
			SCH_tasks_G[running].RunMe = 0;
			Add_Task(running);
		}
		else{
			HAL_UART_Transmit(&huart2, (void *)str, sprintf(str, "Task one-shot die....\r"), 1000);
			SCH_Delete_Task(running);
		}
	}
}
void SCH_Delete_Task(uint32_t ID){
	SCH_tasks_G[ID].busy = 0;
}
