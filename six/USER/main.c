#include <stdio.h>
#include "stm32f10x.h"
#include "lcd.h"
#include "led.h"
#include "I2C.h"
uint32_t HH = 23,MM = 59, SS = 57;
void LCD_Init(void);
u32 TimingDelay = 0;
uint32_t TimeDisplay = 0;
u8 timestr[20];
u32 adc_flag = 0;
float adc_value;
u8 adcstr[20];
u8 k_v = 1;	
u8 led_sw = 1;
u32 adc_time = 0;
extern u8 rx_over;
uint8_t BHH = 0, BMM = 0, BSS = 0;
void Delay_Ms(u32 nTime);
void LED_Init(void);
void LED_c(u16 lED,u8 status);
void RTC_Init(void);
void Time_Display(uint32_t TimeVar);
void ADC_Con(void);
float R_ADC(void);
u8 baojing = 0;
u8 time_set = 0;
void UASRT_Con(void);
void USART_SendString(u8 *str);
void write_c(u8 add,u8 data);
char read_c(u8 add);
void NVIC_Configuration(void);
void key_scan(void);
void LED_shan(void);
int main(void)
{
	SysTick_Config(SystemCoreClock/1000);
	LCD_Init();
	LED_Init();
	RTC_Init();
	LED_c(LEDALL,0);

	ADC_Con();
	UASRT_Con();
	NVIC_Configuration();
    i2c_init();
	k_v =  read_c(0x01);
	Delay_Ms(2);
	USART_SendString("ok\n");
	LED_c(LEDALL,0);
	while(1)
	{	key_scan();
	    LED_shan();
		if(rx_over == 1)
		{
			rx_over = 0;
            sprintf((char*)adcstr,"k :  0.%d",k_v);
			USART_SendString("ok\n");
			write_c(0x01,k_v);
	        Delay_Ms(2);
			USART_ITConfig(USART2, USART_IT_RXNE,ENABLE);
		}		
		if(time_set == 0)
		{
			if (TimeDisplay == 1)
			{
			  /* Display current time */
			  Time_Display(RTC_GetCounter());
			  TimeDisplay = 0;
			}
			if(adc_flag == 1)
			{
				adc_flag = 0;
				adc_value = R_ADC();
				sprintf((char*)adcstr,"V1:  %0.2f",adc_value);
				LCD_DisplayStringLine(Line2,adcstr);
				sprintf((char*)adcstr,"k :  0.%d",k_v);
				LCD_DisplayStringLine(Line4,adcstr);
				if(adc_value > 0.33 * k_v)
				{
					baojing = 1;
				}
				else{baojing = 0;}
	
			}
			
		}
	}
}
void SysTick_Handler(void)
{
	TimingDelay--;
	if(++adc_time == 1000)
	{
		adc_time = 0;
		adc_flag = 1;
	}
}
void LED_shan(void)
{
	LED_c(LEDALL,0);
	if(baojing == 1)
	{
		if(led_sw == 1)
		{
			LED_c(LED1,0);
			Delay_Ms(200);
			LED_c(LED1,1);
			Delay_Ms(200);
		}
		else 
		{
			LED_c(LEDALL,0);	
		}
	}
}

