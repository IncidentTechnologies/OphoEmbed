#include "uart_init.h"
#include <stdarg.h>

static const int8_t * const g_pcHex = "0123456789abcdef";

RESULT InitUART0(uint32_t  ulBaudRate) {
	RESULT r = R_OK;

	if(!g_device.m_fUART0) g_device.m_fUART0 = true;
	else return R_NO_EFFECT;

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Set up Stdio with UART0 (UARTprintf etc)
    UARTStdioInitExpClk(0, ulBaudRate);

	//ROM_IntEnable(INT_UART0);
    //ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);

	ROM_UARTIntClear(UART0_BASE, ROM_UARTIntStatus(UART0_BASE, false));
	ROM_UARTIntEnable(UART0_BASE, (UART_INT_OE | UART_INT_BE | UART_INT_PE |
	                      UART_INT_FE | UART_INT_RT | UART_INT_TX | UART_INT_RX));

	DEBUG_LINEOUT("UART0 initialized");

Error:
	return r;
}

uint32_t  g_i;
int32_t  UART0write(const int8_t *pcBuf, uint32_t  ulLen) {
    for(g_i = 0; g_i < ulLen; g_i++) {
        if(pcBuf[g_i] == '\n')
            UARTCharPut(UART0_BASE, '\r');

        // Send the character to the UART output.
        ROM_UARTCharPut(UART0_BASE, pcBuf[g_i]);
    }

    // Return the number of characters written.
    return g_i;
}

RESULT UART0printf(const int8_t *pcString, ...) {
	RESULT r = R_OK;

	float fValue;
    uint32_t  ulIdx, ulValue, ulPos, ulCount, ulBase, ulNeg;
    int8_t *pcStr, pcBuf[16], cFill;
    va_list vaArgP;

    // Check the arguments.
    CNRM((pcString != NULL), "UART0printf: pcString cannot be null");

    // Start the varargs processing.
    va_start(vaArgP, pcString);

    // Loop while there are more characters in the string.
    while(*pcString) {
        // Find the first non-% character, or the end of the string.
        for(ulIdx = 0; (pcString[ulIdx] != '%') && (pcString[ulIdx] != '\0');
            ulIdx++)
        {/*empty stub*/}

        // Write this portion of the string.
        UART0write(pcString, ulIdx);

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
                    UART0write((int8_t *)&ulValue, 1);
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
                    UART0write(pcStr, ulIdx);

                    // Write any required padding spaces
                    if(ulCount > ulIdx) {
                        ulCount -= ulIdx;
                        while(ulCount--)
                            UART0write(" ", 1);
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
                    UART0write(pcBuf, ulPos);
                } break;

                // Handle the %% command.
                case '%': {
                    // Simply write a single %.
                    UART0write(pcString - 1, 1);
                } break;

                // Handle all other commands.
                default: {
                    // Indicate an error.
                    UART0write("ERROR", 5);
                } break;
            }
        }
    }

    // End the varargs processing.
    va_end(vaArgP);

Error:
	return r;
}
