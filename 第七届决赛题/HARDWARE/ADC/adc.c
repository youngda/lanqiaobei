#include "adc.h"
#include "main.h"


void ADC_INIT(void)
{
		ADC_InitTypeDef ADC_InitStructure;
		GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure PC.02, PC.03 and PC.04 (ADC Channel12, ADC Channel13 and 
     ADC Channel14) as analog inputs */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2 | RCC_APB2Periph_GPIOB, ENABLE);
	
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		
		ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NbrOfChannel = 1;
		ADC_Init(ADC2, &ADC_InitStructure);
		
		ADC_RegularChannelConfig(ADC2, ADC_Channel_8, 1, ADC_SampleTime_28Cycles5); 
	
		 /* Enable ADC1 */
		ADC_Cmd(ADC2, ENABLE);

		/* Enable ADC1 reset calibration register */   
		ADC_ResetCalibration(ADC2);
		/* Check the end of ADC1 reset calibration register */
		while(ADC_GetResetCalibrationStatus(ADC2));

		/* Start ADC1 calibration */
		ADC_StartCalibration(ADC2);
		/* Check the end of ADC1 calibration */
		while(ADC_GetCalibrationStatus(ADC2));
	
}


float Read_adc(void)
{
		float adc_temp;
		ADC_SoftwareStartConvCmd(ADC2, ENABLE);
		//Delay_ms(5);
		adc_temp = ADC_GetConversionValue(ADC2)*3.3/0xfff;
		
		return adc_temp;
}

