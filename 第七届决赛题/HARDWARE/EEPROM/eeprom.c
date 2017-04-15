#include "eeprom.h"
#include "i2c.h"

void x24c02_write(u8 addr, unsigned char dat)
{
		I2CStart();
		I2CSendByte(0xa0);
		I2CWaitAck();
	
		I2CSendByte(addr);
		I2CWaitAck();
	
		I2CSendByte(dat);
		I2CWaitAck();
		I2CStop();	
}


char x24c02_read(u8 addr)
{
		char val;
		I2CStart();
		I2CSendByte(0xa0);
		I2CWaitAck();
	
		I2CSendByte(addr);
		I2CWaitAck();
		
		I2CStart();
		I2CSendByte(0xa1);
		I2CWaitAck();
	
		val = I2CReceiveByte();
		I2CWaitAck();
		I2CStop();	
		return val;
		
}
