#include "sensor.h"
#include "ganv_sensor.h"
#include "grayscale_sensor.h"


static No_MCU_Sensor sensor;

/* 前人的校准值 */
static const unsigned short white_ref[8] = {3051, 3216, 3290, 3249, 3217, 3013, 3216, 2758};
static const unsigned short black_ref[8] = {135, 996, 1202, 1416, 1416, 896, 1593, 353};

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
