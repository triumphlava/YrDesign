/**
 * @file    tracking.c
 * @brief   循迹导航 - 状态机实现去程/返回/找病房
 */
#include "tracking.h"
#include "motor.h"
#include "sensor.h"
#include "led.h"
#include "usart.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"

extern volatile uint32_t sys_tick_ms;
uint8_t tracking_sensor[8] = {0};
/* =================================================================
 *  调 试 参 数  ——  根 据 实 际 赛 道 调 整
 * ================================================================= */

/* ---- 循迹 ---- */
uint8_t  SPEED_NORMAL   =    17;      /* 正常前进/返回的基础速度 */
uint8_t  TURNING_SPEED  =    17;      /* 转向时的速度 */
uint8_t  SPEED_SLOW     =    15;      /* 找病房时的慢速 */
int  P_GAIN             =    -2;      /* 误差→转向的 P 增益，越大拐越猛 */

/* ---- 路口检测 ---- */
#define  JUNCTION_CNT       4      /* 几个传感器同时在线算路口（最大 8） */
/* 不再需要 ALT_THRESHOLD，使用碎片段检测（groups >= 2）*/

/* ---- 传感器 ---- */
#define  LINE_VALUE       0      /* 传感器在黑线上的值（0 或 1），几v 的模块不同 */

#define  TURN_TIME        400     /* 90° 原地转：~0.5圈，估计值，实测调 */

/* ---- 掉头 180° ---- */
#define  BACK_SPEED        25      /* 180° 掉头的旋转速度 */
#define  BACK_TIME        785     /* 180° 掉头：~1.0圈，估计值，实测调 */
#define  SETTLE_MS        400      /* 掉头/拐弯后等车身稳定（ms） */

/* ---- 停顿 ---- */
#define  PAUSE_MS         2500      /* 到病房后停多久再掉头（ms） */

/* ================================================================= */

/* ===== 路口转向命令 ===== */
enum { TURN_STRAIGHT, TURN_LEFT, TURN_RIGHT, PATH_END };

/* ===== 8 个房间的路径表 =====
   每个数组表示从起点到对应病房，在每个路口怎么拐。
   遇到 PATH_END 表示走完所有路口，开始找病房入口。*/  
static const uint8_t route[9][5] = {
    {0,0,0,0,0},                                                        /* 占位，不用 room 0 */
    {TURN_LEFT, PATH_END},                                             /* 1号 */
    {TURN_RIGHT, PATH_END},                                              /* 2号 */
    {TURN_STRAIGHT, TURN_LEFT, PATH_END},                              /* 3号 */
    {TURN_STRAIGHT, TURN_RIGHT, PATH_END},                               /* 4号 */
    {TURN_STRAIGHT, TURN_STRAIGHT, TURN_LEFT, TURN_LEFT, PATH_END},   /* 5号 */
    {TURN_STRAIGHT, TURN_STRAIGHT, TURN_RIGHT, TURN_RIGHT, PATH_END},     /* 6号 */
    {TURN_STRAIGHT, TURN_STRAIGHT, TURN_LEFT, TURN_RIGHT, PATH_END},    /* 7号 */
    {TURN_STRAIGHT, TURN_STRAIGHT, TURN_RIGHT, TURN_LEFT, PATH_END},    /* 8号 */
};

/* ===== 状态机 =====
   IDLE      空闲/停止
   FWD       沿黑线前进，检测路口
   TURNING   在路口左转或右转（去程）
   FIND_ROOM 到达目标区域，慢速找病房入口（终线检测）
   PAUSE     到病房后停顿，模拟"到达"
   TURN_BACK 180°原地掉头
   RETURN    原路返回，经过路口方向取反
   RET_TURN  返回途中在路口拐弯
*/
enum { IDLE, FWD, TURNING, FIND_ROOM, PAUSE, TURN_BACK, RETURN, RET_TURN };

static uint8_t state;           /* 当前状态 */
static uint8_t target;          /* 目标病房 1~8 */
static uint8_t path_idx;        /* 当前走到路径的第几步 */
static uint8_t path_len;        /* 路径总长度（几个路口） */
static uint8_t prev_count;      /* 上一次在线传感器数，用于检测路口上升沿 */
static uint8_t crossed;         /* 返回路上已经过的路口数 */
static uint32_t timer;          /* 通用计时器（记录进入某状态的时刻） */
static uint8_t finish_return_flag = 0;  /* 返回终点检测防抖标志 */
static uint32_t finish_timer = 0;       /* 返回终点检测计时 */
/* ===== 公开接口 ===== */

