#ifndef __PWM_H
#define __PWM_H

#include "stm32f10x.h"

void pwm_init(void);
void pwm_set_duty_ch1(uint16_t duty);
void pwm_set_duty_ch2(uint16_t duty);

#endif


