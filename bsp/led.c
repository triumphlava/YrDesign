#include "led.h"

void led_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);

    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init.GPIO_Pin = GPIO_Pin_13;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC , &gpio_init);

    GPIO_SetBits(GPIOC , GPIO_Pin_13);
    GPIO_ResetBits(GPIOC , GPIO_Pin_13);
}


void led_on(void)
{
    GPIO_ResetBits(GPIOC , GPIO_Pin_13);
}

void led_off(void)
{
    GPIO_SetBits(GPIOC , GPIO_Pin_13);
}






void led_toggle(void)
{
    if (GPIO_ReadOutputDataBit(GPIOC , GPIO_Pin_13) == 1)
        led_on();
    else
        led_off();
}

