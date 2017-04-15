#include "stm32f10x.h"
#include "stdio.h"
#include "lcd.h"
#include "adc.h"
#include "main.h"
#include "pwm.h"
#include "input.h"
#include "rtc.h"
#include "eeprom.h"
#include "i2c.h"
#include "key.h"
#include "led.h"
#include "uart.h"




u32 timingdelay = 0;
u32 timingdisplay = 0;
extern __IO uint32_t Capture;
extern  __IO uint32_t TIM3Freq;

extern u8 flag_C;
extern u8 flag_T;

u8 tem_max = 60;
u8 hum_max = 80;
u8 samples = 1;
float KHz_temp = 1;
u8 switch_temp = 1;
u8 string[60];

float T_temp;
u8 H_temp;
u32 record_times = 0;

u8 LED1 = 0;
u8 LED2 = 0;
u8 LED3 = 0;

float adc_temp;
u32 rtc_second = 0;;
u8 key_val;
u8 key1_setup = 0;
		
void highlight_sel_dis(u8 add);
void button_add_fun(void);
void button_sub_fun(void);
void Data_display(float t, u32 h, u32 r);
void Led_alert(void);
void T_H_R_display(void);
void Record_process(void);
void Key_scan_process(void);
void IIC_process(void);
void Init_All_module(void);
void Uart2_process(void);
void Uart2_process(void);

float tem_array[60];
u8 hum_array[60];
u8 HH_array[60];
u8 MM_array[60];
u8 SS_array[60];
u8 record_index = 0;
u8 full_flag = 0;
u8 record_output = 0;



int main()
{

		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		SysTick_Config(SystemCoreClock / 1000);
		Init_All_module();
	
		LCD_Clear(White);
		LCD_SetTextColor(Black);
		LCD_SetBackColor(White);
		sprintf((char *)string,   "      real data     ");
		LCD_DisplayStringLine(Line0, string);
		
		
		rtc_second = Rtc_get_second();
		Rtc_display();
	
		IIC_process();
	
		while(1)
		{
				Uart2_process();
				if(key1_setup == 0)
				{		
							if(timingdisplay == 1)
							{
									timingdisplay = 0;
									Rtc_display();
							}
							T_H_R_display();
							Led_alert();
							Record_process();

				}
				Key_scan_process();

		}
		
		
}


void Delay_ms(u32 time)
{
		timingdelay = time;
		while(timingdelay != 0);
}
void highlight_sel_dis(u8 add)
{
		
								sprintf((char *)string,   "     max tem : %d   ", (char)tem_max - 20);
								LCD_DisplayStringLine(Line2, string);				
								sprintf((char *)string,   "     max hum : %d   ", hum_max);
								LCD_DisplayStringLine(Line4, string);			
								sprintf((char *)string,   "   sample   :  %dS  ", samples);
								LCD_DisplayStringLine(Line6, string);
								sprintf((char *)string,   " test sig : %.1fKHz ", KHz_temp);
								LCD_DisplayStringLine(Line8, string);
								
	
								switch_temp += add;
								if(switch_temp == 5)
										switch_temp =1;
								LCD_SetTextColor(Green);
								switch(switch_temp)
								{
									case 1: 
										sprintf((char *)string,   "     max tem : %d   ", (char)tem_max - 20);
										LCD_DisplayStringLine(Line2, string);
										break;
									case 2: 
										sprintf((char *)string,   "     max hum : %d   ", hum_max);
										LCD_DisplayStringLine(Line4, string);
										break;
									case 3: 
										sprintf((char *)string,   "   sample   :  %dS  ", samples);
										LCD_DisplayStringLine(Line6, string);
										break;
									case 4: 
										sprintf((char *)string,   " test sig : %.1fKHz ", KHz_temp);
										LCD_DisplayStringLine(Line8, string);
										break;
								}
								LCD_SetTextColor(Black);
}

void button_add_fun(void)
{
		switch(switch_temp)
								{
										case 1: 
										tem_max++;
										if(tem_max > 80)
										{
												tem_max = 80;
										}
										x24c02_write(0xf1, tem_max);
										Delay_ms(3);
										break;
										case 2: 
										hum_max += 5;
										if(hum_max > 90)
										{
												hum_max = 90;
										}
										x24c02_write(0xf2, hum_max);
										Delay_ms(3);
										break;
										case 3: 
										samples++;
										if(samples > 5)
										{
												samples = 5;
										}
										x24c02_write(0xf3, samples);
										Delay_ms(3);
										break;
										case 4: 
										KHz_temp = KHz_temp + 0.5;
										if(KHz_temp > 10)
										{
												KHz_temp = 10;
										}
										x24c02_write(0xf4, KHz_temp);
										TIM_SetAutoreload(TIM2, 1000/KHz_temp - 1);
										Delay_ms(3);
										break;
								}
								highlight_sel_dis(0);
								 Led_alert();
}

void button_sub_fun(void)
{
		switch(switch_temp)
								{
										case 1: 
										if(tem_max == 0);
										else
												tem_max--;			
										x24c02_write(0xf1, tem_max);
										Delay_ms(3);
										break;
										case 2: 
										hum_max -= 5;
										if(hum_max < 10)
										{
												hum_max = 10;
										}
										x24c02_write(0xf2, hum_max);
										Delay_ms(3);
										break;
										case 3: 
										samples--;
										if(samples < 1)
										{
												samples = 1;
										}
										x24c02_write(0xf3, samples);
										Delay_ms(3);
										break;
										case 4: 
										KHz_temp = KHz_temp - 0.5;
										if(KHz_temp < 1)
										{
												KHz_temp = 1;
										}
										x24c02_write(0xf4, KHz_temp);
										TIM_SetAutoreload(TIM2, 1000/KHz_temp - 1);
										Delay_ms(3);
										break;
								}
								highlight_sel_dis(0);
								 Led_alert();
							
}

