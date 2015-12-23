#ifndef UART_H_
#define UART_H_

//#include "gtar.h"
#include "../../Common/EHM.h"
#include "../../Device/Device.h"

#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "driverlib/uart.h"

#include "driverlib/debug.h"	// Needed for ASSERT

RESULT InitUART0(uint32_t  ulBaudRate);
uint32_t  UARTwrite(const int8_t *pcBuf, uint32_t  ulLen);
RESULT UART0printf(const int8_t *pcString, ...);


#endif /* UART_H_ */

