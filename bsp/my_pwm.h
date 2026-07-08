#ifndef __MY_PWM_H
#define __MY_PWM_H

#include "stm32f10x.h"

void pwm_init(void);
void pwm_set_duty_ch1(uint16_t duty);   /* PA6 — 左轮 */
void pwm_set_duty_ch2(uint16_t duty);   /* PA7 — 右轮 */

#endif // __MY_PWM_H