void Data_display(float t, u32 h, u32 r)
{
		sprintf((char *)string,   " current tem:%.0f'C  ", t);
		LCD_DisplayStringLine(Line2, string);
		sprintf((char *)string, "  current hum : %d%% ", h);
		LCD_DisplayStringLine(Line4, string);
		sprintf((char *)string, " record times: %d    ", r);
		LCD_DisplayStringLine(Line8, string);
}

void Led_alert(void)
{
		H_temp  = (u32)((TIM3Freq-1000)*80/9000.0 + 10);
							if((u8)T_temp > (tem_max - 20))
							{
									LED1 = 1;
							}else
							{
									LED1 = 0;
							}
							if(H_temp > hum_max)
							{
									LED2 = 1;
							}else
							{
									LED2 = 0;
							}
							
							LED_Dispaly(LED1,LED2,LED3);
}

void T_H_R_display(void)
{
		adc_temp = Read_adc();
						//	sprintf((char *)string, "123456789  %.2f", adc_temp);
						//	LCD_DisplayStringLine(Line1, string);
							
							T_temp = (adc_temp /3.30* 80.0  - 20);
		
						//	sprintf((char *)string, "-%d-  -%d-  ", Capture,TIM3Freq);
						//	LCD_DisplayStringLine(Line3, string);
							H_temp  = (u32)((TIM3Freq-1000)*80/9000.0 + 10);
							Data_display(T_temp, H_temp, record_times);
}

void Record_process(void)
{
		if(((rtc_second+samples)%60) == Rtc_get_second())
		{
						
						rtc_second = Rtc_get_second();
						record_times++;
						Data_display(T_temp, H_temp, record_times);
						LED3 = ! LED3;
						LED_Dispaly(LED1,LED2,LED3);				
						tem_array[record_index] = T_temp;
						hum_array[record_index] = H_temp;
						HH_array[record_index] = Rtc_get_hour();
						MM_array[record_index] = Rtc_get_minute();
						SS_array[record_index] = Rtc_get_second();
						record_index = record_index + 1;
						if(record_index == 60)
						{
							full_flag = 1;
							record_index = 0;
						}
						
						
		}
}

void Key_scan_process(void)
{
		key_val = Key_scan();
				switch(key_val)
				{	
					case '1':
						key1_setup = !key1_setup;
						if(key1_setup)
						{
								highlight_sel_dis(0);
						}else
						{
								rtc_second = Rtc_get_second();
								Data_display(T_temp, H_temp, record_times);
						}
						Delay_ms(10);
						while(!RB1);	
						Delay_ms(10);
						break;
					case '2':
						if(key1_setup)
						{
								highlight_sel_dis(1);
						}
						Delay_ms(10);
						while(!RB2);
						Delay_ms(10);
						break;
					case '3':
						if(key1_setup)
						{
								button_add_fun();
						}
						Delay_ms(10);
						while(!RB3);
						Delay_ms(10);
						break;
					case '4':
						if(key1_setup)
						{
								button_sub_fun();
						}
						Delay_ms(10);
						while(!RB4);
						Delay_ms(10);
						break;
				}
				key_val = 0;
}

void IIC_process(void)
{
		if(x24c02_read(0xff)== 0x66)
				{
						tem_max = x24c02_read(0xf1);
						Delay_ms(3);
						hum_max = x24c02_read(0xf2);
						Delay_ms(3);
						samples = x24c02_read(0xf3);
						Delay_ms(3);
						KHz_temp = x24c02_read(0xf4);
						Delay_ms(3);
						TIM_SetAutoreload(TIM2, 1000/KHz_temp - 1);
					
				}else
				{
						x24c02_write(0xf1, tem_max);
						Delay_ms(3);
						x24c02_write(0xf2, hum_max);
						Delay_ms(3);
						x24c02_write(0xf3, samples);
						Delay_ms(3);
						x24c02_write(0xf4, KHz_temp);
						Delay_ms(3);
						x24c02_write(0xff, 0x66);
						Delay_ms(3);
				}
}

void Init_All_module(void)
{
		STM3210B_LCD_Init();
		ADC_INIT();
	  Pwm_Init(1000/KHz_temp,100/3);
		Input(0xffff);
		RTC_Init();
		i2c_init();
		Key_init();
		Led_init();
	  Uart2_init();
}

void Uart2_process(void)
{	
		int i;
		if(flag_C == 1)
				{
					sprintf((char *)string,  "tem_max:%d  hum_max:%d real time:%02d-%02d-%02d \r\n",
						(tem_max-20),hum_max,Rtc_get_hour(),Rtc_get_minute(),Rtc_get_second());
					Uart2_sendString(string);
					flag_C = 0;
				}
				if(flag_T == 1)
				{
					if(full_flag == 1)
							record_output = 60;
					else
							record_output = record_index;
					for( i = 0; i<record_output; i++)
					{

								sprintf((char *)string,  "%d. tem:%.0f  hum:%d record time:%02d-%02d-%02d \r\n", i,
								tem_array[i],hum_array[i],HH_array[i],MM_array[i],SS_array[i]);
								Uart2_sendString(string);
					}
					flag_T = 0;
				}
}

