#include "Console.h"

Console *g_pConsole;

CONSOLE_STATE g_ConsoleState = CONSOLE_STATE_NORMAL;

// Reserved Functions
// *************************************
RESULT Exit(Console *pc) {
    exit(0);
    return R_OK;
}

RESULT History(Console *pc) {
	RESULT r = R_OK;
	CR(DumpOutputHistory(pc));
Error:
	return R_OK;
}

RESULT Print(Console *pc, char *pszString) {
    RESULT r = R_OK;
	CR(PrintToOutput(pc, pszString));
Error:
    return r;
}

RESULT Help(Console *pc, char *pszFunctionName) {
    RESULT r = R_OK;

    CR(PrintToOutput(pc, "wowa:%s", pszFunctionName));

Error:
    return r;
}

RESULT Hi(Console *pc) {
    RESULT r = R_OK;

    CR(PrintToOutput(pc, "Hello there!"));

Error:
    return r;
}

// Will list off all the available functions
RESULT List(Console *pc) {
	RESULT r = R_OK;

	ConsoleFunction *ppCmdList = GetCommandList(pc);
	unsigned char ppCmdList_n = pc->m_Commands_n;
	unsigned char i = 0;

	for(i = 0; i < ppCmdList_n; i++) {
		char *pszTemp = ppCmdList[i].pszCommand;
		CR(PrintToOutput(pc, pszTemp));
	}

Error:
	return r;
}
// **************************************

char** GetInputBufferHistory(Console *c) {
	return c->m_InputBufferHistory;
}

ConsoleFunction* GetCommandList(Console *c){
	return c->m_Commands;
}

RESULT ConvertToArgumentList(Console *c, char *pd_InputBuffer, char*** pppn_Args, int *pppn_Args_n) {
    RESULT r = R_OK;

    // TODO: Seems to help?
    int x = 0;
    size_t top_of_stack = (size_t) &x;
    char *cc = (char)malloc(sizeof(char));
    size_t top_of_heap = (size_t) cc;

    int Args_n = 0;
    char *TempChar = strtok(pd_InputBuffer, " -,;");	// dont move
    char **ppArgsList = (char**)malloc(sizeof(char*) * MAX_CONSOLE_ARGS);

    while(TempChar != NULL) {
    	ppArgsList[Args_n] = (char*)calloc(strlen(TempChar) + 1, sizeof(char));
    	memcpy(ppArgsList[Args_n], TempChar, strlen(TempChar));
    	Args_n++;

        TempChar = strtok(NULL, " -,;");
    }

    *pppn_Args = ppArgsList;
    *pppn_Args_n = Args_n;

Error:
    //delete d_InputBuffer;
    return r;
}

