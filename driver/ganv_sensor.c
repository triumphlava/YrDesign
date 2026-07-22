#include "ganv_sensor.h"

void Get_Analog_value(unsigned short *result)
{
    unsigned char i,j;
    unsigned int Anolag=0;
    
    for(i=0;i<8;i++)
    {
        Switch_Address_0(!(i&0x01));
        Switch_Address_1(!(i&0x02));
        Switch_Address_2(!(i&0x04));

        for(j=0;j<8;j++)
        {
            Anolag+=Get_adc_of_user();
        }
				if(!Direction)result[i]=Anolag/8;
        else result[7-i]=Anolag/8;
        Anolag=0;
    }
}

void convertAnalogToDigital(unsigned short *adc_value,unsigned short *Gray_white,unsigned short *Gray_black,unsigned char *Digital)
{
    for (int i = 0; i < 8; i++) {
        if (adc_value[i] > Gray_white[i]) {
            *Digital |= (1 << i);
        } else if (adc_value[i] < Gray_black[i]) {
            *Digital &= ~(1 << i);
        }
    }
}

void normalizeAnalogValues(unsigned short *adc_value,double *Normal_factor,unsigned short *Calibrated_black,unsigned short *result,double bits)
{
    for (int i = 0; i < 8; i++) {
        unsigned short n ;
        if(adc_value[i]<Calibrated_black[i]) n=0;
        else n = (adc_value[i] - Calibrated_black[i]) * Normal_factor[i];

        if (n > bits) {
            n = bits;
        }
        result[i]=n;
    }
}

void No_MCU_Ganv_Sensor_Init_Frist(No_MCU_Sensor*sensor)
{
    memset(sensor->Calibrated_black,0,16);
    memset(sensor->Calibrated_white,0,16);
    memset(sensor->Normal_value,0,16);
    memset(sensor->Analog_value,0,16);
    
    for(int i = 0; i < 8; i++)
    {
        sensor->Normal_factor[i]=0.0;
    }
    
    sensor->Digtal=0;
    sensor->Time_out=0;
    sensor->Tick=0;
    sensor->ok=0;
}

void No_MCU_Ganv_Sensor_Init(No_MCU_Sensor*sensor,unsigned short *Calibrated_white,unsigned short *Calibrated_black)
{
    No_MCU_Ganv_Sensor_Init_Frist(sensor);
    
    if(Sensor_ADCbits==_8Bits)sensor->bits=255.0;
    else if(Sensor_ADCbits==_10Bits)sensor->bits=1024.0;
    else if(Sensor_ADCbits==_12Bits)sensor->bits=4096.0;
    else if(Sensor_ADCbits==_14Bits)sensor->bits=16384.0;

    if(Sensor_Edition==Class)sensor->Time_out=1;
    else sensor->Time_out=10;

    double Normal_Diff[8];
    unsigned short temp;
    
    for (int i = 0; i < 8; i++)
    {
        if(Calibrated_black[i]>=Calibrated_white[i])
        {
            temp=Calibrated_white[i];
            Calibrated_white[i]=Calibrated_black[i];
            Calibrated_black[i]=temp;
        }

        sensor->Gray_white[i]=(Calibrated_white[i]*2+Calibrated_black[i])/3;
        sensor->Gray_black[i]=(Calibrated_white[i]+Calibrated_black[i]*2)/3;

        sensor->Calibrated_black[i]=Calibrated_black[i];
        sensor->Calibrated_white[i]=Calibrated_white[i];

        if ((Calibrated_white[i] == 0 && Calibrated_black[i] == 0)||
            (Calibrated_white[i]==Calibrated_black[i]))
        {
            sensor->Normal_factor[i] = 0.0;
            continue;
        }
        
        Normal_Diff[i] = (double)Calibrated_white[i] - (double)Calibrated_black[i];
        sensor->Normal_factor[i] = sensor->bits / Normal_Diff[i];
    }
    sensor->ok=1;
}

void No_Mcu_Ganv_Sensor_Task_Without_tick(No_MCU_Sensor*sensor)
{
    Get_Analog_value(sensor->Analog_value);
    convertAnalogToDigital(sensor->Analog_value, sensor->Gray_white,sensor->Gray_black,&sensor->Digtal);
    normalizeAnalogValues(sensor->Analog_value,  sensor->Normal_factor,sensor->Calibrated_black,sensor->Normal_value,sensor->bits);
}

void No_Mcu_Ganv_Sensor_Task_With_tick(No_MCU_Sensor*sensor)
{
    if(sensor->Tick>=sensor->Time_out)
    {
        Get_Analog_value(sensor->Analog_value);
        convertAnalogToDigital(sensor->Analog_value,sensor->Gray_white,sensor->Gray_black,&sensor->Digtal);
        normalizeAnalogValues(sensor->Analog_value,sensor->Normal_factor,sensor->Calibrated_black,sensor->Normal_value,sensor->bits);  
        sensor->Tick=0;
    }
}

void Task_tick(No_MCU_Sensor*sensor)
{
    sensor->Tick++;
}

unsigned char Get_Digtal_For_User(No_MCU_Sensor*sensor)
{
    return sensor->Digtal;
}

unsigned char Get_Normalize_For_User(No_MCU_Sensor*sensor,unsigned short* result)
{
    if(!sensor->ok)return 0;
    else 
    {
        memcpy(result,sensor->Normal_value,16);
        return 1;     
    }
}

unsigned char Get_Anolog_Value(No_MCU_Sensor*sensor,unsigned short *result)
{   
    Get_Analog_value(sensor->Analog_value);
    memcpy(result,sensor->Analog_value,16);
    return 1;     
}


