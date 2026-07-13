#include "tracking.h"
#include "motor.h"
#include "sensor.h"
#include "led.h"
#include "usart.h"
extern volatile uint32_t sys_tick_ms;
uint8_t tracking_sensor[8] = {0};
/* =================================================================
 *  调 试 参 数  ——  根 据 实 际 赛 道 调 整
 * ================================================================= */

/* ---- 循迹 ---- */
uint8_t  SPEED_NORMAL =    20;      /* 正常前进/返回的基础速度 */
uint8_t  SPEED_SLOW   =    15;      /* 找病房时的慢速 */
uint8_t  P_GAIN       =    2;      /* 误差→转向的 P 增益，越大拐越猛 */

/* ---- 路口检测 ---- */
#define  JUNCTION_CNT       6      /* 几个传感器同时在线算路口（最大 8） */
/* 不再需要 ALT_THRESHOLD，使用碎片段检测（groups >= 2）*/

/* ---- 传感器 ---- */
#define  LINE_VALUE       0      /* 传感器在黑线上的值（0 或 1），几v 的模块不同 */

/* ---- 拐弯 ---- */
#define  TURN_SPEED        25      /* 90° 拐弯的旋转速度 */
#define  TURN_TIME        200     /* 90° 原地转：~0.5圈，估计值，实测调 */

/* ---- 掉头 180° ---- */
#define  BACK_SPEED        25      /* 180° 掉头的旋转速度 */
#define  BACK_TIME       900     /* 180° 掉头：~1.0圈，估计值，实测调 */
#define  SETTLE_MS        200      /* 掉头/拐弯后等车身稳定（ms） */

/* ---- 停顿 ---- */
#define  PAUSE_MS         400      /* 到病房后停多久再掉头（ms） */

/* ================================================================= */

/* ===== 路口转向命令 ===== */
enum { TURN_STRAIGHT, TURN_LEFT, TURN_RIGHT, PATH_END };

/* ===== 8 个房间的路径表 =====
   每个数组表示从起点到对应病房，在每个路口怎么拐。
   遇到 PATH_END 表示走完所有路口，开始找病房入口。*/
static const uint8_t route[9][5] = {
    {0,0,0,0,0},                            /* 占位，不用 room 0 */
    {TURN_LEFT, PATH_END},                  /* 1号 */
    {TURN_RIGHT, PATH_END},                 /* 2号 */
    {TURN_STRAIGHT, TURN_LEFT, PATH_END},   /* 3号 */
    {TURN_STRAIGHT, TURN_RIGHT, PATH_END},  /* 4号 */
    {TURN_STRAIGHT, TURN_STRAIGHT, TURN_LEFT, TURN_LEFT, PATH_END},   /* 5号 */
    {TURN_STRAIGHT, TURN_STRAIGHT, TURN_RIGHT, TURN_RIGHT, PATH_END}, /* 6号 */
    {TURN_STRAIGHT, TURN_STRAIGHT, TURN_LEFT, TURN_RIGHT, PATH_END},  /* 7号 */
    {TURN_STRAIGHT, TURN_STRAIGHT, TURN_RIGHT, TURN_LEFT, PATH_END},  /* 8号 */
};

/* ===== 状态机 ===== */
enum { IDLE, FWD, TURNING, FIND_ROOM, PAUSE, TURN_BACK, RETURN, RET_TURN };
static uint8_t state;           /* 当前状态 */
static uint8_t target;          /* 目标病房 1~8 */
static uint8_t path_idx;        /* 当前走到路径的第几步 */
static uint8_t path_len;        /* 路径总长度（几个路口） */
static uint8_t prev_count;      /* 上一次在线传感器数，检测路口上升沿 */
static uint8_t crossed;         /* 返回路上经过的路口数 */
static uint32_t timer;          /* 时间戳 */

/* ===== 公开接口 ===== */

void nav_start(uint8_t room)
{
    target     = room;
    path_idx   = 0;
    prev_count = 0;
    crossed    = 0;

    path_len = 0;
    while (route[room][path_len] != PATH_END) path_len++;

    state = room ? FWD : IDLE;
}

uint8_t nav_get_state(void) { return state; }

void tracking_init(void) { state = IDLE; }

/* ===== 辅助函数 ===== */

