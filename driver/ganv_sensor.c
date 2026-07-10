#include "ganv_sensor.h"

/* 鍑芥暟鍔熻兘锛氶噰闆?涓€氶亾鐨勬ā鎷熷€煎苟杩涜鍧囧€兼护娉?
   鍙傛暟璇存槑锛歳esult - 瀛樺偍8涓€氶亾澶勭悊缁撴灉鐨勬暟缁?*/
void Get_Analog_value(unsigned short *result)
{
    unsigned char i,j;
    unsigned int Anolag=0;
    
    // 閬嶅巻8涓紶鎰熷櫒閫氶亾锛?浣嶅湴鍧€绾跨粍鍚堬級
    for(i=0;i<8;i++)
    {
        // 閫氳繃鍦板潃绾跨粍鍚堝垏鎹紶鎰熷櫒閫氶亾锛堟敞鎰忓彇鍙嶉€昏緫锛?
        Switch_Address_0(!(i&0x01));  // 鍦板潃绾?锛屽搴攂it0
        Switch_Address_1(!(i&0x02));  // 鍦板潃绾?锛屽搴攂it1
        Switch_Address_2(!(i&0x04));  // 鍦板潃绾?锛屽搴攂it2

        // 姣忎釜閫氶亾閲囬泦8娆DC鍊艰繘琛屽潎鍊兼护娉?
        for(j=0;j<8;j++)
        {
            Anolag+=Get_adc_of_user();  // 绱姞ADC閲囨牱鍊?
        }
				if(!Direction)result[i]=Anolag/8;  // 璁＄畻骞冲潎鍊?
        else result[7-i]=Anolag/8;  // 璁＄畻骞冲潎鍊?
        Anolag=0;  // 閲嶇疆绱姞鍣?
    }
}

/* 鍑芥暟鍔熻兘锛氬皢妯℃嫙鍊艰浆鎹负鏁板瓧淇″彿锛堜簩鍊煎寲澶勭悊锛?
   鍙傛暟璇存槑锛?
   adc_value - 鍘熷ADC鍊兼暟缁?
   Gray_white - 鐧借壊闃堝€兼暟缁?
   Gray_black - 榛戣壊闃堝€兼暟缁?
   Digital - 杈撳嚭鐨勬暟瀛椾俊鍙凤紙鎸変綅琛ㄧず锛?*/
void convertAnalogToDigital(unsigned short *adc_value,unsigned short *Gray_white,unsigned short *Gray_black,unsigned char *Digital)
{
    for (int i = 0; i < 8; i++) {
        if (adc_value[i] > Gray_white[i]) {
            *Digital |= (1 << i);   // 瓒呰繃鐧介槇鍊肩疆1锛堢櫧鑹诧級
        } else if (adc_value[i] < Gray_black[i]) {
            *Digital &= ~(1 << i);  // 浣庝簬榛戦槇鍊肩疆0锛堥粦鑹诧級
        }
        // 涓棿鐏板害鍊间繚鎸佸師鏈夌姸鎬?
    }
}

/* 鍑芥暟鍔熻兘锛氬綊涓€鍖朅DC鍊煎埌鎸囧畾鑼冨洿
   鍙傛暟璇存槑锛?
   adc_value - 鍘熷ADC鍊兼暟缁?
   Normal_factor - 褰掍竴鍖栫郴鏁版暟缁?
   Calibrated_black - 鏍″噯榛戝€兼暟缁?
   result - 瀛樺偍褰掍竴鍖栫粨鏋滅殑鏁扮粍
   bits - ADC鏈€澶ч噺绋嬪€硷紙濡?55/1024绛夛級 */
void normalizeAnalogValues(unsigned short *adc_value,double *Normal_factor,unsigned short *Calibrated_black,unsigned short *result,double bits)
{
    for (int i = 0; i < 8; i++) {
        unsigned short n ;
        // 璁＄畻褰掍竴鍖栧€硷紙鍑忓幓榛戠數骞冲悗缂╂斁锛?
        if(adc_value[i]<Calibrated_black[i]) n=0;  // 浣庝簬榛戠數骞冲綊闆?
        else n = (adc_value[i] - Calibrated_black[i]) * Normal_factor[i];

        // 闄愬箙澶勭悊
        if (n > bits) {
            n = bits;
        }
        result[i]=n;
    }
}

