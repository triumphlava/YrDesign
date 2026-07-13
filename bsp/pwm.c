#include "pwm.h"

void pwm_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_TIM1,ENABLE);

    /* PA8(TIM1_CH1)銆丳A11(TIM1_CH4) */
    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;

    gpio_init.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &gpio_init);

    gpio_init.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOA, &gpio_init);

    TIM_TimeBaseInitTypeDef tim_init;
    tim_init.TIM_Prescaler         = 7200 - 1;
    tim_init.TIM_Period            = 200 - 1;
    tim_init.TIM_CounterMode       = TIM_CounterMode_Up;
    tim_init.TIM_ClockDivision     = TIM_CKD_DIV1;
    tim_init.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM1, &tim_init);

    TIM_OCInitTypeDef oc_init;
    TIM_OCStructInit(&oc_init);

    oc_init.TIM_OCMode      = TIM_OCMode_PWM1;
    oc_init.TIM_OutputState = TIM_OutputState_Enable;
    oc_init.TIM_OCPolarity  = TIM_OCPolarity_High;

    /* CH1(PA8) */
    oc_init.TIM_Pulse = 100;      // 50%鍗犵┖姣?    
    TIM_OC1Init(TIM1, &oc_init);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

    /* CH4(PA11) */
    oc_init.TIM_Pulse = 100;      // 50%鍗犵┖姣?    
    TIM_OC4Init(TIM1, &oc_init);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

    /* ARR棰勮杞?*/
    TIM_ARRPreloadConfig(TIM1, ENABLE);

    /* 鍚姩TIM1 */
    TIM_Cmd(TIM1, ENABLE);

    /* 楂樼骇瀹氭椂鍣ㄥ繀椤诲紑鍚富杈撳嚭 */
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

void pwm_set_duty_ch1(uint16_t duty)
{
    if (duty > 100) duty = 100;
    TIM_SetCompare1(TIM1, duty * 2);
}


void pwm_set_duty_ch2(uint16_t duty)
{
    if (duty > 100) duty = 100;
    TIM_SetCompare4(TIM1, duty * 2);
}