RESULT ExecuteConsoleFunction(Console *pc, char *pszCommand, int pszCommand_n) {
    RESULT r = R_OK;
    char TempCharBuffer[MAX_COMMAND];

    // First copy over the buffer to our own thing
    char *TempBuffer = (char *)calloc((pszCommand_n + 1), sizeof(char));
    strcpy(TempBuffer, pszCommand);

    // Parse the command buffer and extract the cmd and params
    char** Args = NULL;
    int Args_n;
	unsigned char fUseDefaultArguments = false;
    CRM_NA(ConvertToArgumentList(pc, TempBuffer, &Args, &Args_n), "Failed to convert command to arguments list");

    // Search for the command
    ConsoleFunction *ppCmdList = GetCommandList(pc);
	unsigned char ppCmdList_n = pc->m_Commands_n;
	unsigned char i = 0;
	unsigned char fFoundCmd = false;
    for(i = 0; i < ppCmdList_n; i++) {
        if(strcmp(ppCmdList[i].pszCommand, Args[0]) == 0) {
            fFoundCmd = true;
            break;
        }
    }

    // Check to see that we found the command
    if(!fFoundCmd) goto Error;	// TODO: fix with EHM

    // Add the current command to the history buffer if valid
	char *pszInputHistory = (char*)malloc(sizeof(char) * pc->m_InputBuffer_n + 1);
	memset(pszInputHistory, 0, pc->m_InputBuffer_n + 1);
	strcpy(pszInputHistory, pc->m_InputBuffer);
	PSZCircleBufferPush(pc->m_InputBufferHistory, pszInputHistory);

    // Check to see that the number of parser arguments matches the function
	// and if not we check if the function has default arguments
    if(ppCmdList[i].arguments != Args_n ) {
		if(ppCmdList[i].ppszDefaultArguments_n > 0) {
			fUseDefaultArguments = true;	
		}
		else {
			PrintToOutput(pc, "Expected arguments %d got %d", ppCmdList[i].arguments, Args_n);
			goto Error;
		}
    }

    CRM_NA(ClearCommandLine(pc), "Failed to clear command line");

	int CallbackRet = -1;
    switch(ppCmdList[i].arguments) {
        case 1: {
            CallbackRet = (int)((FP1)(ppCmdList[i].Function))(pc);
            PrintToOutput(pc, "Callback returned:%d", CallbackRet);
        } break;

		case 2: {
			if(!fUseDefaultArguments)
				CallbackRet = (int)((FP2)(ppCmdList[i].Function))(pc, Args[1]);
			else
				CallbackRet = (int)((FP2)(ppCmdList[i].Function))(pc, ppCmdList[i].ppszDefaultArguments[0]);

			PrintToOutput(pc, "Callback returned: %d", CallbackRet);
		} break;

		case 3: {
			if(!fUseDefaultArguments)
				CallbackRet = (int)((FP3)(ppCmdList[i].Function))(pc, Args[1], Args[2]);
			else
				CallbackRet = (int)((FP3)(ppCmdList[i].Function))(pc, ppCmdList[i].ppszDefaultArguments[0],
																	  ppCmdList[i].ppszDefaultArguments[1]);

			PrintToOutput(pc, "Callback returned: %d", CallbackRet);
		} break;

		case 4: {
			if(!fUseDefaultArguments)
				CallbackRet = (int)((FP4)(ppCmdList[i].Function))(pc, Args[1], Args[2], Args[3]);
			else
				CallbackRet = (int)((FP4)(ppCmdList[i].Function))(pc, ppCmdList[i].ppszDefaultArguments[0],
																	  ppCmdList[i].ppszDefaultArguments[1],
																	  ppCmdList[i].ppszDefaultArguments[2]);
			PrintToOutput(pc, "Callback returned: %d", CallbackRet);
		} break;

		case 5: {
			if(!fUseDefaultArguments)
				CallbackRet = (int)((FP5)(ppCmdList[i].Function))(pc, Args[1], Args[2], Args[3], Args[4]);
			else
				CallbackRet = (int)((FP5)(ppCmdList[i].Function))(pc, ppCmdList[i].ppszDefaultArguments[0],
																	  ppCmdList[i].ppszDefaultArguments[1],
																	  ppCmdList[i].ppszDefaultArguments[2],
																      ppCmdList[i].ppszDefaultArguments[3]);
			PrintToOutput(pc, "Callback returned: %d", CallbackRet);
		} break;

		case 6: {
			if(!fUseDefaultArguments)
				CallbackRet = (int)((FP6)(ppCmdList[i].Function))(pc, Args[1], Args[2], Args[3], Args[4], Args[5]);
			else
				CallbackRet = (int)((FP6)(ppCmdList[i].Function))(pc, ppCmdList[i].ppszDefaultArguments[0],
																	  ppCmdList[i].ppszDefaultArguments[1],
																	  ppCmdList[i].ppszDefaultArguments[2],
																	  ppCmdList[i].ppszDefaultArguments[3],
																	  ppCmdList[i].ppszDefaultArguments[4]);
			PrintToOutput(pc, "Callback returned: %d", CallbackRet);
		} break;

		case 7: {
			if(!fUseDefaultArguments)
				CallbackRet = (int)((FP7)(ppCmdList[i].Function))(pc, Args[1], Args[2], Args[3], Args[4], Args[5], Args[6]);
			else
				CallbackRet = (int)((FP7)(ppCmdList[i].Function))(pc, ppCmdList[i].ppszDefaultArguments[0],
																	  ppCmdList[i].ppszDefaultArguments[1],
																	  ppCmdList[i].ppszDefaultArguments[2],
																	  ppCmdList[i].ppszDefaultArguments[3],
																	  ppCmdList[i].ppszDefaultArguments[4],
																	  ppCmdList[i].ppszDefaultArguments[5]);
			PrintToOutput("Callback returned: %d", CallbackRet);
		} break;
    
        default: {
            PrintToOutput("Console does not support %d arguments", ppCmdList[i].arguments);
        } break;
    }

    //delete TempBuffer;
    return (RESULT)CallbackRet;

Error:
    PrintToOutput(pc, "Could not execute:%s", pszCommand);
    //delete TempBuffer;
    return r;
}

