#include "usart.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdint.h"

uint8_t usart_rx_buffer[128];
volatile uint8_t usart_rx_index = 0;
bool usart_rx_complete = false;

void usart_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Pin = GPIO_Pin_9; 
    gpio_init.GPIO_Mode = GPIO_Mode_AF_PP; 
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_init);

    gpio_init.GPIO_Pin = GPIO_Pin_10; 
    gpio_init.GPIO_Mode = GPIO_Mode_IPU; 
    GPIO_Init(GPIOA, &gpio_init);

    USART_InitTypeDef usart_init;
    usart_init.USART_BaudRate = 38400;
    usart_init.USART_WordLength = USART_WordLength_8b;
    usart_init.USART_StopBits = USART_StopBits_1;
    usart_init.USART_Parity = USART_Parity_No;
    usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &usart_init);
    // // 涓柇宓屽鍚戦噺琛?
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef nvic_init;
    nvic_init.NVIC_IRQChannel = USART1_IRQn; 
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    nvic_init.NVIC_IRQChannelPreemptionPriority = 1;
    nvic_init.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nvic_init);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    USART_Cmd(USART1, ENABLE);
}

int fputc(int ch, FILE *f)
{
    // 绛夊緟鍙戦€佹暟鎹瘎瀛樺櫒涓虹┖
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, (uint8_t)ch);
    
    // 濡傛灉闇€瑕佺瓑寰呭彂閫佸畬鎴愶紙鍙€夛級
    // while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    return ch;
}


void USART1_IRQHandler(void)
{
    if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
    {
        uint8_t data = USART_ReceiveData(USART1);
        usart_rx_buffer[usart_rx_index] = data;
        usart_rx_index++;
        if (data == '\n')
            usart_rx_complete = true;
    }
}


void usart_send_byte(uint8_t data)
{
    USART_SendData(USART1, data);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == 0);
}



void usart_send_string(const char* str)
{
    while(*str)
    {
        usart_send_byte(*str);
        str++;
    }
}



