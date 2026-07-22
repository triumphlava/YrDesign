#ifndef GANV_SENSOR_H_
#define GANV_SENSOR_H_
#include <string.h>
#include "stm32f10x.h" 
#include "grayscale_sensor.h"    

/**************************** 算法版本 ****************************/
#define Class		    0

/**************************** ADC精度 ****************************/
#define _14Bits 0
#define _12Bits 1
#define _10Bits 2
#define _8Bits  3

#define Sensor_Edition Class	

/************************* 用户配置 **************************/
#define Direction 1                     // 传感器方向：0正向/1反向
// #define Sensor_ADCbits _14Bits
#define Sensor_ADCbits _12Bits          // ADC精度（12位）
// #define Use_Timer 1                   // 定时器模式

/*************************** 引脚宏定义 ****************************/
#define Switch_Address_2(i) ((i)?(GPIO_WriteBit(GPIOB, GPIO_Pin_5,Bit_SET)) : (GPIO_WriteBit(GPIOB, GPIO_Pin_5,Bit_RESET)))// 地址位2
#define Switch_Address_1(i) ((i)?(GPIO_WriteBit(GPIOB, GPIO_Pin_4,Bit_SET)) : (GPIO_WriteBit(GPIOB, GPIO_Pin_4,Bit_RESET)))// 地址位1
#define Switch_Address_0(i) ((i)?(GPIO_WriteBit(GPIOB, GPIO_Pin_3,Bit_SET)) : (GPIO_WriteBit(GPIOB, GPIO_Pin_3,Bit_RESET)))// 地址位0

#define Get_adc_of_user() adc_getValue()  // ADC读取接口

/*************************** 数据结构 ****************************/
typedef struct {
    unsigned short Analog_value[8];      // 模拟原始值
    unsigned short Normal_value[8];      // 归一化值
    unsigned short Calibrated_white[8];  // 白底校准值
    unsigned short Calibrated_black[8];  // 黑线校准值
    unsigned short Gray_white[8];
    unsigned short Gray_black[8];
    double Normal_factor[8];             // 归一化系数
    double bits;                         // ADC精度位数
    unsigned char Digtal;                // 数字量状态
    unsigned char Time_out;              // 超时标志
    unsigned char Tick;                  // 节拍计数
    unsigned char ok;                    // 初始化标志
} No_MCU_Sensor;

#ifdef __cplusplus
extern "C" {
#endif

/*************************** 接口函数 *****************************/

void No_MCU_Ganv_Sensor_Init_Frist(No_MCU_Sensor* sensor);              // 首次初始化（不含校准值）
void No_MCU_Ganv_Sensor_Init(No_MCU_Sensor* sensor,unsigned short* Calibrated_white, unsigned short* Calibrated_black); // 初始化（含校准值）
#ifndef Use_Timer
void No_Mcu_Ganv_Sensor_Task_Without_tick(No_MCU_Sensor* sensor);       // 任务处理（无定时器）
#else
void No_Mcu_Ganv_Sensor_Task_With_tick(No_MCU_Sensor* sensor);          // 任务处理（有定时器）
void Task_tick(No_MCU_Sensor* sensor);                                  // 1ms节拍函数
#endif

unsigned char Get_Digtal_For_User(No_MCU_Sensor* sensor);                // 获取数字量
unsigned char Get_Normalize_For_User(No_MCU_Sensor* sensor,unsigned short* result); // 获取归一化值
unsigned char Get_Anolog_Value(No_MCU_Sensor* sensor,unsigned short* result);       // 获取模拟值

#ifdef __cplusplus
}
#endif

#endif /* GANV_SENSOR_H_ */

