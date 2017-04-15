#include "pwminput.h"


void Pwm_Input(u16 period)
{
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		GPIO_InitTypeDef GPIO_InitStructure;
		TIM_ICInitTypeDef  TIM_ICInitStructure;
		
		NVIC_InitTypeDef NVIC_InitStructure;

		/* Enable the TIM3 global Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |  RCC_APB2Periph_AFIO, ENABLE);

		/* TIM3 channel 2 pin (PA.07) configuration */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		TIM_TimeBaseStructure.TIM_Period = period;
		TIM_TimeBaseStructure.TIM_Prescaler = 71;
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
		TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
		TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
		TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
		TIM_ICInitStructure.TIM_ICFilter = 0x0;
		TIM_ICInit(TIM3, &TIM_ICInitStructure);

		TIM_ITConfig(TIM3, TIM_IT_CC2 | TIM_IT_Update, ENABLE);
		
		TIM_Cmd(TIM3, ENABLE);
}

