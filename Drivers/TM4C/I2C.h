#ifndef I2C_H_
#define I2C_H_

#include "../../Common/EHM.h"
#include "../../Device/Device.h"

#include "inc/hw_i2c.h"
#include "inc/hw_types.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "driverlib/i2c.h"	// I2C include

// This includes the initialization functions for I2C 
// as well as the read/write functions

// I2C
RESULT InitI2C0(uint8_t fAuthChipOn);
RESULT InitI2C1();
RESULT WriteI2C(uint32_t  ulI2CBase, uint8_t addr, uint8_t *pBuffer, int32_t pBuffer_n, uint8_t reg, int32_t *pBytesWritten);	
RESULT ReadI2C(uint32_t  ulI2CBase,	uint8_t addr,	uint8_t *pBuffer, int32_t pBuffer_n, uint8_t reg, int32_t *pLength);
RESULT I2CSend(		uint32_t   ulI2CBase, uint8_t ucAddr, uint8_t *p_ucBuffer, uint32_t  p_ucBuffer_n, uint32_t  *p_uiBytesWritten, uint32_t  uiDelay,	uint32_t  uiTimeout);
RESULT I2CReceive(	uint32_t   ulI2CBase, uint8_t ucAddr, uint8_t *p_ucBuffer, uint32_t  p_ucBuffer_n, uint32_t  *p_uiBytesRead, uint32_t  uiDelay, uint32_t  uiTimeout);
#endif /*I2C_H_*/
