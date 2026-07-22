/**
 * @file    main.c
 * @brief   主程序文件 - 智能小车控制系统
 * @author  YrDesign
 * @date    2026-07-21
 * @note    基于STM32F103C8, 包含循迹、避障、电机控制等功能
 */

/* Includes ********************************************************************/
#include "stm32f10x.h"
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#include "led.h"
#include "delay.h"
#include "button.h"
#include "usart.h"
#include "tim.h"
#include "pwm.h"

#include "tracking.h"
#include "motor.h"
#include "app_tasks.h"
#include "button_task.h"
#include "sensor.h"

/* 外部变量声明 ***************************************************************/
extern uint8_t usart_rx_buffer[128];   /* 串口接收缓冲区 */
extern volatile uint8_t usart_rx_index; /* 串口接收索引 */
extern bool usart_rx_complete;         /* 串口接收完成标志 */

/* 函数声明 */
void usart_process(void);

/**
 * @brief  主函数，系统入口
 * @note   初始化各外设后进入主循环，循环执行按键任务和串口命令处理
 */
int main(void)
{
	led_init();         /* LED初始化 */
	button_init();      /* 按键初始化 */
  	usart_init();       /* USART1初始化（通信） */
	usart2_init();      /* USART2初始化（调试/蓝牙） */
	tim_init();         /* 定时器初始化 */
	pwm_init();         /* PWM初始化（电机调速） */
	motor_init();       /* 电机初始化 */
	Sensor_Init();      /* 传感器初始化 */
    app_tasks_init();   /* 应用任务初始化 */

	while (1)
	{
		button_task();      /* 按键检测任务 */
		usart_process();	/* 串口命令处理 */
    //    motor_set_speed(15,15);
	}
}

/* 外部变量定义 */
extern uint8_t  P_GAIN;                      /* PID比例增益系数 */
volatile bool finish_line_detected = false;  /* 终点线检测标志 */

/**
 * @brief  串口命令处理函数
 * @note   解析通过串口接收到的命令，支持以下格式：
 *         - "m:1~8\r\n"   : 执行导航任务1~8（带重试防抖机制，连续收到4次相同命令才执行）
 *         - "FinishLine\r\n" : 终点线检测命令（需连续收到2次才确认）
 *         其他未识别命令会返回 "Unknown command\r\n"
 */
void usart_process(void)
{
    static uint8_t last_task = 0;   /* 上次收到的任务号，用于防抖比较 */
    static uint8_t same_cnt = 0;    /* 连续收到相同命令的计数 */
	static uint8_t finish_cnt = 0;  /* 终点线命令连续计数 */

    if (usart_rx_complete)  /* 检查是否收到完整的一帧数据 */
    {
        /* 字符串结束符 */
        usart_rx_buffer[usart_rx_index] = '\0';
 
        /* 处理导航任务命令：格式 m:数字\r\n */
        if (usart_rx_buffer[0] == 'm' &&
            usart_rx_buffer[1] == ':' &&
            usart_rx_buffer[2] >= '1' &&
            usart_rx_buffer[2] <= '8' &&
            usart_rx_buffer[3] == '\r' &&
            usart_rx_buffer[4] == '\n')
        {
            uint8_t task = usart_rx_buffer[2] - '0';  /* 将ASCII数字转为任务编号 */

            /* 防抖处理：连续收到4次相同的任务命令才执行，避免串口误码导致误触发 */
            if (task == last_task)
            {
                if (same_cnt < 4)
                    same_cnt++;

                if (same_cnt == 4)
                {
                    nav_start(task);  /* 执行导航任务 */

                    /* 执行后重置计数，防止后续每次收到相同命令都再次执行 */
                    same_cnt = 0;
                    last_task = 0;
                }
            }
            else  /* 第一次收到该命令，记录任务号并开始计数 */
            {
                last_task = task;
                same_cnt = 1;
            }
        }
        /* 处理终点线检测命令 */
		else if (strcmp((char *)usart_rx_buffer, "FinishLine\r\n") == 0)
		{
			if (finish_cnt < 2)
				finish_cnt++;

			if (finish_cnt == 2)  /* 连续收到2次后确认终点线 */
            {
                finish_line_detected = true;  /* 设置终点线检测标志 */
                finish_cnt = 0;               /* 重置计数 */
            }
		}
        else
            usart2_send_string("Unknown command\r\n");  /* 未识别命令 */

        /* 清空接收缓冲区，准备接收下一条命令 */
        memset(usart_rx_buffer, 0, sizeof(usart_rx_buffer));
        usart_rx_index = 0;
        usart_rx_complete = false;
    }
}


