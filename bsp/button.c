#include "button.h"
#include "stdbool.h"
#include "delay.h"

button_t button[2]; 

void button_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_15;
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_init);
}

void button_scan(void)
{
    button[0].state = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12) ;  
    button[1].state = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) ;  
    for (int i = 0; i < 2; i++) 
    {
        switch(button[i].judge) 
        {
            case 0: 
            {
                if (button[i].state == 0) {  
                    button[i].judge = 1;     
                    button[i].time = 0;      
                }
            }break;
            
            case 1:  
            {
                if (button[i].state == 0)
                    button[i].judge = 2;
                else 
                    button[i].judge = 0;
            }break;

            case 2:  
            {
                if (button[i].state == 0) 
                    button[i].time++;          
                else 
                {
                    if (button[i].time >= 100) 
                        button[i].long_press = 1;
                    else
                        button[i].single = 1;       
                    button[i].judge = 0;        
                }
            }break;

        }
    }

}


