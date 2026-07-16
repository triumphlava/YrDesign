#include "sensor.h"
#include "ganv_sensor.h"
#include "grayscale_sensor.h"
#include "usart.h"


static No_MCU_Sensor sensor;

/* 前人的校准值 */
static const unsigned short white_ref[8] = {3182, 3278, 3337, 3301, 3270, 3166, 3285, 2994};
static const unsigned short black_ref[8] = {1196, 1485, 1996, 1814, 1742, 1479, 2212, 1122};

void Sensor_Init(void)
{
    grayscale_bsp_init();
    No_MCU_Ganv_Sensor_Init(&sensor, (unsigned short *)white_ref, (unsigned short *)black_ref);
}

void Sensor_Scan(uint8_t *digital)
{
    No_Mcu_Ganv_Sensor_Task_Without_tick(&sensor);
    unsigned char d = Get_Digtal_For_User(&sensor);

    for (int i = 0; i < 8; i++)
        digital[i] = (d >> i) & 1;
}


#if SENSOR_CALIB
static void print_u16(uint16_t v)
{
    char s[6]; uint8_t i = 4; s[5] = ' ';
    do { s[i--] = '0' + (v % 10); v /= 10; } while (v);
    for (uint8_t j = i + 1; j < 6; j++) usart2_send_byte(s[j]);
}
#endif

void Sensor_CalibOutput(void)
{
#if SENSOR_CALIB
    unsigned short analog[8];
    Get_Anolog_Value(&sensor, analog);

    for (int i = 0; i < 8; i++) print_u16(analog[i]);
    usart2_send_string("\r\n");

    // uint8_t d[8];
    // Sensor_Scan(d);
    // for (int i = 0; i < 8; i++)
    //     usart2_send_byte(((d[i]) & 1) ? '1' : '0');
    // usart2_send_string("\r\n");
#endif
}
