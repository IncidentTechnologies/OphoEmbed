//*****************************************************************************
//
// uartstdio.c - Utility driver to provide simple UART console functions.
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

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "uartstdio.h"

// The base address of the chosen UART.
static uint32_t  g_ulBase = 0;

// A mapping from an integer between 0 and 15 to its ASCII character equivalent.
static const int8_t * const g_pcHex = "0123456789abcdef";

// The list of possible base addresses for the console UART.
static const uint32_t  g_ulUARTBase[3] = {
    UART0_BASE, UART1_BASE, UART2_BASE
};

// The list of UART peripherals.
static const uint32_t  g_ulUARTPeriph[3] = {
    SYSCTL_PERIPH_UART0, SYSCTL_PERIPH_UART1, SYSCTL_PERIPH_UART2
};

//*****************************************************************************
//! Initializes the UART console.
//!
//! \param ulPortNum is the number of UART port to use for the serial console
//! (0-2)
//!
//! This function will initialize the specified serial port to be used as a
//! serial console.  The serial parameters will be set to 115200, 8-N-1.
//! An application wishing to use a different baud rate may call
//! UARTStdioInitExpClk() instead of this function.
//!
//! This function or UARTStdioInitExpClk() must be called prior to using any
//! of the other UART console functions: UARTprintf() or UARTgets().  In order
//! for this function to work correctly, SysCtlClockSet() must be called prior
//! to calling this function.
//!
//! It is assumed that the caller has previously configured the relevant UART
//! pins for operation as a UART rather than as GPIOs.
//!
//! \return None.
//*****************************************************************************
void UARTStdioInit(uint32_t  ulPortNum) {
    // Pass this call on to the version of the function allowing the baud rate
    // to be specified.
    UARTStdioInitExpClk(ulPortNum, 115200);
}

//*****************************************************************************
//
//! Initializes the UART console and allows the baud rate to be selected.
//!
//! \param ulPortNum is the number of UART port to use for the serial console
//! (0-2)
//! \param ulBaud is the bit rate that the UART is to be configured to use.
//!
//! This function will initialize the specified serial port to be used as a
//! serial console.  The serial parameters will be set to 8-N-1 and the bit
//! rate set according to the value of the \e ulBaud parameter.
//!
//! This function or UARTStdioInit() must be called prior to using any of the
//! other UART console functions: UARTprintf() or UARTgets().  In order for
//! this function to work correctly, SysCtlClockSet() must be called prior to
//! calling this function.  An application wishing to use 115,200 baud may call
//! UARTStdioInit() instead of this function but should not call both
//! functions.
//!
//! It is assumed that the caller has previously configured the relevant UART
//! pins for operation as a UART rather than as GPIOs.
//!
//! \return None.
//
//*****************************************************************************
void UARTStdioInitExpClk(uint32_t  ulPortNum, uint32_t  ulBaud) {
    // Check the arguments.
    ASSERT((ulPortNum == 0) || (ulPortNum == 1) ||
           (ulPortNum == 2));

    // Check to make sure the UART peripheral is present.
    if(!MAP_SysCtlPeripheralPresent(g_ulUARTPeriph[ulPortNum]))
        return;

    // Select the base address of the UART.
    g_ulBase = g_ulUARTBase[ulPortNum];

    // Enable the UART peripheral for use.
    MAP_SysCtlPeripheralEnable(g_ulUARTPeriph[ulPortNum]);


    // Configure the UART CLK
    MAP_UARTConfigSetExpClk(g_ulBase, ROM_SysCtlClockGet(), ulBaud,
                            (UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE | UART_CONFIG_WLEN_8));

    // Enable the UART operation.
    UARTEnable(g_ulBase);
}

