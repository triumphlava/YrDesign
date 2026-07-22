#ifndef __MOTOR_H
#define __MOTOR_H
 
#include "stm32f10x.h"
#include "stdint.h"

#define MOTOR_LEFT  0
#define MOTOR_RIGHT 1

#define MOTOR_FORWARD  1
#define MOTOR_BACKWARD 0

void motor_init(void);
void motor_set_speed(int16_t left_speed, int16_t right_speed);
void motor_stop(uint8_t motor);

#endif


