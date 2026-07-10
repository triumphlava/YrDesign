#include "grayscale_sensor.h"

static void grayscale_sensor_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);	
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//澶嶇敤鎺ㄦ尳杈撳嚭
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_WriteBit(GPIOB,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5,Bit_RESET);
}

static void grayscale_sensor_adc_init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE); // 浣胯兘GPIOA鍜孉DC1鏃堕挓
	RCC_ADCCLKConfig(RCC_PCLK2_Div4); // 璁剧疆ADC鍒嗛鍥犲瓙涓?

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; // 閰嶇疆PA0涓烘ā鎷熻緭鍏?
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	ADC_DeInit(ADC1); // 澶嶄綅ADC1
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; // 鐙珛妯″紡
	ADC_InitStructure.ADC_ScanConvMode = DISABLE; // 鍗曢€氶亾妯″紡
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // 鍗曟杞崲妯″紡
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 杞欢瑙﹀彂
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // 鏁版嵁鍙冲榻?
	ADC_InitStructure.ADC_NbrOfChannel = 1; // 瑙勫垯杞崲閫氶亾鏁扮洰
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_Cmd(ADC1, ENABLE); // 浣胯兘ADC1
	ADC_ResetCalibration(ADC1); // 澶嶄綅鏍″噯
	while (ADC_GetResetCalibrationStatus(ADC1)); // 绛夊緟澶嶄綅鏍″噯缁撴潫
	ADC_StartCalibration(ADC1); // 寮€鍚牎鍑?
	while (ADC_GetCalibrationStatus(ADC1)); // 绛夊緟鏍″噯缁撴潫
}

void grayscale_bsp_init(void)
{
    grayscale_sensor_gpio_init();
    grayscale_sensor_adc_init();
}

unsigned short adc_getValue()
{
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5); // 閰嶇疆ADC閫氶亾
	ADC_SoftwareStartConvCmd(ADC1, ENABLE); // 鍚姩杞崲
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)); // 绛夊緟杞崲缁撴潫
	return ADC_GetConversionValue(ADC1); // 杩斿洖杞崲缁撴灉
}
