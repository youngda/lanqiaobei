#include <stdio.h>
#include "stm32f10x.h"
#include "lcd.h"
#include "led.h"
#include "i2c.h"
#define HH 23
#define MM 59
#define SS 55
u32 BHH = 0,BMM = 0,BSS = 0;
u8 r_flag = 0;
u8 r_time = 0;
u32 TimingDelay = 0;
u32 PA1_v = 0;
u32 PA2_v = 0;
__IO uint32_t TimeDisplay = 0;
  #define USARTz                   USART2
  #define USARTz_GPIO              GPIOA
  #define USARTz_CLK               RCC_APB1Periph_USART2
  #define USARTz_GPIO_CLK          RCC_APB2Periph_GPIOA
  #define USARTz_RxPin             GPIO_Pin_3
  #define USARTz_TxPin             GPIO_Pin_2
  #define USARTz_IRQn              USART2_IRQn
  #define USARTz_IRQHandler        USART2_IRQHandler

extern u8 rx_b[20];
extern u8 RX_flag;

void Delay_Ms(u32 nTime);
void LCD_Init(void);
void LED_Init(void);
void LED_Con(u16 LED,u8 status);
void PWM_Init(uint16_t Channel1Pulse,uint16_t Channel2Pulse);
void key_scan(void);
void W_V(u16 add,u32 data);
unsigned char R_v(u16 add);
void key_init(void);
void NVIC_Configuration(void);
void Time_Display(uint32_t TimeVar);
void RTC_Init(void);
void USART_In(void);
u8 pwm[20];
int main(void)
{
	u8 i;
	SysTick_Config(SystemCoreClock/1000);
	LCD_Init();
	LED_Init();
    LED_Con(LEDALL,0);

	i2c_init();

    PA1_v = R_v(0x01);
	Delay_Ms(5);
	PA2_v = R_v(0x02);
	Delay_Ms(5);
    key_init();
	sprintf((char*)pwm,"PWM-PA1: %d ",PA1_v*10);
	LCD_DisplayStringLine(Line0,pwm);
	sprintf((char*)pwm,"PWM-PA2: %d ",PA2_v*10);
	LCD_DisplayStringLine(Line2,pwm);
	
	RTC_Init();
	USART_In();
    NVIC_Configuration();
	while(1)
	{
		key_scan();
		if (TimeDisplay == 1)
	    {
	      /* Display current time */
	      Time_Display(RTC_GetCounter());
	      TimeDisplay = 0;
	    }
		if(RX_flag == 1)
		{
			RX_flag = 0;
			LCD_ClearLine(Line9);
		 	LCD_DisplayStringLine(Line9,rx_b);
			BHH = (rx_b[3]-48)*10+(rx_b[4]-48);
			BMM = (rx_b[6]-48)*10+(rx_b[7]-48);
			BSS = (rx_b[9]-48)*10+(rx_b[10]-48);
			r_flag = rx_b[14]-48;
			r_time = rx_b[16]-48;
			for(i = 0;i<=20;i++)
			{
				rx_b[i] = 0;
			}
	        USART_ITConfig(USARTz, USART_IT_RXNE, ENABLE);
		}
	}
}
void USART_In(void)
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

  GPIO_InitStructure.GPIO_Pin = USARTz_RxPin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USARTz_GPIO, &GPIO_InitStructure);
  

  
  /* Configure USARTy Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = USARTz_TxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USARTz_GPIO, &GPIO_InitStructure);

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

  NVIC_InitStructure.NVIC_IRQChannel = USARTz_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
u8 timec[20];
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
  if(THH == BHH)
  {
  	if(TMM == BMM)
	{
		if(TSS == BSS)
		{
			if(r_flag == 1)
			{
				PWM_Init(998*PA1_v/10,0);
				sprintf((char*)pwm,"PWM-PA1: %d ",PA1_v*10);
				LCD_DisplayStringLine(Line0,pwm);
	            LCD_DisplayStringLine(Line6,"Channel: PA1        ");
				LED_Con(LEDALL,0);
				LED_Con(LED1,1);	
			}
			else if(r_flag == 2)
			{
				PWM_Init(998*PA2_v/10,0);
				sprintf((char*)pwm,"PWM-PA2: %d ",PA1_v*10);
				LCD_DisplayStringLine(Line2,pwm);
	            LCD_DisplayStringLine(Line6,"Channel: PA2        ");
				LED_Con(LEDALL,0);
				LED_Con(LED2,1);	
			}
		}
		else if(TSS == BSS + r_time)
		{
			PWM_Init(0,0);
			LED_Con(LEDALL,0);	
		    LCD_DisplayStringLine(Line6,"Channel: None       ");
		}
	}
  }

  sprintf((char*)timec,"Time: %0.2d:%0.2d:%0.2d", THH, TMM, TSS);
  LCD_DisplayStringLine(Line4,timec);
}

void RTC_Init(void)
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

  /* Change the current time */
  RTC_SetCounter(HH*3600 + MM*60 + SS);
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
}
void key_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOB, &GPIO_InitStructure);	
}
u8 v1 = 0;
u8 v2 = 0;  
void key_scan(void)
{
	if(RB1 == 0)
	{
		Delay_Ms(10);
		if(RB1 == 0)
		{
			if(v1 == 0)
			{
				v1 = 1;
				PWM_Init(998*PA1_v/10,0);
				sprintf((char*)pwm,"PWM-PA1: %d ",PA1_v*10);
				LCD_DisplayStringLine(Line0,pwm);
	            LCD_DisplayStringLine(Line6,"Channel: PA1        ");
				LED_Con(LEDALL,0);
				LED_Con(LED1,1);
			}
			else if(v1 == 1)
			{
				v1 = 0;
				PWM_Init(0,0);
	            LCD_DisplayStringLine(Line6,"Channel: None       ");
				LED_Con(LEDALL,0);
			}
		USART_In();	
		}
		while(!RB1);
	}
	else if(RB2 == 0)
	{
		Delay_Ms(10);
		if(RB2 == 0)
		{

			PA1_v++;
			if(PA1_v >= 10)PA1_v = 0;
			if(v1 == 1)
			{
				PWM_Init(998*PA1_v/10,0);
			}
			W_V(0x01,PA1_v);
			Delay_Ms(5);
			sprintf((char*)pwm,"PWM-PA1: %d ",PA1_v*10);
			LCD_DisplayStringLine(Line0,pwm);
			USART_In();	
		}
		while(!RB2);
	} 
	else if(RB3 == 0)
	{
		Delay_Ms(10);
		if(RB3 == 0)
		{			
			if(v2 == 0)
			{
				USART_Cmd(USART2, DISABLE);
				USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);		
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, DISABLE);
				v2 = 1;
				PWM_Init(0,998*PA2_v/10);
				sprintf((char*)pwm,"PWM-PA2: %d ",PA2_v*10);
				LCD_DisplayStringLine(Line2,pwm);
	            LCD_DisplayStringLine(Line6,"Channel: PA2        ");
				LED_Con(LEDALL,0);
				LED_Con(LED1,1);
			}
			else if(v2 == 1)
			{
                USART_Cmd(USARTz, ENABLE);
			    USART_ITConfig(USARTz, USART_IT_RXNE, ENABLE);
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
				v2 = 0;
				PWM_Init(0,0);
	            LCD_DisplayStringLine(Line6,"Channel: None       ");
				LED_Con(LEDALL,0);				
				USART_In();	
			}		
		}
		while(!RB3);
	} 
	else if(RB4 == 0)
	{
		Delay_Ms(10);
		if(RB4 == 0)
		{
			PA2_v++;
			if(PA2_v >= 10)PA2_v = 0;
			if(v2 == 1)
			{
				PWM_Init(0,998*PA2_v/10);
			}
			W_V(0x02,PA2_v);
			Delay_Ms(5);
			sprintf((char*)pwm,"PWM-PA2: %d ",PA2_v*10);
			LCD_DisplayStringLine(Line2,pwm);		
		}
	while(!RB4);
	} 
}
unsigned char R_v(u16 add)
{
  u32 data;
  I2CStart();
  I2CSendByte(0xa0);
  I2CWaitAck(); 
  I2CSendByte(add);
  I2CWaitAck();	
  
  I2CStart(); 
  I2CSendByte(0xa1);
  I2CWaitAck();	
  data = I2CReceiveByte();
  I2CWaitAck();
  I2CStop();
  return data;	
}
void W_V(u16 add,u32 data)
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

