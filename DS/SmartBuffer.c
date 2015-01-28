#include "SmartBuffer.h"
#include "string.h"
//#include "SIRDebug.h"	// TODO: Better solution needed
#include "..\OS\Console.h"

SmartBuffer* CreateSmartBuffer(const char *pBuffer, int pBuffer_n) {
    SmartBuffer *pNewSB = (SmartBuffer*)malloc(sizeof(SmartBuffer));
    memset(pNewSB, 0, sizeof(SmartBuffer));

    if(pBuffer == NULL)
        ResetBuffer(pNewSB);
    else if(pBuffer_n == 0)
        SetBufferToPsz(pNewSB, pBuffer);
    else
        SetBufferToBuffer(pNewSB, pBuffer, pBuffer_n);

    return pNewSB;
}

SmartBuffer* CreateSmartBufferFromSB(SmartBuffer *psb) {
	SmartBuffer *pNewSB = (SmartBuffer*)malloc(sizeof(SmartBuffer));
	memset(pNewSB, 0, sizeof(SmartBuffer));

	SetBufferToBuffer(psb, GetBuffer(pNewSB), GetBufferLength(pNewSB));

    return pNewSB;
}

RESULT DeleteSmartBuffer(SmartBuffer *psb) {
    if(psb->m_pBuffer != NULL) {
        free(psb->m_pBuffer);
        psb->m_pBuffer = NULL;
    }

    psb->m_pBuffer_n = 0;
    psb->m_pBuffer_bn = 0;

    return R_OK;
}

RESULT IncrementBufferSize(SmartBuffer *psb) {
    RESULT r = R_OK;

    CPRM_NA(psb->m_pBuffer, "SmartBuffer is not initialized!\n");
    CBRM_NA((psb->m_pBuffer_n > 0), "SmartBuffer size is 0!\n");

    char *SaveBuffer = (char*)malloc(sizeof(char) * psb->m_pBuffer_n);
    CNRM_NA(SaveBuffer, "SmartBuffer: Failed to allocate memory for save pBuffer");
    memcpy(SaveBuffer, psb->m_pBuffer, psb->m_pBuffer_n);

    free(psb->m_pBuffer);
    psb->m_pBuffer = NULL;
    psb->m_pBuffer_bn++;
    psb->m_pBuffer = (char*)malloc(sizeof(char) * SMART_BUFFER_BLOCK_SIZE * (psb->m_pBuffer_bn + 1));

    CNRM_NA(psb->m_pBuffer, "SmartBuffer: Failed to allocate memory for new pBuffer");
    memset(psb->m_pBuffer, 0, SMART_BUFFER_BLOCK_SIZE * (psb->m_pBuffer_bn + 1));

    memcpy(psb->m_pBuffer, SaveBuffer, psb->m_pBuffer_n);

    free(SaveBuffer);
    SaveBuffer = NULL;

Error:
    return r;
}

RESULT SetBufferToPsz(SmartBuffer *psb, const char *pszBuffer) {
    return SetBufferToBuffer(psb, pszBuffer, strlen(pszBuffer));
}

RESULT SetBufferToBuffer(SmartBuffer *psb, const char *pBuffer, int pBuffer_n) {
    RESULT r = R_OK;
    int i = 0;

    CRM_NA(ResetBuffer(psb), "ResetBuffer: Failed to reset pBuffer");

    for(i = 0; i < pBuffer_n; i++)
        CRM(SBAppendByte(psb, pBuffer[i]), "ResetBuffer: Failed to append %c", pBuffer[i]);

Error:
    return r;
}

RESULT ResetBuffer(SmartBuffer *psb) {
    RESULT r = R_OK;

    psb->m_pBuffer_n = 0;
    psb->m_pBuffer_bn = 0;

    if(psb->m_pBuffer != NULL) {
        free(psb->m_pBuffer);
        psb->m_pBuffer = NULL;
    }

    psb->m_pBuffer = (char*)malloc(sizeof(char) * (psb->m_pBuffer_bn + 1) * SMART_BUFFER_BLOCK_SIZE);
    memset(psb->m_pBuffer, 0, (psb->m_pBuffer_bn + 1) * SMART_BUFFER_BLOCK_SIZE);

Error:
    return r;
}

char *CreateBufferCopy(SmartBuffer *psb) {
    char *temp = (char*)malloc(sizeof(char) * psb->m_pBuffer_n + 1);
    memset(temp, 0, psb->m_pBuffer_n + 1);
    memcpy(temp, psb->m_pBuffer, psb->m_pBuffer_n);

    return temp;
}

