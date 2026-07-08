#ifndef __MY_TIM_H
#define __MY_TIM_H

#include "stm32f10x.h"
#include "stdint.h"

void tim_init(void);
extern volatile uint32_t sys_tick_ms;

#endif
