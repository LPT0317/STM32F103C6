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
sTask SCH_tasks_G[SCH_MAX_TASKS];
uint32_t head = -1;
uint32_t tail = -1;
uint32_t size = 0;

/* HELP FUNC ***************************************************************/
uint32_t alloc_task(void){
	for(uint32_t i = 0; i < SCH_MAX_TASKS; i++)
	{
		if(SCH_tasks_G[i].busy == 0)
			return i;
	}
	return -1;
}

void free_task(uint32_t index){
	if(index >= 0 && index < SCH_MAX_TASKS)
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
void Add_Task(uint32_t index){
	if(head == -1){
		head = index;
		tail = index;
	}
	else{
		SCH_tasks_G[tail].next = index;
		SCH_tasks_G[index].prev = tail;
		tail = index;
	}
}
/* MAIN FUNC ***************************************************************/
void SCH_Init(void){
	head = -1;
	tail = -1;
	size = 0;
	for(int i = 0; i < SCH_MAX_TASKS; i++){
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
	if(size < SCH_MAX_TASKS){
		index = alloc_task();
		SCH_tasks_G[index].pTask = pFunction;
		SCH_tasks_G[index].Delay = Delay;
		SCH_tasks_G[index].Period = Period;
		SCH_tasks_G[index].RunMe = 0;
		SCH_tasks_G[index].TaskID = index;
		SCH_tasks_G[index].next = -1;
		SCH_tasks_G[index].prev = -1;
		SCH_tasks_G[index].busy = 1;
		size++;
		Add_Task(index);
	}
}