/* 鍑芥暟鍔熻兘锛氫紶鎰熷櫒缁撴瀯浣撳垵濮嬪寲锛堥娆″垵濮嬪寲锛?
   鍙傛暟璇存槑锛歴ensor - 浼犳劅鍣ㄧ粨鏋勪綋鎸囬拡 */
void No_MCU_Ganv_Sensor_Init_Frist(No_MCU_Sensor*sensor)
{
    // 娓呴浂鎵€鏈夋牎鍑嗘暟鎹拰鐘舵€?
    memset(sensor->Calibrated_black,0,16);
    memset(sensor->Calibrated_white,0,16);
    memset(sensor->Normal_value,0,16);
    memset(sensor->Analog_value,0,16);
    
    // 鍒濆鍖栧綊涓€鍖栫郴鏁?
    for(int i = 0; i < 8; i++)
    {
        sensor->Normal_factor[i]=0.0;
    }
    
    // 鍒濆鍖栫姸鎬佸彉閲?
    sensor->Digtal=0;
    sensor->Time_out=0;
    sensor->Tick=0;
    sensor->ok=0;  // 鏍囪鏈畬鎴愭牎鍑?
}

/* 鍑芥暟鍔熻兘锛氫紶鎰熷櫒瀹屾暣鍒濆鍖栵紙甯︽牎鍑嗗弬鏁帮級
   鍙傛暟璇存槑锛?
   sensor - 浼犳劅鍣ㄧ粨鏋勪綋鎸囬拡
   Calibrated_white - 鏍″噯鐧藉€兼暟缁?
   Calibrated_black - 鏍″噯榛戝€兼暟缁?*/
void No_MCU_Ganv_Sensor_Init(No_MCU_Sensor*sensor,unsigned short *Calibrated_white,unsigned short *Calibrated_black)
{
    No_MCU_Ganv_Sensor_Init_Frist(sensor);
    
    // 鏍规嵁閰嶇疆璁剧疆ADC閲忕▼
    if(Sensor_ADCbits==_8Bits)sensor->bits=255.0;
    else if(Sensor_ADCbits==_10Bits)sensor->bits=1024.0;
    else if(Sensor_ADCbits==_12Bits)sensor->bits=4096.0;
    else if(Sensor_ADCbits==_14Bits)sensor->bits=16384.0;

    // 璁剧疆閲囨牱瓒呮椂鏃堕棿
    if(Sensor_Edition==Class)sensor->Time_out=1;
    else sensor->Time_out=10;

    double Normal_Diff[8];
    unsigned short temp;
    
    for (int i = 0; i < 8; i++)
    {
        // 纭繚鐧藉€?> 榛戝€硷紙蹇呰鏃朵氦鎹級
        if(Calibrated_black[i]>=Calibrated_white[i])
        {
            temp=Calibrated_white[i];
            Calibrated_white[i]=Calibrated_black[i];
            Calibrated_black[i]=temp;
        }

        // 璁＄畻鐏板害闃堝€硷紙1:2鍜?:1鍒嗙晫鐐癸級
        sensor->Gray_white[i]=(Calibrated_white[i]*2+Calibrated_black[i])/3;
        sensor->Gray_black[i]=(Calibrated_white[i]+Calibrated_black[i]*2)/3;

        // 淇濆瓨鏍″噯鏁版嵁
        sensor->Calibrated_black[i]=Calibrated_black[i];
        sensor->Calibrated_white[i]=Calibrated_white[i];

        // 澶勭悊鏃犳晥鏍″噯鏁版嵁锛堝叏榛?鍏ㄧ櫧/鐩哥瓑鎯呭喌锛?
        if ((Calibrated_white[i] == 0 && Calibrated_black[i] == 0)||
            (Calibrated_white[i]==Calibrated_black[i]))
        {
            sensor->Normal_factor[i] = 0.0;  // 鏃犳晥閫氶亾
            continue;
        }
        
        // 璁＄畻褰掍竴鍖栫郴鏁?
        Normal_Diff[i] = (double)Calibrated_white[i] - (double)Calibrated_black[i];
        sensor->Normal_factor[i] = sensor->bits / Normal_Diff[i];
    }
    sensor->ok=1;  // 鏍囪鍒濆鍖栧畬鎴?
}

