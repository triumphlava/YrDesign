#ifndef __MOTOR_H
#define __MOTOR_H
 
#include "stm32f10x.h"
#include "stdint.h"

/* ================================================================
 *  电机引脚定义（参考，用户根据实际接线修改）
 *  左轮: IN1=PB5, IN2=PB6, EN=PA6(TIM3_CH1)
 *  右轮: IN3=PB7, IN4=PB8, EN=PA7(TIM3_CH2)
 * ================================================================ */

#define MOTOR_LEFT  0
#define MOTOR_RIGHT 1

#define MOTOR_FORWARD  1
#define MOTOR_BACKWARD 0

/* ---- 接口声明 ---- */

void motor_init(void);
void motor_set_speed(int16_t left_speed, int16_t right_speed);
void motor_stop(uint8_t motor);

#endif