Console* CreateConsole() {
	int i = 0;

	Console *pc = (Console*)malloc(sizeof(Console));
	memset(pc, 0, sizeof(Console));
	pc->m_nFunctionID = CONSOLE_RESERVED_END;

	pc->m_InputBufferHistory = CreatNewPSZCircleBuffer(MAX_CONSOLE_HISTORY);
	pc->m_OutputBufferHistory = CreatNewPSZCircleBuffer(MAX_CONSOLE_HISTORY);

    pc->m_pOutputBuffer = CreateSmartBuffer(NULL, 0);
    
    memset(pc->m_InputBuffer, 0, sizeof(pc->m_InputBuffer));

    // Add the reserved functions
    for(i = 0; i < ReservedFunctions_n; i++)
        AddConsoleFunction(pc, ReservedFunctions[i]);

    return pc;
}

RESULT DeleteConsole(Console *pc) {
    // empty
}

RESULT Path(Console *pc, char** ppn_pszPath) {
    RESULT r = R_OK;
    
    int path_n = strlen(BASE_CONSOLE_PATH_STRING) + strlen("\r>");

    *ppn_pszPath = (char*)malloc(sizeof(char) * path_n);
    CNRM_NA(*ppn_pszPath, "Could not allocate the path string");
    sprintf(*ppn_pszPath, "\r%s>", BASE_CONSOLE_PATH_STRING);

Error:
    return r;
}

RESULT PrintPath(Console *pc, char *pszOptString) {
	RESULT r = R_OK;

	char *tempPath = NULL;

	CRM_NA(Path(pc, &tempPath), "Cant make path!");
    PrintToOutputPsz(pc, tempPath);

    if(pszOptString != NULL)
    	PrintToOutputPsz(pc, pszOptString);

Error:
	return r;
}

RESULT ClearLine(Console *pc) {
	RESULT r = R_OK;
	int i = 0;

	for(i = 0; i < pc->m_InputBuffer_n + strlen(BASE_CONSOLE_PATH_STRING) + strlen("\r>"); i++)
		CRM_NA(PrintToOutputChar(pc, (char)(127)), "Failed to print delete char to terminal");

Error:
	return r;
}

RESULT ClearCommandLine(Console *pc) {
	RESULT r = R_OK;
	int i = 0;

	for(i = 0; i < pc->m_InputBuffer_n + strlen(BASE_CONSOLE_PATH_STRING) + strlen("\r>"); i++)
		CRM_NA(PrintToOutputChar(pc, (char)(127)), "Failed to print delete char to terminal");

	memset(pc->m_InputBuffer, 0, sizeof(pc->m_InputBuffer));
	    pc->m_InputBuffer_n = 0;

Error:
	return r;
}

RESULT RefreshCommandLine(Console *pc, char *pszOptString) {
	RESULT r = R_OK;

	if(pszOptString == NULL) {
		CRM_NA(ClearLine(pc), "Failed to clear line");
		CRM_NA(PrintPath(pc, pc->m_InputBuffer), "Failed to print current path");
	}
	else {
		CRM_NA(ResetCommandBuffer(pc, pszOptString), "Failed to reset command buffer");
	}

Error:
	return r;
}

RESULT ResetCommandBuffer(Console *pc, char *pszOptString) {
    RESULT r = R_OK;
    char *tempPath = NULL;

    CRM_NA(ClearLine(pc), "Failed to clear line");

    memset(pc->m_InputBuffer, 0, sizeof(pc->m_InputBuffer));
    pc->m_InputBuffer_n = 0;

    CRM_NA(PrintPath(pc, pszOptString), "Failed to print current path");
    if(pszOptString != NULL) {
    	pc->m_InputBuffer_n = strlen(pszOptString);
    	memcpy(pc->m_InputBuffer, pszOptString, strlen(pszOptString));
    }

Error:
    if(tempPath != NULL)  {
        free(tempPath);
        tempPath = NULL;
    }
    return r;
}

RESULT PrintToOutputChar(Console *pc, char c) {
    return SBAppendByte(pc->m_pOutputBuffer, c);
}

