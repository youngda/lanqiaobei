
#include <stdio.h>
#include "stm32f10x.h"
#include "lcd.h"
#include "led.h"
#include "i2c.h"
u32 TimingDelay = 0;
u32 pwmk = 600;
float adc_v = 0;
u8 adcs[20];
u8 hums[20];
u8 times[20];
extern u8 adc_flag;
float hum;
float temp;
float sig;
__IO uint32_t TimeDisplay = 0;

#define HH 23
#define MM 59
#define SS 56

void LCD_init(void);
void Delay_Ms(u32 nTime);
void LED_init(void);
void LED_C(u16 led,u8 status);
void adc_r(void);
void NVIC_Configuration(void);
void RTC_init(void);
void key_init(void);

void pwm_init(u32 TimerPeriod,u32 Channel1Pulse);
void adc_init(void);
void key_scan(void);
void Time_Display(uint32_t TimeVar);
void USART_init(void);
void send_s(u8 *str);
void w_v(u16 add,u8 data);
char r_v(u16 add);


u8 rb1s = 0;
u8 rb2s = 0;
u8 rb3s = 0;
u8 rb4s = 0;
int temp_u = 40;
u8 hum_u = 30;
u8 note_t = 1;
u8 note = 0;
extern u8 rx_b[20];
extern u8 rxover;
u8 rb1[20];
u8 txs[20];
int main(void)
{	
	SysTick_Config(SystemCoreClock/1000);
	LCD_init();
	LED_init();
	LED_C(LEDALL,0);
	pwm_init(1000/sig-1,pwmk/sig);
	adc_init();
	key_init();
	USART_init();
	RTC_init();
	NVIC_Configuration();
	i2c_init();

    temp_u = r_v(0x01);
	Delay_Ms(5);
	hum_u = r_v(0x02);
	Delay_Ms(5);
	note_t = r_v(0x03);
	Delay_Ms(5);
	sig = r_v(0x04);
	Delay_Ms(5);
	while(1)
	{
		key_scan();
		if(rxover == 1)
		{
			rxover = 0;
			USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
			if(rx_b[0] == 'C')
			{
				sprintf((char*)txs,"temp:%.0fhum:%.0fnote:%d",temp,hum,note);
				send_s(txs);		
			}
			else if(rx_b[0] == 'T')
			{
				
			}
		}
		if(rb1s == 0)
		{
			LCD_DisplayStringLine(Line0,"      NOW DATA      ");
			if(TimeDisplay == 1)
		    {
		      /* Display current time */
		      Time_Display(RTC_GetCounter());
		      TimeDisplay = 0;
		    }
			if(adc_flag == 1)
			{
				adc_flag = 0;
				adc_r();
			}
		}	
	}
}
char r_v(u16 add)
{
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
  return temp;
}
void w_v(u16 add,u8 data)
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
void send_s(u8 *str)
{
  uint8_t index = 0;
  do
  {
    USART_SendData(USART2, str[index]);
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);          
  	index++;
  }
  while(str[index] != 0);
}
void USART_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  RCC_APB1PeriphClockCmd(USARTz_CLK, ENABLE);  
  RCC_APB2PeriphClockCmd(USARTz_GPIO_CLK, ENABLE);
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
;
  /* Configure USARTz */
  USART_Init(USARTz, &USART_InitStructure);
  
  USART_ITConfig(USARTz, USART_IT_RXNE, ENABLE);

  USART_Cmd(USARTz, ENABLE);

  GPIO_InitStructure.GPIO_Pin = USARTz_RxPin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USARTz_GPIO, &GPIO_InitStructure);
  

  
  /* Configure USARTy Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = USARTz_TxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USARTz_GPIO, &GPIO_InitStructure); 	
}
void key_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    /* Configure PD0 and PD2 in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;
  GPIO_Init(GPIOB, &GPIO_InitStructure);	
}

void key_scan(void)
{
	if(RB1 == 0)
	{
		Delay_Ms(10);
		if(RB1 == 0)
		{
		  if(rb1s == 0)
		  {
		  	rb1s = 1;
		    LCD_DisplayStringLine(Line0,"   DATA SETTING      ");
			sprintf((char*)rb1,"temp up: %doC       ",temp_u);		
			LCD_DisplayStringLine(Line2,rb1);
			sprintf((char*)rb1,"hum  up: %d%%     ",hum_u);		
			LCD_DisplayStringLine(Line4,rb1);
			sprintf((char*)rb1,"note time: %d       ",note_t);		
			LCD_DisplayStringLine(Line6,rb1);
			sprintf((char*)rb1,"signal  : %0.1f       ",sig);		
			LCD_DisplayStringLine(Line8,rb1);
		  }
		  else if(rb1s == 1)
		  {
		  	rb1s = 0;
		  }			
		}
		while(!RB1);
	}
	else if(RB2 == 0)
	{
		Delay_Ms(10);
		if(RB2 == 0)
		{
		  if(rb1s == 1)
		  {
		  	rb2s++;
		    if(rb2s>= 4)rb2s = 0;
		  	if(rb2s == 0)
			{
				LCD_DisplayStringLine(Line0,"   DATA SETTING      ");
				LCD_SetTextColor(Red);
				sprintf((char*)rb1,"temp up: %doC       ",temp_u);		
				LCD_DisplayStringLine(Line2,rb1);
				LCD_SetTextColor(White);
				sprintf((char*)rb1,"hum  up: %d%%     ",hum_u);		
				LCD_DisplayStringLine(Line4,rb1);
				sprintf((char*)rb1,"note time: %d       ",note_t);		
				LCD_DisplayStringLine(Line6,rb1);
				sprintf((char*)rb1,"signal  : %0.1f       ",sig);		
				LCD_DisplayStringLine(Line8,rb1);	
			}
		   else if(rb2s == 1)
			{
				LCD_DisplayStringLine(Line0,"   DATA SETTING      ");
				sprintf((char*)rb1,"temp up: %doC       ",temp_u);		
				LCD_DisplayStringLine(Line2,rb1);
				LCD_SetTextColor(Red);
				sprintf((char*)rb1,"hum  up: %d%%     ",hum_u);		
				LCD_DisplayStringLine(Line4,rb1);
				LCD_SetTextColor(White);			
				sprintf((char*)rb1,"note time: %d       ",note_t);		
				LCD_DisplayStringLine(Line6,rb1);
				sprintf((char*)rb1,"signal  : %0.1f       ",sig);		
				LCD_DisplayStringLine(Line8,rb1);
			}
			else if(rb2s == 2)
			{
				LCD_DisplayStringLine(Line0,"   DATA SETTING      ");
				sprintf((char*)rb1,"temp up: %doC       ",temp_u);		
				LCD_DisplayStringLine(Line2,rb1);
				sprintf((char*)rb1,"hum  up: %d%%     ",hum_u);		
				LCD_DisplayStringLine(Line4,rb1);
				LCD_SetTextColor(Red);			
				sprintf((char*)rb1,"note time: %d       ",note_t);		
				LCD_DisplayStringLine(Line6,rb1);
				LCD_SetTextColor(White);
				sprintf((char*)rb1,"signal  : %0.1f       ",sig);		
				LCD_DisplayStringLine(Line8,rb1);
			
			}
			else if(rb2s == 3)
			{
				LCD_DisplayStringLine(Line0,"   DATA SETTING      ");
				sprintf((char*)rb1,"temp up: %doC       ",temp_u);		
				LCD_DisplayStringLine(Line2,rb1);
				sprintf((char*)rb1,"hum  up: %d%%     ",hum_u);		
				LCD_DisplayStringLine(Line4,rb1);		
				sprintf((char*)rb1,"note time: %d       ",note_t);		
				LCD_DisplayStringLine(Line6,rb1);
				LCD_SetTextColor(Red);	
				sprintf((char*)rb1,"signal  : %0.1f       ",sig);		
				LCD_DisplayStringLine(Line8,rb1);
				LCD_SetTextColor(White);			
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
		  if(rb1s == 1)
		  {
		  	if(rb2s == 0)
			{
				temp_u++;
				if(temp_u >= 60)temp_u = 60;
				LCD_SetTextColor(Red);
				sprintf((char*)rb1,"temp up: %doC       ",temp_u);		
				LCD_DisplayStringLine(Line2,rb1);
				LCD_SetTextColor(White);
				w_v(0x01,temp_u);
	            Delay_Ms(5);	
			}
			else if(rb2s == 1)
			{
				hum_u = hum_u+5;
				if(temp_u >= 90)temp_u = 90;	
				LCD_SetTextColor(Red);
				sprintf((char*)rb1,"hum  up: %d%%     ",hum_u);		
				LCD_DisplayStringLine(Line4,rb1);
				LCD_SetTextColor(White);
				w_v(0x02,hum_u);
				Delay_Ms(5);
			}
			else if(rb2s == 2)
			{
				note_t++;
				if(note_t >= 5)note_t = 5;	
				LCD_SetTextColor(Red);			
				sprintf((char*)rb1,"note time: %d       ",note_t);		
				LCD_DisplayStringLine(Line6,rb1);
				LCD_SetTextColor(White);
				w_v(0x03,note_t);
				Delay_Ms(5);
			}
			else if(rb2s == 3)
			{
				sig = sig+0.5;
				if(sig >= 10)sig = 10;
				pwm_init(1000/sig-1,pwmk/sig);	
				LCD_SetTextColor(Red);	
				sprintf((char*)rb1,"signal  : %0.1f       ",sig);		
				LCD_DisplayStringLine(Line8,rb1);
				LCD_SetTextColor(White);
				w_v(0x04,sig);
				Delay_Ms(5);
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
		  if(rb1s == 1)
		  {
		  	if(rb2s == 0)
			{
				temp_u--;
				if(temp_u <= -20)temp_u = -20;	
				LCD_SetTextColor(Red);
				sprintf((char*)rb1,"temp up: %doC       ",temp_u);		
				LCD_DisplayStringLine(Line2,rb1);
				LCD_SetTextColor(White);
				w_v(0x01,temp_u);
				Delay_Ms(5);
			}
			else if(rb2s == 1)
			{
				hum_u = hum_u-5;
				if(temp_u <= 10)temp_u = 10;	
				LCD_SetTextColor(Red);
				sprintf((char*)rb1,"hum  up: %d%%     ",hum_u);		
				LCD_DisplayStringLine(Line4,rb1);
				LCD_SetTextColor(White);
				w_v(0x02,hum_u);
				Delay_Ms(5);
			}
			else if(rb2s == 2)
			{
				note_t--;
				if(note_t <= 1)note_t = 1;	
				LCD_SetTextColor(Red);			
				sprintf((char*)rb1,"note time: %d       ",note_t);		
				LCD_DisplayStringLine(Line6,rb1);
				LCD_SetTextColor(White);
				w_v(0x03,note_t);
				Delay_Ms(5);
			}
			else if(rb2s == 3)
			{
				sig = sig-0.5;
				if(sig <= 1)sig = 1;
				pwm_init(1000/sig-1,pwmk/sig);		
				LCD_SetTextColor(Red);	
				sprintf((char*)rb1,"signal  : %0.1f       ",sig);		
				LCD_DisplayStringLine(Line8,rb1);
				LCD_SetTextColor(White);
				w_v(0x04,sig);
				Delay_Ms(5);
			}			
		}
		while(!RB4);
	  }
   }  
}
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  /* Enable the RTC Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable the USARTz Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USARTz_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
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

  sprintf((char*)times,"Time: %0.2d-%0.2d-%0.2d", THH, TMM, TSS);
  LCD_DisplayStringLine(Line6,times);
}

void RTC_init(void)
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  /* Allow access to BKP Domain */
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
void adc_r(void)
{
	adc_v = ADC_GetConversionValue(ADC1);
	adc_v = adc_v *3.3/0xfff;
	adc_v =  adc_v*24.24-20;
	temp = adc_v;
	sprintf((char*)adcs,"NOW temp: %.0foC   ",adc_v);		
	LCD_DisplayStringLine(Line2,adcs);
	hum = sig*8.88+1.22;
	sprintf((char*)adcs,"NOW hum : %.0f%%   ",hum);		
	LCD_DisplayStringLine(Line4,adcs);
	sprintf((char*)adcs,"         NOTE : %d   ",note);	
	LCD_DisplayStringLine(Line8,adcs);
}
void adc_init(void)
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
void pwm_init(u32 TimerPeriod,u32 Channel1Pulse)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseStructure.TIM_Prescaler = 71;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	/* Channel 1, 2,3 and 4 Configuration in PWM mode */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = Channel1Pulse;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
	
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
	
	TIM_Cmd(TIM2, ENABLE);
	TIM_CtrlPWMOutputs(TIM2, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

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
	LCD_DisplayStringLine(Line0,"      NOW DATA      ");	
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
