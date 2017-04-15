#ifndef  __EEPROM_H__
#define  __EEPROM_H__
#include "stm32f10x.h"

void x24c02_write(u8 addr, unsigned char dat);
char x24c02_read(u8 addr);

#endif
