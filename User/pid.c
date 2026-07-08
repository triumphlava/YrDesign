#include "pid.h"

/* ================================================================
 *  PID 控制器初始化
 *  用户在此设置：kp, ki, kd, 输出限幅
 * ================================================================ */
void pid_init(pid_t *pid, float kp, float ki, float kd, float limit)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->limit = limit;
    pid_reset(pid);
}


void pid_reset(pid_t *pid)
{
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
}


/* ================================================================
 *  PID 计算
 *  @return  控制输出值 (范围 -limit ~ +limit)
 *  用户在此实现：位置式 PID 公式
 * ================================================================ */
float pid_calc(pid_t *pid, float setpoint, float measurement)
{
    float error = setpoint - measurement;

    /* P */
    float p_out = pid->kp * error;

    /* I — 带积分限幅（抗饱和） */
    pid->integral += error;
    if (pid->integral >  pid->limit) pid->integral =  pid->limit;
    if (pid->integral < -pid->limit) pid->integral = -pid->limit;
    float i_out = pid->ki * pid->integral;

    /* D */
    float d_out = pid->kd * (error - pid->prev_error);
    pid->prev_error = error;

    /* 合拢 */
    float output = p_out + i_out + d_out;

    /* 输出限幅 */
    if (output >  pid->limit) output =  pid->limit;
    if (output < -pid->limit) output = -pid->limit;

    return output;
}
