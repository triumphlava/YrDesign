/**
 * @file    app_tasks.h
 * @brief   应用任务调度 - 头文件
 * @author  YrDesign
 * @note    声明软件调度器的初始化及回调函数
 */

#ifndef __APP_TASKS_H
#define __APP_TASKS_H

#include "stdint.h"

/**
 * @brief  软件调度器节拍回调（由 TIM2 中断每 1ms 调用）
 * @note   内部按固定周期分发循迹(5ms)和按键扫描(10ms)任务
 */
void app_scheduler_tick(void);

/**
 * @brief  应用任务初始化
 * @note   向 TIM2 注册 app_scheduler_tick 回调，启动调度
 */
void app_tasks_init(void);


#endif
