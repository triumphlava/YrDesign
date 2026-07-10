#ifndef GANV_SENSOR_H_
#define GANV_SENSOR_H_
#include <string.h>
#include "stm32f10x.h" 
#include "grayscale_sensor.h"    
/**************************** 锟斤拷锟斤拷锟斤拷锟芥本锟斤拷锟斤拷 ****************************/
#define Class		    0

/**************************** ADC锟街憋拷锟斤拷锟斤拷锟斤拷 ****************************/
#define _14Bits 0     // 14位ADC模式
#define _12Bits 1     // 12位ADC模式
#define _10Bits 2     // 10位ADC模式
#define _8Bits  3     // 8位ADC模式

/**************************** 锟矫伙拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 ***************************/
// 锟斤拷锟斤拷锟斤拷锟芥本选锟斤拷
#define Sensor_Edition Class	


/************************* 锟斤拷锟捷碉拷片锟斤拷锟斤拷锟斤拷选锟斤拷 **************************/
// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷预锟节凤拷锟斤拷同选1
#define Direction 1
// ADC锟街憋拷锟斤拷选锟斤拷锟斤拷选一锟斤拷
// #define Sensor_ADCbits _14Bits
#define Sensor_ADCbits _12Bits
// #define Sensor_ADCbits _10Bits
// #define Sensor_ADCbits _8Bits

// 锟斤拷时锟斤拷锟斤拷锟杰匡拷锟截ｏ拷锟斤拷要时取锟斤拷注锟酵ｏ拷
// #define Use_Timer 1

/*************************** 硬锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟?****************************/
// GPIO锟斤拷址锟叫伙拷锟疥定锟斤拷
#define Switch_Address_2(i) ((i)?(GPIO_WriteBit(GPIOB, GPIO_Pin_5,Bit_SET)) : (GPIO_WriteBit(GPIOB, GPIO_Pin_5,Bit_RESET)))// 锟斤拷址位0锟斤拷锟斤拷

#define Switch_Address_1(i) ((i)?(GPIO_WriteBit(GPIOB, GPIO_Pin_4,Bit_SET)) : (GPIO_WriteBit(GPIOB, GPIO_Pin_4,Bit_RESET)))// 锟斤拷址位1锟斤拷锟斤拷

#define Switch_Address_0(i) ((i)?(GPIO_WriteBit(GPIOB, GPIO_Pin_3,Bit_SET)) : (GPIO_WriteBit(GPIOB, GPIO_Pin_3,Bit_RESET)))// 锟斤拷址位2锟斤拷锟斤拷

// ADC值锟斤拷取锟接口宏定锟斤拷 锟斤拷要锟皆硷拷锟斤拷锟捷碉拷片锟斤拷锟斤拷啥锟接ξ伙拷锟斤拷锟紸DC锟斤拷锟斤拷锟斤拷锟斤拷
#define Get_adc_of_user() adc_getValue()  // 锟矫伙拷锟皆讹拷锟斤拷ADC锟斤拷取锟斤拷锟斤拷
/**********************************************************************/

/*************************** 锟斤拷锟斤拷锟斤拷锟斤拷锟捷结构 ***************************/
typedef struct {
    unsigned short Analog_value[8];    // 原始模锟斤拷锟斤拷值
    unsigned short Normal_value[8];   // 锟斤拷一锟斤拷锟斤拷锟街?
    unsigned short Calibrated_white[8]; // 锟斤拷校准锟斤拷准值
    unsigned short Calibrated_black[8]; // 锟斤拷校准锟斤拷准值
    unsigned short Gray_white[8];      // 锟斤拷平锟斤拷叶锟街?
    unsigned short Gray_black[8];      // 锟斤拷平锟斤拷叶锟街?
    double Normal_factor[8];          // 锟斤拷一锟斤拷系锟斤拷
    double bits;                      // ADC锟街憋拷锟绞讹拷应位锟斤拷
    unsigned char Digtal;              // 锟斤拷锟斤拷锟斤拷锟阶刺?
    unsigned char Time_out;            // 锟斤拷时锟斤拷志
    unsigned char Tick;                // 时锟斤拷锟斤拷锟斤拷锟斤拷
    unsigned char ok;                  // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷志
} No_MCU_Sensor;

#ifdef __cplusplus
extern "C" {
#endif

/*************************** 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 *****************************/
// 锟斤拷始锟斤拷锟斤拷锟斤拷
void No_MCU_Ganv_Sensor_Init_Frist(No_MCU_Sensor* sensor); // 锟阶次筹拷始锟斤拷
void No_MCU_Ganv_Sensor_Init(No_MCU_Sensor* sensor,unsigned short* Calibrated_white, unsigned short* Calibrated_black);// 锟斤拷校准锟斤拷锟斤拷锟侥筹拷始锟斤拷
#ifndef Use_Timer
// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
void No_Mcu_Ganv_Sensor_Task_Without_tick(No_MCU_Sensor* sensor); // 锟斤拷时锟斤拷锟芥本
//锟斤拷锟金：猴拷锟斤拷使锟矫讹拷时锟斤拷锟结供锟斤拷锟斤拷要一锟斤拷1ms锟斤拷准锟侥讹拷时锟斤拷锟斤拷锟斤拷Task_tick(&sensor)锟斤拷锟斤拷锟斤拷锟劫斤拷锟斤拷锟斤拷No_Mcu_Ganv_Sensor_Task_With_tick(&sensor)锟斤拷锟斤拷while1
//前锟斤拷只锟斤拷要锟斤拷while(1)锟斤，delay 1ms锟斤拷锟斤拷锟竭讹拷时锟斤拷1ms锟斤拷锟斤拷
//前锟斤拷锟脚点：锟津单ｏ拷锟斤拷锟姐，锟斤拷锟斤拷锟脚点：锟斤拷锟酵凤拷CPU锟斤拷锟斤拷锟斤拷要通锟斤拷delay锟斤拷锟斤拷锟斤拷锟斤拷锟今，憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷同时锟斤拷锟斤拷锟斤拷锟叫讹拷锟斤处锟斤拷锟铰硷拷锟斤拷锟斤拷驻锟斤拷锟斤拷锟斤拷锟饺硷拷锟斤拷锟斤拷
#else
void No_Mcu_Ganv_Sensor_Task_With_tick(No_MCU_Sensor* sensor);  // 锟斤拷时锟斤拷锟芥本
void Task_tick(No_MCU_Sensor* sensor);                          // 时锟斤拷锟斤拷锟铰猴拷锟斤拷
#endif

// 锟矫伙拷锟接口猴拷锟斤拷
unsigned char Get_Digtal_For_User(No_MCU_Sensor* sensor);          									// 锟斤拷取锟斤拷锟斤拷锟斤拷
unsigned char Get_Normalize_For_User(No_MCU_Sensor* sensor,unsigned short* result); // 锟斤拷取锟斤拷一锟斤拷值
unsigned char Get_Anolog_Value(No_MCU_Sensor* sensor,unsigned short* result);       // 锟斤拷取模锟斤拷值

#ifdef __cplusplus
}
#endif

#endif /* GANV_SENSOR_H_ */

