/**
 * @file    sensor.c
 * @brief   灰度传感器驱动封装 - 8路模拟量采集与数字量化
 * @author  YrDesign
 * @note    基于无MCU灰度传感器模块，提供初始化、扫描和校准输出功能
 */

#include "sensor.h"
#include "ganv_sensor.h"
#include "grayscale_sensor.h"
#include "usart.h"

/* 传感器实例（静态，不对外暴露） */
static No_MCU_Sensor sensor;

/**
 * @brief  8路灰度传感器校准参考值
 * @note   数组中索引对应传感器通道 0~7
 *         white_ref[] = 白纸反射值（最大值），black_ref[] = 黑线反射值（最小值）
 *         用于归一化计算，需根据实际路面重新校准
 */
static const unsigned short white_ref[8] = {3105, 3202, 3241, 3190, 3166, 3093, 3202, 2819};
static const unsigned short black_ref[8] = {1119, 1518, 1807, 1657, 1322, 1172, 1533, 738};
/**
 * @brief  传感器初始化
 * @note   初始化灰度传感器BSP底层硬件，并传入白/黑参考值完成校准配置
 */
void Sensor_Init(void)
{
    grayscale_bsp_init();                                              /* BSP层硬件初始化 */
    No_MCU_Ganv_Sensor_Init(&sensor, (unsigned short *)white_ref,
                            (unsigned short *)black_ref);              /* 传感器算法初始化 */
}

/**
 * @brief  传感器扫描：读取8路数字量结果
 * @param  digital[8] 输出缓冲区，digital[i] = 0(白)/1(黑)
 * @note   内部调用无MCU传感器的任务处理函数获取8位数字量，
 *         按位拆分为数组，digital[0]对应通道0（最低位）
 */
void Sensor_Scan(uint8_t *digital)
{
    No_Mcu_Ganv_Sensor_Task_Without_tick(&sensor);  /* 传感器算法处理（无硬件定时器模式） */
    unsigned char d = Get_Digtal_For_User(&sensor); /* 获取8位数字量（bit0~bit7 = 通道0~7） */

    for (int i = 0; i < 8; i++)
        digital[i] = (d >> i) & 1;                  /* 拆分为数组，便于逐个判断 */
}


#if SENSOR_CALIB
/**
 * @brief  打印16位数值（调试用，仅在校准模式下编译）
 * @note   将数值格式化为固定宽度5位输出，不足位补前导零
 *         例如：3182 → "03182"
 */
static void print_u16(uint16_t v)
{
    char s[6]; uint8_t i = 4; s[5] = ' ';
    do { s[i--] = '0' + (v % 10); v /= 10; } while (v);
    for (uint8_t j = i + 1; j < 6; j++) usart2_send_byte(s[j]);
}
#endif

/**
 * @brief  传感器校准输出（调试用）
 * @note   通过串口2打印8路模拟量原始值，用于手动校准white_ref/black_ref
 *         需在编译时定义 SENSOR_CALIB=1 才生效
 *         输出格式：每行8个5位数字（如 "03182 03278 ..."）
 */
void Sensor_CalibOutput(void)
{
#if SENSOR_CALIB
    unsigned short analog[8];
    Get_Anolog_Value(&sensor, analog);          /* 获取8路模拟原始值 */

    for (int i = 0; i < 8; i++) print_u16(analog[i]);  /* 逐个打印 */
    usart2_send_string("\r\n");                         /* 换行 */

    /* 如需同时输出数字量结果，取消下面注释 */
    // uint8_t d[8];
    // Sensor_Scan(d);
    // for (int i = 0; i < 8; i++)
    //     usart2_send_byte(((d[i]) & 1) ? '1' : '0');
    // usart2_send_string("\r\n");
#endif
}




