#include "button_task.h"
#include "button.h"
#include "led.h"
#include "delay.h"
#include "tracking.h"
extern button_t button[2];

/* ================================================================
 *  非阻塞按键任务 — 每 10ms 由调度器调用
 *
 *  用户在此添加：
 *    按键状态机（消抖、单次触发、长按检测等）
 *    示例: 检测到按键按下后执行对应动作
 * ================================================================ */
static void button1_single_task(void);
static void button2_single_task(void);
static void button1_long_press_task(void);
static void button2_long_press_task(void);
int mission = 0;

void button_task(void)
{
    if (button[0].single == 1) 
    {
        button[0].single = 0;  
        button1_single_task(); 
    }
    else if (button[1].single)
    {
        button[1].single = 0;  
        button2_single_task(); 
    }
    else if (button[0].long_press) 
    {
        button[0].long_press = 0;  
        button1_long_press_task(); 
    }
    else if (button[1].long_press) 
    {
        button[1].long_press = 0;  
        button2_long_press_task(); 
    }
}

static void button1_single_task(void)
{
    mission = mission == 9 ? 0 : mission + 1;
}

static void button2_single_task(void)
{
    if (mission == 4)
        led_on();
    nav_start(mission);
}

static void button1_long_press_task(void)
{

}

static void button2_long_press_task(void)
{

}
