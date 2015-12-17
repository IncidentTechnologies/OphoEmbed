#ifndef SPI_CONTROLLER_H_
#define SPI_CONTROLLER_H_

//#include "gtar.h"
#include "../../Common/EHM.h"
#include "../../Device/Device.h"

#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "driverlib/ssi.h"	// SSI include

// TODO: ?
typedef RESULT (*PinInterruptCallback)(void *);

/*
#define NECK_SS_GPIO_PERIPH	SYSCTL_PERIPH_GPIOK
#define NECK_SS_GPIO_BASE	GPIO_PORTK_BASE
#define NECK_SS_GPIO_PIN	GPIO_PIN_3

#define FRET_LED_GPIO_PERIPH SYSCTL_PERIPH_GPIOA
#define FRET_LED_GPIO_BASE	 GPIO_PORTA_BASE
#define FRET_LED_SPI_PERIPH	 SYSCTL_PERIPH_SSI0
#define FRET_LED_SPI_BASE	 SSI0_BASE
#define FRET_LED_CLK_PIN 	 GPIO_PIN_2
#define FRET_LED_FSS_PIN 	 GPIO_PIN_3
#define FRET_LED_RX_PIN		 GPIO_PIN_4
#define FRET_LED_TX_PIN		 GPIO_PIN_5
*/

#define DEFAULT_SPI_BITRATE 100000
#define SPI_SHORT(s) ((s & 0xFF) << 8) + ((s & 0xFF00) >> 8)

#define MAX_SPI_MSG_LENGTH 16

// TODO:  Allow addition of custom configurations

typedef struct {
	uint32_t protocol;
	uint32_t mode;
	uint32_t bitrate;
	uint32_t dataWidth;
} SSI_SETTINGS;

typedef struct {
	uint32_t SSIBase;
	uint32_t SSIPeripheral;
	uint32_t GPIOPeripheral;
	uint32_t GPIOPort;

	uint32_t CLKPin;
	uint32_t CLKGPIOConfig;

	uint32_t FSSPin;
	uint32_t FSSGPIOConfig;

	uint32_t RXPin;
	uint32_t RXGPIOConfig;

	uint32_t TXPin;
	uint32_t TXGPIOConfig;
} SSI_GPIO_INFO;

typedef struct {
	unsigned fEnabled: 1;
	uint32_t GPIOPeripheral;
	uint32_t GPIOPort;
	uint32_t pin;
} SSI_CS_GPIO_INFO;

typedef struct {
	unsigned fEnabled: 1;
	uint32_t GPIOPeripheral;
	uint32_t GPIOPort;
	uint32_t pin;
	uint32_t type;
	PinInterruptCallback cbInt;
} SSI_INT_GPIO_INFO;

typedef struct {
	unsigned fInitialized: 1;
	SSI_SETTINGS settings;
	SSI_GPIO_INFO gpio;
	SSI_CS_GPIO_INFO select;
	SSI_INT_GPIO_INFO intpin;	// interrupt pin
} SSI_PERIPHERAL_INFO;

int GetSSIConfigCount();
SSI_PERIPHERAL_INFO *GetSSIConfig(uint8_t configNum);

RESULT SSIInit(uint8_t spiNum);
RESULT SSISendChar(uint8_t spiNum, uint8_t cTX, uint8_t *pcRX);
RESULT SSISendBuffer(uint8_t spiNum, uint16_t *pBuffer, int pBuffer_n);

RESULT SSISendShort(uint8_t spiNum, uint16_t sTX, uint16_t *psRX);

bool SSICheckInterruptLine(uint8_t spiNum);

RESULT SPI0Init(uint32_t bitrate);
RESULT SPI0SendChar(uint8_t cTX, uint8_t *pcRX);
RESULT SSI0SendBuffer(uint16_t *pBuffer, int pBuffer_n);

RESULT SPI1Init();
RESULT SPI1SendChar(uint8_t cTX, uint8_t *pcRX);
RESULT SSI1SendBuffer(uint16_t *pBuffer, int pBuffer_n);

#endif // !SPI_CONTROLLER_H_
