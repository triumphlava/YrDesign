#ifndef __TIM_H
#define __TIM_H

#include "stm32f10x.h"
#include "stdint.h"

/* 定时器回调函数类型 */
typedef void (*tim_callback_t)(void);

/* 系统滴答计数器（外部可访问），单位1ms */
extern volatile uint32_t sys_tick_ms;

/**
 * @brief  系统定时器初始化（TIM2，1ms中断）
 */
void tim_init(void);

/**
 * @brief  注册定时器调度回调
 * @param  cb 回调函数指针
 */
void tim_register_callback(tim_callback_t cb);

#endif /* __TIM_H */


