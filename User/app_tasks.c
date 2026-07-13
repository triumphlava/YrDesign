#include "app_tasks.h"
#include "tim.h"
#include "tracking.h"
#include "button_task.h"
#include "motor.h"
#include "button.h"

void app_tasks_init(void)
{
    tim_register_callback(app_scheduler_tick);
}

/**
 * @brief  Software scheduler tick — called every 1ms from TIM2 IRQ.
 *         Dispatches time-triggered tasks based on sys_tick_ms.
 *
 *         5ms  → tracking_update()   循迹任务（用户实现）
 *         10ms → button_task()       按键任务 (User/button_task.c)
 *
 *         数据流: tracking.c → pid.c → motor.c
 *                button_task.c ← button.c (bsp 层)
 */
void app_scheduler_tick(void)
{
    static uint32_t last_5ms = 0;
    static uint32_t last_10ms = 0;

    uint32_t now = sys_tick_ms;

    /* ---- 5ms 循迹任务 ---- */
    if (now - last_5ms >= 3) {
        last_5ms = now;
        tracking_update();          /* 循迹: 传感器 → PID → motor_set_speed() */
    }

    /* ---- 10ms 按键任务 ---- */
    if (now - last_10ms >= 10) {
        last_10ms = now;
        button_scan();              /* 按键: 状态机 (User/button_task.c) */
    }
}

//状态机  循迹   总体的结构
