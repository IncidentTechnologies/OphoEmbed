#include "SysTickController.h"

volatile uint32_t  g_ulSysTickCount = 0;
volatile uint32_t  g_SysTime = 0;

RESULT InitSysTick() {

	if(!g_device.m_fSysTick) g_device.m_fSysTick = true;
	else return R_NO_EFFECT;

	g_ulSysTickCount = 0;
	SysTickPeriodSet(ROM_SysCtlClockGet() / SYSTICKS_PER_SECOND);
    SysTickIntEnable();
    SysTickEnable();

    DEBUG_LINEOUT("Initialized SysTick with %d frequency systick:%d", SYSTICKS_PER_SECOND, g_ulSysTickCount);

	return R_OK;
}
