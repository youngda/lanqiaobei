
#include <stdio.h>
#include "stm32f10x.h"
#include "lcd.h"
#include "led.h"
#include "i2c.h"

u32 TimingDelay = 0;
float adc_v = 0;
u8 adc_flag = 0;
u8 adc_s[20];
u8 rb1s = 1;
u8 rb2s = 0;
u8 khz = 1;
u8 pa[20];
void LCD_init(void);
void Delay_Ms(u32 nTime);
void LED_init(void);
void LED_C(u16 led,u8 status);
void buzz_init(void);
void ADC_init(void);
void ADC_r(void);
void PWM_init(u32 TimerPeriod,u32 Channe);
void key_init(void);
void key_scan(void);
u16 r_v(u16 add);
void w_v(u16 add,u16 data);
int main(void)
{	
	SysTick_Config(SystemCoreClock/1000);
	LCD_init();
	LED_init();
	LED_C(LEDALL,0);
	ADC_init();
	key_init();
	i2c_init();
	khz = r_v(0x01);
	Delay_Ms(5);
	PWM_init(999,600);
	while(1)
	{
		key_scan();
		if(rb2s == 0)
		{
			LCD_DisplayStringLine(Line1,"   value display    ");
			if(adc_flag == 1)
			{
				adc_flag = 0;
				ADC_r();
				if(rb1s == 1)
				{
					PWM_init(1000/khz - 1,1000/khz*adc_v/3.3);
				    LCD_DisplayStringLine(Line5,"OUTPUT: START");
				}
				else
				{
				    LCD_DisplayStringLine(Line5,"OUTPUT: STOP ");
				} 
			}
		}
		else if(rb2s == 1)
		{
			LCD_ClearLine(Line0);
			LCD_ClearLine(Line2);
			LCD_ClearLine(Line3);
			LCD_ClearLine(Line4);
			LCD_ClearLine(Line6);
			LCD_ClearLine(Line7);
			LCD_ClearLine(Line8);
			LCD_ClearLine(Line9);
			LCD_DisplayStringLine(Line1,"     SETTING        "); 	
		    sprintf((char*)pa,"      %dKHz        ",khz);
            LCD_DisplayStringLine(Line5,pa);
		}
	}
}
u16 r_v(u16 add)
{
    u16 temp;
	I2CStart();	
	I2CSendByte(0xa0);	
    I2CWaitAck();
	I2CSendByte(add);	
    I2CWaitAck();
    
	I2CStart();	
	I2CSendByte(0xa1);	
    I2CWaitAck();
    temp  = I2CReceiveByte();
    I2CWaitAck();
	return temp;
}
void w_v(u16 add,u16 data)
{
	I2CStart();	
	I2CSendByte(0xa0);	
    I2CWaitAck();
	I2CSendByte(add);	
    I2CWaitAck();
	I2CSendByte(data);	
    I2CWaitAck();
	I2CStop();
}
void key_scan(void)
{
    TIM_OCInitTypeDef  TIM_OCInitStructure;
	if(RB1 == 0)
	{
		Delay_Ms(10);																										   
		if(RB1 == 0)
		{
			if(rb1s == 0)
			{

				rb1s = 1;
				LCD_DisplayStringLine(Line5,"OUTPUT: START");
                LED_C(LEDALL,0);
				LED_C(LED1,1);
			}
			else if(rb1s == 1)
			{
				LED_C(LEDALL,0);
				rb1s = 0;

				TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
				TIM_OC2Init(TIM1, &TIM_OCInitStructure);
				LCD_DisplayStringLine(Line5,"OUTPUT: STOP ");
			}		
		}
		while(!RB1);
	}
	else if(RB2 == 0)
	{
		Delay_Ms(10);
		if(RB2 == 0)
		{
			if(rb2s == 0)
			{
				rb2s = 1;

			}		
			else if(rb2s == 1)
			{
				rb2s = 0; 
			}
		}
		while(!RB2);
	}
	else if(RB3 == 0)
	{
		Delay_Ms(10);
		if(RB3 == 0)
		{
			if(rb2s == 1)
			{
				khz++;
				if(khz>= 11)
				khz = 1;
				w_v(0x01,khz);
				Delay_Ms(5);
			}		
		}
		while(!RB3);
	}
}
void key_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);	

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}
void PWM_init(u32 TimerPeriod,u32 Channe)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA|
                         RCC_APB2Periph_GPIOB |RCC_APB2Periph_AFIO, ENABLE);
  
  TIM_TimeBaseStructure.TIM_Prescaler = 71;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

  /* Channel 1, 2 and 3 Configuration in PWM mode */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
  TIM_OCInitStructure.TIM_Pulse = Channe;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

  TIM_OC2Init(TIM1, &TIM_OCInitStructure);
  TIM_Cmd(TIM1, ENABLE);
  TIM_CtrlPWMOutputs(TIM1, ENABLE);	

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* GPIOB Configuration: Channel 1N, 2N and 3N as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
 
}
void ADC_r(void)
{
	adc_v = ADC_GetConversionValue(ADC1);
	adc_v = adc_v*3.3/0xfff;
	sprintf((char*)adc_s,"adc_V:%.2fV",adc_v);
	LCD_DisplayStringLine(Line3,adc_s);
	sprintf((char*)pa,"signal_v: PA9: %.0f%%",adc_v/3.3*100);
    LCD_DisplayStringLine(Line6,pa);
    sprintf((char*)pa,"          PB14: %.0f%%",100-adc_v/3.3*100);
    LCD_DisplayStringLine(Line7,pa);
    sprintf((char*)pa,"          %dKHz",khz);
    LCD_DisplayStringLine(Line8,pa);
}
void ADC_init(void)
{
  ADC_InitTypeDef ADC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure; 	
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOB, ENABLE);
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel14 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_55Cycles5);

  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
     
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void LED_C(u16 led,u8 status)
{
	if(status == 0)
	{
		GPIO_SetBits(GPIOC,led);
		GPIO_SetBits(GPIOD,GPIO_Pin_2);
		GPIO_ResetBits(GPIOD,GPIO_Pin_2);
	}
	else if(status == 1)
	{
		GPIO_ResetBits(GPIOC,led);
		GPIO_SetBits(GPIOD,GPIO_Pin_2);
		GPIO_ResetBits(GPIOD,GPIO_Pin_2);
	}
}
void LED_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    /* Configure PD0 and PD2 in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}
void LCD_init(void)
{	
	STM3210B_LCD_Init();
	LCD_Clear(Blue);
	LCD_SetBackColor(Blue);
	LCD_SetTextColor(White);
	LCD_DisplayStringLine(Line0,"                    ");	
	LCD_DisplayStringLine(Line1,"   value display    ");	
	LCD_DisplayStringLine(Line2,"                    ");
	LCD_DisplayStringLine(Line3,"                    ");
	LCD_DisplayStringLine(Line4,"                    ");					
    LCD_DisplayStringLine(Line5,"OUTPUT: START   ");	
	LCD_DisplayStringLine(Line6,"                    ");	
	LCD_DisplayStringLine(Line7,"                    ");	
	LCD_DisplayStringLine(Line8,"                    ");		
	LCD_DisplayStringLine(Line9,"                    ");		
}
void Delay_Ms(u32 nTime)
{
	TimingDelay = nTime;
	while(TimingDelay != 0);	
}