RESULT PrintNewline(Console *pc) {
	RESULT r = R_OK;
	CR(SBAppendByte(pc->m_pOutputBuffer, '\n'));
	CR(SBAppendByte(pc->m_pOutputBuffer, '\r'));

Error:
	return r;
}

RESULT PrintToOutputBinaryBuffer(Console *pc, unsigned char *Buffer, int Buffer_n, int itemsPerRow) {
	RESULT r = R_OK;
	int i = 0;

	CRM_NA(ClearLine(pc), "Failed to clear line");
	CRM_NA(PrintToOutputChar(pc, '\r'), "Failed to print carriage return");

	for(i = 1; i <= Buffer_n; i++) {
		char HexBuffer[MAX_COMMAND];
		sprintf(HexBuffer, "%02x ", (unsigned char)(Buffer[i - 1]));
		CRM_NA(PrintToOutputPsz(pc, HexBuffer), "Could not print hex buffer!");

		if(i % itemsPerRow == 0 && i != 0)
			PrintNewline(pc);
	}

	PrintNewline(pc);
	RefreshCommandLine(pc, NULL);

Error:
	return R_OK;
}

RESULT PrintToOutputPsz(Console *pc, char* pszOutput) {
    RESULT r = R_OK;
    int i = 0;

    for(i = 0; i < strlen(pszOutput); i++)
        CRM_NA(PrintToOutputChar(pc, pszOutput[i]), "Could not print char to output");

Error:
    return r;
}

RESULT PrintToOutput(Console *pc, const char* output_format, ...) {
    RESULT r = R_OK;
    int i = 0;

    CRM_NA(ClearLine(pc), "Failed to clear line");
    CRM_NA(PrintToOutputChar(pc, '\r'), "Failed to print carriage return");

    va_list ap;
    va_start(ap, output_format);

    for(i = 0; i < strlen(output_format); i++) {
        if(output_format[i] == '%') {
            // first increment i to skip this letter
            i++;
            switch(output_format[i]) {
                case 'd': {
                    int InputInt = va_arg(ap, int);
                    char IntBuffer[MAX_COMMAND];
                    sprintf(IntBuffer, "%d", InputInt);

                    CRM_NA(PrintToOutputPsz(pc, IntBuffer), "Could not print int buffer!");
                } break;   

                case 'c': {
                    char InputChar = va_arg(ap, char);
                    CRM_NA(PrintToOutputChar(pc, InputChar), "Could not print char to buffer!");
                } break;

                case 'x':
                case 'h': {
                    char InputValue = va_arg(ap, int);
                    char HexBuffer[MAX_COMMAND];
                    sprintf(HexBuffer, "%x", InputValue);
                    CRM_NA(PrintToOutputPsz(pc, HexBuffer), "Could not print hex buffer!");
                } break;

                // For wide strings
                case 'S': {
                    wchar_t *wpszInput = va_arg(ap, wchar_t*);
                    char WideBuffer[MAX_COMMAND];
                    int Converted_n = wcstombs(WideBuffer, wpszInput, MAX_COMMAND);
                    CRM_NA(PrintToOutputPsz(pc, WideBuffer), "Could not print wide string buffer!");
                } break;

                case 's': {
                    char* InputBuffer = va_arg(ap, char*);
                    CRM_NA(PrintToOutputPsz(pc, InputBuffer), "Could not print input buffer!");
                } break;

                default: {
                    CBRM_NA(0, "Unknown format specifier");
                } break;
            }
        }
        else if(output_format[i] == '\\') {
            i++;
            switch(output_format[i]) {
                case 'n': {
                    CRM_NA(PrintToOutputChar(pc, '\n'), "Could not print newline to output");
                } break;

                default: {
                    CBRM_NA(0, "Unknown special character");
                } break;
            }
        }
        else {
            CRM_NA(PrintToOutputChar(pc, output_format[i]), "Could not print char to output!");
        }
    }

    PrintNewline(pc);
    RefreshCommandLine(pc, NULL);

Error:
    va_end(ap);
    return r;
}

unsigned char g_fSpecialTermCharCount = 0;

