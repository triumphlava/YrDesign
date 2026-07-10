#ifndef __USART_H
#define __USART_H  

#include "stm32f10x.h"

void usart_init(void);
void usart_send_byte(uint8_t data);
void usart_send_string(const char* str);

#endif  





