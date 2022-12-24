/*
 * Sched.h
 *
 *  Created on: Dec 24, 2022
 *      Author: lephu
 */

#ifndef INC_SCHED_H_
#define INC_SCHED_H_

#include "stdint.h"
#include "main.h"
#include "stdio.h"

void SCH_Init(void);
void SCH_Update(void);
void SCH_Add_Task(void (*pFunction)(), uint32_t Delay, uint32_t Period);
void SCH_Dispatch_Tasks(void);
void SCH_Delete_Task(uint32_t ID);

void print_Array(void);
void print_Task(void);

#endif /* INC_SCHED_H_ */
