#include "midi.h"

// TOOD: Pull the gTar defines out of usbmidi
//#include "usbmidi.h"

MIDI_MSG MakeMIDIMessage(MIDI_MSG_TYPE type, uint8_t data1, uint8_t data2) {
	MIDI_MSG retVal;

	memcpy(&(retVal.type), &type, sizeof(MIDI_TYPE));
	retVal.data1 = data1;
	retVal.data2 = data2;

	return retVal;
}

MIDI_MSG MakeMIDINoteMessage(uint8_t fOnOff, uint8_t channel, uint8_t midiVal, uint8_t vel) {
	uint8_t type = (fOnOff) ? MIDI_NOTE_ON : MIDI_NOTE_OFF;
	type += channel;
	return MakeMIDIMessage(type, midiVal, vel);
}

MIDI_MSG MakeMIDICCMessage(uint8_t channel, uint8_t data1, uint8_t data2) {
	uint8_t type = MIDI_CONTROL_CHANGE + channel;
	return MakeMIDIMessage(type, data1, data2);
}
