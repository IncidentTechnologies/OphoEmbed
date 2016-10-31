#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include "../Common/EHM.h"
#include "../Device/Device.h"

// Bootloader
// opho/bootloader/bootloader.h
// This wraps the functionality of the bootloader so that it's possible to
// jump into the bootloader code and take an update via the USB DFU mode

RESULT InitializeBootloader();
RESULT JumpToBootLoader();

#endif // ! BOOTLOADER_H_
