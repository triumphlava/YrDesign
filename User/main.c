#include "stm32f10x.h"
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#include "led.h"
#include "delay.h"
#include "button.h"
#include "usart.h"
#include "tim.h"
#include "pwm.h"

#include "tracking.h"
#include "motor.h"
#include "app_tasks.h"
#include "button_task.h"
#include "sensor.h"


extern uint8_t usart_rx_buffer[128];
extern volatile uint8_t usart_rx_index;
extern bool usart_rx_complete;


void usart_process(void);

int main(void)
{
	led_init();
	button_init();
  	usart_init();
	usart2_init();
	app_tasks_init();
	tim_init();
	pwm_init();
	motor_init();
	Sensor_Init();
	while (1)
	{
		button_task();
		usart_process();
		// motor_set_speed(20, 20); 
	}
}


extern uint8_t  P_GAIN;

void usart_process(void)
{
	if (usart_rx_complete)
	{
		usart_rx_buffer[usart_rx_index] = '\0'; // 

		if (strcmp((char*)usart_rx_buffer, "m1z") == 0)
			nav_start(1);
		else if (strcmp((char*)usart_rx_buffer, "m2z") == 0)
			nav_start(2);
		else if (strcmp((char*)usart_rx_buffer, "m3z") == 0)
			nav_start(3);
		else if (strcmp((char*)usart_rx_buffer, "m4z") == 0)
			nav_start(4);
		else if (strcmp((char*)usart_rx_buffer, "m5z") == 0)
			nav_start(5);
		else if (strcmp((char*)usart_rx_buffer, "m6z") == 0)
			nav_start(6);
		else if (strcmp((char*)usart_rx_buffer, "m7z") == 0)
			nav_start(7);
		else if (strcmp((char*)usart_rx_buffer, "m8z") == 0)
			nav_start(8);
		else if (strcmp((char*)usart_rx_buffer, "-z") == 0)
			P_GAIN--;
		else if (strcmp((char*)usart_rx_buffer, "+z") == 0)
			P_GAIN++;
		else
		{
			usart2_send_string("Unknown command\r\n");
		}
		usart_rx_index = 0;	
		usart_rx_complete = false;
	}
}

