#include "motor.h"
#include "pwm.h"

void motor_set_direction(uint8_t motor, uint8_t dir);
void motor_set_pwm(uint8_t motor, uint16_t duty);


void motor_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Pin  = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio_init);

    /* 初始状态：所有电机停止 */
    GPIO_ResetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13);
    GPIO_ResetBits(GPIOB, GPIO_Pin_14 | GPIO_Pin_15);
}

#define LEFT_MOTOR_COMPENSATION 110    // 85%

void motor_set_speed(int16_t left_speed, int16_t right_speed)
{
    uint16_t duty;

    /*---------------- 左电机（实际处理右电机的参数） ----------------*/
    // 注意：这里处理的是 right_speed，但操作 MOTOR_LEFT 的硬件
    if (right_speed > 100)
        right_speed = 100;
    if (right_speed < -100)  
        right_speed = -100;

    if (right_speed > 0)
    {
        motor_set_direction(MOTOR_LEFT, MOTOR_FORWARD);
        duty = (uint16_t)(right_speed * LEFT_MOTOR_COMPENSATION / 100);
        motor_set_pwm(MOTOR_LEFT, duty);
    }
    else if (right_speed < 0)
    {
        motor_set_direction(MOTOR_LEFT, MOTOR_BACKWARD);
        duty = (uint16_t)((-right_speed) * LEFT_MOTOR_COMPENSATION / 100);
        motor_set_pwm(MOTOR_LEFT, duty);
    }
    else
    {
        motor_stop(MOTOR_LEFT);
    }

    /*---------------- 右电机（实际处理左电机的参数） ----------------*/
    // 注意：这里处理的是 left_speed，但操作 MOTOR_RIGHT 的硬件
    if (left_speed > 100)
        left_speed = 100;
    if (left_speed < -100)
        left_speed = -100;

    if (left_speed > 0)
    {
        motor_set_direction(MOTOR_RIGHT, MOTOR_FORWARD);
        duty = left_speed;
        motor_set_pwm(MOTOR_RIGHT, duty);
    }
    else if (left_speed < 0)
    {
        motor_set_direction(MOTOR_RIGHT, MOTOR_BACKWARD);
        duty = -left_speed;
        motor_set_pwm(MOTOR_RIGHT, duty);
    }
    else
    {
        motor_stop(MOTOR_RIGHT);
    }
}

void motor_set_direction(uint8_t motor, uint8_t dir)
{
    if (motor == MOTOR_LEFT) {
        if (dir == MOTOR_FORWARD) {
            GPIO_SetBits(GPIOB, GPIO_Pin_13);
            GPIO_ResetBits(GPIOB, GPIO_Pin_12);
        } else {
            GPIO_ResetBits(GPIOB, GPIO_Pin_13);
            GPIO_SetBits(GPIOB, GPIO_Pin_12);
        }
    } else {
        if (dir == MOTOR_FORWARD) {
            GPIO_SetBits(GPIOB, GPIO_Pin_15);
            GPIO_ResetBits(GPIOB, GPIO_Pin_14);
        } else {
            GPIO_ResetBits(GPIOB, GPIO_Pin_15);
            GPIO_SetBits(GPIOB, GPIO_Pin_14);
        }
    }
}

void motor_set_pwm(uint8_t motor, uint16_t duty)
{
    if (duty > 100) duty = 100;

    if (motor == MOTOR_LEFT)
        pwm_set_duty_ch1(duty);
    else
        pwm_set_duty_ch2(duty);
}

void motor_stop(uint8_t motor)
{

    if (motor == MOTOR_LEFT) {
        GPIO_ResetBits(GPIOB, GPIO_Pin_1 | GPIO_Pin_2);
        pwm_set_duty_ch1(0);
    } else {
        GPIO_ResetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_4);
        pwm_set_duty_ch2(0);
    }
}
