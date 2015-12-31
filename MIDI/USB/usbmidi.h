#ifndef USBDEFS_H_
#define USBDEFS_H_

#include "../../Common/EHM.h"
#include "../../Device/Device.h"
//#define USB_VERBOSE

#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"

// USB includes
#include "usblib/usb-ids.h"
#include "driverlib/usb.h"
#include "usblib/usblib.h"
#include "usblib/device/usbdevice.h"

#include "usblib/usbaudio.h"
#include "usblib/device/usbdaudio.h"
#include "usblib/usblibpriv.h"

// uDMA used in USB
#include "driverlib/udma.h"

#define USB_VID_INCIDENTTECH 				0xDEAD
#define USB_PID_GTAR						0xBEEF
#define USB_AUDIO_SUBCLASS_AUDIO_CONTROL	0x01
#define USB_AUDIO_SUBCLASS_MIDISTREAMING	0x03

// Audio Control Interface
#define USB_CS_INTERFACE					0x24
#define USB_CS_ENDPOINT						0x25
#define USB_HEADER_SUBTYPE					0x01

#define USB_MS_GENERAL						0x01

// MIDI 
#define USB_MIDI_IN_JACK					0x02
#define USB_MIDI_OUT_JACK					0x03

#define USB_MIDI_JACK_EMBEDDED				0x01
#define USB_MIDI_JACK_EXTERNAL				0x02

// MIDI Mode Defines
#define IAP_MIDI	0
#define USB_MIDI	1

#define MIDI_OUT_EP		USB_EP_3

//#define IAP_ONLY

// Class Specific Audio Control Interface Descriptor
typedef struct AudioControlInterfaceDescriptor  {
	uint8_t bLength;									// Size of the interface descriptor
	uint8_t bDescriptorType;					// Type of descriptor: CS_INTERFACE 
	uint8_t bDescriptorSubtype;					// Header subtype
	uint16_t bcdADC;					// Revision of class spec 1.0
	uint16_t wTotalLength;					// Total size of class specific descriptors
	uint8_t bInCollection;									// Number of streaming interfaces
	uint8_t baInterfaceNum;									// MIDIStreaming interface 1 belongs to this AudioControl Interface
} AudioControlInterfaceDescriptor;

typedef struct MidiStreamingInterfaceDescriptor {
	uint8_t bLength;									// size of this descriptor in bytes
	uint8_t bDescriptorType;					// CS_INTERFACE descriptor
	uint8_t bDescriptorSubtype;					// MS_Header subtype
	uint16_t bcdADC;					// Revision of this class spec 1.0
	uint16_t wTotalLength;					// Total size of class specific descriptors
} MidiStreamingInterfaceDescriptor;

// Midi class specific In Jack Interface Descriptor
typedef struct MidiInJackInterfaceDescriptor {
	uint8_t bLength;									// size of the descriptor
	uint8_t bDescriptorType;					// Descriptor type is a CS_INTERFACE desc
	uint8_t bDescriptorSubtype;					// MIDI_IN_JACK subtype
	uint8_t bJackType;				// Embedded jack
	uint8_t bJackID;									// Jack ID
	uint8_t iJack;									// String index for jack (unused)
} MidiInJackInterfaceDescriptor;

typedef struct MidiOutJackInterfaceDescriptor {
	uint8_t bLength;									// size of the descriptor
	uint8_t bDescriptorType;					// Descriptor type is a CS_INTERFACE desc
	uint8_t bDescriptorSubtype;					// MIDI Out Jack subtype
	uint8_t bJackType;				// Embedded Jack
	uint8_t bJackID;									// Jack ID
	uint8_t bNumInputPins;									// number of input pins on this jack
	uint8_t baSourceID;									// ID of entity to which pin is connected
	uint8_t baSourcePin;									// Output Pin number of the entity to which input is connected
	uint8_t iJack;									// String index for Jack (unused)
} MidiOutJackInterfaceDescriptor;

