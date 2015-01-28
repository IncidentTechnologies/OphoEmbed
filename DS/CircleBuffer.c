#include "CircleBuffer.h"

// Buffer
PSZCircleBuffer* CreatNewPSZCircleBuffer(int length) {
	PSZCircleBuffer *pPszCircBuf = (PSZCircleBuffer *)malloc(sizeof(PSZCircleBuffer));
	memset(pPszCircBuf, 0, sizeof(PSZCircleBuffer));

	pPszCircBuf->m_buffer = (char**)malloc(sizeof(char*) * length);
	memset(pPszCircBuf->m_buffer, 0, sizeof(char*) * length);

	pPszCircBuf->m_buffer_e = length - 1;
	pPszCircBuf->m_buffer_l = length;
	return pPszCircBuf;
}

char* PSZCircleBufferPush(PSZCircleBuffer *pb, char *pszItem) {
	char *popItem = pb->m_buffer[pb->m_buffer_c];

	pb->m_buffer[pb->m_buffer_c] = pszItem;
	pb->m_buffer_e = pb->m_buffer_c;

	pb->m_buffer_c++;
	if(pb->m_buffer_c >= pb->m_buffer_l)
		pb->m_buffer_c = 0;

	return popItem;
}

RESULT DeletePSZCircleBuffer(PSZCircleBuffer *pb) {
	unsigned char i = 0;
	for(i = 0; i < pb->m_buffer_l; i++) {
		if(pb->m_buffer[i] != NULL) {
			free(pb->m_buffer[i]);
			pb->m_buffer[i] = NULL;
		}
	}

	if(pb->m_buffer != NULL) {
		free(pb->m_buffer);
		pb->m_buffer = NULL;
	}

	return R_OK;
}

// Iterator
PSZCircleBufferIterator *CreatePSZCircleBufferIterator(PSZCircleBuffer* pbuffer) {
	PSZCircleBufferIterator *it = (PSZCircleBufferIterator *)malloc(sizeof(PSZCircleBufferIterator));
	it->m_pbuffer = pbuffer;
	it->m_buffer_i = pbuffer->m_buffer_e;
	it->m_buffer_s = pbuffer->m_buffer_e;

	return it;
}

char* NextCircBuf(PSZCircleBufferIterator *it) {
	char *retItem = it->m_pbuffer->m_buffer[it->m_buffer_i];
	it->m_buffer_i--;
	if(it->m_buffer_i < 0)
		it->m_buffer_i = it->m_pbuffer->m_buffer_l - 1;

	return retItem;
}

char* PreviousCircBuf(PSZCircleBufferIterator *it) {
	char *retItem = it->m_pbuffer->m_buffer[it->m_buffer_i];
	it->m_buffer_i++;
	if(it->m_buffer_i >= it->m_pbuffer->m_buffer_l)
		it->m_buffer_i = 0;

	return retItem;
}
