#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

#define RB1 GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)
#define RB2 GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8)
#define RB3 GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1)
#define RB4 GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_2)


void Key_init(void);
u8 Key_scan(void);

#endif
