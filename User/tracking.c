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

uint8_t  SPEED_NORMAL   =    17;
uint8_t  TURNING_SPEED  =    17;
uint8_t  SPEED_SLOW     =    15;
int  P_GAIN             =    -2;

#define  JUNCTION_CNT       4
#define  LINE_VALUE         0
#define  TURN_TIME        400
#define  BACK_SPEED        25
#define  BACK_TIME        785
#define  SETTLE_MS        400
#define  PAUSE_MS        2500

enum { TURN_STRAIGHT, TURN_LEFT, TURN_RIGHT, PATH_END };

static const uint8_t route[9][5] = {
    {0,0,0,0,0},
    {TURN_LEFT, PATH_END},
    {TURN_RIGHT, PATH_END},
    {TURN_STRAIGHT, TURN_LEFT, PATH_END},
    {TURN_STRAIGHT, TURN_RIGHT, PATH_END},
    {TURN_STRAIGHT, TURN_STRAIGHT, TURN_LEFT, TURN_LEFT, PATH_END},
    {TURN_STRAIGHT, TURN_STRAIGHT, TURN_RIGHT, TURN_RIGHT, PATH_END},
    {TURN_STRAIGHT, TURN_STRAIGHT, TURN_LEFT, TURN_RIGHT, PATH_END},
    {TURN_STRAIGHT, TURN_STRAIGHT, TURN_RIGHT, TURN_LEFT, PATH_END},
};

enum { IDLE, FWD, TURNING, FIND_ROOM, PAUSE, TURN_BACK, RETURN, RET_TURN };

static uint8_t state;
static uint8_t target;
static uint8_t path_idx;
static uint8_t path_len;
static uint8_t prev_count;
static uint8_t crossed;
static uint32_t timer;
static uint8_t finish_return_flag = 0;
static uint32_t finish_timer = 0;
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

static uint8_t is_finish_mark(void)
{
    return finish_line_detected;
}

static uint8_t reverse_turn(uint8_t fwd_cmd)
{
    if (fwd_cmd == TURN_LEFT)   return TURN_RIGHT;
    if (fwd_cmd == TURN_RIGHT)  return TURN_LEFT;
    return TURN_STRAIGHT;
}

void tracking_update(void)
{

    if ( SENSOR_CALIB ) 
        Sensor_CalibOutput();
    else
    {
        Sensor_Scan(tracking_sensor);

        uint32_t now = sys_tick_ms;

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
            case IDLE:
                motor_set_speed(0, 0);
                led_off();
                break;

            case FWD:
                if (now < timer)
                {
                    motor_set_speed(left, right);
                    prev_count = sensor_count;
                    break;
                }

                if (route[target][path_idx] == PATH_END)
                {
                    path_idx = 0;
                    timer = now + 1500;
                    state = FIND_ROOM;
                    prev_count = sensor_count;
                    break;
                }

                if (sensor_count >= JUNCTION_CNT && prev_count < JUNCTION_CNT)
                {
                    uint8_t cmd = route[target][path_idx];

                    if (cmd != TURN_STRAIGHT)
                    {
                        timer = now;
                        state = TURNING;
                        prev_count = sensor_count;   
                        break;
                    }

                    path_idx++;
                    timer = now + 1500;
                }

                motor_set_speed(left, right);
                prev_count = sensor_count;
                break;

            case TURNING:
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

            case FIND_ROOM:
            {
                if (now < timer)
                {
                    motor_set_speed(left_slow, right_slow);
                    prev_count = sensor_count;
                    break;
                }

                static bool flag = 0;
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

                if (!flag && is_finish_mark())
                {
                    flag = 1;
                    timer = now;
                    finish_line_detected = 0;
                }
                
                motor_set_speed(left_slow, right_slow);
                break;
            }

            case PAUSE:
                if (now - timer >= PAUSE_MS)
                { timer = now; state = TURN_BACK; }
                break;

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

            case RETURN:
                if (now < timer)
                {
                    motor_set_speed(left, right);
                    prev_count = sensor_count;
                    break;
                }

                if (crossed == path_len - 1 )
                    finish_line_detected = 0;

                if (crossed >= path_len)
                {
                    if (is_finish_mark())
                    {
                        if (!finish_return_flag)
                        {
                            led_on();
                            finish_return_flag = 1;
                            finish_timer = now;
                            motor_set_speed(left, right);
                        }
                        else
                        {
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

                if (sensor_count >= JUNCTION_CNT && prev_count < JUNCTION_CNT)
                {
                    uint8_t fwd_cmd = route[target][path_len - 1 - crossed];
                    uint8_t ret_cmd = reverse_turn(fwd_cmd);
                    crossed++;

                    if (ret_cmd != TURN_STRAIGHT)
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

            case RET_TURN:
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





