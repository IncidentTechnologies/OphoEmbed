#include "GPIOInterruptController.h"

int GPIOPinToInt(uint8_t pin) {
	switch(pin) {
		case GPIO_PIN_0: return 0;
		case GPIO_PIN_1: return 1;
		case GPIO_PIN_2: return 2;
		case GPIO_PIN_3: return 3;
		case GPIO_PIN_4: return 4;
		case GPIO_PIN_5: return 5;
		case GPIO_PIN_6: return 6;
		case GPIO_PIN_7: return 7;

		default: return 0;
	}

	//return 0;
}

GPIO_INTERRUPT_LUT g_PortInterrupts[] = {
	/*{
		.fEnabled = 0,
		.port = GPIO_PORTF_BASE,
		.pin = 0
	},*/ // TODO: Add piezo to the int controller
	{
		.fEnabled = 0,
		.InterruptHandlerCallback = GPIOPortHIntHandler,
		.port = GPIO_PORTH_BASE,
		.pin = 0
	},
	{
		.fEnabled = 0,
		.InterruptHandlerCallback = GPIOPortKIntHandler,
		.port = GPIO_PORTK_BASE,
		.pin = 0
	}
};
int g_PortInterrupts_n = sizeof(g_PortInterrupts) / sizeof(g_PortInterrupts[0]);

int g_PortHConfig = 0;
int g_PortKConfig = 0;


RESULT InitializeGPIOInterruptController() {
	RESULT r = R_OK;
	int i = 0;

	for(i = 0; i < g_PortInterrupts_n; i++) {
		GPIOIntRegister(g_PortInterrupts[i].port, g_PortInterrupts[i].InterruptHandlerCallback);
		g_PortInterrupts[i].fEnabled = 1;

		switch(g_PortInterrupts[i].port) {
			case GPIO_PORTH_BASE: g_PortHConfig = i; break;
			case GPIO_PORTK_BASE: g_PortKConfig = i; break;
		}
	}

	DEBUG_LINEOUT_NA("GPIO Interrupt Controller Initialized");

Error:
	return r;
}

RESULT RegisterInterrupt(uint32_t port, uint8_t pin, uint32_t type, PinInterruptCallback cbPin, void *pContext) {
	RESULT r = R_OK;
	int i = 0;

	for(i = 0; i < g_PortInterrupts_n; i++)
		if(port == g_PortInterrupts[i].port)
			break;

	CBRM((i < g_PortInterrupts_n), "Port 0x%x is not configured for interrupts", port);
	CBRM((g_PortInterrupts[i].fEnabled), "Port 0x%x interrupts are not enabled", port);
	CBRM((g_PortInterrupts[i].pin[pin].fEnabled == 0), "Port 0x%x pin %d already set up", port, pin);

	g_PortInterrupts[i].pin[pin].type = type;
	g_PortInterrupts[i].pin[pin].PinCallback = cbPin;
	g_PortInterrupts[i].pin[pin].pContext = pContext;

	GPIOIntTypeSet(port, GPIO_PIN(pin), type);

	g_PortInterrupts[i].pin[pin].fEnabled = 1;
	GPIOIntEnable(port, GPIO_PIN(pin));

Error:
	return r;
}

inline void GPIOIntHandler(uint32_t port, int config) {
	uint32_t ulInts = GPIOIntStatus(port, true) ;
	int i = 0;

	for(i = 0; i < 8; i++) {
		if(ulInts & GPIO_PIN(i) && g_PortInterrupts[config].pin[i].fEnabled) {
			g_PortInterrupts[config].pin[i].PinCallback(g_PortInterrupts[config].pin[i].pContext);
		}
	}

	GPIOIntClear(port, ulInts);
}

// Port H Interrupt Handler
void GPIOPortHIntHandler(void) {
	return GPIOIntHandler(GPIO_PORTH_BASE, g_PortHConfig);
}

// Port K Interrupt Handler
void GPIOPortKIntHandler(void) {
	return GPIOIntHandler(GPIO_PORTK_BASE, g_PortKConfig);
}