RESULT ReceiveInputChar(Console *pc, char c) {
    RESULT r = R_OK;
    char TempChar[MAX_COMMAND];

    static PSZCircleBufferIterator *it = NULL;// = CreatePSZCircleBufferIterator(pc->m_InputBufferHistory);
	
    switch(g_ConsoleState) {
		case CONSOLE_STATE_NORMAL: {
			switch(c) {
			        case '\n':
			        case '\r': {
			            if(it != NULL) {
			            	free(it);
			            	it = NULL;
			            }

			            PrintNewline(pc);

			            // Execute the command here
						if(pc->m_InputBuffer_n > 0)
							r = ExecuteConsoleFunction(pc, pc->m_InputBuffer, pc->m_InputBuffer_n);

			            // Reset the command buffer
			            ResetCommandBuffer(pc, NULL);
			        } break;

			        case 127:
			        case 8: {
			            if(pc->m_InputBuffer_n != 0) {
			                // back space then erase the character and then backspace again
			                //sprintf(TempChar, "%c %c", 8, 8);
			                //PrintToOutputChar(pc, TempChar);	// don't need for term

			            	PrintToOutputChar(pc, c);	// just print the backspace
			            	DELETE_CHAR_FROM_STATIC_BUFFER(pc->m_InputBuffer);
			            }
			        } break;

			        case 27: {
			        	g_ConsoleState = CONSOLE_STATE_ESC;
			        } break;

			        default: {
			            // Add the char to the command buffer
			            ADD_CHAR_TO_STATIC_BUFFER(pc->m_InputBuffer, c);
			            PrintToOutputChar(pc, c);
			        } break;
			    }
		} break;

		case CONSOLE_STATE_ESC: {
			switch(c) {
				case 91: {
					g_ConsoleState = CONSOLE_STATE_ESC_BRACKET;
				} break;

				default: {
					g_ConsoleState = CONSOLE_STATE_NORMAL;
				} break;
			}
		} break;

		case CONSOLE_STATE_ESC_BRACKET: {
			g_ConsoleState = CONSOLE_STATE_NORMAL;

			switch(c) {
				case 65: {	// Up Arrow
					if(it == NULL)
						it = CreatePSZCircleBufferIterator(pc->m_InputBufferHistory);

					char *historyItem = NextCircBuf(it);
					ResetCommandBuffer(pc, historyItem);
				} break;

				case 66: {	// DOWN
					if(it == NULL)
						it = CreatePSZCircleBufferIterator(pc->m_InputBufferHistory);

					char *historyItem = PreviousCircBuf(it);
					ResetCommandBuffer(pc, historyItem);
				} break;

				case 67: { // RIGHT
					// TODO
				} break;

				case 68: { // LEFT
					// TODO
				} break;
			}
		} break;
    }

Error:
    return r;
}

// This function will receive input as a string of some kind
// and will in turn parse the input and update the command buffer
// Note: this function will destroy the buffer that it is passed
RESULT ReceiveInput(Console *pc, char** ppd_input, int ppd_input_n) {
    RESULT r = R_OK;
    int i = 0;

    for(i = 0; i < ppd_input_n; i++)
        CRM_NA(ReceiveInputChar(pc, *ppd_input[i]), "Since character receive failed");

Error:
	free(*ppd_input);
	*ppd_input = NULL;
    return r;
}

RESULT CheckForOutput(Console *pc) {
    return (GetBufferLength(pc->m_pOutputBuffer) > 0) ? R_YES : R_NO;
}

RESULT DispatchOutput(Console *pc, char **ppn_output, int *pn_output_n) {
    RESULT r = R_OK;

    if(GetBufferLength(pc->m_pOutputBuffer) > 0) {
        *pn_output_n = GetBufferLength(pc->m_pOutputBuffer);
        *ppn_output = CreateBufferCopy(pc->m_pOutputBuffer);
    }
    else {
        *pn_output_n = 0;
        *ppn_output = NULL;
    }

Error:
    // Reset the output buffer
	ResetBuffer(pc->m_pOutputBuffer);
    
    return r;
}

// This will dump the output history to either a file as named in
// pszFilename or to the output buffer if pszFilename is set to NULL
RESULT DumpOutputHistory(Console *pc) {
    RESULT r = R_OK;

	PSZCircleBufferIterator *it = CreatePSZCircleBufferIterator(pc->m_InputBufferHistory);
	unsigned char i = 0;

	for(i = 0; i < pc->m_InputBufferHistory->m_buffer_l; i++) {
		char *tempItem = NextCircBuf(it);
		if(tempItem != NULL) {
			CR(PrintToOutput(pc, tempItem));
		}
	}

Error:
	if(it != NULL) {
		free(it);
		it = NULL;
	}
    return r;
}

