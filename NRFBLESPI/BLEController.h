#ifndef BLE_CONTROLLER_H_
#define BLE_CONTROLLER_H_

#include "../Common/RESULT.h"

// Not much here, just a switch whether or not BLE is connected or not

//#define SPOOF_BLE true

RESULT SetBLEConnect(uint8_t fStatus);
bool IsBLEConnected();

#endif // ! BLE_CONTROLLER_H_