// class specific bulk endpoint descriptor
typedef struct MidiStreamingEndpointDescriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bNumEmbMIDIJacks;
	uint8_t baAssocJackID;	
} MidiStreamingEndpointDescriptor;

typedef struct USBMIDIDevice {
	tDeviceInfo *m_pDevInfo;
} USBMIDIDevice;

typedef enum USBMIDIPacketMachineStates {
	UMPMS_INITIAL,
	UMPMS_SET_LED_0,	// set led
	UMPMS_SET_LED_1,
	UMPMS_SET_LED_2,
	UMPMS_SET_LED_EX_0,		// set led ex (same as set led, but uses 0b0MRRGGBB format for color)
	UMPMS_SET_LED_EX_1,
	UMPMS_SET_LED_EX_2,
	UMPMS_SET_NA_0,		// note active
	UMPMS_SET_NA_1,
	UMPMS_SET_NA_2,
	UMPMS_SET_FW_0,		// fret follow
	UMPMS_SET_FW_1,
	UMPMS_SET_FW_2,

	UMPMS_DWLD_FW_0,	// downlaod firmware
	UMPMS_DWLD_FW_1,	// get page numbers etc
	UMPMS_DWLD_FW_2,		// download data
	UMPMS_DWLD_FW_3,		// download data
	UMPMS_DWLD_FW_DONE_2,		// download done with 2 bytes left
	UMPMS_DWLD_FW_DONE_3,		// download done with 3 bytes left
	UMPMS_SET_PIEZO_STATE_0,	// set the piezo sensor state
	UMPMS_SET_PIEZO_THRESH_0,	// set piezo thresh
	UMPMS_SET_SMART_PICK_THRESH_0,
	UMPMS_SET_FRETBOARD_THRESH_0,		// set fretboard thresh

	// Piezo Actions
	UMPMS_SET_PIEZO_TIMEOUT_0,			// set piezo sensor timeout
	UMPMS_GET_PIEZO_TIMEOUT_0,				// get piezo sensor timeout

	UMPMS_SET_PIEZO_CT_MATRIX_0,		// set piezo sensor cross talk matrix
	UMPMS_SET_PIEZO_SENSITIVITY_0,		// set piezo sensor sensitivity
	UMPMS_SET_PIEZO_WINDOW_0,			// set piezo sensor window values

	UMPMS_GET_PIEZO_CT_MATRIX_0,		// get piezo sensor cross talk matrix
	UMPMS_GET_PIEZO_SENSITIVITY_0,		// get piezo sensor sensitivity
	UMPMS_GET_PIEZO_WINDOW_0,			// get piezo sensor window values


	UMPMS_CALIBRATE_PIEZO_STRING_0,		// calibrate given string

	UMPMS_PIEZO_CMD,					// direct pipe of piezo commands from controller to TI module

	UMPMS_SET_SERIAL_NUMBER_0,				// Set serial number
	UMPMS_REQ_SERIAL_NUM_0,				// request serial #

	UMPMS_SET_ACCELEROMETER_STATE_0,		// Set Acceleromtere state

	UMPMS_INVALID
} USB_MIDI_PACKET_MACHINE_STATES;

extern USB_MIDI_PACKET_MACHINE_STATES g_USBMIDIPacketMachineState;

typedef enum USBMIDICCSetLEDMachineState {
	UMCCSLMS_INITIAL,
	UMCCSLMS_SET_LED_0,
	UMCCSLMS_INVALID
} USB_MIDI_CC_SET_LED_MACHINE_STATES;

extern USB_MIDI_CC_SET_LED_MACHINE_STATES g_USBMIDICCSetLEDMachineState;

// The MIDI Device Descriptor
extern const uint8_t gc_pDeviceDescriptor[];

// The MIDI Adapter Configuration Descriptor
extern const uint8_t gc_pConfigurationDescriptor[];

