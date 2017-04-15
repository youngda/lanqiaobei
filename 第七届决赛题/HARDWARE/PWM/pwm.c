#include "pwm.h"


void Pwm_Init(u16 period, u16 pulse)
{
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		TIM_OCInitTypeDef  TIM_OCInitStructure;
		GPIO_InitTypeDef GPIO_InitStructure;
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_AFIO , ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
		
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		TIM_TimeBaseStructure.TIM_Period = period-1;
		TIM_TimeBaseStructure.TIM_Prescaler = 71;
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_Pulse = pulse;
		TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
		TIM_OC2Init(TIM2, &TIM_OCInitStructure);

		TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
		TIM_ARRPreloadConfig(TIM2, ENABLE);

  /* TIM3 enable counter */
		TIM_Cmd(TIM2, ENABLE);
	
}
