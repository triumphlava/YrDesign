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
volatile bool finish_line_detected = false;
void usart_process(void)
{
    static uint8_t last_task = 0;
    static uint8_t same_cnt = 0;
	static uint8_t finish_cnt = 0;
    if (usart_rx_complete)
    {
        usart_rx_buffer[usart_rx_index] = '\0';

        /* 命令格式：m:1\r\n */
        if (usart_rx_buffer[0] == 'm' &&
            usart_rx_buffer[1] == ':' &&
            usart_rx_buffer[2] >= '1' &&
            usart_rx_buffer[2] <= '8' &&
            usart_rx_buffer[3] == '\r' &&
            usart_rx_buffer[4] == '\n')
        {
            uint8_t task = usart_rx_buffer[2] - '0';

            if (task == last_task)
            {
                if (same_cnt < 4)
                    same_cnt++;

                if (same_cnt == 4)
                {
                    nav_start(task);

                    /* 防止之后每收到一次都再次执行 */
                    same_cnt = 0;
                    last_task = 0;
                }
            }
            else
            {
                last_task = task;
                same_cnt = 1;
            }
        }
		else if (strcmp((char *)usart_rx_buffer, "FinishLine\r\n") == 0)
		{
			if (finish_cnt < 2)
				finish_cnt++;

			if (finish_cnt == 2)
			{
				/* TODO: 连续收到5次 FinishLine 后执行 */
				finish_line_detected = true;
				finish_cnt = 0;      // 如果希望再次连续5次还能触发
			}
		}
		else if (strcmp((char *)usart_rx_buffer, "-z") == 0)
			P_GAIN--;

		else if (strcmp((char *)usart_rx_buffer, "+z") == 0)
			P_GAIN++;
        else
            usart2_send_string("Unknown command\r\n");

        usart_rx_index = 0;
        usart_rx_complete = false;
    }
}