#ifdef IPHONE_IAP
// iAP interface descriptor
extern const uint8_t gc_piAPInterfaceDescriptors[];
#endif

// Midi Device Audio Control and MIDI Streaming Interface descriptors
// Required although no audio functionality is actually implemented
extern const uint8_t gc_pMidiInterfaceDescriptors[];

#ifdef IPHONE_IAP
// iAP endpoints
extern const uint8_t gc_piAPEndpoints[];
#endif

// MIDI endpoints
extern const uint8_t gc_pMidiEndpoints[];

// Set up the Config Sections for the Config Header
extern const tConfigSection gc_ConfigSection;
extern const tConfigSection gc_MidiInterfaceSection;
extern const tConfigSection gc_MidiEndpointSection;
#ifdef IPHONE_IAP
extern const tConfigSection gc_iAPInterfaceSection;
extern const tConfigSection gc_iAPEndpointSection;
#endif

// Combine all of the sections into one config section array
extern const tConfigSection *gc_pConfigSections[];

// This is the header of the configuration that we support
extern const tConfigHeader gc_ConfigHeader;
extern const tConfigHeader * const gc_pConfigDescriptors[];

// String Descriptors
extern const uint8_t gc_pLangDescriptor[];
extern const uint8_t gc_pManufacturerString[]; 
extern const uint8_t gc_pProductString[]; 
extern const uint8_t gc_pSerialString[];
#ifdef IPHONE_IAP
extern const uint8_t gc_piAPInterfaceString[];
#endif

// The array of stringh descriptors as needed by the enumeration code
extern const uint8_t * const gc_ppStringDescriptors[]; 
extern uint8_t g_fUSBConnected;

// Audio / USB status
extern uint8_t g_USBStatus;
extern uint8_t g_AudioStatus;
extern uint8_t g_LastUSBStatus;
extern uint8_t g_LastAudioStatus;

typedef enum {
	USB_MIDI_SYS_EX				= 0x4,
	USB_MIDI_NOTE_OFF 			= 0x8,
	USB_MIDI_NOTE_ON 			= 0x9,
	USB_MIDI_POLY_AFTER_TOUCH 	= 0xA,
	USB_MIDI_CONTROL_CHANGE		= 0xB,
	USB_MIDI_PROGRAM_CHANGE		= 0xC,
	USB_MIDI_AFTER_TOUCH		= 0xD,
	USB_MIDI_PITCH				= 0xE
} USB_MIDI_MSG_TYPE;

//USB-MIDI port
#define USB_MIDI_PERIPH		SYSCTL_PERIPH_GPIOJ
#define USB_MIDI_PORT_BASE	GPIO_PORTJ_BASE
#define USB_MIDI_DP_PIN		GPIO_PIN_0
#define USB_MIDI_DM_PIN		GPIO_PIN_1

// Initialize USB-MIDI
RESULT InitUSBMIDI();

//USB and Audio connect status HW defines
#define USB_STATUS_PERIPH	SYSCTL_PERIPH_GPIOK
#define USB_STATUS_PORT		GPIO_PORTK_BASE
#define USB_STATUS_PIN		GPIO_PIN_2

#define AUDIO_STATUS_PERIPH SYSCTL_PERIPH_GPIOH
#define AUDIO_STATUS_PORT	GPIO_PORTH_BASE
#define AUDIO_STATUS_PIN	GPIO_PIN_4

void ReadUSBStatus();
void ReadAudioStatus();

/*
void GPIOPortHIntHandler(void); //TODO: convert to contextual JACK_PLATE handlers
void GPIOPortKIntHandler(void); //simplify interrupt calls and place all jack related
								//interrupts into single handler
*/

RESULT AudioStatusCallback(void *pContext);
RESULT USBStatusCallback(void *pContext);

