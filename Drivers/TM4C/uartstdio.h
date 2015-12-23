//*****************************************************************************
//
// uartstdio.h - Prototypes for the UART console functions.
//
// Copyright (c) 2007-2011 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 8264 of the Stellaris Firmware Development Package.
//
//*****************************************************************************

#ifndef __UARTSTDIO_H__
#define __UARTSTDIO_H__

// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
#ifdef __cplusplus
extern "C"
{
#endif

// Prototypes for the APIs.
extern void UARTStdioInit(uint32_t  ulPort);
extern void UARTStdioInitExpClk(uint32_t  ulPort, uint32_t  ulBaud);
extern uint32_t  UARTgets(int8_t *pcBuf, uint32_t  ulLen);
extern uint8_t UARTgetc(void);
extern void UARTprintf(const int8_t *pcString, ...);
extern uint32_t  UARTwrite(const int8_t *pcBuf, uint32_t  ulLen);

// Mark the end of the C bindings section for C++ compilers.
#ifdef __cplusplus
}
#endif

#endif // __UARTSTDIO_H__