RESULT AddConsoleFunction(Console *pc, ConsoleFunction cf) {
    RESULT r = R_OK;
    char TempCharBuffer[MAX_COMMAND];

    ConsoleFunction *ppCmdList = GetCommandList(pc);
	unsigned char ppCmdList_n = pc->m_Commands_n;
	unsigned char i = 0;
	unsigned char fFoundCmd = false;

	if(pc->m_Commands_n >= MAX_CONSOLE_COMMANDS) {
		PrintToOutput(pc, "Max command amount reached %d!", pc->m_Commands_n);
		CBRM_NA(0, "");
	}

    for(i = 0; i < ppCmdList_n; i++) {
        if((strcmp(ppCmdList[i].pszCommand, cf.pszCommand) == 0 ) || (ppCmdList[i].Function == cf.Function)) {
            PrintToOutput(pc, "Console Function %s already present!", cf.pszCommand);
            CBRM_NA(0, "");
        }

        if(ppCmdList[i].ID == cf.ID) {
            PrintToOutput(pc, "Console Function ID %d taken, assigned new id %d", cf.ID, pc->m_nFunctionID);
            cf.ID = pc->m_nFunctionID;
            pc->m_nFunctionID++;
        }
    }

    // Assign an id if one is not given already
    if(cf.ID <= 0){
        cf.ID = pc->m_nFunctionID;
        pc->m_nFunctionID++;
    }

    pc->m_Commands[pc->m_Commands_n] = cf;
    pc->m_Commands_n++;
    PrintToOutput(pc, "Added:%s with %d args", cf.pszCommand, cf.arguments);

    ResetCommandBuffer(pc, NULL);

Error:
    return r;
}

// Args can only be char*
ConsoleFunction CreateConsoleFunction(int ID, void *fun, char *pszCommand, int arguments, int nDefaultArgs, ... ) {
	int i = 0;
    va_list ap;
    va_start(ap, nDefaultArgs);

    ConsoleFunction cf = {ID, fun, pszCommand, NULL, 0, arguments};

    if(nDefaultArgs >= 0 ) {
        char **ppszDefaultArgs = (char*)malloc(sizeof(char*) * nDefaultArgs);

        for(i = 0; i < nDefaultArgs; i++)
            ppszDefaultArgs[i] = va_arg(ap, char*);          

        cf.ppszDefaultArguments = ppszDefaultArgs;
        cf.ppszDefaultArguments_n = nDefaultArgs;
    }

    va_end(ap);

    return cf;
}

RESULT AddConsoleFunctionByArgs(Console *pc, void *fun, char *pszCommand, int arguments, int nDefaultArgs, ... ) {
    RESULT r = R_OK;
    int i = 0;

    va_list ap;
    va_start(ap, nDefaultArgs);

    ConsoleFunction cf = {0, fun, pszCommand, NULL, 0, arguments};

    if(nDefaultArgs > 0 ) {
    	char **ppszDefaultArgs = (char*)malloc(sizeof(char*) * nDefaultArgs);

        for(i = 0; i < nDefaultArgs; i++)
            ppszDefaultArgs[i] = va_arg(ap, char*);          

        cf.ppszDefaultArguments = ppszDefaultArgs;
        cf.ppszDefaultArguments_n = nDefaultArgs;
    }
    else {
        cf.ppszDefaultArguments = NULL;
        cf.ppszDefaultArguments_n = 0;
    }

    CRM(AddConsoleFunction(pc, cf), "Failed to add console function %s", *pszCommand);

Error:
    va_end(ap);
    return r;
}


/*
RESULT TestConsole() {
    RESULT r = R_OK;
    unsigned char fdone = false;

    Console *pConsole = new Console();

    while(!fdone) {
        if(kbhit()) {
            char *c = new char;
            c[0] = getch();
            r = pConsole->ReceiveInput(c, 1);
            if(r == R_EXIT)
            {
                done = true;
            }
        }

        // If output buffer is not empty then spit it out!
        if(pConsole->CheckForOutput() == R_YES)
        {
            char *TempOutputBuffer;
            int TempOutputBuffer_n;
            pConsole->DispatchOutput(TempOutputBuffer, TempOutputBuffer_n);
            if(TempOutputBuffer_n > 0)
            {
                printf("%s", TempOutputBuffer);
            }

        }
    }

Error:
    return r;
}
*/
