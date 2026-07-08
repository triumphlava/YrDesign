#ifndef __PID_H
#define __PID_H

#include "stdint.h"

/* ================================================================
 *  PID 控制器
 *  用户在此实现：位置式或增量式 PID
 * ================================================================ */

typedef struct {
    float kp;           /* 比例系数 */
    float ki;           /* 积分系数 */
    float kd;           /* 微分系数 */
    float integral;     /* 积分累加值 */
    float prev_error;   /* 上次偏差（用于微分项） */
    float limit;        /* 输出限幅 (上限, -limit ~ +limit) */
} pid_t;

void     pid_init(pid_t *pid, float kp, float ki, float kd, float limit);
void     pid_reset(pid_t *pid);
float    pid_calc(pid_t *pid, float setpoint, float measurement);

#endif
