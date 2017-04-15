#include "rtc.h"
#include "stdio.h"
#include "lcd.h"

#define HH 	10
#define MM 	59
#define SS 	50



void RTC_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the RTC Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	
  /* Enable PWR and BKP clocks */
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
  //BKP_TamperPinCmd(DISABLE);

  /* Enable the RTC Second Output on Tamper Pin */
 // BKP_RTCOutputConfig(BKP_RTCOutputSource_Second);
	
	
	RTC_SetCounter(HH*3600+MM*60+SS);
	RTC_WaitForLastTask();
}

void Rtc_display()
{
		u8 string[20];
		u32 THH = 0 , TMM = 0 ,TSS = 0;
		u32 temp = RTC_GetCounter();
		THH = temp/3600;
		TMM = (temp%3600)/60;
		TSS = (temp%3600)%60;
	
		sprintf((char *)string, " real time:%02d-%02d-%02d", THH,TMM,TSS);
		LCD_DisplayStringLine(Line6, string);
		
}

u32 Rtc_get_second(void)
{
		u32 TSS = 0;
		TSS = (RTC_GetCounter()%3600)%60;
		return TSS;
}

u32 Rtc_get_hour(void)
{
		u32 THH = 0;
		THH = RTC_GetCounter()/3600;
		return THH;
}

u32 Rtc_get_minute(void)
{
		u32 TMM = 0;
		TMM = (RTC_GetCounter()%3600)/60;
		return TMM;
}
