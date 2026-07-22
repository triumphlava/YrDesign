/**
 * @file    sensor.h
 * @brief   灰度传感器驱动封装 - 头文件
 * @author  YrDesign
 * @note    提供8路灰度传感器的初始化、扫描和校准输出接口
 */

#ifndef __SENSOR_H
#define __SENSOR_H

#include "stm32f10x.h"

/**
 * @brief  传感器校准模式开关
 * @note   = 1：通过串口打印8路模拟量原始值，用于手动校准 white/black_ref
 *         = 0：正常工作模式，输出0/1数字量
 */
#define SENSOR_CALIB  0

/**
 * @brief  传感器初始化
 * @note   完成BSP硬件初始化，传入白/黑参考值，在启动时调用一次
 */
void Sensor_Init(void);

/**
 * @brief  扫描8路传感器，输出数字量结果
 * @param  digital[8] 输出数组，digital[i] = 0(白)/1(黑)
 * @note   在主循环中周期调用，digital[0]对应通道0（最低位）
 */
void Sensor_Scan(uint8_t *digital);

/**
 * @brief  串口输出模拟量校准数据（调试用）
 * @note   仅在 SENSOR_CALIB=1 时生效，打印8路原始AD值供校准参考
 */
void Sensor_CalibOutput(void);

#endif
