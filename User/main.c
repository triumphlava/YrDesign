#include "stm32f10x.h"
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#include "led.h"
#include "delay.h"
#include "button.h"
#include "my_usart.h"
#include "my_tim.h"
#include "my_pwm.h"

#include "tracking.h"
#include "motor.h"
#include "pid.h"
#include "app_tasks.h"
#include "button_task.h"


extern uint8_t usart_rx_buffer[128];
extern volatile uint8_t usart_rx_index;
extern bool usart_rx_complete;

void usart_process(void){;}

int main(void)
{
	led_init();
	button_init();
  	usart_init();
	tim_init();
	pwm_init();
	motor_init();
	while (1)
	{
		button_task();
		usart_process();
		motor_set_speed(20, 20); // 设置左电机和右电机的速度为 50
	}
}
