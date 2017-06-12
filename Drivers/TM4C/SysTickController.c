#include "SysTickController.h"

volatile uint32_t  m_ulSysTickCount = 0;
volatile uint32_t  m_SysTime = 0;
volatile bool  m_fSysTickEnabled = false;

RESULT InitSysTick() {
    RESULT r = R_OK;

	if(!g_device.m_fSysTick) g_device.m_fSysTick = true;
	else return R_NO_EFFECT;

	m_ulSysTickCount = 0;
	m_SysTime = 0;


	SysTickPeriodSet(ROM_SysCtlClockGet() / SYSTICKS_PER_SECOND);
    SysTickIntEnable();

    CR(EnableSysTick());

    DEBUG_LINEOUT("Initialized SysTick with %d frequency systick:%d", SYSTICKS_PER_SECOND, m_ulSysTickCount);

Error:
	return r;
}

inline uint64_t SystemTickCount() {
	return m_ulSysTickCount;
}

inline uint32_t SystemTime() {
	return m_SysTime;
}

inline uint64_t IncrementSysTick() {
	return ++m_ulSysTickCount;
}

inline uint32_t IncrementSysTime(uint32_t val) {
	return (m_SysTime += val);
}

RESULT EnableSysTick() {
    if(m_fSysTickEnabled == false) {
        SysTickEnable();
    }

    m_fSysTickEnabled = true;

    return R_PASS;
}

RESULT DisableSysTick() {
    if(m_fSysTickEnabled == true) {
        SysTickDisable();
    }

    m_fSysTickEnabled = false;

    return R_PASS;
}
