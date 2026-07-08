#ifndef __BUTTON_TASK_H
#define __BUTTON_TASK_H

#include "stm32f10x.h"
/* 10ms 非阻塞按键任务 — 由 app_scheduler_tick() 调用 */
void button_task(void);

#endif
