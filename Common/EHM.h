#ifndef EHM_H_
#define EHM_H_

// ****************************************************************************
// *                              * EHM for Embedded *                        *
// ****************************************************************************
/*  EHM for Embedded - Error handling macros for Embedded.  Required to include
 *  the RESULT.h for embedded as well.  These are striped down versions of the
 *  platform side EHM library to aid with a cohesive codebase between firmware
 *  and platform
// ****************************************************************************/

#include "RESULT.h"

#define DEBUG_FILE_LINE
#ifdef DEBUG_FILE_LINE
	#define CurrentFileLine "%s line#%d: ", __FILE__, __LINE__
#else
	#define CurrentFileLine ""
#endif

#ifndef IN
	#define IN
#endif

#ifndef OUT 
	#define OUT
#endif

#define CONSOLE

// Debug Console
#ifdef DEBUG
	#define DEBUG_MSG(x, ...) UART0printf(x, __VA_ARGS__);
	#define DEBUG_MSG_NA(x) UART0printf(x);
	#define DEBUG_CURRENT_LINE() UART0printf(CurrentFileLine)
	#define DEBUG_LINEOUT(x, ...) UART0printf(x, __VA_ARGS__); UART0printf("\r\n");
	#define DEBUG_LINEOUT_NA(x) UART0printf(x); UART0printf("\r\n");
#elif defined(CONSOLE)
	#define DEBUG_MSG(x, ...) PrintToOutput(g_pConsole, x, __VA_ARGS__);
	#define DEBUG_MSG_NA(x) PrintToOutput(g_pConsole, x);
	#define DEBUG_CURRENT_LINE() PrintToOutput(g_pConsole,CurrentFileLine)
	#define DEBUG_LINEOUT(x, ...) PrintToOutput(g_pConsole, x, __VA_ARGS__);
	#define DEBUG_LINEOUT_NA(x) PrintToOutput(g_pConsole, x);
#else
	#define DEBUG_MSG(x, ...)
	#define DEBUG_MSG_NA(x)
	#define DEBUG_CURRENT_LINE()
	#define DEBUG_LINEOUT(x, ...) 
	#define DEBUG_LINEOUT_NA(x)
#endif

// Check RESULT value
// Ensures that RESULT is successful
#define CR(res) r = r; if(res < 0){ DEBUG_CURRENT_LINE(); DEBUG_MSG("Error: 0x%x\r\n", r); goto Error;}

// Check RESULT value
// Ensures that RESULT is successful
#define CRM(res, msg, ...) r = res; if(CHECK_ERR(r)){ DEBUG_CURRENT_LINE(); DEBUG_MSG(msg, __VA_ARGS__); DEBUG_MSG("Error: 0x%x\r\n", r); goto Error;}
#define CRM_NA(res, msg) r = res; if(CHECK_ERR(r)){ DEBUG_CURRENT_LINE(); DEBUG_MSG_NA(msg); DEBUG_LINEOUT("Error: 0x%x", r); goto Error;}

// Check Boolean Result
// Ensures that condition evaluates to true
#define CBR(condition) if(!condition) { DEBUG_CURRENT_LINE(); DEBUG_LINEOUT_NA("Failed condition!\r\n"); r = R_FAIL; goto Error; }

// Check Boolean Result Message
// Ensures that condition evaluates to true
#define CBRM(condition, msg, ...) if(!condition) { DEBUG_CURRENT_LINE(); DEBUG_LINEOUT(msg, __VA_ARGS__); r = R_FAIL; goto Error; }
#define CBRM_WARN(condition, msg, ...) if(!condition) { DEBUG_CURRENT_LINE(); DEBUG_MSG_NA("warn:"); DEBUG_LINEOUT(msg, __VA_ARGS__); }
#define CBRM_NA(condition, msg) if(!condition) { DEBUG_CURRENT_LINE(); DEBUG_LINEOUT_NA(msg); r = R_FAIL; goto Error; }
#define CBRM_NA_WARN(condition, msg) if(!condition) { DEBUG_CURRENT_LINE(); DEBUG_MSG_NA("warn:"); DEBUG_LINEOUT_NA(msg); }

// Check Boolean Result Message Error
// Ensures that the condition evaluates to true
// Will set the RESULT to a user specified error
#define CBRME(condition, msg, e) if(!condition) { DEBUG_CURRENT_LINE(); DEBUG_MSG(msg); r = e; goto Error; }

// Check Pointer Result 
// Ensures that the pointer is not a NULL
#define CPR(pointer) if(pointer == NULL) { DEBUG_CURRENT_LINE(); DEBUG_MSG_NA("Null pointer error!\r\n"); r = R_ERROR; goto Error; }
#define CPRM(pointer, msg, ...) if(pointer == NULL) { DEBUG_CURRENT_LINE(); DEBUG_LINEOUT(msg, __VA_ARGS__); r = R_ERROR; goto Error; }
#define CPRM_NA(pointer, msg) if(pointer == NULL) { DEBUG_CURRENT_LINE(); DEBUG_LINEOUT_NA(msg); r = R_ERROR; goto Error; }

// Check NULL Result
// Ensures that the pointer is not a NULL
#define CNR(pointer) if(pointer == NULL) { DEBUG_CURRENT_LINE(); DEBUG_MSG_NA("Null error!\r\n"); r = R_ERROR; goto Error; }
#define CNRM(pointer, msg, ...) if(pointer == NULL) { DEBUG_CURRENT_LINE(); DEBUG_LINEOUT(msg, __VA_ARGS__); r = R_ERROR; goto Error; }
#define CNRM_NA(pointer, msg) if(pointer == NULL) { DEBUG_CURRENT_LINE(); DEBUG_LINEOUT_NA(msg); r = R_ERROR; goto Error; }

// Check NULL Result Message
// Ensures that the pointer is not a NULL
//#define CNRM(pointer, msg, ...) if(pointer == NULL) { DEBUG_CURRENT_LINE(); DEBUG_MSG(msg, __VA_ARGS__); DEBUG_MSG("\r\n"); r = R_ERROR; goto Error; }

// Calculate the memory offset of a field in a struct
#define STRUCT_FIELD_OFFSET(struct_type, field_name)    ((long)(long*)&(((struct_type *)0)->field_name))


#endif /*EHM_H_*/
