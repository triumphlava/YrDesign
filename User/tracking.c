#include "tracking.h"
#include "motor.h"
#include "sensor.h"
extern volatile uint32_t sys_tick_ms;
uint8_t tracking_sensor[8] = {0};

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
enum { IDLE, FWD, TURNING, FIND_ROOM, PAUSE, TURN_BACK, RETURN };
static uint8_t state;           /* 当前状态 */
static uint8_t target;          /* 目标病房 1~8 */
static uint8_t path_idx;        /* 当前走到路径的第几步 */
static uint8_t path_len;        /* 路径总长度（几个路口） */
static uint8_t prev_count;      /* 上一次见白传感器数，检测路口上升沿 */
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

uint8_t nav_get_state(void)
{
    return state;
}

void tracking_init(void)
{
    state = IDLE;
}

/* ===== 辅助函数 ===== */

/* 加权误差：负=线偏左，正=线偏右；*sensor_count=见白传感器数 */
static int16_t line_err(int16_t *sensor_count)
{
    static const int8_t weight[8] = {-4, -3, -2, -1, 1, 2, 3, 4};
    int16_t sum = 0;
    *sensor_count = 0;

    for (int i = 0; i < 8; i++)
    {
        if (tracking_sensor[i])
        {
            sum += weight[i];
            (*sensor_count)++;
        }
    }

    if (*sensor_count == 0) return 0;
    return sum / *sensor_count;
}

/* 判断是否黑白交替（到达病房时线走到头了，传感器呈现交替图案） */
static uint8_t is_alternating(void)
{
    uint8_t transitions = 0;
    for (int i = 0; i < 7; i++)
    {
        if (tracking_sensor[i] != tracking_sensor[i+1])
            transitions++;
    }
    /* 7个相邻对中有5个以上交替 -> 线末端 */
    return transitions >= 5;
}

/* ===== 主循环：每5ms调用一次 ===== */

void tracking_update(void)
{
    Sensor_Scan(tracking_sensor);
    uint32_t now = sys_tick_ms;

    /* ---- P 循迹 ---- */
    int16_t sensor_count;
    int16_t error = line_err(&sensor_count);
    int16_t steer = error * 25;         /* P 增益 */
    int16_t left  = 35 + steer;
    int16_t right = 35 - steer;

    if (left  >  100) left  = 100;
    if (left  < -100) left  = -100;
    if (right >  100) right = 100;
    if (right < -100) right = -100;

    switch (state)
    {

    /* ===== 空闲 ===== */
    case IDLE:
        motor_set_speed(0, 0);
        break;

    /* ===== 前进：沿线走，到路口时查路径表决定怎么走 ===== */
    case FWD:
        /* 检测到路口（多数传感器同时见白） */
        if (sensor_count >= 6 && prev_count < 6)
        {
            uint8_t cmd = route[target][path_idx];

            if (cmd == PATH_END)
            {
                /* 所有路口走完了 -> 进入找病房模式 */
                path_idx = 0;
                timer = now;
                state = FIND_ROOM;
                break;
            }

            if (cmd != TURN_STRAIGHT)
            {
                /* 需要拐弯 */
                timer = now;
                state = TURNING;
                break;
            }

            /* 直行：继续往前走，路径步进+1 */
            path_idx++;
        }
        prev_count = sensor_count;
        motor_set_speed(left, right);
        break;

    /* ===== 拐弯：原地旋转90度，拐完切回前进 ===== */
    case TURNING:
    {
        uint8_t cmd = route[target][path_idx];
        if (cmd == TURN_LEFT)
            motor_set_speed(-25, 25);    /* 原地左转 */
        else
            motor_set_speed(25, -25);    /* 原地右转 */

        if (now - timer >= 500)          /* 转弯时长，实测调整 */
        {
            path_idx++;
            timer = now;
            state = FWD;
            motor_set_speed(0, 0);       /* 拐完停一下再走 */
        }
        break;
    }

    /* ===== 找病房：路口都走完了，沿线慢慢开到线末端 ===== */
    case FIND_ROOM:
        if (is_alternating())
        {
            /* 黑白交替 -> 到病房了 */
            motor_set_speed(0, 0);
            timer = now;
            state = PAUSE;
            break;
        }
        /* 慢速前进 */
        motor_set_speed(left * 60 / 100, right * 60 / 100);
        break;

    /* ===== 到病房：停400ms准备掉头 ===== */
    case PAUSE:
        if (now - timer >= 400)
        {
            timer = now;
            state = TURN_BACK;
        }
        break;

    /* ===== 掉头180度 ===== */
    case TURN_BACK:
        motor_set_speed(-30, 30);
        if (now - timer >= 1000)          /* 掉头时长，实测调整 */
        {
            motor_set_speed(0, 0);
            crossed = 0;
            timer = now + 200;
            state = RETURN;
        }
        break;

    /* ===== 返回：沿线回去，每个路口直行通过 ===== */
    case RETURN:
        if (now < timer) break;           /* 掉头后等200ms再出发 */

        if (sensor_count >= 6 && prev_count < 6)
        {
            /* 经过一个路口 */
            crossed++;
            if (crossed >= path_len)
            {
                /* 路口数够了 -> 回到起点 */
                motor_set_speed(0, 0);
                state = IDLE;
                break;
            }
        }
        prev_count = sensor_count;
        motor_set_speed(left, right);
        break;

    default:
        motor_set_speed(0, 0);
        break;
    }
}
