#ifndef SYS_TICK_CONTROLLER_H_
#define SYS_TICK_CONTROLLER_H_

#include "../../Common/EHM.h"
#include "../../Device/Device.h"

// Sets the frequency of the SysTick Interrupt
#define SYSTICKS_PER_SECOND 100
#define SYS_TIME_INTERVAL 10		// stay alive

// System Time
extern volatile uint32_t  g_ulSysTickCount;
extern volatile uint32_t  g_SysTime;
//void SysTickHandler(void);		// TODO: Move this here, generalize the task manager

RESULT InitSysTick();

#endif // !SYS_TICK_CONTROLLER_H_
