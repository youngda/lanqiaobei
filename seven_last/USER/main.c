
#include <stdio.h>
#include "stm32f10x.h"
#include "lcd.h"
#include "led.h"
#include "i2c.h"
u32 TimingDelay = 0;
#define HH 23
#define MM 59
#define SS 57

extern u8 rx_b[20];
extern u8 rxover;
__IO uint32_t TimeDisplay = 0;
u8 rtc_s[20];
u8 adc_s[20];
u8 set_s[20];
u8 sends[20];

u8 BHH = 0;
u8 BMM = 0;
u8 BSS = 0;

float adc_v = 0;
extern u8 adc_flag;
u8 k_v = 1;
u8 rb1s = 0;
u8 rb2s = 0;
u8 rb3s = 0;
u8 rb4s = 0;


void LCD_init(void);
void Delay_Ms(u32 nTime);
void LED_init(void);
void LED_C(u16 led,u8 status);
void Time_Display(uint32_t TimeVar);
void RTC_init(void);
void adc_init(void);
void adc_r(void);
void key_init(void);
void key_scan(void);
void USART_init(void);
void send_s(u8 *str);
char r_v(u16 add);
void w_v(u16 add,u16 data);
void NVIC_Configuration(void);
int main(void)
{	
	u8 i = 0;
	SysTick_Config(SystemCoreClock/1000);

	LCD_init();
	key_init();
	LED_init();

	LED_C(LEDALL,0);
	adc_init();
    USART_init();
	RTC_init();
    NVIC_Configuration();
	i2c_init();
	k_v = r_v(0x01);
	Delay_Ms(5);
	while(1)
	{
		key_scan();
		if(rxover == 1)
		{
			rxover = 0;
			send_s("ok");
		    k_v = rx_b[3];
			k_v = k_v-48;
			w_v(0x01,k_v);
			Delay_Ms(5);
			for(i = 0;i <= 20;i++)
			{
				rx_b[i] = 0;
			}
			 USART_ITConfig(USARTz, USART_IT_RXNE, ENABLE);
		}
		if(rb2s == 0)
		{										 
			LCD_DisplayStringLine(Line9,"                 1     ");
			if(TimeDisplay == 1)
		    {
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
  u8 temp;
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
void send_s(u8 *str)
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
void USART_init(void)
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

  /* Configure USARTz */
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
void key_scan(void)
{
	if(RB1 == 0)
	{
		Delay_Ms(10);
		if(RB1 == 0)
		{
	       	if(rb2s == 0)
			{
			   if(rb1s == 1)
			   {
			   		rb1s = 0;
	                LCD_DisplayStringLine(Line4,"LED: ON      ");
			   }
			   else if(rb1s == 0)
			   {
			   		rb1s = 1;
	                LCD_DisplayStringLine(Line4,"LED: OFF     ");
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
	       if(rb2s == 0)
		   {
		   		rb2s = 1;
				LCD_ClearLine(Line3);
				LCD_ClearLine(Line5);
			    LCD_DisplayStringLine(Line2,"    SETTING        ");
				sprintf((char*)set_s,"   %0.2d-%0.2d-%0.2d", BHH, BMM, BSS);
                LCD_DisplayStringLine(Line4,set_s);
			    LCD_DisplayStringLine(Line9,"                 2     ");
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
				if(rb3s == 0)
				{
					 LCD_DisplayStringLine(Line5,"   ~~            ");
				}
				else if(rb3s == 1)
				{
					 LCD_DisplayStringLine(Line5,"      ~~            ");
				}
				else if(rb3s == 2)
				{
					 LCD_DisplayStringLine(Line5,"         ~~          ");
				}
				rb3s++;
				if(rb3s >= 3)rb3s = 0;
		   }			
		}
		while(!RB3);
	}
	else if(RB4 == 0)
	{
		Delay_Ms(10);
		if(RB4 == 0)
		{
	       LCD_DisplayStringLine(Line7," 444      ");			
		}
		while(!RB4);
	}
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
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOB, &GPIO_InitStructure);	
}
void adc_r(void)
{
  adc_v = ADC_GetConversionValue(ADC1);	
  adc_v = adc_v*3.3/0xfff;
  sprintf((char*)adc_s,"V1:  %0.2fV     ", adc_v);
  LCD_DisplayStringLine(Line2,adc_s);
  sprintf((char*)adc_s,"K :  0.%d",k_v);
  LCD_DisplayStringLine(Line3,adc_s);
  if(rb1s == 0)
   {
        LCD_DisplayStringLine(Line4,"LED: ON      ");
   }
   else if(rb1s == 12)
   {
        LCD_DisplayStringLine(Line4,"LED: OFF     ");
   }
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
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  /* Enable the RTC Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = USARTz_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
void Time_Display(uint32_t TimeVar)
{
  uint32_t THH = 0, TMM = 0, TSS = 0;
  
  /* Reset RTC Counter when Time is 23:59:59 */

  
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
			//sprintf((char*)sends,"%0.2f+0.%d+%0.2d%0.2d%0.2d\n",adc_v,k_v,BHH, BMM, BSS);
			send_s("hahha");	
		}
	}
  }
  sprintf((char*)rtc_s,"T: %0.2d-%0.2d-%0.2d", THH, TMM, TSS);
  LCD_DisplayStringLine(Line5,rtc_s);
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

  /* To output second signal on Tamper pin, the tamper functionality
       must be disabled (by default this functionality is disabled) */
  RTC_SetCounter(HH*3600 + MM*60 + SS);
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

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
	LCD_DisplayStringLine(Line1,"                    ");	
	LCD_DisplayStringLine(Line2,"                    ");
	LCD_DisplayStringLine(Line3,"                    ");
    LCD_DisplayStringLine(Line4,"LED: ON             ");					
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