unsigned char NotAllWhitespace(SmartBuffer *psb) {
	int i = 0;
    for(i = 0; i < psb->m_pBuffer_n; i++)
        if(psb->m_pBuffer[i] != ' ' && psb->m_pBuffer[i] != '\t')
            return 1;

    return 0;
}

RESULT SBAppendByte(SmartBuffer *psb, char byte) {
    RESULT r = R_OK;

    if(psb->m_pBuffer == NULL) {
        psb->m_pBuffer_bn = 0;
        psb->m_pBuffer_n = 0;
        psb->m_pBuffer = (char*)malloc((psb->m_pBuffer_bn + 1) * SMART_BUFFER_BLOCK_SIZE);
        memset(psb->m_pBuffer, 0, (psb->m_pBuffer_bn + 1) * SMART_BUFFER_BLOCK_SIZE);
    }

    *(psb->m_pBuffer + psb->m_pBuffer_n) = byte;
    psb->m_pBuffer_n++;

    if(psb->m_pBuffer_n >= (psb->m_pBuffer_bn + 1) * SMART_BUFFER_BLOCK_SIZE)
        CRM_NA(IncrementBufferSize(psb), "SmartBuffer: Failed to increment the pBuffer size!");

Error:
    return r;
}

RESULT SBAppendPsz(SmartBuffer *psb, char *psz, int psz_n) {
	RESULT r = R_OK;
	int i = 0;

	for(i = 0; i < psz_n; i++)
		CRM(SBAppendByte(psb, psz[i]), "SmartBuffer: Failed to append: %c", psz[i]);

Error:
	return r;
}

RESULT SBAppendSB(SmartBuffer *psb, SmartBuffer sb) {
    RESULT r = R_OK;
    int i = 0;

    char *TempBuffer = GetBuffer(&sb);

    for(i = 0; i < GetBufferLength(&sb); i++)
        CRM(SBAppendByte(psb, TempBuffer[i]), "SmartBuffer: Failed to append: %c", TempBuffer[i]);

Error:
    return r;
}

char* GetBuffer(SmartBuffer *psb) {
    return psb->m_pBuffer;
}

int GetBufferLength(SmartBuffer *psb) {
    return psb->m_pBuffer_n;
}

int GetBufferBlockCount(SmartBuffer *psb) {
    return psb->m_pBuffer_bn;
}

RESULT SetBufferEqual(SmartBuffer *psb, const SmartBuffer *rhs) {
    if(psb != &rhs) {
        free(psb->m_pBuffer);
        psb->m_pBuffer_n = rhs->m_pBuffer_n;
        psb->m_pBuffer_bn = rhs->m_pBuffer_bn;
        psb->m_pBuffer = (char*)malloc(sizeof(char) * (psb->m_pBuffer_bn + 1) * SMART_BUFFER_BLOCK_SIZE);

        memset(psb->m_pBuffer, 0, (psb->m_pBuffer_bn + 1) * SMART_BUFFER_BLOCK_SIZE);
        memcpy(psb->m_pBuffer, rhs->m_pBuffer, psb->m_pBuffer_n);
    }

    return R_OK;
}

RESULT RemoveCharacter(SmartBuffer *psb, char *pChar) {
    RESULT r = R_OK;

    if((pChar < psb->m_pBuffer) || (pChar > psb->m_pBuffer + ((psb->m_pBuffer_bn + 1) * SMART_BUFFER_BLOCK_SIZE)))
        CBRM_NA(0, "RemoveCharacter: Char pointer out of the bounds of the SmartBuffer Buffer");

    // Two cases, either char is last character (then it's easy!) or not
    if(pChar == psb->m_pBuffer + psb->m_pBuffer_n) {
        psb->m_pBuffer[psb->m_pBuffer_n] = '\0';
        psb->m_pBuffer_n--;
    }
    else {
        char *TempBuffer = (char*)malloc(sizeof(char) * strlen(pChar + 1));
        strcpy(TempBuffer, pChar + 1);
        memset(pChar, 0, psb->m_pBuffer_n - (pChar - psb->m_pBuffer));
        strcat(psb->m_pBuffer, TempBuffer);
        psb->m_pBuffer_n--;
    }    

Error:
    return r;
}


// Finds the first instance of byte and returns a pointer to it
RESULT Find(SmartBuffer *psb, const char byte, char **ppChar) {
    RESULT r = R_OK;

    *ppChar = psb->m_pBuffer;

    while((**ppChar) != '\0' && (**ppChar) != byte)
        *ppChar++;

    if((**ppChar) == byte)
        r = R_OK;
    else
        r = R_FAIL;

Error:
    return r;
}