//*****************************************************************************
//! Writes a string of characters to the UART output.
//!
//! \param pcBuf points to a buffer containing the string to transmit.
//! \param ulLen is the length of the string to transmit.
//!
//! This function will transmit the string to the UART output.  The number of
//! characters transmitted is determined by the \e ulLen parameter.  This
//! function does no interpretation or translation of any characters.  Since
//! the output is sent to a UART, any LF (/n) characters encountered will be
//! replaced with a CRLF pair.
//!
//! Besides using the \e ulLen parameter to stop transmitting the string, if a
//! null character (0) is encountered, then no more characters will be
//! transmitted and the function will return.
//!
//! In non-buffered mode, this function is blocking and will not return until
//! all the characters have been written to the output FIFO.  In buffered mode,
//! the characters are written to the UART transmit buffer and the call returns
//! immediately.  If insufficient space remains in the transmit buffer,
//! additional characters are discarded.
//!
//! \return Returns the count of characters written.
//*****************************************************************************
uint32_t  UARTwrite(const int8_t *pcBuf, uint32_t  ulLen) {
    uint32_t  uIdx;

    // Check for valid UART base address, and valid arguments.
    ASSERT(g_ulBase != 0);
    ASSERT(pcBuf != 0);

    for(uIdx = 0; uIdx < ulLen; uIdx++) {
        /*if(pcBuf[uIdx] == '\n')
            UARTCharPut(g_ulBase, '\r');*/

        // Send the character to the UART output.
        MAP_UARTCharPut(g_ulBase, pcBuf[uIdx]);
    }

    // Return the number of characters written.
    return(uIdx);
}

//*****************************************************************************
//
//! A simple UART based get string function, with some line processing.
//!
//! \param pcBuf points to a buffer for the incoming string from the UART.
//! \param ulLen is the length of the buffer for storage of the string,
//! including the trailing 0.
//!
//! This function will receive a string from the UART input and store the
//! characters in the buffer pointed to by \e pcBuf.  The characters will
//! continue to be stored until a termination character is received.  The
//! termination characters are CR, LF, or ESC.  A CRLF pair is treated as a
//! single termination character.  The termination characters are not stored in
//! the string.  The string will be terminated with a 0 and the function will
//! return.
//!
//! In both buffered and unbuffered modes, this function will block until
//! a termination character is received.  If non-blocking operation is required
//! in buffered mode, a call to UARTPeek() may be made to determine whether
//! a termination character already exists in the receive buffer prior to
//! calling UARTgets().
//!
//! Since the string will be null terminated, the user must ensure that the
//! buffer is sized to allow for the additional null character.
//!
//! \return Returns the count of characters that were stored, not including
//! the trailing 0.
//
//*****************************************************************************
uint32_t  UARTgets(int8_t *pcBuf, uint32_t  ulLen) {
    uint32_t  ulCount = 0;
    int8_t cChar;
    static int8_t bLastWasCR = 0;

    // Check the arguments.
    ASSERT(pcBuf != 0);
    ASSERT(ulLen != 0);
    ASSERT(g_ulBase != 0);

    // Adjust the length back by 1 to leave space for the trailing null terminator.
    ulLen--;

    // Process characters until a newline is received.
    while(1) {
        // Read the next character from the console.
        cChar = UARTCharGet(g_ulBase);

        // See if the backspace key was pressed.
        if(cChar == '\b') {
            // If there are any characters already in the buffer, then delete the last.
            if(ulCount) {
                // Rub out the previous character.
                UARTwrite("\b \b", 3);

                // Decrement the number of characters in the buffer.
                ulCount--;
            }

            // Skip ahead to read the next character.
            continue;
        }

        // If this character is LF and last was CR, then just gobble up the
        // character because the EOL processing was taken care of with the CR.
        if((cChar == '\n') && bLastWasCR) {
            bLastWasCR = 0;
            continue;
        }

        // See if a newline or escape character was received.
        if((cChar == '\r') || (cChar == '\n') || (cChar == 0x1b)) {
            // If the character is a CR, then it may be followed by a LF which
            // should be paired with the CR.  So remember that a CR was
            // received.
            if(cChar == '\r')
                bLastWasCR = 1;

            // Stop processing the input and end the line.
            break;
        }

        // Process the received character as long as we are not at the end of
        // the buffer.  If the end of the buffer has been reached then all
        // additional characters are ignored until a newline is received.
        if(ulCount < ulLen) {
            // Store the character in the caller supplied buffer.
            pcBuf[ulCount] = cChar;

            // Increment the count of characters received.
            ulCount++;

            // Reflect the character back to the user.
            MAP_UARTCharPut(g_ulBase, cChar);
        }
    }

    // Add a null termination to the string.
    pcBuf[ulCount] = 0;

    // Send a CRLF pair to the terminal to end the line.
    UARTwrite("\r\n", 2);

    // Return the count of chars in the buffer, not counting the trailing 0.
    return(ulCount);
}

