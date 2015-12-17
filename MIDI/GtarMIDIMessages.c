#include "GtarMIDIMessages.h"

RGBM UintToRGBM(uint8_t uin) {
	RGBM retVal = {
		.r = (uin & 0xC0) >> 6,
		.g = (uin & 0x30) >> 4,
		.b = (uin & 0xC0) >> 2,
		.m = (uin & 0x3)
	};

	return retVal;
}

uint8_t RGBMToUint(RGBM rgbm) {
	uint8_t retVal = (rgbm.r << 6) + (rgbm.g << 4) + (rgbm.b << 2) + rgbm.m;
	return retVal;
}

MRGB UintToMRGB(uint8_t uin) {
	MRGB retVal = {
		.m = (uin & 0xC0) >> 6,
		.r = (uin & 0x30) >> 4,
		.g = (uin & 0xC0) >> 2,
		.b = (uin & 0x3)
	};

	return retVal;
}

uint8_t MRGBToUint(MRGB mrgb)  {
	uint8_t retVal = (mrgb.m << 6) + (mrgb.r << 4) + (mrgb.g << 2) + mrgb.b;
	return retVal;
}

MIDI_MSG MakeMIDIGtarFretMessage(uint8_t fFretDown, uint8_t channel, uint8_t fret) {
	uint8_t data1 = (fFretDown) ? GTAR_SEND_MSG_FRET_DOWN : GTAR_SEND_MSG_FRET_UP;
	return MakeMIDICCMessage(channel, data1, fret);
}