/** 启动导航到指定病房 (1~8) */
void nav_start(uint8_t room)
{
    target     = room;
    path_idx   = 0;
    prev_count = 0;
    crossed    = 0;

    path_len = 0;
    while (route[room][path_len] != PATH_END)
        path_len++;

    state = room ? FWD : IDLE;
}

uint8_t nav_get_state(void) { return state; }

void tracking_init(void)
{
    state = IDLE;
}

/* ===== 辅助函数 ===== */
/**
 * 计算循迹加权偏差：负值=线偏左，正值=线偏右
 * @param sensor_count 输出：当前检测到黑线的传感器数量
 * @return 加权平均偏差
 * @note  weight[0]=12(最左)~weight[7]=-12(最右)
 *        线越靠左 → 正权重传感器见黑 → 正偏差 → 右转修正
 *        线越靠右 → 负权重传感器见黑 → 负偏差 → 左转修正
 */
static int16_t line_err(int16_t *sensor_count)
{
    static const int8_t weight[8] = {12, 9, 5, 2, -2, -5, -9, -12};
    int16_t sum = 0;
    *sensor_count = 0;

    for (int i = 0; i < 8; i++)
    {
        if (tracking_sensor[i] == LINE_VALUE)
        {
            sum += weight[i];
            (*sensor_count)++;
        }
    }

    if (*sensor_count == 0) return 0;

    return sum / *sensor_count;
}

extern volatile bool finish_line_detected ;
/* 判断是否检测到终点标志 */
static uint8_t is_finish_mark(void)
{
    return finish_line_detected;
}

/* 返回时用的转向：直行→直行，左拐→右拐，右拐→左拐 */
static uint8_t reverse_turn(uint8_t fwd_cmd)
{
    if (fwd_cmd == TURN_LEFT)   return TURN_RIGHT;
    if (fwd_cmd == TURN_RIGHT)  return TURN_LEFT;
    return TURN_STRAIGHT;
}

