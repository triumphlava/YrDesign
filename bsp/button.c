/**
 * @file    button.c
 * @brief   按键驱动 - 基于状态机的去抖与单击/长按检测
 * @author  YrDesign
 * @note    支持两个按键（PA12, PA15），内部上拉，低电平有效
 *          使用三段式状态机实现硬件去抖，区分单击和长按（≥100ms）
 */

#include "button.h"
#include "stdbool.h"
#include "delay.h"

/* 按键对象数组，button[0]=KEY1(PA12), button[1]=KEY2(PA15) */
button_t button[2];

/**
 * @brief  按键GPIO初始化
 * @note   PA12 - KEY1, PA15 - KEY2
 *         配置为推挽上拉输入（IPU），高电平空闲，低电平按下
 */
void button_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_15;
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;      
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_init);
}

/**
 * @brief  按键扫描函数（由 app_scheduler_tick 每10ms调用一次）
 * @note   三段式按键状态机：
 *
 *         ┌───────┬──────────────────────────────────────────────┐
 *         │ 状态  │ 说明                                         │
 *         ├───────┼──────────────────────────────────────────────┤
 *         │  0    │ 空闲态：检测到按键按下（低电平）→ 进入状态1   │
 *         │  1    │ 确认态：再次检测，仍按下 → 进入状态2（去抖）  │
 *         │       │         已释放          → 回状态0（抖动误触）│
 *         │  2    │ 计时态：持续按下 → time++ 累积计次           │
 *         │       │         释放时：time≥100 → 长按事件          │
 *         │       │                 time<100 → 单击事件          │
 *         └───────┴──────────────────────────────────────────────┘
 *
 *         长按判定阈值：100次 × 10ms = 1000ms（即按住超过1秒算长按）
 */
void button_scan(void)
{
    /* 读取两个按键的当前电平状态（0=按下，1=释放） */
    button[0].state = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12);
    button[1].state = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15);

    for (int i = 0; i < 2; i++)
    {
        switch (button[i].judge)
        {
            case 0:     /* 状态0 - 空闲态：等待按键按下 */
            {
                if (button[i].state == 0)       /* 检测到按下 */
                {
                    button[i].judge = 1;        /* 进入确认态 */
                    button[i].time = 0;         /* 清零计时 */
                }
            }
            break;

            case 1:     /* 状态1 - 确认态：去抖确认 */
            {
                if (button[i].state == 0)       /* 仍为按下，确认有效 */
                    button[i].judge = 2;        /* 进入计时态 */
                else                            /* 已释放，视为抖动 */
                    button[i].judge = 0;        /* 回到空闲态 */
            }
            break;

            case 2:     /* 状态2 - 计时态：累计按下时间 */
            {
                if (button[i].state == 0)       /* 保持按下 */
                {
                    button[i].time++;           /* 累计计时（每10ms +1） */
                }
                else                            /* 按键释放 */
                {
                    if (button[i].time >= 100)  /* 超过1秒 → 长按事件 */
                        button[i].long_press = 1;
                    else                        /* 不足1秒 → 单击事件 */
                        button[i].single = 1;
                    button[i].judge = 0;        /* 回到空闲态 */
                }
            }
            break;
        }
    }
}