//*****************************************************************************
//! Read a single character from the UART, blocking if necessary.
//!
//! This function will receive a single character from the UART and store it at
//! the supplied address.
//!
//! In both buffered and unbuffered modes, this function will block until a
//! character is received.  If non-blocking operation is required in buffered
//! mode, a call to UARTRxAvail() may be made to determine whether any
//! characters are currently available for reading.
//!
//! \return Returns the character read.
//*****************************************************************************
uint8_t UARTgetc(void) {
    // Block until a character is received by the UART then return it to the caller.
    return(UARTCharGet(g_ulBase));
}

//*****************************************************************************
//! A simple UART based printf function supporting \%c, \%d, \%p, \%s, \%u,
//! \%x, and \%X.
//!
//! \param pcString is the format string.
//! \param ... are the optional arguments, which depend on the contents of the
//! format string.
//!
//! This function is very similar to the C library <tt>fprintf()</tt> function.
//! All of its output will be sent to the UART.  Only the following formatting
//! characters are supported:
//!
//! - \%c to print a character
//! - \%d to print a decimal value
//! - \%s to print a string
//! - \%u to print an unsigned decimal value
//! - \%x to print a hexadecimal value using lower case letters
//! - \%X to print a hexadecimal value using lower case letters (not upper case
//! letters as would typically be used)
//! - \%p to print a pointer as a hexadecimal value
//! - \%\% to print out a \% character
//!
//! For \%s, \%d, \%u, \%p, \%x, and \%X, an optional number may reside
//! between the \% and the format character, which specifies the minimum number
//! of characters to use for that value; if preceded by a 0 then the extra
//! characters will be filled with zeros instead of spaces.  For example,
//! ``\%8d'' will use eight characters to print the decimal value with spaces
//! added to reach eight; ``\%08d'' will use eight characters as well but will
//! add zeroes instead of spaces.
//!
//! The type of the arguments after \e pcString must match the requirements of
//! the format string.  For example, if an integer was passed where a string
//! was expected, an error of some kind will most likely occur.
//!
//! \return None.
//*****************************************************************************
void UARTprintf(const int8_t *pcString, ...) {
    uint32_t  ulIdx, ulValue, ulPos, ulCount, ulBase, ulNeg;
    int8_t *pcStr, pcBuf[16], cFill;
    va_list vaArgP;

    // Check the arguments.
    ASSERT(pcString != 0);

    // Start the varargs processing.
    va_start(vaArgP, pcString);

    // Loop while there are more characters in the string.
    while(*pcString) {
        // Find the first non-% character, or the end of the string.
        for(ulIdx = 0; (pcString[ulIdx] != '%') && (pcString[ulIdx] != '\0');
            ulIdx++)
        {/*empty stub*/}

        // Write this portion of the string.
        UARTwrite(pcString, ulIdx);

        // Skip the portion of the string that was written.
        pcString += ulIdx;

        // See if the next character is a %.
        if(*pcString == '%') {
            // Skip the %.
            pcString++;

            // Set the digit count to zero, and the fill character to space (i.e. to the defaults).
            ulCount = 0;
            cFill = ' ';

            // It may be necessary to get back here to process more characters.
            // Goto's aren't pretty, but effective.  I feel extremely dirty for
            // using not one but two of the beasts.
again:

            // Determine how to handle the next character.
            switch(*pcString++) {
                // Handle the digit characters.
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                    // If this is a zero, and it is the first digit, then the
                    // fill character is a zero instead of a space.
                    if((pcString[-1] == '0') && (ulCount == 0))
                        cFill = '0';

                    // Update the digit count.
                    ulCount *= 10;
                    ulCount += pcString[-1] - '0';

                    // Get the next character.
                    goto again;
                }

                // Handle the %c command.
                case 'c': {
                    // Get the value from the varargs.
                    ulValue = va_arg(vaArgP, uint32_t );

                    // Print out the character.
                    UARTwrite((int8_t *)&ulValue, 1);
                } break;

                // Handle the %d command.
                case 'd': {
                    // Get the value from the varargs.
                    ulValue = va_arg(vaArgP, uint32_t );

                    // Reset the buffer position.
                    ulPos = 0;

                    // If the value is negative, make it positive and indicate
                    // that a minus sign is needed.
                    if(( int32_t )ulValue < 0) {
                        // Make the value positive.
                        ulValue = -( int32_t )ulValue;

                        // Indicate that the value is negative.
                        ulNeg = 1;
                    }
                    else {
                        // Indicate that the value is positive so that a minus
                        // sign isn't inserted.
                        ulNeg = 0;
                    }

                    // Set the base to 10.
                    ulBase = 10;

                    // Convert the value to ASCII.
                    goto convert;
                }

                // Handle the %s command.
                case 's': {
                    // Get the string pointer from the varargs.
                    pcStr = va_arg(vaArgP, int8_t *);

                    // Determine the length of the string.
                    for(ulIdx = 0; pcStr[ulIdx] != '\0'; ulIdx++)
                    {/*empty*/}

                    // Write the string.
                    UARTwrite(pcStr, ulIdx);

                    // Write any required padding spaces
                    if(ulCount > ulIdx) {
                        ulCount -= ulIdx;
                        while(ulCount--)
                            UARTwrite(" ", 1);
                    }
                } break;

                // Handle the %u command.
                case 'u': {
                    // Get the value from the varargs.
                    ulValue = va_arg(vaArgP, uint32_t );

                    // Reset the buffer position.
                    ulPos = 0;

                    // Set the base to 10.
                    ulBase = 10;

                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    ulNeg = 0;

                    // Convert the value to ASCII.
                    goto convert;
                }

                // Handle the %x and %X commands.  Note that they are treated
                // identically; i.e. %X will use lower case letters for a-f
                // instead of the upper case letters is should use.  We also
                // alias %p to %x.
                case 'x':
                case 'X':
                case 'p': {
                    // Get the value from the varargs.
                    ulValue = va_arg(vaArgP, uint32_t );

                    // Reset the buffer position.
                    ulPos = 0;

                    // Set the base to 16.
                    ulBase = 16;

                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    ulNeg = 0;

                    // Determine the number of digits in the string version of
                    // the value.
convert:
                    for(ulIdx = 1;
                        (((ulIdx * ulBase) <= ulValue) &&
                         (((ulIdx * ulBase) / ulBase) == ulIdx));
                        ulIdx *= ulBase, ulCount--)
                    {/*empty*/}

                    // If the value is negative, reduce the count of padding
                    // characters needed.
                    if(ulNeg)
                        ulCount--;

                    // If the value is negative and the value is padded with
                    // zeros, then place the minus sign before the padding.
                    if(ulNeg && (cFill == '0')) {
                        // Place the minus sign in the output buffer.
                        pcBuf[ulPos++] = '-';

                        // The minus sign has been placed, so turn off the
                        // negative flag.
                        ulNeg = 0;
                    }

                    // Provide additional padding at the beginning of the
                    // string conversion if needed.
                    if((ulCount > 1) && (ulCount < 16))
                        for(ulCount--; ulCount; ulCount--)
                            pcBuf[ulPos++] = cFill;

                    // If the value is negative, then place the minus sign
                    // before the number.
                    if(ulNeg) {
                        // Place the minus sign in the output buffer.
                        pcBuf[ulPos++] = '-';
                    }

                    // Convert the value into a string.
                    for(; ulIdx; ulIdx /= ulBase)
                        pcBuf[ulPos++] = g_pcHex[(ulValue / ulIdx) % ulBase];

                    // Write the string.
                    UARTwrite(pcBuf, ulPos);
                } break;

                // Handle the %% command.
                case '%': {
                    // Simply write a single %.
                    UARTwrite(pcString - 1, 1);
                } break;

                // Handle all other commands.
                default: {
                    // Indicate an error.
                    UARTwrite("ERROR", 5);
                } break;
            }
        }
    }

    //
    // End the varargs processing.
    //
    va_end(vaArgP);
}
