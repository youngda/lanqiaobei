#include <stdio.h>
#include "stm32f10x.h"
#include "lcd.h"
#include "led.h"
#include "i2c.h"

  #define USARTz                   USART2
  #define USARTz_GPIO              GPIOA
  #define USARTz_CLK               RCC_APB1Periph_USART2
  #define USARTz_GPIO_CLK          RCC_APB2Periph_GPIOA
  #define USARTz_RxPin             GPIO_Pin_3
  #define USARTz_TxPin             GPIO_Pin_2
  #define USARTz_IRQn              USART2_IRQn
  #define USARTz_IRQHandler        USART2_IRQHandler

u32 TimingDelay = 0;
extern u8 adc_flag;
float adc_v;
u8 adc_h;
u8 string[20];
u8 low = 10;
u8 mid = 20;
u8 high = 30;
int level;
u8 r1set = 0;
u8 xs = 0;
u8 r2xs = 0;
u8 r3xs = 4;
u8 r4xs = 4;
void Delay_Ms(u32 nTime);
void LCD_Init(void);
void LED_Init(void);
void LED_Con(u16 LED,u8 status);
void ADC_Con(void);
float ADC_R(void);
void KEY_Scan(void);
void KEY_Init(void);
unsigned char Read(u8 add);
void Write(u8 add,u8 info);
void TX(u8 *str);
extern u8 RX_b[20];
extern u8 C_flag;
extern u8 S_flag;
void Uinit(void);
void adc_dis(void);
u8 UART[20];
int main(void)
{
	SysTick_Config(SystemCoreClock/1000);	
	LCD_Init();
	LED_Init();
	LED_Con(LEDALL,0);
	ADC_Con();
	KEY_Init();
	i2c_init();
	low = Read(0xff);
	Delay_Ms(2);
	mid = Read(0xfe);
	Delay_Ms(2);
	high = Read(0xfd);
	Delay_Ms(2);
	Uinit();
	TX("SB");
	while(1)
	{	
	if(xs == 0)
	{
		if(adc_flag == 1)
		{
			adc_flag = 0;
		    adc_dis();
		}
	}
	if(C_flag == 1){
			C_flag = 0;
			sprintf((char*)UART,"C:H%d+L%d\r\n",adc_h,level);
			TX(UART);
			USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
		}
		else if(S_flag == 1){
			S_flag = 0;
			sprintf((char*)UART,"S:TL%d+TM%d+TH%d\r\n",high,mid,low);
			TX(UART);
			USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
		}
	KEY_Scan();
	}
}
void adc_dis(void)
{
		adc_v = ADC_R();
			adc_h = adc_v * 30.4;
			LCD_SetTextColor(White);
		    LCD_DisplayStringLine(Line1,"    Liquid Level    ");
			sprintf((char*)string,"%s%d%s","  Height: ",adc_h,"cm       ");
			LCD_DisplayStringLine(Line3,string);
			sprintf((char*)string,"%s%.2f%s","  ADC: ",adc_v,"V         ");
			LCD_DisplayStringLine(Line5,string);
			if(adc_h<=low)
			{
				if(level == 1)
				{
					sprintf((char*)UART,"A:H%d+L0+D\r\n",adc_h);
					TX(UART);
				}
			    level = 0;
			}
			else if(adc_h <=mid)
			{
				if(level == 2)
				{
					sprintf((char*)UART,"A:H%d+L1+D\r\n",adc_h);
					TX(UART);
				}
				else if(level == 0)
				{
					sprintf((char*)UART,"A:H%d+L1+U\r\n",adc_h);
					TX(UART);
				}
				level  = 1;
			}
			else if(adc_h <=high)
			{
				if(level == 3)
				{
					sprintf((char*)UART,"A:H%d+L2+D\r\n",adc_h);
					TX(UART);
				}
				else if(level == 1)
				{
					sprintf((char*)UART,"A:H%d+L2+U\r\n",adc_h);
					TX(UART);
				}
				level  = 2;
			}
			else
			{
				if(level == 2)
				{
					sprintf((char*)UART,"A:H%d+L3+U\r\n",adc_h);
					TX(UART);
				}
				level  = 3;
			}
			sprintf((char*)string,"%s%d%s","  Level: ",level,"        ");
			LCD_DisplayStringLine(Line7,string);
}
void Uinit(void)
{
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure;
  RCC_APB2PeriphClockCmd(USARTz_GPIO_CLK, ENABLE);
  RCC_APB1PeriphClockCmd(USARTz_CLK, ENABLE);
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USARTz_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable the USARTz Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USARTz_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  /* Configure USARTy */
  /* Configure USARTz */
  USART_Init(USARTz, &USART_InitStructure);
 
  /* Enable USARTz Receive and Transmit interrupts */
  USART_ITConfig(USARTz, USART_IT_RXNE, ENABLE);

  /* Enable the USARTz */
  USART_Cmd(USARTz, ENABLE);

  GPIO_InitStructure.GPIO_Pin = USARTz_RxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USARTz_GPIO, &GPIO_InitStructure);
   
  
  /* Configure USARTy Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = USARTz_TxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USARTz_GPIO, &GPIO_InitStructure);  
}
void TX(u8 *str)
{
  u8 index = 0;
  do
  {
    USART_SendData(USART2, str[index]);
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET); 
	index++;         
  }
  while(str[index] != 0);

}
void Write(u8 add,u8 info)
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(add);
	I2CWaitAck();
	I2CSendByte(info);
	I2CWaitAck();
	I2CStop();
}
unsigned char Read(u8 add)
{
	unsigned char temp;
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(add);
	I2CWaitAck();
	
	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	temp = I2CReceiveByte();
	I2CWaitAck();
	I2CStop();
	return temp;		
} 
void KEY_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOB, &GPIO_InitStructure);	
}
void KEY_Scan(void)
{
	if(RB1 == 0)
	{
		Delay_Ms(10);
		if(RB1 == 0)
		{
			if(r1set == 0)
			{
				xs = 1;
				r1set = 1;
				LCD_SetTextColor(White);
			    LCD_DisplayStringLine(Line1,"   Parameter Setup    ");
				sprintf((char*)string,"%s%d%s","Threshold 1: ",low,"cm        ");
				LCD_DisplayStringLine(Line3,string);
				sprintf((char*)string,"%s%d%s","Threshold 2: ",mid,"cm        ");
				LCD_DisplayStringLine(Line5,string);
				sprintf((char*)string,"%s%d%s","Threshold 3: ",high,"cm       ");
				LCD_DisplayStringLine(Line7,string);
			}
			else
			{
				xs = 0;
				r1set = 0;		
			}
		}
		while(!RB1);
	}
	else if(RB2 == 0)
	{
		Delay_Ms(10);
		if(RB2 == 0)
		{
			if(r1set == 1)
			{
			   if(r2xs == 0)
			   {
				    LCD_DisplayStringLine(Line1,"   Parameter Setup    ");
					LCD_SetTextColor(Red);
					sprintf((char*)string,"%s%d%s","Threshold 1: ",low,"cm        ");
					LCD_DisplayStringLine(Line3,string);
					LCD_SetTextColor(White);
					sprintf((char*)string,"%s%d%s","Threshold 2: ",mid,"cm        ");
					LCD_DisplayStringLine(Line5,string);
					sprintf((char*)string,"%s%d%s","Threshold 3: ",high,"cm       ");
					LCD_DisplayStringLine(Line7,string);
					r3xs = 0;
					r4xs = 0;
					r2xs++;	
			   }
			   else if(r2xs == 1)
			   {	   	
				    LCD_DisplayStringLine(Line1,"   Parameter Setup    ");
					sprintf((char*)string,"%s%d%s","Threshold 1: ",low,"cm        ");
					LCD_DisplayStringLine(Line3,string);
					LCD_SetTextColor(Red);
					sprintf((char*)string,"%s%d%s","Threshold 2: ",mid,"cm        ");
					LCD_DisplayStringLine(Line5,string);
					LCD_SetTextColor(White);
					sprintf((char*)string,"%s%d%s","Threshold 3: ",high,"cm       ");
					LCD_DisplayStringLine(Line7,string);
					r3xs = 1;
					r4xs = 1;
					r2xs++;	
			   }
			   else if(r2xs == 2)
			   {	   	
				    LCD_DisplayStringLine(Line1,"   Parameter Setup    ");
					sprintf((char*)string,"%s%d%s","Threshold 1: ",low,"cm        ");
					LCD_DisplayStringLine(Line3,string);
					sprintf((char*)string,"%s%d%s","Threshold 2: ",mid,"cm        ");
					LCD_DisplayStringLine(Line5,string);
					LCD_SetTextColor(Red);
					sprintf((char*)string,"%s%d%s","Threshold 3: ",high,"cm       ");
					LCD_DisplayStringLine(Line7,string);
					LCD_SetTextColor(White);
					r3xs = 2;
					r4xs = 2;
					r2xs++;
					if(r2xs>=3)r2xs = 0;	
			   }
			}
			else
			{
					r3xs = 4;
					r4xs = 4;
			}
		}
	while(!RB2);
	}
	else if(RB3 == 0)
	{
		Delay_Ms(10);
		if(RB3 == 0)
		{	
			if(r1set == 1)
			{
				if(r3xs == 0)
				{
					low = low+5;
					if(low >= 95)low = 95;
					LCD_DisplayStringLine(Line1,"   Parameter Setup    ");
					LCD_SetTextColor(Red);
					sprintf((char*)string,"%s%d%s","Threshold 1: ",low,"cm        ");
					LCD_DisplayStringLine(Line3,string);
					LCD_SetTextColor(White);
					sprintf((char*)string,"%s%d%s","Threshold 2: ",mid,"cm        ");
					LCD_DisplayStringLine(Line5,string);
					sprintf((char*)string,"%s%d%s","Threshold 3: ",high,"cm       ");
					LCD_DisplayStringLine(Line7,string);		
				}
				else if(r3xs == 1)
				{
					mid = mid+5;
					if(mid >= 95)mid = 95;
				    LCD_DisplayStringLine(Line1,"   Parameter Setup    ");
					sprintf((char*)string,"%s%d%s","Threshold 1: ",low,"cm        ");
					LCD_DisplayStringLine(Line3,string);
					LCD_SetTextColor(Red);
					sprintf((char*)string,"%s%d%s","Threshold 2: ",mid,"cm        ");
					LCD_DisplayStringLine(Line5,string);
					LCD_SetTextColor(White);
					sprintf((char*)string,"%s%d%s","Threshold 3: ",high,"cm       ");
					LCD_DisplayStringLine(Line7,string);	
				}
				else if(r3xs == 2)
				{
					high = high+5;
					if(high >= 95)high = 95;
					LCD_DisplayStringLine(Line1,"   Parameter Setup    ");
					sprintf((char*)string,"%s%d%s","Threshold 1: ",low,"cm        ");
					LCD_DisplayStringLine(Line3,string);
					sprintf((char*)string,"%s%d%s","Threshold 2: ",mid,"cm        ");
					LCD_DisplayStringLine(Line5,string);
					LCD_SetTextColor(Red);
					sprintf((char*)string,"%s%d%s","Threshold 3: ",high,"cm       ");
					LCD_DisplayStringLine(Line7,string);
					LCD_SetTextColor(White);				
				}
			}	
		}
	Write(0xff,low);
	Delay_Ms(2);
	Write(0xfe,mid);
	Delay_Ms(2);
	Write(0xfd,high);
	Delay_Ms(2);
	while(!RB3);
	}
	else if(RB4 == 0)
	{
		Delay_Ms(10);
		if(RB4 == 0)
		{
			if(r1set == 1)
			{
				if(r4xs == 0)
				{
					low = low-5;
					if(low <= 5)low = 5;
					LCD_DisplayStringLine(Line1,"   Parameter Setup    ");
					LCD_SetTextColor(Red);
					sprintf((char*)string,"%s%d%s","Threshold 1: ",low,"cm        ");
					LCD_DisplayStringLine(Line3,string);
					LCD_SetTextColor(White);
					sprintf((char*)string,"%s%d%s","Threshold 2: ",mid,"cm        ");
					LCD_DisplayStringLine(Line5,string);
					sprintf((char*)string,"%s%d%s","Threshold 3: ",high,"cm       ");
					LCD_DisplayStringLine(Line7,string);						
				}
				else if(r4xs == 1)
				{
					mid = mid-5;
					if(mid <= 5)mid = 5;
					LCD_DisplayStringLine(Line1,"   Parameter Setup    ");
					sprintf((char*)string,"%s%d%s","Threshold 1: ",low,"cm        ");
					LCD_DisplayStringLine(Line3,string);
					LCD_SetTextColor(Red);
					sprintf((char*)string,"%s%d%s","Threshold 2: ",mid,"cm        ");
					LCD_DisplayStringLine(Line5,string);
					LCD_SetTextColor(White);
					sprintf((char*)string,"%s%d%s","Threshold 3: ",high,"cm       ");
					LCD_DisplayStringLine(Line7,string);			
				}
				else if(r4xs == 2)
				{
					high = high - 5;
					if(high <= 5)high = 5;
					LCD_DisplayStringLine(Line1,"   Parameter Setup    ");
					sprintf((char*)string,"%s%d%s","Threshold 1: ",low,"cm        ");
					LCD_DisplayStringLine(Line3,string);
					sprintf((char*)string,"%s%d%s","Threshold 2: ",mid,"cm        ");
					LCD_DisplayStringLine(Line5,string);
					LCD_SetTextColor(Red);
					sprintf((char*)string,"%s%d%s","Threshold 3: ",high,"cm       ");
					LCD_DisplayStringLine(Line7,string);
					LCD_SetTextColor(White);				
				}
			}	
		}
	Write(0xff,low);
	Delay_Ms(2);
	Write(0xfe,mid);
	Delay_Ms(2);
	Write(0xfd,high);
	Delay_Ms(2);	
	while(!RB4);
	}
}
float ADC_R(void)
{
	float temp;
	temp = ADC_GetConversionValue(ADC1);
	Delay_Ms(5);
	temp = temp*3.3/0xfff;
	return temp;
}
void ADC_Con(void)
{
  ADC_InitTypeDef ADC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOB, ENABLE);	
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel14 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_13Cycles5);

  /* Configure high and low analog watchdog thresholds */
  ADC_AnalogWatchdogThresholdsConfig(ADC1, 0x0B00, 0x0300);
  /* Configure channel14 as the single analog watchdog guarded channel */
  ADC_AnalogWatchdogSingleChannelConfig(ADC1, ADC_Channel_14);
  /* Enable analog watchdog on one regular channel */
  ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_SingleRegEnable);

  /* Enable AWD interrupt */
  ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE);

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
void LED_Con(u16 LED,u8 status)
{
	if(status == 0)
	{
		GPIO_SetBits(GPIOC, LED);
		GPIO_SetBits(GPIOD, GPIO_Pin_2);
		GPIO_ResetBits(GPIOD,GPIO_Pin_2);
	}
	else
	{
		GPIO_ResetBits(GPIOC, LED);
		GPIO_SetBits(GPIOD, GPIO_Pin_2);
		GPIO_ResetBits(GPIOD,GPIO_Pin_2);	
	}	
}
void LED_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
  GPIO_InitStructure.GPIO_Pin = LEDALL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}
void LCD_Init(void)
{
	STM3210B_LCD_Init();
	LCD_Clear(Blue);
	LCD_SetBackColor(Blue);
	LCD_SetTextColor(White);	
	LCD_DisplayStringLine(Line0,"                    ");
	LCD_DisplayStringLine(Line1,"                    ");	
	LCD_DisplayStringLine(Line2,"                    ");
	LCD_DisplayStringLine(Line3,"                    ");
	LCD_DisplayStringLine(Line4,"                    ");						
	LCD_DisplayStringLine(Line5,"                    ");	
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

