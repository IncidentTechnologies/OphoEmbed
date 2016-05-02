#ifndef MIDI_QUEUE_H_
#define MIDI_QUEUE_H_

#include "../Common/EHM.h"
#include "../Device/Device.h"

#include "USB/usbmidi.h"		// TODO: move the defines?

#define MAX_MIDI_EVENT_PARAMS 4
#define MAX_PENDING_EVENTS 40

// MIDI_QUEUE_VERBOSE - Used for debugging the MIDI queue
// Will output debug messages when events are queued or executed
//#define MIDI_QUEUE_VERBOSE

// MIDI message queue
// This is used for messages inside of the interrupt handler
// this is later picked up by the SysTick
// TODO: Get rid of GTAR here
typedef enum DeviceMidiEventType {
	DEVICE_SEND_MIDI_NOTE,
	//DEVICE_SEND_MIDI_FRET,
	DEVICE_SEND_FW_VERSION,
	DEVICE_SEND_FW_ACK,
	DEVICE_SEND_BATTERY_STATUS,
	DEVICE_SEND_BATTERY_CHARGE,
	DEVICE_SEND_COMMIT_USERSPACE,
	DEVICE_SEND_RESET_USERSPACE,
	DEVICE_SEND_SERIAL_NUMBER,
	DEVICE_SEND_MIDI_CONTROL_CHANGE,
	DEVICE_SEND_INVALID

	/*
	// Gtar Events (TODO: move to Gtar)
	GTAR_SEND_PIEZO_FW_ACK,
	GTAR_SEND_PIEZO_CT_MATRIX,
	GTAR_SEND_PIEZO_SENSITIVITY,
	GTAR_SEND_PIEZO_WINDOW,
	GTAR_SEND_PIEZO_TIMEOUT,
	GTAR_SEND_SERIAL_NUMBER,
	GTAR_SEND_CALIBRATE_PIEZO_STRING,
	GTAR_SEND_COMMIT_USERSPACE,
	GTAR_SEND_RESET_USERSPACE,
	GTAR_SEND_PIEZO_CMD_ACK,
	GTAR_SEND_PIEZO_CMD_RESPONSE,
	*/


} DEVICE_MIDI_EVENT_TYPE;

typedef struct DeviceMidiEvent {
	DEVICE_MIDI_EVENT_TYPE m_gmet;
	uint8_t m_params_n;
	uint8_t m_params[MAX_MIDI_EVENT_PARAMS];
} DEVICE_MIDI_EVENT;

//extern DEVICE_MIDI_EVENT g_gTarPendingMidiEvents[];

RESULT InitializeMIDIQueue();

typedef RESULT (*cbHandleMIDIQueueEvent)(DEVICE_MIDI_EVENT);
extern cbHandleMIDIQueueEvent g_HandleMIDIQueueEventCallback;
RESULT RegisterHandleMIDIQueueEventCallback(cbHandleMIDIQueueEvent HandleMIDIQueueEventCB);
RESULT UnregisterHandleMIDIQueueEventCallback();

// Manage the pending queue
RESULT QueueNewMidiEvent(DEVICE_MIDI_EVENT event);
uint8_t IsMidiEventPending();
RESULT ExecuteQueuedMidiEvent();

#endif // ! MIDI_QUEUE_H_
