#ifndef __UART_H
#define __UART_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

void Uart2_init(void);
void Uart2_sendString(u8 *str);

#endif
