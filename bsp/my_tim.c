#include "my_tim.h"
#include "app_tasks.h"

volatile uint32_t sys_tick_ms = 0;
 
void tim_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);

    TIM_TimeBaseInitTypeDef tim_init;
    tim_init.TIM_CounterMode = TIM_CounterMode_Up;
    tim_init.TIM_Prescaler = 72 - 1;
    tim_init.TIM_Period = 1000 - 1;
    tim_init.TIM_RepetitionCounter = 0; 
    tim_init.TIM_ClockDivision = 0;
    TIM_TimeBaseInit(TIM2 , &tim_init);

    NVIC_InitTypeDef nvic_init;
    nvic_init.NVIC_IRQChannel = TIM2_IRQn;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    nvic_init.NVIC_IRQChannelPreemptionPriority = 1;
    nvic_init.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nvic_init);

    TIM_ITConfig(TIM2 , TIM_IT_Update , ENABLE);

    TIM_Cmd(TIM2,ENABLE);
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        sys_tick_ms++;
        app_scheduler_tick();
 
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

