#include "tim.h"

static tim_callback_t sched_cb = 0;

void tim_register_callback(tim_callback_t cb) { sched_cb = cb; }

volatile uint32_t sys_tick_ms = 0;

void tim_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef tim_init;
    tim_init.TIM_CounterMode       = TIM_CounterMode_Up;
    tim_init.TIM_Prescaler         = 72 - 1;
    tim_init.TIM_Period            = 1000 - 1;
    tim_init.TIM_RepetitionCounter = 0;
    tim_init.TIM_ClockDivision     = 0;
    TIM_TimeBaseInit(TIM2, &tim_init);

    NVIC_InitTypeDef nvic_init;
    nvic_init.NVIC_IRQChannel                   = TIM2_IRQn;
    nvic_init.NVIC_IRQChannelCmd                = ENABLE;
    nvic_init.NVIC_IRQChannelPreemptionPriority = 2;
    nvic_init.NVIC_IRQChannelSubPriority        = 2;
    NVIC_Init(&nvic_init);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        sys_tick_ms++;
        if (sched_cb) sched_cb();
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}


