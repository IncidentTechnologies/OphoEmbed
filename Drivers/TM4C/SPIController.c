#include "SPIController.h"

#include "../../Common/EHM.h"
//#include "FretLed.h"

// The active SSI configurations, more are possible - there are 4 SSI peripherals on
// the TM4C

SSI_PERIPHERAL_INFO *m_pSSIPeripherals = NULL;
int m_SSIPeripherals_n = 0;

int GetSSIConfigCount() { return m_SSIPeripherals_n; }

SSI_PERIPHERAL_INFO *GetSSIConfig(uint8_t configNum) {
	RESULT r = R_OK;

	CBRM((configNum < m_SSIPeripherals_n), "SSIInit: SPI configuration %d not available", configNum);

	return &(m_pSSIPeripherals[configNum]);

Error:
	return NULL;
}

// TODO:
RESULT InitializeSSIConfiguration(SSI_PERIPHERAL_INFO *pSSIPeripherals, int SSIPeripherals_n) {
	m_pSSIPeripherals = pSSIPeripherals;
	m_SSIPeripherals_n = SSIPeripherals_n;

	return R_OK;
}

RESULT SSIInit(uint8_t spiNum) {
	RESULT r = R_OK;

	CNRM((m_pSSIPeripherals), "SSIInit: SPI Configuration not initialized");
	CBRM((spiNum < m_SSIPeripherals_n), "SSIInit: SPI configuration %d not available", spiNum);
	//SSI_PERIPHERAL_INFO *pSSIInfo = &(m_pSSIPeripherals[spiNum]);
	SSI_PERIPHERAL_INFO *pSSIInfo = GetSSIConfig(spiNum);

	// TODO: Horrible design, but for now
	switch(spiNum) {
		case 0: {
			if(!g_device.m_fSPI0) {
				g_device.m_fSPI0 = true;
			}
			else {
				DEBUG_LINEOUT("SSIInit: SPI0 Already initialized");
				return R_NO_EFFECT;
			}
		} break;

		case 1: {
			if(!g_device.m_fSPI1) {
				g_device.m_fSPI1 = true;
			}
			else {
				DEBUG_LINEOUT("SSIInit: SPI1 Already initialized");
				return R_NO_EFFECT;
			}
		} break;

		default: {
			DEBUG_LINEOUT("Initializing SSI %d");

		} break;
	}

	// Enable and configure the SPI SS
	if(pSSIInfo->select.fEnabled) {
		ROM_SysCtlPeripheralEnable(pSSIInfo->select.GPIOPeripheral);
		//ROM_GPIOPadConfigSet(pSSIInfo->select.GPIOPort, pSSIInfo->select.pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
		ROM_GPIOPadConfigSet(pSSIInfo->select.GPIOPort, pSSIInfo->select.pin, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
		GPIOPinTypeGPIOOutput(pSSIInfo->select.GPIOPort, pSSIInfo->select.pin);
		GPIOPinWrite(pSSIInfo->select.GPIOPort, pSSIInfo->select.pin, 0xFF);		// active low
	}

	// If there's an interrupt line set that up
	if(pSSIInfo->intpin.fEnabled) {
		ROM_SysCtlPeripheralEnable(pSSIInfo->intpin.GPIOPeripheral);
		//ROM_GPIOPadConfigSet(pSSIInfo->intpin.GPIOPort, pSSIInfo->intpin.pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
		GPIOPinTypeGPIOInput(pSSIInfo->intpin.GPIOPort, pSSIInfo->intpin.pin);

		CRM(RegisterInterrupt(pSSIInfo->intpin.GPIOPort, GPIOPinToInt(pSSIInfo->intpin.pin), pSSIInfo->intpin.type, pSSIInfo->intpin.cbInt, NULL),
					"Failed to set interrupt cb for port %x on pin %d", pSSIInfo->intpin.GPIOPort, GPIOPinToInt(pSSIInfo->intpin.pin));
	}

	// Enable and configure SPI0
	ROM_SysCtlPeripheralEnable(pSSIInfo->gpio.GPIOPeripheral);
	ROM_SysCtlPeripheralEnable(pSSIInfo->gpio.SSIPeripheral);

	// If we have a separate SS signal, then don't turn it on here
	if(pSSIInfo->select.fEnabled)
		GPIOPinTypeSSI(pSSIInfo->gpio.GPIOPort, pSSIInfo->gpio.CLKPin | pSSIInfo->gpio.RXPin | pSSIInfo->gpio.TXPin);
	else
		GPIOPinTypeSSI(pSSIInfo->gpio.GPIOPort, pSSIInfo->gpio.CLKPin | pSSIInfo->gpio.FSSPin | pSSIInfo->gpio.RXPin | pSSIInfo->gpio.TXPin);

	// Configure the pin periphs
	GPIOPinConfigure(pSSIInfo->gpio.CLKGPIOConfig);

	if(pSSIInfo->select.fEnabled == 0)
		GPIOPinConfigure(pSSIInfo->gpio.FSSGPIOConfig);

	GPIOPinConfigure(pSSIInfo->gpio.RXGPIOConfig);
	GPIOPinConfigure(pSSIInfo->gpio.TXGPIOConfig);

	SysCtlPeripheralReset(pSSIInfo->gpio.SSIBase);
	SSIDisable(pSSIInfo->gpio.SSIBase);

	// Set up the SSI as SPI with 200 KHz Clock Operation
	SSIConfigSetExpClk(pSSIInfo->gpio.SSIBase, SysCtlClockGet(), pSSIInfo->settings.protocol, pSSIInfo->settings.mode, pSSIInfo->settings.bitrate, pSSIInfo->settings.dataWidth);

	// Enable SSI0
	SSIEnable(pSSIInfo->gpio.SSIBase);
	pSSIInfo->fInitialized = 1;

	unsigned char temp;
	while(SSIDataGetNonBlocking(pSSIInfo->gpio.SSIBase, &temp)){/* empty the FIFO */ };

	DEBUG_LINEOUT("SSI %d configuration initialized as SPI", spiNum);

Error:
	return r;
}

// This is specialized since the Neck is using it differently than standard SPI

bool SSICheckInterruptLine(uint8_t spiNum) {
	if(spiNum < m_SSIPeripherals_n) {
			//SSI_PERIPHERAL_INFO *pSSIInfo = &(m_pSSIPeripherals[spiNum]);
			SSI_PERIPHERAL_INFO *pSSIInfo = GetSSIConfig(spiNum);

			if(pSSIInfo->intpin.fEnabled == true)
				if(GPIOPinRead(pSSIInfo->intpin.GPIOPort, pSSIInfo->intpin.pin) != 0)
					return true;
	}

	return false;
}

// Slightly overweight when we want to move data fast
RESULT SSISendChar(uint8_t spiNum, uint8_t cTX, uint8_t *pcRX) {
	RESULT r = R_OK;

	CBRM((spiNum < m_SSIPeripherals_n), "SSISendChar: SPI configuration %d not available", spiNum);

	if((m_pSSIPeripherals)[spiNum].select.fEnabled)
		GPIOPinWrite((m_pSSIPeripherals)[spiNum].select.GPIOPort, (m_pSSIPeripherals)[spiNum].select.pin, 0x00);		// active low

	SSIDataPut((m_pSSIPeripherals)[spiNum].gpio.SSIBase, cTX);
	while(SSIBusy((m_pSSIPeripherals)[spiNum].gpio.SSIBase)){ /* wait while busy */ };

	if((m_pSSIPeripherals)[spiNum].select.fEnabled)
		GPIOPinWrite((m_pSSIPeripherals)[spiNum].select.GPIOPort, (m_pSSIPeripherals)[spiNum].select.pin, 0xFF);		// active low

	if(pcRX != NULL)
		SSIDataGetNonBlocking((m_pSSIPeripherals)[spiNum].gpio.SSIBase, pcRX);

Error:
	return R_OK;
}

// Flipping around the short
RESULT SSISendShort(uint8_t spiNum, uint16_t sTX, uint16_t *psRX) {
	RESULT r = R_OK;
	uint16_t tempShort;

	CBRM((spiNum < m_SSIPeripherals_n), "SSISendShort: SPI configuration %d not available", spiNum);

	if((m_pSSIPeripherals)[spiNum].select.fEnabled)
		GPIOPinWrite((m_pSSIPeripherals)[spiNum].select.GPIOPort, (m_pSSIPeripherals)[spiNum].select.pin, 0x00);		// active low

	SSIDataPut((m_pSSIPeripherals)[spiNum].gpio.SSIBase, sTX);
	while(SSIBusy((m_pSSIPeripherals)[spiNum].gpio.SSIBase)){ /* wait while busy */ };

	if((m_pSSIPeripherals)[spiNum].select.fEnabled)
		GPIOPinWrite((m_pSSIPeripherals)[spiNum].select.GPIOPort, (m_pSSIPeripherals)[spiNum].select.pin, 0xFF);		// active low

	// Copy over the received short
	if(psRX != NULL) {
		SSIDataGetNonBlocking((m_pSSIPeripherals)[spiNum].gpio.SSIBase, &tempShort);
		*psRX = SPI_SHORT(tempShort);
	}

Error:
	return r;
}

// While sending a buffer, the incoming data is thrown away
// TODO: Bi-directional read/write, temp is currently thrown away
// in both this call as well as the protocol
RESULT SSISendBuffer(uint8_t spiNum, uint16_t *pBuffer, int pBuffer_n) {
	RESULT r = R_OK;
	int i = 0;
	uint8_t temp = 0x00;

	CBRM((spiNum < m_SSIPeripherals_n), "SSIInit: SPI configuration %d not available", spiNum);

	if((m_pSSIPeripherals)[spiNum].select.fEnabled)
			GPIOPinWrite((m_pSSIPeripherals)[spiNum].select.GPIOPort, (m_pSSIPeripherals)[spiNum].select.pin, 0x00);		// active low

	for(i = 0; i < pBuffer_n; i++) {
		unsigned short s = pBuffer[i];
		SSIDataPut((m_pSSIPeripherals)[spiNum].gpio.SSIBase, SPI_SHORT(s));
		SSIDataGet((m_pSSIPeripherals)[spiNum].gpio.SSIBase, &temp);
	}

	while(SSIBusy((m_pSSIPeripherals)[spiNum].gpio.SSIBase)){ /* wait while busy */ };

	if((m_pSSIPeripherals)[spiNum].select.fEnabled)
			GPIOPinWrite((m_pSSIPeripherals)[spiNum].select.GPIOPort, (m_pSSIPeripherals)[spiNum].select.pin, 0xFF);		// active low

Error:
	return r;
}

RESULT SPI1Init(uint32_t bitrate) {

	if(!g_device.m_fSPI1) g_device.m_fSPI1 = true;
	else return R_NO_EFFECT;

	return SSIInit(1);
}

RESULT SPI1SendChar(uint8_t cTX, uint8_t *pcRX) {
	return SSISendChar(1, cTX, pcRX);
}

RESULT SSI1SendBuffer(uint16_t *pBuffer, int pBuffer_n) {
	return SSISendBuffer(1, pBuffer, pBuffer_n);
}

RESULT SPI0Init(uint32_t bitrate) {

	if(!g_device.m_fSPI0) g_device.m_fSPI0 = true;
	else return R_NO_EFFECT;

	return SSIInit(0);
}

RESULT SPI0SendChar(uint8_t cTX, uint8_t *pcRX) {
	return SSISendChar(0, cTX, pcRX);
}

RESULT SSI0SendBuffer(uint16_t *pBuffer, int pBuffer_n) {
	return SSISendBuffer(0, pBuffer, pBuffer_n);
}