/* 鍑芥暟鍔熻兘锛氫紶鎰熷櫒涓讳换鍔★紙鏃犲畾鏃跺櫒鐗堟湰锛?*/
void No_Mcu_Ganv_Sensor_Task_Without_tick(No_MCU_Sensor*sensor)
{
    Get_Analog_value(sensor->Analog_value);  // 閲囬泦鏁版嵁
    convertAnalogToDigital(sensor->Analog_value, sensor->Gray_white,sensor->Gray_black,&sensor->Digtal);// 浜屽€煎寲澶勭悊
    normalizeAnalogValues(sensor->Analog_value,  sensor->Normal_factor,sensor->Calibrated_black,sensor->Normal_value,sensor->bits);// 褰掍竴鍖栧鐞?
}

/* 鍑芥暟鍔熻兘锛氫紶鎰熷櫒涓讳换鍔★紙甯﹀畾鏃跺櫒鐗堟湰锛?*/
void No_Mcu_Ganv_Sensor_Task_With_tick(No_MCU_Sensor*sensor)
{
    if(sensor->Tick>=sensor->Time_out)  // 妫€鏌ユ槸鍚﹀埌杈鹃噰鏍峰懆鏈?
    {
        // 鎵ц鏁版嵁閲囬泦鍜屽鐞?
        Get_Analog_value(sensor->Analog_value);
        convertAnalogToDigital(sensor->Analog_value,sensor->Gray_white,sensor->Gray_black,&sensor->Digtal);
        normalizeAnalogValues(sensor->Analog_value,sensor->Normal_factor,sensor->Calibrated_black,sensor->Normal_value,sensor->bits);  
        sensor->Tick=0;  // 閲嶇疆瀹氭椂鍣?
    }
}

	/* 鍑芥暟鍔熻兘锛氬畾鏃跺櫒tick閫掑 */
void Task_tick(No_MCU_Sensor*sensor)
{
    sensor->Tick++;
}

/* 鍑芥暟鍔熻兘锛氳幏鍙栨暟瀛椾俊鍙风姸鎬?*/
unsigned char Get_Digtal_For_User(No_MCU_Sensor*sensor)
{
    return sensor->Digtal;  // 杩斿洖8浣嶆暟瀛楃姸鎬侊紙姣忎綅瀵瑰簲涓€涓紶鎰熷櫒锛?
}

/* 鍑芥暟鍔熻兘锛氳幏鍙栧綊涓€鍖栧悗鐨勬暟鎹?
   杩斿洖鍊硷細1-鎴愬姛 0-鏈垵濮嬪寲 */
unsigned char Get_Normalize_For_User(No_MCU_Sensor*sensor,unsigned short* result)
{
    if(!sensor->ok)return 0;
    else 
    {
        memcpy(result,sensor->Normal_value,16);  // 鎷疯礉褰掍竴鍖栨暟鎹?
        return 1;     
    }
}

/* 鍑芥暟鍔熻兘锛氳幏鍙栧師濮嬫牎鍑嗘暟鎹?
   杩斿洖鍊硷細1-鎴愬姛 0-鏈垵濮嬪寲 */
unsigned char Get_Anolog_Value(No_MCU_Sensor*sensor,unsigned short *result)
{   

    Get_Analog_value(sensor->Analog_value);  // 閲嶆柊閲囬泦鏁版嵁
    memcpy(result,sensor->Analog_value,16);
    return 1;     
}