void PWM_Init(uint16_t Channel1Pulse,uint16_t Channel2Pulse)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;	
	  GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 999;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  TIM_PrescalerConfig(TIM2,71, TIM_PSCReloadMode_Immediate);
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
  TIM_OCInitStructure.TIM_Pulse = Channel1Pulse;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

  TIM_OC2Init(TIM2, &TIM_OCInitStructure);

  TIM_OCInitStructure.TIM_Pulse = Channel2Pulse;
  TIM_OC3Init(TIM2, &TIM_OCInitStructure);

  /* TIM1 counter enable */
  TIM_Cmd(TIM2, ENABLE);

  /* TIM1 Main Output Enable */
  TIM_CtrlPWMOutputs(TIM2, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

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
void LED_Con(u16 LED,u8 status)
{
	if(status == 0)
	{
		GPIO_SetBits(GPIOC,LED);
		GPIO_SetBits(GPIOD,GPIO_Pin_2);
		GPIO_ResetBits(GPIOD,GPIO_Pin_2);
	}
	else 
	{
        GPIO_ResetBits(GPIOC,LED);
		GPIO_SetBits(GPIOD,GPIO_Pin_2);
		GPIO_ResetBits(GPIOD,GPIO_Pin_2);		
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
	LCD_DisplayStringLine(Line6,"Channel: None       ");	
	LCD_DisplayStringLine(Line7,"                    ");	
	LCD_DisplayStringLine(Line8,"Command:            ");		
	LCD_DisplayStringLine(Line9,"         None       ");	
	
}
void Delay_Ms(u32 nTime)
{
	TimingDelay = nTime;
	while(TimingDelay != 0);	
}
