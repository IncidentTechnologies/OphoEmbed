#ifndef MIDI_H_
#define MIDI_H_

#include "stdint.h"

typedef enum {
	MIDI_NOTE_OFF 			= 0x80,
	MIDI_NOTE_ON 			= 0x90,
	MIDI_POLY_AFTER_TOUCH 	= 0xA0,
	MIDI_CONTROL_CHANGE		= 0xB0,
	MIDI_PROGRAM_CHANGE		= 0xC0,
	MIDI_AFTER_TOUCH		= 0xD0,
	MIDI_PITCH				= 0xE0,
	MIDI_SYS_EX				= 0xF0,
	// undefined 			= 0xF1,
	MIDI_SYS_COM_SONG_POS	= 0xF2,
	MIDI_SYS_COM_SONG_SEL	= 0xF3,
	// undefined			= 0xF4,
	// undefined			= 0xF5,
	MIDI_SYS_COM_TUNE_REQ	= 0xF6,
	MIDI_SYS_EX_END			= 0xF7,
	MIDI_SYS_REALTIME_CLK	= 0xF8,
	// undefined			= 0xF9,
	MIDI_SYS_REALTIME_START	= 0xFA,
	MIDI_SYS_REALTIME_CONT	= 0xFB,
	MIDI_SYS_REALTIME_STOP	= 0xFC,
	// undefined			= 0xFD,
	MIDI_SYS_REALTIME_ACTIVE_SENSE	= 0xFE,
	MIDI_SYS_REALTIME_RESET	= 0xFF,
} MIDI_MSG_TYPE;

// This is used for the MIDI_TYPE union
typedef enum {
	MIDI_NIBBLE_NOTE_OFF 			= 0x8,
	MIDI_NIBBLE_NOTE_ON 			= 0x9,
	MIDI_NIBBLE_POLY_AFTER_TOUCH 	= 0xA,
	MIDI_NIBBLE_CONTROL_CHANGE		= 0xB,
	MIDI_NIBBLE_PROGRAM_CHANGE		= 0xC,
	MIDI_NIBBLE_AFTER_TOUCH			= 0xD,
	MIDI_NIBBLE_PITCH				= 0xE,
	MIDI_NIBBLE_SYS_EX				= 0xF
} MIDI_NIBBLE_TYPE;

#pragma pack(push, 1) // exact fit - no padding
typedef struct midiType {
	unsigned channel: 4;
	unsigned type: 4;
} MIDI_TYPE;
#pragma pack(pop)

#pragma pack(push, 1) // exact fit - no padding
typedef struct {
	MIDI_TYPE type;
	uint8_t data1;
	uint8_t data2;
} MIDI_MSG;
#pragma pack(pop)

MIDI_MSG MakeMIDIMessage(MIDI_MSG_TYPE type, uint8_t data1, uint8_t data2);
MIDI_MSG MakeMIDICCMessage(uint8_t channel, uint8_t data1, uint8_t data2);

MIDI_MSG MakeMIDINoteMessage(uint8_t fOnOff, uint8_t channel, uint8_t midiVal, uint8_t vel);

// TODO: Put in the wrapping / unwrapping functions here

#endif // ! MIDI_H_
