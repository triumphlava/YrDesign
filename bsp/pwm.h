#ifndef __PWM_H
#define __PWM_H

#include "stm32f10x.h"

void pwm_init(void);
void pwm_set_duty_ch1(uint16_t duty);   /* PA6 鈥?宸﹁疆 */
void pwm_set_duty_ch2(uint16_t duty);   /* PA7 鈥?鍙宠疆 */

#endif // __PWM_H


