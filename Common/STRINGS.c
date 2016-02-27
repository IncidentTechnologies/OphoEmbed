#include "STRINGS.h"

#ifdef STRING_DEBUG
	const int8_t *g_MIDIputErrorMSG = "Failed to put EP3 data in FIFO";
	const int8_t *g_MIDIsendErrorMSG = "SendUSBMidiNoteMsg: Failed to send EP3 data";
#else
	const int8_t *g_MIDIputErrorMSG = "";
	const int8_t *g_MIDIsendErrorMSG = "";
#endif


