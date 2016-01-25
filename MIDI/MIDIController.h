#ifndef MIDI_CONTROLLER_H_
#define MIDI_CONTROLLER_H_

// This is designed to act as a hub to all MIDI messages

#include "../Common/EHM.h"
#include "../Device/Device.h"


#include "USB/usbmidi.h"
#include "MIDIMessages.h"
#include "MIDIQueue.h"

#define MAX_SYS_EX_SIZE 1024*2

RESULT InitializeMIDIController();

typedef RESULT (*cbHandleCustomDeviceSysEx)(DEVICE_MSG*);
extern cbHandleCustomDeviceSysEx g_HandleCustomDeviceSysExCallback;
RESULT RegisterHandleCustomDeviceSysExCallback(cbHandleCustomDeviceSysEx HandleCustomDeviceSysExCB);
RESULT UnregisterHandleCustomDeviceSysExCallback();

typedef RESULT (*cbHandleDebugSysEx)(bool);
extern cbHandleDebugSysEx g_HandleDebugSysExCallback;
RESULT RegisterHandleDebugSysExCallback(cbHandleDebugSysEx HandleDebugSysExCB);
RESULT UnregisterHandleDebugSysExCallback();

// channel, value, RGBM color
typedef RESULT (*cbHandleLEDStateCC)(uint8_t, uint8_t, uint8_t);
extern cbHandleLEDStateCC g_HandleLEDStateCCCallback;
RESULT RegisterHandleLEDStateCCCallback(cbHandleLEDStateCC HandleLEDStateCCCB);
RESULT UnregisterHandleLEDStateCCCallback();

// MIDI functions
RESULT SendMidiNoteMsg(uint8_t midiVal, uint8_t midiVelocity, uint8_t channel, uint8_t fOnOff);
RESULT SendMidiCC(uint8_t index, uint8_t value);

RESULT SendFirmwareVersion();
RESULT SendFirmwareDownloadAck(uint8_t status);
RESULT SendBatteryStatusAck();
RESULT SendBatteryChargePercentageAck();
RESULT SendRequestSerialNumberAck(uint8_t byteNumber);

RESULT SendAck(uint8_t SendBuffer[4]);
RESULT SendCommitUserspaceAck(uint8_t status);
RESULT SendResetUserspaceAck(uint8_t status);

RESULT HandleMIDIPacket(MIDI_MSG midiPacket);
RESULT HandleMIDISysExPacket(midiPacket);

RESULT UnwrapBuffer7F(uint8_t *pBuffer7F, int pBuffer7F_n, uint8_t **n_pBuffer, int *pn_pBuffer_n);
RESULT WrapBuffer7F(uint8_t *pBuffer, int pBuffer_n, uint8_t **n_pBuffer7F, int *pn_pBuffer7F_n);
long int Convert7FToInt(int num, ...);

#endif
