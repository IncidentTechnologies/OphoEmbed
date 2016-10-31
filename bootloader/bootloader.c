#include "bootloader.h"

#include "inc/hw_nvic.h"

RESULT InitializeBootloader() {
	RESULT r = R_PASS;

	DEBUG_LINEOUT("Bootloader Initialized");

Error:
	return r;
}

RESULT JumpToBootLoader() {
    RESULT r = R_PASS;

    DEBUG_LINEOUT("Jumping to bootloader - this will not return");

    // We must make sure we turn off SysTick and its interrupt before entering
    // the boot loader!
    ROM_SysTickIntDisable();
    ROM_SysTickDisable();

    // Disable all processor interrupts.  Instead of disabling them
    // one at a time, a direct write to NVIC is done to disable all
    // peripheral interrupts.
    HWREG(NVIC_DIS0) = 0xffffffff;
    HWREG(NVIC_DIS1) = 0xffffffff;

    // Return control to the boot loader.  This is a call to the SVC
    // handler in the boot loader.
    (*((void (*)(void))(*(uint32_t *)0x2c)))();

Error:
	return r;
}
