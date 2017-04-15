#ifndef __RTC_H
#define __RTC_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
void RTC_Init(void);
void Rtc_display(void);
u32 Rtc_get_second(void);
u32 Rtc_get_hour(void);
u32 Rtc_get_minute(void);
#endif
