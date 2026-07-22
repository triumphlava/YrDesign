#ifndef __PWM_H
#define __PWM_H

#include "stm32f10x.h"

void pwm_init(void);
void pwm_set_duty_ch1(uint16_t duty);   /* PA8 - TIM1_CH1 */
void pwm_set_duty_ch2(uint16_t duty);   /* PA11 - TIM1_CH4 */

#endif /* __PWM_H */


