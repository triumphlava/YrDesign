/**
 * @file    button_task.c
 * @brief   按键任务处理 - 用于调试模式下选择与启动导航任务
 * @author  YrDesign
 * @note    此模块主要用于调试阶段，通过物理按键选择并启动不同的导航任务
 *          （正式运行时任务由串口命令触发，本模块可作调试备用）
 */

#include "button_task.h"
#include "button.h"
#include "led.h"
#include "delay.h"
#include "tracking.h"

/* 按键对象数组（在 button.c 中定义） */
extern button_t button[2];

/* 内部函数声明 */
static void button1_single_task(void);
static void button2_single_task(void);
static void button1_long_press_task(void);
static void button2_long_press_task(void);

/**
 * @brief  调试用导航任务编号
 * @note   Button1单击循环切换 0~9，Button2单击启动当前编号的任务
 *         任务说明（具体实现在 tracking / app_tasks 中）：
 *          - 0~8: 不同的循迹/避障导航任务
 *          - 9:   任务编号上限，切换时归零
 */
int mission = 0;

/**
 * @brief  按键任务主函数（由 main 循环调用）
 * @note   检测两个按键（Button1=KEY1, Button2=KEY2）的单击和长按事件，
 *         并分发到对应的处理函数。
 *          - Button1 单击：切换任务编号（mission）
 *          - Button2 单击：启动当前编号的导航任务
 *          - 长按：预留功能，当前为空实现
 */
void button_task(void)
{
    if (button[0].single == 1)      /* Button1 单击 */
    {
        button[0].single = 0;       
        button1_single_task();      
    }
    else if (button[1].single)      /* Button2 单击 */
    {
        button[1].single = 0;       
        button2_single_task();      
    }
    else if (button[0].long_press)  /* Button1 长按 */
    {
        button[0].long_press = 0;   
        button1_long_press_task();  
    }
    else if (button[1].long_press)  /* Button2 长按 */
    {
        button[1].long_press = 0;   
        button2_long_press_task();  
    }
}

/**
 * @brief  Button1 单击处理：循环切换任务编号
 * @note   每按一次 mission 加 1，当 mission 达到 9 时回到 0
 */
static void button1_single_task(void)
{
    mission = mission == 9 ? 0 : mission + 1;
}

/**
 * @brief  Button2 单击处理：启动当前选中的导航任务
 * @note   当 mission == 4 时点亮 LED 作为视觉提示
 *         然后调用 nav_start(mission) 启动对应任务
 */
static void button2_single_task(void)
{
    if (mission == 4)
        led_on();           /* 任务4的视觉指示 */
    nav_start(mission);     /* 启动导航任务 */
}

/**
 * @brief  Button1 长按处理（预留）
 * @note   可根据需要扩展功能，如紧急停止、复位等
 */
static void button1_long_press_task(void)
{

}

/**
 * @brief  Button2 长按处理（预留）
 * @note   可根据需要扩展功能，如紧急停止、复位等
 */
static void button2_long_press_task(void)
{

}
