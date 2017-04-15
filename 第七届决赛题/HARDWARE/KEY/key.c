#include "key.h"



void Key_init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
		
		/* Configure PD0 and PD2 in output pushpull mode */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_8;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
}

u8 Key_scan(void)
{
		u8 key_val = 0;
		if(RB1 == 0)
				key_val = '1';
		if(RB2 == 0)
				key_val = '2';
		if(RB3 == 0)
				key_val = '3';
		if(RB4 == 0)
				key_val = '4';
		return key_val;
}


