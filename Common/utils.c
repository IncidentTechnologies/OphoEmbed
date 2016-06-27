#include "utils.h"

RESULT UARTprintfBinaryData(uint8_t *Buffer, uint32_t Buffer_n, uint32_t itemsPerRow) {
 	uint32_t i = 0;

 	for(i = 1; i <= Buffer_n; i++) {
		DEBUG_MSG("%02x ", Buffer[i - 1]);
		if(i % itemsPerRow == 0 && i != 0)
			DEBUG_MSG_NA("\r\n");
	}

	DEBUG_MSG_NA("\r\n");

	return R_OK;
}
