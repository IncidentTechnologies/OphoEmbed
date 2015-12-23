#ifndef MIDI_QUEUE_H_
#define MIDI_QUEUE_H_

#include "../Common/EHM.h"
#include "../Device/Device.h"

#include "USB/usbmidi.h"		// TODO: move the defines?

#define MAX_MIDI_EVENT_PARAMS 4

#define MAX_PENDING_EVENTS 20

// MIDI message queue
// This is used for messages inside of the interrupt handler
// this is later picked up by the SysTick
typedef enum gTarMidiEventType {
	GTAR_SEND_MIDI_NOTE,
	GTAR_SEND_MIDI_FRET,
	GTAR_SEND_FW_VERSION,
	GTAR_SEND_FW_ACK,
	GTAR_SEND_PIEZO_FW_ACK,
	GTAR_SEND_BATTERY_STATUS,
	GTAR_SEND_BATTERY_CHARGE,
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
	GTAR_SEND_INVALID
} GTAR_MIDI_EVENT_TYPE;

typedef struct gTarMidiEvent {
	GTAR_MIDI_EVENT_TYPE m_gmet;
	uint8_t m_params_n;
	uint8_t m_params[MAX_MIDI_EVENT_PARAMS];
} GTAR_MIDI_EVENT;

//extern GTAR_MIDI_EVENT g_gTarPendingMidiEvents[];

RESULT InitializeMIDIQueue();

// Manage the pending queue
RESULT QueueNewMidiEvent(GTAR_MIDI_EVENT event);
uint8_t IsMidiEventPending();
RESULT ExecuteQueuedMidiEvent();

#endif // ! MIDI_QUEUE_H_
