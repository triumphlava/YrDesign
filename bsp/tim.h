#ifndef __TIM_H
#define __TIM_H

#include "stm32f10x.h"
#include "stdint.h"

typedef void (*tim_callback_t)(void);

extern volatile uint32_t sys_tick_ms;

void tim_init(void);
void tim_register_callback(tim_callback_t cb);

#endif