// USB CALLBACKS
tStdRequest GetDescriptorCB(void *pvInstance, tUSBRequest *pUSBRequest);
tStdRequest RequestHandlerCB(void *pvInstance, tUSBRequest *pUSBRequest);
tInterfaceCallback InterfaceChangeCB(void *pvInstance, uint8_t ucInterfaceNum, uint8_t ucAlternateSetting);
tInfoCallback ConfigChangeCB(void *pvInstance, uint32_t  ulInfo);
tInfoCallback DataReceivedCB(void *pvInstance, uint32_t  ulInfo);

tUSBIntHandler BusResetCB(void *pvInstance);
tUSBIntHandler BusSuspendCB(void *pvInstance);
tUSBIntHandler BusResumeCB(void *pvInstance);
tUSBIntHandler BusDisconnectCB(void *pvInstance);

void EndpointCB(void *pvInstance, uint32_t  ulStatus);

tUSBDeviceHandler DeviceHandlerCB(void *pvInstance, uint32_t  ulRequest, void *pvRequestData);

extern tDeviceInfo gc_MidiDevice;

RESULT ClearPiezoCmd();
RESULT WrapMIDIBuffer(uint8_t **pBuffer, uint8_t *pBuffer_np);
RESULT UnwrapMIDIBuffer(uint8_t *pBuffer, uint8_t *pBuffer_n);

// Opho Device Callbacks
typedef RESULT (*cbOnBusSuspend)(void);
extern cbOnBusSuspend g_OnBusSuspendCallback;
RESULT RegisterOnBusSuspendCallback(cbOnBusSuspend OnBusSuspendCB);
RESULT UnregisterOnBusSuspendCallback();

typedef RESULT (*cbOnUSBStatus)(void);
extern cbOnUSBStatus g_OnUSBStatusCallback;
RESULT RegisterOnUSBStatusCallback(cbOnUSBStatus OnUSBStatusCB);
RESULT UnregisterOnUSBStatusCallback();

// Utility to send Zero Length Packet
RESULT SendZLP(uint32_t ui32Base, uint32_t ui32Endpoint);

RESULT SendUSBBuffer(uint32_t ui32Base, uint32_t ui32Endpoint, uint8_t *pBuffer, uint32_t pBuffer_n);
RESULT SendUSBBufferNoError(uint32_t ui32Base, uint32_t ui32Endpoint, uint8_t *pBuffer, uint32_t pBuffer_n);

// Device MIDI Functions
RESULT SendUSBMidiNoteMsg(uint8_t midiVal, uint8_t midiVelocity, uint8_t channel, uint8_t fOnOff);
RESULT SendUSBMidiCC(uint8_t index, uint8_t value);
RESULT SendUSBFirmwareVersion();
RESULT SendUSBFirmwareDownloadAck(uint8_t status);
RESULT SendUSBBatteryStatusAck();
RESULT SendUSBBatteryChargePercentageAck();

// Gtar MIDI Functions
RESULT SendUSBMidiFret(uint8_t fret, uint8_t channel, uint8_t fFretDown);
RESULT SendUSBPiezoFirmwareDownloadAck(uint8_t status);

RESULT SendUSBRequestSerialNumberAck(uint8_t byteNumber);

RESULT SendUSBAck(uint8_t SendBuffer[4]);
RESULT SendUSBGetPiezoCrossTalkMatrixAck(uint8_t row, uint8_t col);
RESULT SendUSBGetPiezoSensitivityAck(uint8_t string);
RESULT SendUSBGetPiezoWindowAck(uint8_t index);
RESULT SendUSBCalibratePiezoStringAck(uint8_t string);
RESULT SendUSBCommitUserspaceAck(uint8_t status);
RESULT SendUSBResetUserspaceAck(uint8_t status);

RESULT SendUSBPiezoCmdAck(uint8_t status);
RESULT DeployFirmwarePage();

// Handle Midi Packets (endpoint 2)
RESULT HandleUSBMIDIPacket(uint8_t *pBuffer, uint16_t pBuffer_n);

#endif /*USBDEFS_H_*/