u8 btime[20];
u8 r3flag = 0;
void key_scan(void)
{
	if(RB1 == 0)
	{
		Delay_Ms(10);
		if(RB1 == 0)
		{
			if(time_set == 0)
			{
				if(led_sw == 1)
				{
					led_sw = 0;
					LCD_DisplayStringLine(Line6,"LED: OFF            ");	
				}	
				else if(led_sw == 0)
				{
					led_sw = 1;
					LCD_DisplayStringLine(Line6,"LED: ON               ");	
				}
			}
		}
		while(!RB1);
	}
	else if(RB2 == 0)
	{
		Delay_Ms(10);
		if(RB2 == 0)
		{
			if(time_set == 0)
			{
				time_set = 1;
				LCD_DisplayStringLine(Line0,"                    ");	
				LCD_DisplayStringLine(Line1,"                    ");	
				LCD_DisplayStringLine(Line2,"      Setting       ");
				LCD_DisplayStringLine(Line3,"                    ");
				LCD_DisplayStringLine(Line4,"                    ");
				sprintf((char*)btime,"   %0.2d - %0.2d  - %0.2d  ",BHH, BMM, BSS);					
				LCD_DisplayStringLine(Line5,btime);	
				LCD_DisplayStringLine(Line6,"   ~~               ");	
				LCD_DisplayStringLine(Line7,"                    ");	
				LCD_DisplayStringLine(Line8,"                    ");		
				LCD_DisplayStringLine(Line9,"                    ");
			}
			else if(time_set == 1)
			{
					time_set = 0;
					LCD_DisplayStringLine(Line2,"                    ");
					LCD_DisplayStringLine(Line5,"                    ");
					LCD_DisplayStringLine(Line2,adcstr);
				    LCD_DisplayStringLine(Line4,adcstr);
					if(led_sw == 0)
					{
						LCD_DisplayStringLine(Line6,"LED: OFF             ");	
					}	
					else if(led_sw == 1)
					{
						LCD_DisplayStringLine(Line6,"LED: ON                ");	
					}		
			}		
		}
		while(!RB2);
	}
	else if(RB3 == 0)
	{
		Delay_Ms(10);
		if(RB3 == 0)
		{
			if(time_set == 1)
			{
				r3flag++;
				if(r3flag>= 3)r3flag = 0;
				if(r3flag == 0)
				{
					LCD_DisplayStringLine(Line6,"   ~~               ");	
				}
				else if(r3flag == 1)
				{
					LCD_DisplayStringLine(Line6,"        ~~           ");	
				}
				else if(r3flag == 2)
				{
					LCD_DisplayStringLine(Line6,"              ~~    ");	
				}
			}	
		}
		while(!RB3);
	}
	else if(RB4 == 0)
	{
		Delay_Ms(10);
		if(RB4 == 0)
		{
			if(time_set == 1)
			{
				if(r3flag == 0)
				{

					BHH++;
					if(BHH >= 24)BHH = 0;
					sprintf((char*)btime,"   %0.2d - %0.2d  - %0.2d  ",BHH, BMM, BSS);					
				    LCD_DisplayStringLine(Line5,btime);	
				}
				else if(r3flag == 1)
				{
					BMM++;
					if(BMM >= 60)BMM = 0;
					sprintf((char*)btime,"   %0.2d - %0.2d  - %0.2d  ",BHH, BMM, BSS);					
				    LCD_DisplayStringLine(Line5,btime);	
				}
				else if(r3flag == 2)
				{
					BSS++;
					if(BSS >= 60)BSS = 0;
					sprintf((char*)btime,"   %0.2d - %0.2d  - %0.2d  ",BHH, BMM, BSS);					
				    LCD_DisplayStringLine(Line5,btime);	
				}
			}			
		}
		while(!RB4);
	}
}
void write_c(u8 add,u8 data)
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
char read_c(u8 add)
{
	char data;
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(add);

	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	data = I2CReceiveByte();
	I2CWaitAck();
	I2CStop();
	return data;		
}
void USART_SendString(u8 *str)
{
    uint8_t index = 0;
    
    do
    {
        USART_SendData(USART2,str[index]);
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET);
        index++;        
    }
    while(str[index] != 0);  //¼ì²é×Ö·û´®½áÊø±êÖ¾
    
}
void UASRT_Con(void)
{
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(USARTz_GPIO_CLK, ENABLE);
  RCC_APB1PeriphClockCmd(USARTz_CLK, ENABLE); 

  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTz, &USART_InitStructure);
  

  /* Enable USARTz Receive and Transmit interrupts */
  USART_ITConfig(USARTz, USART_IT_RXNE, ENABLE);

  /* Enable the USARTz */
  USART_Cmd(USARTz, ENABLE);

   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_Init(GPIOA, &GPIO_InitStructure);

    //ÅäÖÃUSART2 RXÒý½Å¹¤×÷Ä£Ê½
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

}
float R_ADC(void)
{	float ADC_VALUE;
	
	ADC_VALUE = ADC_GetConversionValue(ADC1)*3.30/0xfff;
	
	return ADC_VALUE;
}
void ADC_Con(void)
{
  ADC_InitTypeDef ADC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOB, ENABLE);
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
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
u8 sendtime[20];
void Time_Display(uint32_t TimeVar)
{
  uint32_t THH = 0, TMM = 0, TSS = 0;
  
  /* Reset RTC Counter when Time is 23:59:59 */
  if (RTC_GetCounter() == 0x0001517F)
  {
     RTC_SetCounter(0x0);
     /* Wait until last write operation on RTC registers has finished */
     RTC_WaitForLastTask();
  }
  
  /* Compute  hours */
  THH = TimeVar / 3600;
  /* Compute minutes */
  TMM = (TimeVar % 3600) / 60;
  /* Compute seconds */
  TSS = (TimeVar % 3600) % 60;
	if(THH==BHH)
	{
		if(TMM==BMM)
		{
			if(TSS==BSS)
			{
			  sprintf((char*)sendtime,"%.2f+0.%0.1d+%0.2d%0.2d%0.2d\n",adc_value,k_v, THH, TMM, TSS);
			  USART_SendString(sendtime);	
		   }
	}
  }
  sprintf((char*)timestr,"Time: %0.2d:%0.2d:%0.2d", THH, TMM, TSS);
  LCD_DisplayStringLine(Line8,timestr);	
}
void RTC_Init(void)
{ 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  PWR_BackupAccessCmd(ENABLE);

  /* Reset Backup Domain */
  BKP_DeInit();

  /* Enable the LSI OSC */
  RCC_LSICmd(ENABLE);
  /* Wait till LSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {}
  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

  /* Enable RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC registers synchronization */
  RTC_WaitForSynchro();

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  /* Enable the RTC Second */
  RTC_ITConfig(RTC_IT_SEC, ENABLE);

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  /* Set RTC prescaler: set RTC period to 1sec */
  RTC_SetPrescaler(40000);

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

   RTC_SetCounter(HH*3600 + MM*60 + SS);
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

}
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);	
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
void LED_c(u16 LED,u8 status)
{
	if(status == 0)
	{
        GPIO_SetBits(GPIOC,LED);
        GPIO_SetBits(GPIOD,GPIO_Pin_2);
        GPIO_ResetBits(GPIOD,GPIO_Pin_2);  //×´Ì¬Ëø´æ
	}
	else{
        GPIO_ResetBits(GPIOC,LED);
        GPIO_SetBits(GPIOD,GPIO_Pin_2);
        GPIO_ResetBits(GPIOD,GPIO_Pin_2);  //×´Ì¬Ëø´æ    
	}
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
	LCD_DisplayStringLine(Line6,"LED: ON");	
	LCD_DisplayStringLine(Line7,"                    ");	
	LCD_DisplayStringLine(Line8,"                    ");		
	LCD_DisplayStringLine(Line9,"                    ");	
	
}
void Delay_Ms(u32 nTime)
{
	TimingDelay = nTime;
	while(TimingDelay != 0);	
}