/* 加权误差：负=线偏左，正=线偏右；*sensor_count=见白传感器数 */
static int16_t line_err(int16_t *sensor_count)
{
    static const int8_t weight[8] = {8, 6, 4, 2, -2, -4, -6, -8};
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


/* 判断是否检测到终点标志 */
static uint8_t is_finish_mark(void)
{
    uint8_t bits = 0;
    bits |= (tracking_sensor[0] == 0) << 0;
    bits |= (tracking_sensor[1] == 0) << 1;
    bits |= (tracking_sensor[2] == 0) << 2;
    bits |= (tracking_sensor[3] == 0) << 3;
    bits |= (tracking_sensor[4] == 0) << 4;
    bits |= (tracking_sensor[5] == 0) << 5;
    bits |= (tracking_sensor[6] == 0) << 6;
    bits |= (tracking_sensor[7] == 0) << 7;
    
    switch(bits)
    {
        case 0x88:  // 01110111
        case 0x66:  // 10011001
        case 0x11:  // 11101110
        case 0xE3:  // 00111011 / 11111100
        case 0x62:  // 10111001
        case 0x9C:  // 11000110 (修正！)
        case 0xCC:  // 11001100
        case 0x52:  // 10111011 (修正！)
        case 0xE6:  // 10110011
        case 0x4C:  // 11001110
        case 0x39:  // 01100111
        case 0xF3:  // 00110011
        case 0xF1:  // 01110011
        case 0x3C:  // 11100110
        case 0xC6:  // 10011110 / 10011111
        case 0x63:  // 00111001 (新增！)
        case 0xB1:  // 01111011 (新增！)
        case 0x03:  // 00111111
        case 0x9F:  // 11111001
        case 0xBF:  // 11111101
            return 1;
        default:
            return 0;
    }
}








/* 返回时用的转向：直行→直行，左拐→右拐，右拐→左拐 */
static uint8_t reverse_turn(uint8_t fwd_cmd)
{
    if (fwd_cmd == TURN_LEFT)   return TURN_RIGHT;
    if (fwd_cmd == TURN_RIGHT)  return TURN_LEFT;
    return TURN_STRAIGHT;
}

/* ===== 主循环：每5ms调用一次 ===== */


void tracking_update(void)
{

    if ( SENSOR_CALIB ) 
        Sensor_CalibOutput();
    else
    {
        Sensor_Scan(tracking_sensor);
        uint32_t now = sys_tick_ms;

        /* ---- P 循迹 ---- */
        int16_t sensor_count;
        int16_t error = line_err(&sensor_count);
        int16_t steer = error * P_GAIN;

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
            break;

        /* ===== 前进：沿线走，到路口时查路径表 ===== */
        case FWD:
        if (sensor_count >= JUNCTION_CNT && prev_count < JUNCTION_CNT)
            {
                uint8_t cmd = route[target][path_idx];

                if (cmd == PATH_END)
                {
                    path_idx = 0;
                    timer = now;
                    state = FIND_ROOM;
                    break;
                }

                if (cmd != TURN_STRAIGHT)
                {
                    timer = now;
                    state = TURNING;
                    break;
                }

                path_idx++;
            }
            prev_count = sensor_count;
            motor_set_speed(left, right);
            break;

        /* ===== 拐弯（去程）：原地旋转90度 ===== */
        case TURNING:
        {
            uint8_t cmd = route[target][path_idx];
            if (cmd == TURN_LEFT)  motor_set_speed(-TURN_SPEED, TURN_SPEED);
            else                   motor_set_speed(TURN_SPEED, -TURN_SPEED);

            if (now - timer >= TURN_TIME)
            {
                path_idx++;
                timer = now;
                state = FWD;
                motor_set_speed(0, 0);
            }
            break;
        }

        /* ===== 找病房：慢速走到线末端 ===== */
        case FIND_ROOM:
            if (is_finish_mark())
            {
                motor_set_speed(0, 0);
                timer = now;
                state = PAUSE;
                break;
            }
            motor_set_speed(left_slow, right_slow);
            break;

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
            break;

        /* ===== 返回：原路返回，方向取反 ===== */
    case RETURN:

        if (now < timer)
            break;

        /* 已完成所有返回拐弯，检测到终点停止 */
        if (crossed >= path_len)
        {
            if (is_finish_mark())
                {
                    motor_set_speed(0,0);
                    state = IDLE;
                }
            motor_set_speed(left, right);
            break;
        }
        if (sensor_count >= JUNCTION_CNT && prev_count < JUNCTION_CNT)
        {
            uint8_t fwd_cmd = route[target][path_len - 1 - crossed];
            uint8_t ret_cmd = reverse_turn(fwd_cmd);
            crossed++;

            if (ret_cmd != TURN_STRAIGHT)
            {
                timer = now;
                state = RET_TURN;
                break;
            }
        }

        prev_count = sensor_count;
        motor_set_speed(left, right);
        break;

        /* ===== 返回途中拐弯（方向与去的时候相反） ===== */
        case RET_TURN:
        {
            uint8_t fwd_cmd = route[target][path_len - 1 - (crossed - 1)];
            uint8_t ret_cmd = reverse_turn(fwd_cmd);

            if (ret_cmd == TURN_LEFT)  motor_set_speed(-TURN_SPEED, TURN_SPEED);
            else                       motor_set_speed(TURN_SPEED, -TURN_SPEED);

            if (now - timer >= TURN_TIME)
            {
                timer = now;
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


// void tracking_update(void)
// {
//     // if ( SENSOR_CALIB ) 
//         Sensor_CalibOutput();
//     // else
//     // {
//         Sensor_Scan(tracking_sensor);
//         if (is_finish_mark())
//         {
//             led_on();
//             usart2_send_string("Finish mark detected!\r\n");
//         }
//         else
//         {
//             led_off();
//         }
//     // }
// }
