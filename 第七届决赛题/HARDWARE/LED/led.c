#include "led.h"


void Led_init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD |RCC_APB2Periph_GPIOC, ENABLE);

		/* Configure PD0 and PD2 in output pushpull mode */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9|GPIO_Pin_10| GPIO_Pin_11 | GPIO_Pin_12|GPIO_Pin_13| 
		GPIO_Pin_14|GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
	
		GPIO_SetBits(GPIOC,  GPIO_Pin_8 | GPIO_Pin_9|GPIO_Pin_10| GPIO_Pin_11 | GPIO_Pin_12|GPIO_Pin_13| 
		GPIO_Pin_14|GPIO_Pin_15);
		GPIO_SetBits(GPIOD,  GPIO_Pin_2);
		GPIO_ResetBits(GPIOD,  GPIO_Pin_2);
	
}

void LED_Dispaly(u8 LED1,u8 LED2,u8 LED3)
{
		if(LED1 == 1)
		{
				GPIO_ResetBits(GPIOC,  GPIO_Pin_8);
		}else
		{
				GPIO_SetBits(GPIOC,  GPIO_Pin_8);
		}
		
		if(LED2 == 1)
		{
				GPIO_ResetBits(GPIOC,  GPIO_Pin_9);
		}else
		{
				GPIO_SetBits(GPIOC,  GPIO_Pin_9);
		}
		
		if(LED3 == 1)
		{
				GPIO_ResetBits(GPIOC,  GPIO_Pin_10);
		}else
		{
				GPIO_SetBits(GPIOC,  GPIO_Pin_10);
		}
		
		GPIO_SetBits(GPIOC,  GPIO_Pin_11 | GPIO_Pin_12|GPIO_Pin_13| 
		GPIO_Pin_14|GPIO_Pin_15);
	
		GPIO_SetBits(GPIOD,  GPIO_Pin_2);
		GPIO_ResetBits(GPIOD,  GPIO_Pin_2);
}
