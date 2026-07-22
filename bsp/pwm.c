#include "pwm.h"

/**
 * @brief  电机PWM初始化
 *         PA8  -> TIM1_CH1
 *         PA11 -> TIM1_CH4
 *         输出50Hz的PWM波，占空比范围为0~100%
 */
void pwm_init(void)
{
    /* 使能GPIOA和TIM1时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1, ENABLE);

    /* 配置PA8、PA11为复用推挽输出 */
    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;

    gpio_init.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &gpio_init);

    gpio_init.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOA, &gpio_init);

    /* 配置TIM1时基：预分频7200，自动重装200 => PWM频率 = 72MHz / 7200 / 200 = 50Hz */
    TIM_TimeBaseInitTypeDef tim_init;
    tim_init.TIM_Prescaler         = 7200 - 1;
    tim_init.TIM_Period            = 200 - 1;
    tim_init.TIM_CounterMode       = TIM_CounterMode_Up;
    tim_init.TIM_ClockDivision     = TIM_CKD_DIV1;
    tim_init.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &tim_init);

    /* 配置PWM输出通道 */
    TIM_OCInitTypeDef oc_init;
    TIM_OCStructInit(&oc_init);
    oc_init.TIM_OCMode      = TIM_OCMode_PWM1;
    oc_init.TIM_OutputState = TIM_OutputState_Enable;
    oc_init.TIM_OCPolarity  = TIM_OCPolarity_High;

    /* CH1 (PA8) - 初始占空比50% */
    oc_init.TIM_Pulse = 100;
    TIM_OC1Init(TIM1, &oc_init);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

    /* CH4 (PA11) - 初始占空比50% */
    oc_init.TIM_Pulse = 100;
    TIM_OC4Init(TIM1, &oc_init);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

    /* 使能ARR预装载 */
    TIM_ARRPreloadConfig(TIM1, ENABLE);

    /* 启动TIM1 */
    TIM_Cmd(TIM1, ENABLE);

    /* 高级定时器必须开启主输出 */
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

/**
 * @brief  设置通道1 (PA8) PWM占空比
 * @param  duty 占空比百分比 (0~100)
 */
void pwm_set_duty_ch1(uint16_t duty)
{
    if (duty > 100) duty = 100;
    TIM_SetCompare1(TIM1, duty * 2);
}

/**
 * @brief  设置通道4 (PA11) PWM占空比
 * @param  duty 占空比百分比 (0~100)
 */
void pwm_set_duty_ch2(uint16_t duty)
{
    if (duty > 100) duty = 100;
    TIM_SetCompare4(TIM1, duty * 2);
}


