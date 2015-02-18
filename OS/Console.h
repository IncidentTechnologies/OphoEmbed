#ifndef CONSOLE_H_
#define CONSOLE_H_

/* Console.h
 * A console designed for basic embedded work
 */ 

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../DS/SmartBuffer.h"
#include "../DS/CircleBuffer.h"

#define BASE_CONSOLE_PATH_STRING "umb:$"

#define MAX_COMMAND 256
#define MAX_OUTPUT
#define MAX_CONSOLE_HISTORY 10
#define MAX_CONSOLE_COMMANDS 30
#define MAX_CONSOLE_ARGS 7

// Buffer manipulation macros
#define ADD_CHAR_TO_STATIC_BUFFER(buffer, c) buffer[buffer ## _n] = c; buffer ## _n++;
#define DELETE_CHAR_FROM_STATIC_BUFFER(buffer) if(buffer ## _n > 0) {               \
                                                   buffer ## _n--;                  \
                                                   buffer[buffer ## _n] = '\0';     \
                                               }

// Function Pointer templates
// we treat these as possibly having any input/output values with a 
// void* pointer. It is up to the function implementation / call to handle these
// since Console does this it is handled then.  Currently we only handle
// a maximum of 7 input arguments
typedef void*(*FP0)();
typedef void*(*FP1)(void*);
typedef void*(*FP2)(void*, void*);
typedef void*(*FP3)(void*, void*, void*);
typedef void*(*FP4)(void*, void*, void*, void*);
typedef void*(*FP5)(void*, void*, void*, void*, void*);
typedef void*(*FP6)(void*, void*, void*, void*, void*, void*);
typedef void*(*FP7)(void*, void*, void*, void*, void*, void*, void*);

typedef struct CONSOLE_FUNCTION {
    int ID;
    void *Function;
    char *pszCommand;
	char **ppszDefaultArguments;	// Default arguments
	int ppszDefaultArguments_n;		// Number of default arguments
    int arguments;
    char *pszHelpText;
} ConsoleFunction;

typedef struct {
	char m_InputBuffer[MAX_COMMAND];
	int m_InputBuffer_n;

	PSZCircleBuffer* m_InputBufferHistory;
	PSZCircleBuffer* m_OutputBufferHistory;

	SmartBuffer *m_pOutputBuffer;

	ConsoleFunction m_Commands[MAX_CONSOLE_COMMANDS];
	unsigned char m_Commands_n;

	long int m_nFunctionID;
} Console;

extern Console *g_pConsole;

Console* CreateConsole();
RESULT DeleteConsole(Console *pc);

RESULT ReceiveInput(Console *pc, char** ppd_input, int ppd_input_n);
RESULT ReceiveInputChar(Console *pc, char c);

RESULT CheckForOutput(Console *pc);
RESULT DispatchOutput(Console *pc, char **ppn_output, int *pn_output_n);
RESULT DumpOutputHistory(Console *pc);

// Console maintains the output buffer
// through these functions
RESULT PrintToOutputBinaryBuffer(Console *pc, unsigned char *Buffer, int Buffer_n, int itemsPerRow);
RESULT PrintToOutputPsz(Console *pc, char* pszOutput);
RESULT PrintToOutputChar(Console *pc, char c);
RESULT PrintToOutput(Console *pc, const char* output_format, ...);
RESULT PrintNewline(Console *pc);
RESULT PrintPath(Console *pc, char *pszOptString);
RESULT ClearLine(Console *pc);
RESULT RefreshCommandLine(Console *pc, char *pszOptString);
RESULT ClearCommandLine(Console *pc);

RESULT AddConsoleFunction(Console *pc, ConsoleFunction cf);
RESULT AddConsoleFunctionByArgs(Console *pc, void *fun, char *pszCommand, int arguments, int nDefaultArgs, ... );
ConsoleFunction CreateConsoleFunction(int ID, void *fun, char *pszCommand, int arguments, int nDefaultArgs, ... );

RESULT ExecuteConsoleFunction(Console *pc, char *pszCommand, int pszCommand_n);
RESULT ConvertToArgumentList(Console*, char*, char***, int*);

char** GetInputBufferHistory(Console *pc);
ConsoleFunction* GetCommandList(Console *pc);

RESULT ResetCommandBuffer(Console *pc, char *pszOptString);
RESULT Path(Console *pc, char** ppn_pszPath);

// Reserved Functions
// All functions must take one input which is a reference to the Console itself
RESULT Exit(Console* pc);
RESULT History(Console* pc);
RESULT Print(Console* pc, char *pszString);
RESULT List(Console *pc);
RESULT Help(Console* pc, char *pszFunctionName);
RESULT Hi(Console* pc);

typedef enum ConsoleReservedCommands {
    CONSOLE_EXIT,
	CONSOLE_HISTORY,
    CONSOLE_PRINT,
	CONSOLE_LIST,
    CONSOLE_HELP,
    CONSOLE_HI,
    CONSOLE_RESERVED_END
} CONSOLE_RESERVED_COMMANDS;

static ConsoleFunction ReservedFunctions[] = {
    {
        CONSOLE_EXIT,
        Exit,
        "exit",
		NULL,
		0,
        1,
        NULL
    },
	{
		CONSOLE_HISTORY,
		History,
		"history",
		NULL,
		0,
		1,
		NULL
	},
    {
        CONSOLE_PRINT,
        Print,
        "print",
		NULL,
		0,
        2,
        NULL
    },
	{
		CONSOLE_LIST,
		List,
		"list",
		NULL,
		0,
		1,
		NULL
	},
    {
        CONSOLE_HELP,
        Help,
        "help",
		NULL,
		0,
        2,
        NULL
    },
    {
		CONSOLE_HI,
		Hi,
		"hi",
		NULL,
		0,
		1,
		NULL
    }
};
static int ReservedFunctions_n = sizeof(ReservedFunctions) / sizeof(ReservedFunctions[0]);

typedef enum CONSOLE_STATE_MACHINE {
	CONSOLE_STATE_NORMAL,
	CONSOLE_STATE_ESC,
	CONSOLE_STATE_ESC_BRACKET,
	CONSOLE_STATE_INVALID
} CONSOLE_STATE;

extern CONSOLE_STATE g_ConsoleState;




//RESULT TestConsole();

#endif // CONSOLE_H_
