#include "tim.h"

/* 调度回调函数指针 */
static tim_callback_t sched_cb = 0;

/**
 * @brief  注册调度回调函数
 * @param  cb 回调函数指针
 */
void tim_register_callback(tim_callback_t cb) { sched_cb = cb; }

/* 系统滴答计数器，单位1ms */
volatile uint32_t sys_tick_ms = 0;

/**
 * @brief  系统定时器初始化
 *         使用 TIM2 作为系统滴答定时器
 *         时钟频率 = 72MHz / 72 / 1000 = 1kHz => 定时周期1ms
 */
void tim_init(void)
{
    /* 使能TIM2时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* 配置时基：预分频72，自动重装1000 => 1ms中断 */
    TIM_TimeBaseInitTypeDef tim_init;
    tim_init.TIM_CounterMode       = TIM_CounterMode_Up;
    tim_init.TIM_Prescaler         = 72 - 1;
    tim_init.TIM_Period            = 1000 - 1;
    tim_init.TIM_RepetitionCounter = 0;
    tim_init.TIM_ClockDivision     = 0;
    TIM_TimeBaseInit(TIM2, &tim_init);

    /* 配置NVIC中断优先级 */
    NVIC_InitTypeDef nvic_init;
    nvic_init.NVIC_IRQChannel                   = TIM2_IRQn;
    nvic_init.NVIC_IRQChannelCmd                = ENABLE;
    nvic_init.NVIC_IRQChannelPreemptionPriority = 2;
    nvic_init.NVIC_IRQChannelSubPriority        = 2;
    NVIC_Init(&nvic_init);

    /* 使能更新中断 */
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    /* 启动TIM2 */
    TIM_Cmd(TIM2, ENABLE);
}

/**
 * @brief  TIM2更新中断服务函数
 *         每1ms触发一次，递增系统滴答计数，并调用调度回调
 */
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        sys_tick_ms++;
        if (sched_cb) sched_cb();

        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}


