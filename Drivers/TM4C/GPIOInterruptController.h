#ifndef GPIO_INTERRUPT_CONTROLLER_H_
#define GPIO_INTERRUPT_CONTROLLER_H_

#include "../../Common/EHM.h"
#include "../../Device/Device.h"

// TODO: Should probably figure out what ports are available
// at run/compile time per/target
void GPIOPortAIntHandler(void);
void GPIOPortBIntHandler(void);
void GPIOPortCIntHandler(void);
void GPIOPortDIntHandler(void);
void GPIOPortEIntHandler(void);
void GPIOPortFIntHandler(void);
void GPIOPortGIntHandler(void);
void GPIOPortHIntHandler(void);
void GPIOPortJIntHandler(void);
void GPIOPortKIntHandler(void);

#define GPIO_PIN(pin) (1 << pin)

int GPIOPinToInt(uint8_t pin);

typedef RESULT(*PinInterruptCallback)(void *);
typedef void(*GPIOInterruptCB)(void);

typedef struct {
	unsigned fEnabled: 1;
	uint32_t type;
	PinInterruptCallback PinCallback;
	void *pContext;
} GPIO_PIN_INTERRUPT;

typedef struct {
	uint32_t port;
	unsigned fEnabled: 1;
	GPIOInterruptCB InterruptHandlerCallback;
	GPIO_PIN_INTERRUPT pin[8];
} GPIO_INTERRUPT_LUT;

RESULT InitializeGPIOInterruptController();

#endif // ! GPIO_INTERRUPT_CONTROLLER_H_
