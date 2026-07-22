/**
 * @file    app_tasks.c
 * @brief   应用任务调度 - 基于时间触发的软件调度器
 * @author  YrDesign
 * @note    利用 TIM2 1ms 定时中断驱动，按固定时间片轮询执行各任务
 *          （非抢占式协作调度）
 */

#include "app_tasks.h"
#include "tim.h"
#include "tracking.h"
#include "button_task.h"
#include "motor.h"
#include "button.h"

/**
 * @brief  应用任务初始化
 * @note   向 TIM2 注册调度回调函数 app_scheduler_tick，
 *         之后每 1ms 进入一次中断回调
 * 
 * 
 *         无需理会
 * 
 * 
 */
void app_tasks_init(void)
{
    tim_register_callback(app_scheduler_tick);
}

/**
 * @brief  软件调度器节拍函数 — 由 TIM2 中断每 1ms 调用一次
 * @note   基于 sys_tick_ms 系统滴答计数，按固定周期分发任务：
 *         每 5ms  → tracking_update()   循迹控制：传感器采集 → PID计算 → 电机调速
 *         每 10ms → button_scan()       按键扫描：去抖状态机检测按键事件
 *
 *         时间片说明：
 *         ┌─────────────┬──────────┬──────────────────────────┐
 *         │ 任务         │ 周期(ms) │ 功能                     │
 *         ├─────────────┼──────────┼──────────────────────────┤
 *         │ 循迹控制     │    5     │ 读取灰度传感器 → PID → 电机 │
 *         │ 按键扫描     │   10     │ 检测按键单击/长按事件      │
 *         └─────────────┴──────────┴──────────────────────────┘
 *
 * @warning 此函数在中断上下文中执行，任务应尽量简短，避免阻塞
 */
void app_scheduler_tick(void)
{
    static uint32_t last_5ms = 0;   /* 上次执行循迹任务的时间戳 */
    static uint32_t last_10ms = 0;  /* 上次执行按键扫描的时间戳 */

    uint32_t now = sys_tick_ms;     /* 获取当前系统滴答计数 */

    /* ---- 5ms 循迹任务 */
    if (now - last_5ms >= 5) {
        last_5ms = now;
        tracking_update();          /* 循迹闭环控制 */
    }

    /* ---- 10ms 按键扫描任务：检测按键状态变化 ---- */
    if (now - last_10ms >= 10) {
        last_10ms = now;
        button_scan();              /* 按键去抖状态机扫描 */
    }
}