/** 循迹主任务：每5ms调用一次，状态机驱动 */
void tracking_update(void)
{

    if ( SENSOR_CALIB ) 
        Sensor_CalibOutput();
    else
    {
        // get the val of gray
        Sensor_Scan(tracking_sensor);

        uint32_t now = sys_tick_ms;

        /* ---- P 循迹控制器 ----
           根据线偏移量计算左右轮差速，实现沿黑线行驶          */
        int16_t sensor_count;                       /* 当前见黑的传感器数量 */
        int16_t error = line_err(&sensor_count);    /* 线偏移误差 */
        int16_t steer = error * P_GAIN;             /* 误差×比例增益=转向修正量 */

        int16_t left  = SPEED_NORMAL + steer;
        int16_t right = SPEED_NORMAL - steer;

        if (left  >  100) left  = 100;
        if (left  < -100) left  = -100;
        if (right >  100) right = 100;
        if (right < -100) right = -100;

            int16_t left_slow  = SPEED_SLOW + steer;
            int16_t right_slow = SPEED_SLOW - steer;
            if (left_slow  >  100) left_slow  = 100;
            if (left_slow  < -100) left_slow  = -100;
            if (right_slow >  100) right_slow = 100;
            if (right_slow < -100) right_slow = -100;

        switch (state)
        {

        /* ===== 空闲 ===== */
        case IDLE:
            motor_set_speed(0, 0);
            led_off();
            break;

        /* ===== 前进：沿黑线循迹，同时检测路口 ===== */
        case FWD:

            /* 刚过路口/直行后的300ms稳定期，只循迹不检测新路口 */
            if (now < timer)
            {
                motor_set_speed(left, right);
                prev_count = sensor_count;
                break;
            }

            /* 所有路口已走完 → 进入找病房状态 */
            if (route[target][path_idx] == PATH_END)
            {
                path_idx = 0;
                timer = now + 1500;  //1500ms 
                state = FIND_ROOM;
                prev_count = sensor_count;
                break;
            }

            /* 检测到路口（传感器见黑数从少变多，上升沿触发） */
            if (sensor_count >= JUNCTION_CNT && prev_count < JUNCTION_CNT)
            {
                uint8_t cmd = route[target][path_idx];

                if (cmd != TURN_STRAIGHT)   /* 需要转弯 */
                {
                    timer = now;
                    state = TURNING;
                    prev_count = sensor_count;   
                    break;
                }

                /* 直行通过路口：步进到下一个路口，加300ms防抖 */
                path_idx++;
                timer = now + 1500;
            }

            motor_set_speed(left, right);
            prev_count = sensor_count;
            break;
        /* ===== 拐弯（去程）：原地旋转90度 ===== */
        case TURNING:               /* 去程路口拐弯 */
        {
            uint8_t cmd = route[target][path_idx];
            if (cmd == TURN_LEFT)  motor_set_speed(SPEED_NORMAL-TURNING_SPEED, SPEED_NORMAL+TURNING_SPEED);
            else                   motor_set_speed(SPEED_NORMAL+TURNING_SPEED, SPEED_NORMAL-TURNING_SPEED);
            
            if (now - timer >= TURN_TIME)
            {
                path_idx++;
                timer = now;
                state = FWD;
                motor_set_speed(0, 0);
            }
            break;
        }
        /* ===== 找病房：慢速前进，等待检测到终点线 ===== */
        case FIND_ROOM:
		{
            if (now < timer)
            {
                motor_set_speed(left_slow, right_slow);
                prev_count = sensor_count;
                break;
            }

            static bool flag = 0;   /* 是否已检测到终点线 */
            /* 检测到终点线后的300ms：继续慢速前进到线末端 */
            if (flag == true )
            {
                if (now - timer >= 300)
                {
                    flag = 0;
                    motor_set_speed(0, 0);
                    
                    state = PAUSE;
                    break;
                }
            }

            /* 首次检测到终点线，记录时间并继续前进一段 */
            if (!flag && is_finish_mark())
            {
                flag = 1;
                timer = now;
                finish_line_detected = 0;  /* 清除标志，防止重复触发 */
            }
            
            motor_set_speed(left_slow, right_slow);  /* 慢速循迹 */
            break;
		}
        /* ===== 到病房：停400ms ===== */
        case PAUSE:
            if (now - timer >= PAUSE_MS)
            { timer = now; state = TURN_BACK; }
            break;

        /* ===== 掉头180度 ===== */
        case TURN_BACK:
            motor_set_speed(-BACK_SPEED, BACK_SPEED);
            if (now - timer >= BACK_TIME)
            {
                motor_set_speed(0, 0);
                crossed = 0;
                timer = now + SETTLE_MS;
                state = RETURN;
            }
            finish_line_detected = 0;
            break;

        /* ===== 返回：原路返回，方向取反 ===== */
        case RETURN:

            /* 掉头后的稳定期，只循迹不检测路口 */
            if (now < timer)
            {

                motor_set_speed(left, right);
                prev_count = sensor_count;
                break;
            }

            /* 最后一个路口之后，清除终点线标志防止误判 */
            if (crossed == path_len - 1 )
                finish_line_detected = 0;

            /* ===== 返回终点检测：所有路口已走完，检测起点终点线 ===== */
            if (crossed >= path_len)
            {
                if (is_finish_mark())
                {
                    if (!finish_return_flag)
                    {
                        /* 第一次检测到终点线，记录时间并持续前进 */
                        led_on();
                        finish_return_flag = 1;
                        finish_timer = now;
                        motor_set_speed(left, right);
                    }
                    else
                    {
                        /* 终点线持续300ms */
                        if (now - finish_timer >= 300)
                        {
                            motor_set_speed(0,0);
                            state = IDLE;
                            finish_return_flag = 0;
                            break;
                        }
                    }
                }
                else
                {
                    finish_return_flag = 0;
                }

                motor_set_speed(left_slow, right_slow);
                prev_count = sensor_count;
                break;
            }

            /* ===== 返回经过路口 ===== */
            if (sensor_count >= JUNCTION_CNT && prev_count < JUNCTION_CNT)
            {
                uint8_t fwd_cmd = route[target][path_len - 1 - crossed];
                uint8_t ret_cmd = reverse_turn(fwd_cmd);
                crossed++;

                if (ret_cmd != TURN_STRAIGHT) // 要转弯
                {
                    timer = now;
                    state = RET_TURN;

                    prev_count = sensor_count;
                    break;
                }
            }
            prev_count = sensor_count;
            motor_set_speed(left, right);
            break;        
        /* ===== 返回途中拐弯（方向与去的时候相反） ===== */
        case RET_TURN:              /* 返回途中路口拐弯（方向与去程相反） */
        {
            uint8_t fwd_cmd = route[target][path_len - 1 - (crossed - 1)];
            uint8_t ret_cmd = reverse_turn(fwd_cmd);

            if (ret_cmd == TURN_LEFT)  motor_set_speed(SPEED_NORMAL-TURNING_SPEED, SPEED_NORMAL+TURNING_SPEED);
            else                       motor_set_speed(SPEED_NORMAL+TURNING_SPEED, SPEED_NORMAL-TURNING_SPEED);

            if (now - timer >= TURN_TIME)
            {
                timer = (crossed == path_len) ? now + 1500 : now;
                state = RETURN;
                motor_set_speed(0, 0);
            }
            break;
        }

        default:
            motor_set_speed(0, 0);
            break;
        }
    }
}





