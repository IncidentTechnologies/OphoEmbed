#include "usbmidi.h"
#include "../../Common/STRINGS.h"
//#include "gtar.h"
#include "../../Device/Device.h"
#include "../../Device/UserspaceController.h"

/*
#include "FretLed.h"
#include "piezo_fw_update.h"
*/


//#include "opho/MIDI/GtarMIDIMessages.h"
#include "../MIDIMessages.h"

#ifdef IPHONE_IAP
	#include "iPhoneAuth.h"
#endif

uint8_t g_fUSBConnected = 0;
uint8_t g_fFirst = 0;

USB_MIDI_PACKET_MACHINE_STATES g_USBMIDIPacketMachineState = UMPMS_INITIAL;
USB_MIDI_CC_SET_LED_MACHINE_STATES g_USBMIDICCSetLEDMachineState = UMCCSLMS_INITIAL;


#ifdef USB_UDMA
//*****************************************************************************
// The control table used by the uDMA controller.  This table must be aligned
// to a 1024 byte boundary.  In this application uDMA is only used for USB,
// so only the first 6 channels are needed.
//*****************************************************************************
#if defined(ewarm)
	#pragma data_alignment=1024
	tDMAControlTable g_sDMAControlTable[64];
#elif defined(ccs)
	#pragma DATA_ALIGN(g_sDMAControlTable, 1024)
	tDMAControlTable g_sDMAControlTable[64];
#else
	tDMAControlTable g_sDMAControlTable[64] __attribute__ ((aligned(1024)));
#endif
#endif // ! USB_UDMA

//#define IAP_ENABLED

// The MIDI Device Descriptor
const uint8_t gc_pDeviceDescriptor[] =  {
	18,								// size of the descriptor
	USB_DTYPE_DEVICE,				// The type of this structure
	USBShort(0x0200),				// Say rev 1.1 otherwise hosts assume high speed
	USB_CLASS_DEVICE,				// USB Device class
	0,								// USB Sub device class (unused for this)	
	0,								// USB Device Protocol (unused for this)
	//64,								// MIDI Maximum Packet Size
	64,
    USBShort(USB_VID_TI_1CBE), 	// Vendor ID (VID).
    USBShort(USB_PID_SCOPE),     	// Product ID (PID).
    USBShort(0x100),             	// Device Version BCD.
	1,								// Manufacturer string id
	2,								// Product string id
	3, 								// Product serial number string id 
	1 								// Number of configurations (1 for this device)
};

// The MIDI Adapter Configuration Descriptor
const uint8_t gc_pConfigurationDescriptor[] = {
	9,								// size of the configuration descriptor
	USB_DTYPE_CONFIGURATION,		// Type of this descriptor as a config desc
	USBShort(101),					// The total size of this full structure
									// (Value is patched by the USB library so is
									// not important here)
	//3,								// The number of interfaces in this config
	2,	// without iAP interface
	1,								// The unique ID for this configuration
	0,								// String ID for this configuration (unused)
	//USB_CONF_ATTR_BUS_PWR,			// Power settings: Bus powered device, not self powered, no remote wakeup capabilites
	 USB_CONF_ATTR_SELF_PWR, 
	50								// The max power in 2mA increments
};

#ifdef IPHONE_IAP
// The MIDI Adapter Configuration Descriptor
const uint8_t gc_pConfigurationDescriptor_iAP[] = {
	9,								// size of the configuration descriptor
	USB_DTYPE_CONFIGURATION,		// Type of this descriptor as a config desc
	USBShort(101),					// The total size of this full structure
									// (Value is patched by the USB library so is
									// not important here)
	//3,								// The number of interfaces in this config
	3,								// adding the iAP interface
	1,								// The unique ID for this configuration
	0,								// String ID for this configuration (unused)
	//USB_CONF_ATTR_BUS_PWR,			// Power settings: Bus powered device, not self powered, no remote wakeup capabilites
	 USB_CONF_ATTR_SELF_PWR,
	50								// The max power in 2mA increments
};

const uint8_t gc_piAPInterfaceDescriptors[] = {
	// iAP interface
	9,									// Size of the interface descriptor
	USB_DTYPE_INTERFACE,				// Type of this descriptor
	0,									// The index of this interface
	0,									// The index of this setting										
	3,									// The number of endpoints used by this interface
	USB_CLASS_VEND_SPECIFIC,			// The interface class
	0xF0,								// The interface sub-class (specified in the MFI hardware spec)
	0x00,								// The interface protocol for the sublass (unused)
	4,									// The string index for this interface
};
#endif

// Midi Device Audio Control and MIDI Streaming Interface descriptors
// Required although no audio functionality is actually implemented
const uint8_t gc_pMidiInterfaceDescriptors[] = {
	// MIDI STUFF
	// Audio Control Standard Interface Descriptor
	9,									// Size of the interface descriptor
	USB_DTYPE_INTERFACE,				// Type of this descriptor
	0,									// The index of this interface (no iAP)
	0,									// The index of this setting
	0,									// The number of endpoints used by this interface
	USB_CLASS_AUDIO,					// The interface class
	USB_AUDIO_SUBCLASS_AUDIO_CONTROL,	// The interface sub-class
	0,									// The interface protocol for the sublass (unused)
	0,									// The string index for this interface
	// Audio Control Class Specific Interface Descriptor
	9,									// Size of the interface descriptor
	USB_CS_INTERFACE,					// Type of descriptor: CS_INTERFACE
	USB_HEADER_SUBTYPE,					// Header subtype
	USBShort(0x0100),					// Revision of class spec 1.0
	USBShort(0x0009),					// Total size of class specific descriptors
	1,									// Number of streaming interfaces
	1,									// MIDIStreaming interface 2 belongs to this AudioControl Interface (no iAP)
	// MIDI Streaming Interface Descriptor
	9,									// Size of the interface descriptor
	USB_DTYPE_INTERFACE,				// Type of this descriptor
	1,									// The index of this interface (no iAP)
	0,									// the index of this setting
	2,									// the number of endpoints used by this interface
	USB_CLASS_AUDIO,					// The interface class
	USB_AUDIO_SUBCLASS_MIDISTREAMING,	// the interface sub-class
	0,									// Interface protocol for the subclass (unused)
	0,									// String index for this interface (unused)
	// MIDI Streaming Class Specific Interface Descriptor
	7,									// size of this descriptor in bytes
	USB_CS_INTERFACE,					// CS_INTERFACE descriptor
	USB_HEADER_SUBTYPE,					// MS_Header subtype
	USBShort(0x0100),					// Revision of this class spec 1.0
	USBShort(0x0041),					// Total size of class specific descriptors
	// The Midi IN Jack Descriptors
	// Embedded Jack
	6,									// size of the descriptor
	USB_CS_INTERFACE,					// Descriptor type is a CS_INTERFACE desc
	USB_MIDI_IN_JACK,					// MIDI_IN_JACK subtype
	USB_MIDI_JACK_EMBEDDED,				// Embedded jack
	1,									// Jack ID
	0,									// String index for jack (unused)
	// External Jack
	6,									// size of the endpoint descriptor
	USB_CS_INTERFACE,					// Descriptor type is a CS_INTERFACE desc
	USB_MIDI_IN_JACK,					// MIDI_IN_JACK subtype
	USB_MIDI_JACK_EXTERNAL,				// Embedded jack
	2,									// Jack ID
	0,									// String index for jack (unused)
	// The MIDI OUT Jack Descriptors
	// Embedded
	9,									// size of the descriptor
	USB_CS_INTERFACE,					// Descriptor type is a CS_INTERFACE desc
	USB_MIDI_OUT_JACK,					// MIDI Out Jack subtype
	USB_MIDI_JACK_EMBEDDED,				// Embedded Jack
	3,									// Jack ID
	1,									// number of input pins on this jack
	2,									// ID of entity to which pin is connected
	1,									// Output Pin number of the entity to which input is connected
	0,									// String index for Jack (unused)
	// External
	9,									// size of the descriptor
	USB_CS_INTERFACE,					// Descriptor type is a CS_INTERFACE desc
	USB_MIDI_OUT_JACK,					// MIDI Out Jack subtype
	USB_MIDI_JACK_EXTERNAL,				// Embedded Jack
	4,									// Jack ID
	1,									// number of input pins on this jack
	1,									// ID of entity to which pin is connected
	1,									// Output Pin number of the entity to which input is connected
	0,									// String index for Jack (unused)
};

#ifdef IPHONE_IAP
// Midi Device Audio Control and MIDI Streaming Interface descriptors
// iAP specific version
const uint8_t gc_pMidiInterfaceDescriptors_iAP[] = {
	// MIDI STUFF
	// Audio Control Standard Interface Descriptor
	9,									// Size of the interface descriptor
	USB_DTYPE_INTERFACE,				// Type of this descriptor
	1,									// The index of this interface
	0,									// The index of this setting
	0,									// The number of endpoints used by this interface
	USB_CLASS_AUDIO,					// The interface class
	USB_AUDIO_SUBCLASS_AUDIO_CONTROL,	// The interface sub-class
	0,									// The interface protocol for the sublass (unused)
	0,									// The string index for this interface
	// Audio Control Class Specific Interface Descriptor
	9,									// Size of the interface descriptor
	USB_CS_INTERFACE,					// Type of descriptor: CS_INTERFACE 
	USB_HEADER_SUBTYPE,					// Header subtype
	USBShort(0x0100),					// Revision of class spec 1.0
	USBShort(0x0009),					// Total size of class specific descriptors
	1,									// Number of streaming interfaces
	2,									// MIDIStreaming interface 2 belongs to this AudioControl Interface
	// MIDI Streaming Interface Descriptor
	9,									// Size of the interface descriptor
	USB_DTYPE_INTERFACE,				// Type of this descriptor
	2,									// The index of this interface
	0,									// the index of this setting
	2,									// the number of endpoints used by this interface
	USB_CLASS_AUDIO,					// The interface class
	USB_AUDIO_SUBCLASS_MIDISTREAMING,	// the interface sub-class
	0,									// Interface protocol for the subclass (unused)
	0,									// String index for this interface (unused)
	// MIDI Streaming Class Specific Interface Descriptor
	7,									// size of this descriptor in bytes
	USB_CS_INTERFACE,					// CS_INTERFACE descriptor
	USB_HEADER_SUBTYPE,					// MS_Header subtype
	USBShort(0x0100),					// Revision of this class spec 1.0
	USBShort(0x0041),					// Total size of class specific descriptors
	// The Midi IN Jack Descriptors
	// Embedded Jack
	6,									// size of the descriptor
	USB_CS_INTERFACE,					// Descriptor type is a CS_INTERFACE desc
	USB_MIDI_IN_JACK,					// MIDI_IN_JACK subtype
	USB_MIDI_JACK_EMBEDDED,				// Embedded jack
	1,									// Jack ID
	0,									// String index for jack (unused)
	// External Jack
	6,									// size of the endpoint descriptor
	USB_CS_INTERFACE,					// Descriptor type is a CS_INTERFACE desc
	USB_MIDI_IN_JACK,					// MIDI_IN_JACK subtype
	USB_MIDI_JACK_EXTERNAL,				// Embedded jack
	2,									// Jack ID
	0,									// String index for jack (unused)
	// The MIDI OUT Jack Descriptors
	// Embedded
	9,									// size of the descriptor
	USB_CS_INTERFACE,					// Descriptor type is a CS_INTERFACE desc
	USB_MIDI_OUT_JACK,					// MIDI Out Jack subtype
	USB_MIDI_JACK_EMBEDDED,				// Embedded Jack
	3,									// Jack ID
	1,									// number of input pins on this jack
	2,									// ID of entity to which pin is connected
	1,									// Output Pin number of the entity to which input is connected
	0,									// String index for Jack (unused)
	// External
	9,									// size of the descriptor
	USB_CS_INTERFACE,					// Descriptor type is a CS_INTERFACE desc
	USB_MIDI_OUT_JACK,					// MIDI Out Jack subtype
	USB_MIDI_JACK_EXTERNAL,				// Embedded Jack
	4,									// Jack ID
	1,									// number of input pins on this jack
	1,									// ID of entity to which pin is connected
	1,									// Output Pin number of the entity to which input is connected
	0,									// String index for Jack (unused)
};


// iAP endpoints
const uint8_t gc_piAPEndpoints[] = {
	// Standard interrupt IN endpoint (for iAP)
	7,									// size of the descriptor in bytes
	USB_DTYPE_ENDPOINT,					// Endpoint descriptor
	0x81,								// IN Endpoint 1
	USB_EP_ATTR_INT,					// Interrupt
	USBShort(128),						// 64 bytes per packet
	16,									// Interval
	// Standard Bulk IN Endpoint Descriptor
	7,									// size of the descriptor in bytes
	USB_DTYPE_ENDPOINT,					// Endpoint descriptor
	0x82,								// IN Endpoint 2
	USB_EP_ATTR_BULK,					// Bulk, not shared
	USBShort(64 * 3),						// 64 bytes per packet
	//USBShort(128),						// 64 bytes per packet
	0,									// Interval, ignored for Bulk (unused)
	// Standard Bulk OUT Endpoint Descriptor
	7,									// Size of the descriptor
	USB_DTYPE_ENDPOINT,					// Endpoint descriptor
	1,									// OUT Endpoint 1
	USB_EP_ATTR_BULK,					// Bulk, Not Shared
	USBShort(128),						// 64 bytes per packet
	0,									// Interval, ignored for Bulk (unused)
};
#endif

// MIDI endpoints
const uint8_t gc_pMidiEndpoints[] =  {
	// MIDI
	// Standard Bulk OUT Endpoint Descriptor
	9,									// Size of the descriptor
	USB_DTYPE_ENDPOINT,					// Endpoint descriptor
	2,									// OUT Endpoint 2
	USB_EP_ATTR_BULK,					// Bulk, Not Shared
	USBShort(64),						// 64 bytes per packet
	0,									// Interval, ignored for Bulk (unused)
	0,									// reserved
	0,									// reserved
	// Class specific bulk out endpoint descriptor
	5,									// size of descriptor
	USB_CS_ENDPOINT,					// Type of descriptor
	USB_MS_GENERAL,						// MS_GENERAL subtype
	1,									// Number of embedded MIDI IN jacks
	1,									// ID of embedded MIDI IN jack	
	// Standard Bulk IN Endpoint Descriptor
	9,									// size of the descriptor in bytes
	USB_DTYPE_ENDPOINT,					// Endpoint descriptor
	0x83,								// IN Endpoint 3
	USB_EP_ATTR_BULK,					// Bulk, not shared
	USBShort(64),						// 64 bytes per packet
	0,									// Interval, ignored for Bulk (unused)
	0,									// reserved
	0,									// reserved
	// Class specific bulk in endpoint descriptor
	5,									// size of descriptor
	USB_CS_ENDPOINT,					// Type of descriptor
	USB_MS_GENERAL,						// MS_GENERAL subtype
	1,									// Number of embedded MIDI OUT jacks
	3,									// ID of embedded MIDI OUT jack
};

// Set up the Config Sections for the Config Header
const tConfigSection gc_ConfigSection =  {
	sizeof(gc_pConfigurationDescriptor),
	gc_pConfigurationDescriptor
};

#ifdef IPHONE_IAP
// Set up the Config Sections for the Config Header
const tConfigSection gc_ConfigSection_iAP = {
	sizeof(gc_pConfigurationDescriptor_iAP),
	gc_pConfigurationDescriptor_iAP
};

const tConfigSection gc_iAPInterfaceSection = {
	sizeof(gc_piAPInterfaceDescriptors),
	gc_piAPInterfaceDescriptors	
};

const tConfigSection gc_iAPEndpointSection = {
	sizeof(gc_piAPEndpoints),
	gc_piAPEndpoints
};
#endif

const tConfigSection gc_MidiInterfaceSection = {
	sizeof(gc_pMidiInterfaceDescriptors),
	gc_pMidiInterfaceDescriptors	
};

#ifdef IPHONE_IAP
const tConfigSection gc_MidiInterfaceSection_iAP = {
	sizeof(gc_pMidiInterfaceDescriptors_iAP),
	gc_pMidiInterfaceDescriptors_iAP
};
#endif

const tConfigSection gc_MidiEndpointSection =  {
	sizeof(gc_pMidiEndpoints),
	gc_pMidiEndpoints
};

// Combine all of the sections into one config section array
const tConfigSection *gc_pConfigSections[] =  {
	&gc_ConfigSection,
	&gc_MidiInterfaceSection,
	&gc_MidiEndpointSection
};

#define NUM_CONFIG_SECTIONS (sizeof(gc_pConfigSections) / sizeof(tConfigSection *))

// This is the header of the configuration that we support
const tConfigHeader gc_ConfigHeader = {
	NUM_CONFIG_SECTIONS,
	gc_pConfigSections
};

const tConfigHeader * const gc_pConfigDescriptors[] = {
	&gc_ConfigHeader
};

#ifdef IPHONE_IAP
// Combine all of the sections into one config section array (iAP version)
const tConfigSection *gc_pConfigSections_iAP[] = {
	&gc_ConfigSection_iAP,
	&gc_iAPInterfaceSection,
	&gc_iAPEndpointSection,
	&gc_MidiInterfaceSection_iAP,
	&gc_MidiEndpointSection
};
#endif

#define NUM_CONFIG_SECTIONS_IAP (sizeof(gc_pConfigSections_iAP) / sizeof(tConfigSection *))

#ifdef IPHONE_IAP
// This is the header of the configuration that we support
const tConfigHeader gc_ConfigHeader_iAP = {
	NUM_CONFIG_SECTIONS_IAP,
	gc_pConfigSections_iAP
};

const tConfigHeader * const gc_pConfigDescriptors_iAP[] = {
	&gc_ConfigHeader_iAP
};
#endif

// String Descriptors
const uint8_t gc_pLangDescriptor[] = {
	4,
	USB_DTYPE_STRING,
	USBShort(USB_LANG_EN_US)	
};

const uint8_t gc_pManufacturerString[] =  {
	(12 + 1) * 2,
	USB_DTYPE_STRING,
	'I', 0, 'n', 0, 'c', 0, 'i', 0, 'd', 0, 'e', 0, 'n', 0, 't', 0, 'T', 0, 'e', 0, 'c', 0, 'h', 0
	
};

const uint8_t gc_pProductString[] =  {
	(4 + 1) * 2,
	USB_DTYPE_STRING,
	'g', 0, 'T', 0, 'a', 0, 'r', 0
};

const uint8_t gc_pSerialString[] = {
	(4 + 1) * 2,
	USB_DTYPE_STRING,
	'G', 0, 'T', 0, 'A', 0, 'R', 0
};

const uint8_t gc_piAPInterfaceString[] = {
	(13 + 1) * 2,
	USB_DTYPE_STRING,
	'i', 0, 'A', 0, 'P', 0, ' ', 0, 'I', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0 	
};

// The array of stringh descriptors as needed by the enumeration code
const uint8_t * const gc_ppStringDescriptors[] = {
	gc_pLangDescriptor,
	gc_pManufacturerString,
	gc_pProductString,
	gc_pSerialString,
};

#ifdef IPHONE_IAP
// The array of stringh descriptors as needed by the enumeration code
const uint8_t * const gc_ppStringDescriptors_iAP[] = {
	gc_pLangDescriptor,
	gc_pManufacturerString,
	gc_pProductString,
	gc_pSerialString,
	gc_piAPInterfaceString
};
#endif

const tCustomHandlers gc_Callbacks = {
	// Set up the event handlers
	// Setting a function to NULL disables notification for that specific event
	GetDescriptorCB,					// non-standard descriptor request (unused)
	RequestHandlerCB,				// non-standard request (unused)
	InterfaceChangeCB,				// Interface Configuration change (unused)
	ConfigChangeCB,							// Configuration change (unused)
	DataReceivedCB,					// Data RX on ep0 (unused)
	0,									// Data TX on ep0 (unused)
	BusResetCB,								// Bus Reset Handler (unused)
	BusSuspendCB,							// Bus Suspend Handler (unused)
	BusResumeCB,						// Bus Resume Handler (unused)
	BusDisconnectCB,					// Bus Disconnect Handler (unused)
	EndpointCB,								// Handle all other end points
	DeviceHandlerCB							// Used for a composite device (unused for now)
};

// USB Device Info
tDeviceInfo gc_MidiDevice = {
	&gc_Callbacks,
	gc_pDeviceDescriptor,			// Device Desc
	gc_pConfigDescriptors,			// Configuration Descs
	gc_ppStringDescriptors,				// string descs				
	(uint32_t )(sizeof(gc_ppStringDescriptors) / sizeof (uint8_t const *))		// number of strings
};

#ifdef IPHONE_IAP
// USB Device Info
tDeviceInfo gc_MidiDevice_iAP =
{
	&gc_Callbacks,
	gc_pDeviceDescriptor,			// Device Desc
	gc_pConfigDescriptors_iAP,			// Configuration Descs
	gc_ppStringDescriptors_iAP,				// string descs
	(uint32_t )(sizeof(gc_ppStringDescriptors_iAP) / sizeof (uint8_t const *))		// number of strings
};
#endif

tStdRequest GetDescriptorCB(void *pvInstance, tUSBRequest *pUSBRequest) {
#ifdef USB_VERBOSE
	DEBUG_LINEOUT_NA("GetDescriptorCB");
#endif
	return NULL;
}

tStdRequest RequestHandlerCB(void *pvInstance, tUSBRequest *pUSBRequest) {
#ifdef USB_VERBOSE
	DEBUG_LINEOUT("RequestHandlerCB type:0x%x r:0x%x val:0x%x i:0x%x len:0x%x",
				   pUSBRequest->bmRequestType,  pUSBRequest->bRequest,
				   pUSBRequest->wValue, pUSBRequest->wIndex, pUSBRequest->wLength);
#endif
	
	return NULL;
}

tInterfaceCallback InterfaceChangeCB(void *pvInstance, uint8_t ucInterfaceNum, uint8_t ucAlternateSetting) {
#ifdef USB_VERBOSE
	DEBUG_LINEOUT_NA("InterfaceChangeCB");
#endif
	return NULL;	
}

tInfoCallback ConfigChangeCB(void *pvInstance, uint32_t  ulInfo) {
#ifdef USB_VERBOSE
	DEBUG_LINEOUT("ConfigChangeCB: 0x%x", ulInfo);
#endif
	
	// flush out the FIFOs
	USBFIFOFlush(USB0_BASE, USB_EP_1, USB_EP_DEV_IN);
	USBFIFOFlush(USB0_BASE, USB_EP_2, USB_EP_DEV_IN);
	USBFIFOFlush(USB0_BASE, USB_EP_1, USB_EP_DEV_OUT);
	
	// send zlp on interrupt pipe	
	SendZLP(USB0_BASE, USB_EP_1);
	
	return NULL;
}

tInfoCallback DataReceivedCB(void *pvInstance, uint32_t  ulInfo) {
#ifdef USB_VERBOSE
	DEBUG_LINEOUT_NA("DataReceivedCB");
#endif
	return NULL;
}

tUSBIntHandler BusResetCB(void *pvInstance) {
#ifdef USB_VERBOSE
	DEBUG_LINEOUT_NA("BusResetCB");
#endif
	return NULL;
}

tUSBIntHandler BusSuspendCB(void *pvInstance) {
#ifdef USB_VERBOSE
	DEBUG_LINEOUT_NA("BusSuspendCB");
#endif
	
	g_fFirst = 0;
	g_fUSBConnected = 0;

#ifdef IPHONE_IAP
	g_fiPhoneAuthenticated = 0;
#endif

	// Call the Device Callback
	if(g_OnBusSuspendCallback != NULL)
		g_OnBusSuspendCallback();
	
	return NULL;
}

tUSBIntHandler BusResumeCB(void *pvInstance) {
#ifdef USB_VERBOSE
	DEBUG_LINEOUT_NA("BusResumeCB");
#endif
	return NULL;
}

tUSBIntHandler BusDisconnectCB(void *pvInstance) {
#ifdef USB_VERBOSE
	DEBUG_LINEOUT_NA("BusDisconnectCB");
#endif
	return NULL;
}

static void EndpointCB(void *pvInstance, uint32_t  ulStatus) {
	 int32_t  r = 0;
	 int32_t  i = 0;
	
	if(ulStatus == 0)
		return;
	
#ifdef USB_VERBOSE
	DEBUG_LINEOUT("EndpointCB status:0x%x\r\n", ulStatus);
#endif
	
	 int32_t  ep = USB_EP_0;

	if(ulStatus & 0x80000)
		ep = USB_EP_3;
	else if(ulStatus & 0x2 || ulStatus & 0x20000 )
		ep = USB_EP_1;
	else if(ulStatus & 0x4 || ulStatus & 0x40000)
		ep = USB_EP_2;  	
		
	// lets not mess with it if it's EP0
	if(ep != USB_EP_0)
	{
		uint32_t  ulFlags = USBEndpointStatus(USB0_BASE, ep);	
		
#ifdef USB_VERBOSE
		if(ulFlags != 0x0) 
			DEBUG_LINEOUT("Endpoint 0x%x flags:0x%x", ulStatus, ulFlags);
#endif
		
#ifdef IPHONE_IAP
		// First endpoint one message, initiate iPhone auth
		if(ep == USB_EP_1 && g_fFirst == 0) {
			g_TransactionIdCounter = 0;
			//SendIDPSMessage(g_TransactionIdCounter++);
			iAP2SendInitialization();
			g_fFirst = 1;
		}
		
		// If we received data on endpoint 1, read it here
		if(ulStatus & USB_INTEP_DEV_OUT_1) {
			//DEBUG_LINEOUT_NA("RX iPhone Data\r\n");

			uint32_t  ulBytesReturned = 64;
			uint8_t Buffer[64]; //Give it the max bytes for now // = (uint8_t *)malloc(ulBytesAvail);
			
			r = USBEndpointDataGet(USB0_BASE, USB_EP_1, Buffer, &ulBytesReturned);
			if(r == -1 || ulBytesReturned == 0) {
#ifdef USB_VERBOSE
				DEBUG_LINEOUT("USBEndpointDataGet failed to get the packet count:%d", ulBytesReturned);
#endif
				goto Error;	
			} 
			else {
				USBDevEndpointDataAck(USB0_BASE, USB_EP_1, false);		// ACK the data
				//DEBUG_LINEOUT("EP1: %d bytes available", ulBytesReturned);
			}			
			
#ifdef USB_VERBOSE
			// Read out the packet for debug
			DEBUG_LINEOUT_NA("*** rx pkt ***");
			UARTprintfBinaryData(Buffer, ulBytesReturned, 20);
#endif
			
			// Clear the status before handling the return data
			USBDevEndpointStatusClear(USB0_BASE, ep, ulFlags);
			//DEBUG_LINEOUT("EP1: Status on ep:0x%x cleared: 0x%x", ep, ulFlags);
			
			//HandleiPhoneACK(Buffer, ulBytesReturned);
			HandleiPhoneData(Buffer, ulBytesReturned);		
		}
#endif

		if(ulStatus & USB_INTEP_DEV_OUT_2) {
			uint32_t  ulBytesReturned = 64;
			uint8_t Buffer[64]; //Give it the max bytes for now // = (uint8_t *)malloc(ulBytesAvail);
			
			CRM(USBEndpointDataGet(USB0_BASE, USB_EP_2, Buffer, &ulBytesReturned), 
					"USBEndpointDataGet failed to get the packet count:%d\r\n", ulBytesReturned);

			USBDevEndpointDataAck(USB0_BASE, USB_EP_2, false);		// ACK the data
#ifdef USB_VERBOSE
			DEBUG_LINEOUT("EP2: %d bytes available", ulBytesReturned);
#endif
			
			// Clear the status before handling the return data
			USBDevEndpointStatusClear(USB0_BASE, ep, ulFlags);
#ifdef USB_VERBOSE
			DEBUG_LINEOUT("EP2: Status on ep:0x%x cleared: 0x%x", ep, ulFlags);
#endif
			
			// Handle the data here if needed!
			HandleUSBMIDIPacket(Buffer, ulBytesReturned);
		}

		// for any others
		if(ulStatus & 0xFFF80000) {
			// Clear the status
			USBDevEndpointStatusClear(USB0_BASE, ep, ulFlags);
#ifdef USB_VERBOSE
			DEBUG_LINEOUT("Unhandled status on ep:0x%x cleared: 0x%x", ep, ulFlags);
#endif
		}		
	}

Error:
	return;
}

tUSBDeviceHandler DeviceHandlerCB(void *pvInstance, uint32_t  ulRequest, void *pvRequestData) {
#ifdef USB_VERBOSE
	DEBUG_LINEOUT_NA("DeviceHandlerCB");
#endif
	return NULL;
}

// Opho Device Callbacks
// ************************************************************************************************************************
cbOnBusSuspend g_OnBusSuspendCallback = NULL;
RESULT RegisterOnBusSuspendCallback(cbOnBusSuspend OnBusSuspendCB) {
	RESULT r = R_OK;

	CBRM_NA((g_OnBusSuspendCallback == NULL), "RegisterOnBusSuspendCallback: On Bus Suspend Callback already registered");
	g_OnBusSuspendCallback = OnBusSuspendCB;

Error:
	return r;
}

RESULT UnregisterOnBusSuspendCallback() {
	RESULT r = R_OK;

	CBRM_NA((g_OnBusSuspendCallback != NULL), "UnregisterOnBusSuspendCallback: On Bus Suspend Callback not registered");
	g_OnBusSuspendCallback = NULL;

Error:
	return r;
}

cbOnUSBStatus g_OnUSBStatusCallback = NULL;
RESULT RegisterOnUSBStatusCallback(cbOnUSBStatus OnUSBStatusCB) {
	RESULT r = R_OK;

	CBRM_NA((g_OnUSBStatusCallback == NULL), "RegisterOnUSBStatusCallback: On USB Status Callback already registered");
	g_OnUSBStatusCallback = OnUSBStatusCB;

Error:
	return r;
}

RESULT UnregisterOnUSBStatusCallback() {
	RESULT r = R_OK;

	CBRM_NA((g_OnUSBStatusCallback != NULL), "UnregisterOnUSBStatusCallback: On USB Status Callback not registered");
	g_OnUSBStatusCallback = NULL;

Error:
	return r;
}

// ************************************************************************************************************************

RESULT SendZLP(uint32_t ui32Base, uint32_t ui32Endpoint) {
	RESULT r = R_OK;

	// Flush the FIFO first
	USBFIFOFlush(ui32Base, ui32Endpoint, USB_EP_DEV_IN);

	int32_t lStat = USBEndpointDataSend(ui32Base, ui32Endpoint, USB_TRANS_IN);
	CBRM((lStat != -1), "Failed to send ZLP on EP 0x%x USB Base 0x%x", ui32Endpoint, ui32Base);

Error:
	return r;
}

RESULT SendUSBBuffer(uint32_t ui32Base, uint32_t ui32Endpoint, uint8_t *pBuffer, uint32_t pBuffer_n) {
	RESULT r = R_OK;

	// Put data in FIFO
	int32_t lStat = USBEndpointDataPut(ui32Base, ui32Endpoint, pBuffer, pBuffer_n);
	CBRM((lStat != -1), "Failed to put data in FIFO base 0x%x ep 0x%x", ui32Base, ui32Endpoint);

	// Send the FIFO data
	lStat = USBEndpointDataSend(ui32Base, ui32Endpoint, USB_TRANS_IN);
	CBRM((lStat != -1), "Failed to send base 0x%x ep 0x%x data", ui32Base, ui32Endpoint);

Error:
	return r;
}

RESULT SendUSBBufferNoError(uint32_t ui32Base, uint32_t ui32Endpoint, uint8_t *pBuffer, uint32_t pBuffer_n) {

	// Put data in FIFO and fucking send it
	USBEndpointDataPut(ui32Base, ui32Endpoint, pBuffer, pBuffer_n);
	USBEndpointDataSend(ui32Base, ui32Endpoint, USB_TRANS_IN);

Error:
	return R_OK;
}

// SendUSBMidiNoteMsg 
// Sends a USB midi note with the given value, velocity, and channel
// whether this is an on or off message is indicated by the pszOnOff parameter which is case
// sensitive

// Since this is being called from the fret update code, need to make it very fast

uint8_t g_TempSendBuffer[4];	// optimize SendUSBMidiNoteMsg
int32_t  g_lStat = 0;

//inline RESULT SendUSBMidiNoteMsg(uint8_t midiVal, uint8_t midiVelocity, uint8_t channel, uint8_t fOnOff) {
RESULT SendUSBMidiNoteMsg(uint8_t midiVal, uint8_t midiVelocity, uint8_t channel, uint8_t fOnOff) {

#ifdef USB_SAFE
	// Added the SAFE flag since this function needs to be optimized for speed
	DEBUG_LINEOUT("+ SendUSBMidiNoteMsg val:%d ch:%d onoff: %d", midiVal, channel, fOnOff);

	// Do some error checks
	CBRM((channel >= 1 && channel <= 16), "SendUSBMidiNoteMsg: Channel %d must be [1,16]", channel);
	CBRM((midiVal <= 127), "SendUSBMidiNoteMsg: Midi value %d must be <= 127", midiVal);
	CBRM((midiVelocity <= 127), "SendUSBMidiNoteMsg: Midi velocuty %d must be <= 127", midiVelocity);
#endif
	
    if(fOnOff) {
    	g_TempSendBuffer[0] = 0x09;
    	g_TempSendBuffer[1] = MIDI_NOTE_ON;
    }
    else {
    	g_TempSendBuffer[0] = 0x08;
    	g_TempSendBuffer[1] = MIDI_NOTE_OFF;
    }
    
    g_TempSendBuffer[1] += channel;
    g_TempSendBuffer[2] = midiVal;
    g_TempSendBuffer[3] = midiVelocity;
    
    return SendUSBBufferNoError(USB0_BASE, MIDI_OUT_EP, &g_TempSendBuffer, 4);
}

RESULT SendUSBMidiCC(uint8_t index, uint8_t value) {
	RESULT r = R_OK;
	 int32_t  lStat = 0;
	uint8_t SendBuffer[4];

	if(!g_device.m_fUSB0)
		return R_NO_EFFECT;

	//DEBUG_LINEOUT("+SendUSBMidiCC index:%d value:%d", index, value);

	// Control message (cable 0)
	SendBuffer[0] = 0x0B;
	SendBuffer[1] = MIDI_CONTROL_CHANGE;
    SendBuffer[2] = index;
    SendBuffer[3] = value;

    return SendUSBBuffer(USB0_BASE, MIDI_OUT_EP, SendBuffer, 4);
}

// SendFirmwareVersion
// This will send the major and minor firmware version to the host
RESULT SendUSBFirmwareVersion() {
	RESULT r = R_OK;
	int32_t  lStat = 0;
	uint8_t SendBuffer[4];
	DEVICE_FIRMWARE_VERSION devFWVersion = GetDeviceFirmwareVersion();

#ifdef USB_VERBOSE
	DEBUG_LINEOUT("Sending Firmware Version %d.%d", devFWVersion.major, devFWVersion.minor);
#endif
	SendBuffer[0] = 0x0B;
	SendBuffer[1] = MIDI_CONTROL_CHANGE;
	SendBuffer[2] = DEVICE_SEND_MSG_FIRMWARE_VERSION;
	//SendBuffer[3] = ((FW_MAJOR_VERSION & 0xF) << 4) + ((FW_MINOR_VERSION) & 0xF);

	// TODO: This is silly
	uint8_t uiDevFWVersion = 0;
	memcpy(&uiDevFWVersion, &devFWVersion, sizeof(uint8_t));
	SendBuffer[3] = (uint8_t)(uiDevFWVersion);

	return SendUSBBuffer(USB0_BASE, MIDI_OUT_EP, SendBuffer, 4);
}

// SendFirmwareDownloadAck
// This will send the major and minor firmware version to the host
RESULT SendUSBFirmwareDownloadAck(uint8_t status) {
	RESULT r = R_OK;
	int32_t  lStat = 0;
	uint8_t SendBuffer[4];

	SendBuffer[0] = 0x0B;
	SendBuffer[1] = MIDI_CONTROL_CHANGE;
	SendBuffer[2] = DEVICE_ACK_DOWNLOAD_FW;
	SendBuffer[3] = status;

	return SendUSBBuffer(USB0_BASE, MIDI_OUT_EP, SendBuffer, 4);
}

// Send Battery Status Ack
RESULT SendUSBBatteryStatusAck() {
	RESULT r = R_OK;
	int32_t  lStat = 0;
	uint8_t SendBuffer[4];

	SendBuffer[0] = 0x0B;
	SendBuffer[1] = MIDI_CONTROL_CHANGE;
	SendBuffer[2] = DEVICE_ACK_BATTERY_STATUS;
	//SendBuffer[3] = ((uint8_t)g_gtar.m_fCharging & 0x7F);
	SendBuffer[3] = ((uint8_t)IsDeviceCharging() & 0x7F);

	return SendUSBBuffer(USB0_BASE, MIDI_OUT_EP, SendBuffer, 4);
}


RESULT SendUSBAck(uint8_t SendBuffer[4]) {
	return SendUSBBuffer(USB0_BASE, MIDI_OUT_EP, SendBuffer, 4);
}

// Send Battery Status Ack
RESULT SendUSBBatteryChargePercentageAck() {
	uint8_t SendBuffer[4];

	SendBuffer[0] = 0x0B;
	SendBuffer[1] = MIDI_CONTROL_CHANGE;
	SendBuffer[2] = DEVICE_ACK_BATTERY_PERCENTAGE;
	//SendBuffer[3] = g_Percentage & 0x7F;
	SendBuffer[3] = GetDeviceBatteryPercentage() & 0x7F;

	return SendUSBBuffer(USB0_BASE, MIDI_OUT_EP, SendBuffer, 4);
}

RESULT SendUSBCommitUserspaceAck(uint8_t status) {
	uint8_t SendBuffer[4];

	SendBuffer[0] = 0x0B;
	SendBuffer[1] = MIDI_CONTROL_CHANGE;
	SendBuffer[2] = DEVICE_ACK_COMMIT_USERSPACE;
	SendBuffer[3] = (uint8_t)(status);

	return SendUSBBuffer(USB0_BASE, MIDI_OUT_EP, SendBuffer, 4);
}

RESULT SendUSBResetUserspaceAck(uint8_t status) {
	uint8_t SendBuffer[4];

	SendBuffer[0] = 0x0B;
	SendBuffer[1] = MIDI_CONTROL_CHANGE;
	SendBuffer[2] = DEVICE_ACK_RESET_USERSPACE;
	SendBuffer[3] = (uint8_t)(status);

	return SendUSBBuffer(USB0_BASE, MIDI_OUT_EP, SendBuffer, 4);
}

RESULT SendUSBRequestSerialNumberAck(uint8_t byteNumber) {
	uint8_t SendBuffer[4];
	uint8_t serialByte = (uint8_t)(GetDeviceSerialNumber(byteNumber));

	// max 16 bytes in serial number
	if(byteNumber > 0x0F)
		return R_OUT_OF_BOUNDS;

	// pass byte number through channel data
	SendBuffer[0] = 0x0B;
	SendBuffer[1] = MIDI_CONTROL_CHANGE + (byteNumber & 0x0F);
	SendBuffer[2] = DEVICE_ACK_REQUEST_SERIAL_NUMBER;

	/*
	SendBuffer[3] = (0x7F & (uint8_t)(g_SerialNumber[byteNumber]));
	DEBUG_LINEOUT("Serial: Sending byte %d val:0x%x", byteNumber, (0x7F & (uint8_t)(g_SerialNumber[byteNumber])));
	*/

	//SendBuffer[3] = (uint8_t)(g_UserSpace.serial[byteNumber]);
	SendBuffer[3] = serialByte;

#ifdef USB_VERBOSE
	DEBUG_LINEOUT("Serial: Sending byte %d val:0x%x", byteNumber, serialByte);
#endif

	return SendUSBBuffer(USB0_BASE, MIDI_OUT_EP, SendBuffer, 4);
}

// Storage for state machine
uint8_t g_StateMachineString = 0x00;
uint8_t g_StateMachineFret = 0x00;
uint8_t g_StateMachineRGBM = 0x00;
uint8_t g_StateMachineMRGB = 0x00;

uint8_t g_SetLEDCCFret = 0;

// piezo command
uint8_t g_piezoCmd = 0;
uint8_t g_piezoCmd_txBytes = 0;
uint8_t g_piezoCmdChecksum = 0;
uint8_t *g_pBuffer_PiezoCmd = NULL;
uint8_t g_piezoCmdPacketCount = 0;

RESULT ClearPiezoCmd() {
	g_piezoCmd = 0;
	g_piezoCmd_txBytes = 0;
	g_piezoCmdChecksum = 0;

	if(g_pBuffer_PiezoCmd != NULL) {
		free(g_pBuffer_PiezoCmd);
		g_pBuffer_PiezoCmd = NULL;
	}

	g_piezoCmdPacketCount = 0;

	return R_OK;
}

RESULT WrapMIDIBuffer(uint8_t **pBuffer, uint8_t *pBuffer_np) {
	RESULT r = R_OK;

	// All data bytes must be converted to midi data bytes which have a zero for the MSB
	int32_t  j = 0;
	uint8_t pBuffer_n = *pBuffer_np;
	uint8_t tempBuffer_n = ((pBuffer_n * 8 ) / 7) + ((pBuffer_n * 8 ) % 7 != 0);
	uint8_t *tempBuffer = malloc(tempBuffer_n * sizeof(uint8_t));
	CNR(tempBuffer);

	int8_t startCounter = 1;
	int8_t endCounter = 6;

	int32_t  i;
	for (i = 0; i < pBuffer_n; i++ ) {
		// add current fragment and begining of next
		tempBuffer[j] += ((*pBuffer)[i] >> startCounter) & 0x7F;
		tempBuffer[j + 1] = (0x7F & ((*pBuffer)[i] << endCounter));

		// var upkeep
		j += 1;
		startCounter += 1;
		endCounter -= 1;

		// boundary check
		if ( startCounter == 8 && endCounter < 0 ) {
			j++;
			startCounter = 1;
			endCounter = 6;
		}

		//CBRM_NA(j < tempBuffer_n, "WrapMIDIBuffer: wrapped data counter mismatch with calc'd value");
	}

	j++;
	//CBRM_NA(j == tempBuffer_n, "WrapMIDIBuffer: wrapped data counter mismatch with calc'd value");

	free(*pBuffer);
	*pBuffer = tempBuffer;
	*pBuffer_np = tempBuffer_n;

Error:
	if(r != R_OK)
		if(tempBuffer != NULL)
			free(tempBuffer);

	return r;
}

RESULT UnwrapMIDIBuffer(uint8_t *pBuffer, uint8_t *pBuffer_np) {
	RESULT r = R_OK;

	// need to unwrap the data since it was converted to midi data packets
	int32_t  pBuffer_n = *pBuffer_np;
	int32_t  pBuffer_nc = 0;

	int8_t j = 6, k = 1;
	int32_t  i = 0;

	// unwrap data
	for(i = 0; i < pBuffer_n; i++) {
		pBuffer[pBuffer_nc] = (pBuffer[i] << k) + (pBuffer[i + 1] >> j);

		k += 1;
		j -= 1;

		if( i != pBuffer_n - 1)
			pBuffer_nc++;

		if(k == 8 || j < 0) {
			j = 6;
			k = 1;
			i += 1;
		}
	}

	*pBuffer_np = pBuffer_nc;

Error:
	return r;
}

// firmware download
int32_t  g_totFWBytes = 0;
uint8_t g_totPages = 0;
uint8_t g_curPage = 0;

uint8_t *g_pBuffer = NULL;
int32_t  g_pBuffer_n = 0;
int32_t  g_pBuffer_c = 0;
uint8_t g_checkSum = 0;

RESULT DeployFirmwarePage() {
	RESULT r = R_OK;

	DEBUG_LINEOUT("DeployFirmwarePage: Deploying page %d of %d pages, %d bytes, checksum: 0x%x", g_curPage, g_totPages, g_pBuffer_n, g_checkSum);
	DEBUG_LINEOUT("*** transmitted buffer:%d bytes ***", g_pBuffer_n);
	//UARTprintfBinaryData(g_pBuffer, g_pBuffer_n, 20);

	DEBUG_LINEOUT_NA("*** rx buffer ***");
	// need to unwrap the data since it was converted to midi data packets
	int32_t  g_pBuffer_nc = 0;

	int8_t j = 6, k = 1;
	int32_t  i = 0;

	// unwrap data
	for(i = 0; i < g_pBuffer_n; i++) {
		g_pBuffer[g_pBuffer_nc] = (g_pBuffer[i] << k) + (g_pBuffer[i + 1] >> j);

		k += 1;
		j -= 1;

		if( i != g_pBuffer_n - 1)
			g_pBuffer_nc++;

		if(k == 8 || j < 0) {
			j = 6;
			k = 1;
			i += 1;
		}
	}

	//UARTprintfBinaryData(g_pBuffer, g_pBuffer_nc, 20);

#ifdef USB_VERBOSE
	DEBUG_LINEOUT("*** decoded transmitted buffer:%d bytes ***", g_pBuffer_nc);
#endif

	// check the checksum
	uint8_t tempChecksum = 0x00;

	for(i = 0; i < g_pBuffer_nc; i++)
		tempChecksum += g_pBuffer[i];

	tempChecksum = (tempChecksum >> 1) & 0x7F;

	CBRM((g_checkSum == tempChecksum), "DeployFirmwarePage: checksum mismatch 0x%x should be 0x%x", tempChecksum, g_checkSum);

	CBRM((g_DownloadedFirmwarePages == g_curPage),
		 "DeployFirmwarePage: Downloaded Pages %d mismatches with downloaded page %d!  Must download pages in order",
		 g_DownloadedFirmwarePages, g_curPage);

	uint32_t  tempAddr = (g_FirmwareDownloadAddress + g_DownloadedFirmwareBytes);
	DEBUG_LINEOUT("DeployFirmwarePage: Writing page %d with %d bytes to 0x%x address of memory", g_curPage, g_pBuffer_nc, tempAddr);

	if(g_curPage < g_totPages - 1)
		CBRM((g_pBuffer_nc == 1024), "DeployFirmwarePage: Failed to deploy firmware page %d as only 1024 bytes are acceptable page sizes got: %d", g_curPage, g_pBuffer_nc);

	// First erase that KB of memory
	uint32_t  ulRes = FlashErase((void*)(tempAddr));

	SysCtlDelay(ROM_SysCtlClockGet() / 100);

	int32_t  tempBufferCount = g_pBuffer_nc + (g_pBuffer_nc % 4);
	DEBUG_LINEOUT("DeployFirmware: writing %d bytes, actually %d bytes (divide by 4 diff: %d)", g_pBuffer_nc, tempBufferCount, (g_pBuffer_nc % 4));

	// Might copy over up to 3 bytes of garbage but should only happen on last transfer
	// up to client to ensure that packets are divisible by 4
	ulRes = FlashProgram((uint32_t  *)(g_pBuffer), (void*)(tempAddr), tempBufferCount);
	g_DownloadedFirmwareBytes += g_pBuffer_nc;
	g_DownloadedFirmwarePages += 1;

	SysCtlDelay(ROM_SysCtlClockGet() / 100);

	// Jump to the new firmware bundle
	if(g_curPage == g_totPages - 1) {
		//PC = 0x10000;

		/* Fix with the Piezo revive
		if(g_DownloadedFirmwareIsPiezo) {
			g_DownloadedPiezoFirmware_size = g_totFWBytes;
			PiezoFWUpdate(g_DownloadedPiezoFirmware_size);
		}
		else {
			g_DownloadedFirmwarePages = g_totPages;
			SetUSFwUpdateStatus(FW_UPDATE_PENDING);

			DEBUG_LINEOUT_NA("Resetting the device");
			SysCtlDelay(ROM_SysCtlClockGet() / 10);
			ROM_SysCtlReset();
		}
		*/

		g_DownloadedFirmwarePages = g_totPages;
		SetUSFwUpdateStatus(FW_UPDATE_PENDING);

		DEBUG_LINEOUT_NA("Resetting the device");
		SysCtlDelay(ROM_SysCtlClockGet() / 10);
		ROM_SysCtlReset();
	}


Error:
	if(g_pBuffer != NULL) {
		free(g_pBuffer);
		g_pBuffer = NULL;
		g_pBuffer_n = 0;
		g_pBuffer_c = 0;
		g_checkSum = 0;
	}

	DEVICE_MIDI_EVENT gme;

	/* TODO: Fix with the piezo revive
	if(g_DownloadedFirmwareIsPiezo)
		gme.m_gmet = DEVICE_SEND_PIEZO_FW_ACK;
	else
		gme.m_gmet = DEVICE_SEND_FW_ACK;
	*/
	gme.m_gmet = DEVICE_SEND_FW_ACK;

	gme.m_params_n = 1;
	gme.m_params[0] = (r == R_OK) ? 0x00 : 0x01;
	QueueNewMidiEvent(gme);

	return r;
}

uint8_t g_SerialNumberCounter = 0;
uint8_t g_TempSerial[20];

// TODO: Redesign this shit
// Split up into a bunch of MIDI messages and then split this up into the
// MIDI controller
RESULT HandleUSBMIDIPacket(uint8_t *pBuffer, uint16_t pBuffer_n) {
	RESULT r = R_OK;
	int32_t i = 0;

	CNRM_NA(pBuffer, "HandleMidiPacket: buffer is null");
	CBRM((pBuffer >= 2), "HandleMidiPacket: buffer must be at least 2 bytes, length: %d", pBuffer_n);

	// Print out packet for debug
	/*
	DEBUG_LINEOUT("*** rx pkt: %d bytes ***", pBuffer_n);
	UARTprintfBinaryData(pBuffer, pBuffer_n, 20);
	//*/

	uint8_t pBuffer_c = 0;
	while(pBuffer_c < pBuffer_n) {
		// Figure out what kind of packet this is
		uint8_t CodeIndexNumber = (pBuffer[pBuffer_c + 0] & 0xF);		// TODO: forego this for now?

		MIDI_TYPE type;
		memcpy(&type, &pBuffer[pBuffer_c + 1], sizeof(MIDI_TYPE));
		uint8_t data1 = (uint8_t)(pBuffer[pBuffer_c + 2]);
		uint8_t data2 = (uint8_t)(pBuffer[pBuffer_c + 3]);

		MIDI_MSG midiPacket = {
			.type = type,
			.data1 = data1,
			.data2 = data2
		};

		CRM_NA(HandleMIDIPacket(midiPacket), "Failed to handle MIDI Message");

		pBuffer_c += 4;
	}

Error:
	return r;
}

/*
// TODO: Actually remove this code
RESULT HandleMidiPacket_old(uint8_t *pBuffer, uint16_t pBuffer_n) {
	RESULT r = R_OK;
	 int32_t  i = 0;
	
	CNRM_NA(pBuffer, "HandleMidiPacket: buffer is null");
	CBRM((pBuffer >= 2), "HandleMidiPacket: buffer must be at least 2 bytes, length: %d", pBuffer_n);
	
	// Print out packet for debug
	/*
	DEBUG_LINEOUT("*** rx pkt: %d bytes ***", pBuffer_n);
	UARTprintfBinaryData(pBuffer, pBuffer_n, 20);
	//
	
	uint8_t pBuffer_c = 0;
	
	while(pBuffer_c < pBuffer_n) {

		// Figure out what kind of packet this is
		uint8_t CodeIndexNumber = (pBuffer[pBuffer_c + 0] & 0xF);
		
		switch(g_USBMIDIPacketMachineState) {
			case UMPMS_INITIAL: {
				switch(CodeIndexNumber) {
					case USB_MIDI_NOTE_ON: {
#ifdef USB_VERBOSE
						DEBUG_LINEOUT("Rx unhandled note on event: 0x%x 0x%x", pBuffer[pBuffer_c + 2], pBuffer[pBuffer_c + 3]);
#endif
						MIDI_MSG midiPacket = {
							.type = MIDI_NOTE_ON,
							.data1 = pBuffer[pBuffer_c + 2],
							.data2 = pBuffer[pBuffer_c + 3]
						};

						CRM_NA(HandleMIDIPacket(midiPacket), "Failed to handle MIDI Message");

						g_USBMIDIPacketMachineState = UMPMS_INITIAL;
					} break;

					case USB_MIDI_NOTE_OFF:	{
#ifdef USB_VERBOSE
						DEBUG_LINEOUT("Rx unhandled note off event: 0x%x 0x%x", pBuffer[pBuffer_c + 2], pBuffer[pBuffer_c + 3]);
#endif

						MIDI_MSG midiPacket = {
							.type = MIDI_NOTE_OFF,
							.data1 = pBuffer[pBuffer_c + 2],
							.data2 = pBuffer[pBuffer_c + 3]
						};

						CRM_NA(HandleMIDIPacket(midiPacket), "Failed to handle MIDI Message");

						g_USBMIDIPacketMachineState = UMPMS_INITIAL;
					} break;

					case USB_MIDI_CONTROL_CHANGE: {
						if(pBuffer[pBuffer_c + 2] == 0x33 && g_USBMIDICCSetLEDMachineState == UMCCSLMS_INITIAL ) {
								g_SetLEDCCFret = pBuffer[pBuffer_c + 3];
								g_USBMIDICCSetLEDMachineState = UMCCSLMS_SET_LED_0;
						}
						else if(pBuffer[pBuffer_c + 2] == 0x34 && g_USBMIDICCSetLEDMachineState == UMCCSLMS_SET_LED_0) {
								uint8_t SetLEDCCStrNum = pBuffer[pBuffer_c + 1] & 0xF;
								uint8_t SetLEDCCRGBM = pBuffer[pBuffer_c + 3];

								SendStringFretLEDStateRGBM(SetLEDCCStrNum, g_SetLEDCCFret, SetLEDCCRGBM);
								g_USBMIDICCSetLEDMachineState = UMCCSLMS_INITIAL;
						}
						else {
#ifdef USB_VERBOSE
							DEBUG_LINEOUT("Rx unhandled control change event: 0x%x 0x%x", pBuffer[pBuffer_c + 2], pBuffer[pBuffer_c + 3]);
#endif
							g_SetLEDCCFret = 0;
							g_USBMIDICCSetLEDMachineState = UMCCSLMS_INITIAL;
						}

						// Control changes don't affect the overall SM
						g_USBMIDIPacketMachineState = UMPMS_INITIAL;
					} break;

					case USB_MIDI_SYS_EX: {
						// Handle First Packet
						if(pBuffer[pBuffer_c + 2] != GTAR_DEVICE_ID) {
							DEBUG_LINEOUT_NA("err: HandleMidiPacket: gTar device id incorrect");
						}
						else {
							uint8_t msgType = pBuffer[pBuffer_c + 3];
							switch(msgType) {
								case GTAR_MSG_SET_LED: 	g_USBMIDIPacketMachineState = UMPMS_SET_LED_0;
														break;

								case GTAR_MSG_SET_LED_EX: 	g_USBMIDIPacketMachineState = UMPMS_SET_LED_EX_0;
															break;

								case GTAR_MSG_SET_NOTE_ACTIVE: 	g_USBMIDIPacketMachineState = UMPMS_SET_NA_0;
																break;

								case GTAR_MSG_SET_FRET_FOLLOW: 	g_USBMIDIPacketMachineState = UMPMS_SET_FW_0;
																break;

								case GTAR_MSG_REQ_AUTH_REDOWNLOAD:	{
									g_USBMIDIPacketMachineState = UMPMS_INITIAL;

#ifdef IPHONE_IAP
									// Erase and re-download cert
									EraseAuthCert();
									DownloadAuthCertificate();
#else
									DEBUG_LINEOUT_NA("Auth redownload not supported");
#endif
								} break;

								case GTAR_MSG_REQ_FW_VERSION: {
									g_USBMIDIPacketMachineState = UMPMS_INITIAL;	// one message operation
									// Send firmware version
									GTAR_MIDI_EVENT gme;
									gme.m_gmet = GTAR_SEND_FW_VERSION;
									gme.m_params_n = 0;
									QueueNewMidiEvent(gme);
								} break;


								case GTAR_MSG_GET_PIEZO_SENSITIVITY: {
									g_USBMIDIPacketMachineState = UMPMS_GET_PIEZO_SENSITIVITY_0;	// one message operation
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("GetPiezoSensitivity");
#endif
								} break;

								case GTAR_MSG_GET_PIEZO_WINDOW: {
									g_USBMIDIPacketMachineState = UMPMS_GET_PIEZO_WINDOW_0;
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("GetPiezoWindow");
#endif
									} break;

								case GTAR_MSG_GET_PIEZO_CT_MATRIX: {
									g_USBMIDIPacketMachineState = UMPMS_GET_PIEZO_CT_MATRIX_0;
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("GetPiezoCrossTalkMatric");
#endif
									} break;

								case GTAR_MSG_SET_PIEZO_CT_MATRIX: {
									// Set piezo sensor cross talk matrix
									g_USBMIDIPacketMachineState = UMPMS_SET_PIEZO_CT_MATRIX_0;
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("SetPiezoCrossTalkMatrix");
#endif
									} break;

								case GTAR_MSG_SET_PIEZO_SENSITIVITY: {
									// Set piezo sensor sensitivity
									g_USBMIDIPacketMachineState = UMPMS_SET_PIEZO_SENSITIVITY_0;
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("SetPiezoSensorSensitivity");
#endif
									} break;

								case GTAR_MSG_CALIBRATE_PIEZO_STRING: {
									// Calibrate string on piezo
									g_USBMIDIPacketMachineState = UMPMS_CALIBRATE_PIEZO_STRING_0;
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("CalibratePiezoString");
#endif
									} break;

								case GTAR_MSG_SET_PIEZO_WINDOW: {
									// Set piezo sensor window
									g_USBMIDIPacketMachineState = UMPMS_SET_PIEZO_WINDOW_0;
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("SetPiezoSensorWindow");
#endif
								} break;

								case GTAR_MSG_REQ_BATTERY_STATUS: {
									// Request battery status
									g_USBMIDIPacketMachineState = UMPMS_INITIAL;	// one message operation

									// Queue up send battery status and send battery charge
									GTAR_MIDI_EVENT gme_status;
									gme_status.m_gmet = GTAR_SEND_BATTERY_STATUS;
									gme_status.m_params_n = 0;
									QueueNewMidiEvent(gme_status);

									GTAR_MIDI_EVENT gme_charge;
									gme_charge.m_gmet = GTAR_SEND_BATTERY_CHARGE;
									gme_charge.m_params_n = 0;
									QueueNewMidiEvent(gme_charge);
								} break;

								case GTAR_MSG_REQ_SERIAL_NUM: {
									g_USBMIDIPacketMachineState = UMPMS_REQ_SERIAL_NUM_0;
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("ReqSerialNumber");
#endif
								} break;

								case GTAR_MSG_ENABLE_VELOCITY: {
									// Enable velocity sensing
									g_USBMIDIPacketMachineState = UMPMS_INITIAL;	// one message operation
									g_gtar.m_fVelocityEnabled = 1;

								} break;

								case GTAR_MSG_DISABLE_VELOCITY: {
									// Disable velocity sensing
									g_USBMIDIPacketMachineState = UMPMS_INITIAL;	// one message operation
									g_gtar.m_fVelocityEnabled = 0;

								} break;

								case GTAR_MSG_DOWNLOAD_NEW_PIEZO_FW_PACKAGE: {
									// Download firmware version
									g_DownloadedFirmwareIsPiezo = 1;
									g_USBMIDIPacketMachineState = UMPMS_DWLD_FW_0;
									DEBUG_LINEOUT("HandleMidiPacket: Piezo firmware package c:%d", pBuffer_c);
								} break;

								case GTAR_MSG_DOWNLOAD_NEW_FW_PACKAGE: {
									// Download firmware version
									g_USBMIDIPacketMachineState = UMPMS_DWLD_FW_0;
									DEBUG_LINEOUT("HandleMidiPacket: Firmware package c:%d", pBuffer_c);
								} break;

								case GTAR_MSG_SET_PIEZO_STATE: {
									// Set piezo sensor state
									g_USBMIDIPacketMachineState = UMPMS_SET_PIEZO_STATE_0;	// set the piezo sensor state
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("SetPiezoSensorState");
#endif
									} break;

								case GTAR_MSG_SET_PIEZO_THRESH: {
									// Set piezo sensor state
									g_USBMIDIPacketMachineState = UMPMS_SET_PIEZO_THRESH_0;	// set the piezo sensor state
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("SetPiezoSensorThreshold");
#endif
									} break;

								case GTAR_MSG_PIEZO_CMD: {
									// Set piezo sensor state
									g_USBMIDIPacketMachineState = UMPMS_PIEZO_CMD;	// set the piezo sensor state
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("PiezoCommand");
#endif
									} break;

								case GTAR_MSG_SET_SERIAL_NUMBER: {
									// Set Serial Number
									if(IsSerialNumberZero() == R_OK) {
#ifdef USB_VERBOSE
										DEBUG_LINEOUT_NA("SetSerialNumber");
#endif
										g_SerialNumberCounter = 0;
										memset(g_TempSerial, 0, sizeof(g_TempSerial));
										g_USBMIDIPacketMachineState = UMPMS_SET_SERIAL_NUMBER_0;
									}
									else {
#ifdef USB_VERBOSE
										DEBUG_LINEOUT_NA("Cannot set serial number, already set");
#endif
										g_USBMIDIPacketMachineState = UMPMS_INITIAL;
									}

								} break;

								case GTAR_MSG_SET_SMART_PICK_THRESH: {
									// Set smart pick threshold
									g_USBMIDIPacketMachineState = UMPMS_SET_SMART_PICK_THRESH_0;	// set the piezo sensor state
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("SetSmartPickThreshold");
#endif
									} break;

								case GTAR_MSG_SET_FRETBOARD_THRESH: {
									// Set fret board threshold
									g_USBMIDIPacketMachineState = UMPMS_SET_FRETBOARD_THRESH_0;	// set the fretboard threshold
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("SetFretboardThreshold");
#endif
									} break;

								case GTAR_MSG_SET_ACCELEROMETER_STATE: {
									// Set fret board threshold
									g_USBMIDIPacketMachineState = UMPMS_SET_ACCELEROMETER_STATE_0;	// set the accel state
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("SetAccelerometerState");
#endif
									} break;

								case GTAR_MSG_COMMIT_USERSPACE: {
									// Commit UserSpace
									g_USBMIDIPacketMachineState = UMPMS_INITIAL;	// one message operation

									CommitUserSpace();
									PrintPiezoSettingsUserspace();

									GTAR_MIDI_EVENT gme;
									gme.m_gmet = GTAR_SEND_COMMIT_USERSPACE;
									gme.m_params_n = 1;
									gme.m_params[0] = 0;
									QueueNewMidiEvent(gme);

#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("CommitUserSpace");
#endif
									} break;

								case GTAR_MSG_RESET_USERSPACE: {
									// Reset UserSpace
									g_USBMIDIPacketMachineState = UMPMS_INITIAL;	// one message operation

									EraseUserSpace();
									InitUserSpace();

									GTAR_MIDI_EVENT gme;
									gme.m_gmet = GTAR_SEND_RESET_USERSPACE;
									gme.m_params_n = 1;
									gme.m_params[0] = 0;
									QueueNewMidiEvent(gme);

#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("ResetUserSpace");
#endif
									} break;

								case GTAR_MSG_ENABLE_DEBUG: {
									// Enable Debug Mode
									g_USBMIDIPacketMachineState = UMPMS_INITIAL;	// one message operation
									InitJTAG();		// initializes JTAG debug mode
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("EnableDebugMode");
#endif
									} break;

								case GTAR_MSG_DISABLE_DEBUG: {
									// Disable Debug Mode
									g_USBMIDIPacketMachineState = UMPMS_INITIAL;	// one message operation
									InitJTAGStrings();		// sets port C as GPIO
#ifdef USB_VERBOSE
									DEBUG_LINEOUT_NA("DisableDebugMode");
#endif
									} break;

								default: {
									DEBUG_LINEOUT("Rx unhandled msg type 0x%x", msgType);
									g_USBMIDIPacketMachineState = UMPMS_INITIAL;
								} break;
							}
						}
					} break;

					default: {
						DEBUG_LINEOUT("Rx unhandled msg 0x%x", CodeIndexNumber);
						g_USBMIDIPacketMachineState = UMPMS_INITIAL;
					} break;
				}
			} break;

			// SET LED
			case UMPMS_SET_LED_0: {
				if(CodeIndexNumber != 0x04) {
#ifdef USB_VERBOSE
					DEBUG_LINEOUT_NA("err: HandleMidiPacket: SET_LED_0 expected 0x04 packet");
#endif
					g_USBMIDIPacketMachineState = UMPMS_INITIAL;
				}
				else {
					g_StateMachineString = pBuffer[pBuffer_c + 1];
					g_StateMachineFret = pBuffer[pBuffer_c + 2];
					g_StateMachineRGBM = pBuffer[pBuffer_c + 3];
					g_USBMIDIPacketMachineState = UMPMS_SET_LED_1;
				}
			} break;

			case UMPMS_SET_LED_1: {
				if(CodeIndexNumber != 0x05 || pBuffer[pBuffer_c + 1] != 0xF7 ) {
#ifdef USB_VERBOSE
					DEBUG_LINEOUT_NA("err: HandleMidiPacket: SET_LED_1 expected 0x05 packet with second byte of 0xF7");
#endif
				}
				else {
					//DEBUG_LINEOUT("LED state: str:%d fret: %d RGBM:0x%x", g_StateMachineString, g_StateMachineFret, g_StateMachineRGBM);
					SendStringFretLEDStateRGBM(g_StateMachineString, g_StateMachineFret, g_StateMachineRGBM);
				}

				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			// SET LED EX
			case UMPMS_SET_LED_EX_0: {
				if(CodeIndexNumber != 0x04) {
					DEBUG_LINEOUT_NA("err: HandleMidiPacket: SET_LED_EX_0 expected 0x04 packet");
					g_USBMIDIPacketMachineState = UMPMS_INITIAL;
				}
				else {
					g_StateMachineString = pBuffer[pBuffer_c + 1];
					g_StateMachineFret = pBuffer[pBuffer_c + 2];
					g_StateMachineMRGB = pBuffer[pBuffer_c + 3];
					g_USBMIDIPacketMachineState = UMPMS_SET_LED_EX_1;
				}
			} break;

			case UMPMS_SET_LED_EX_1: {
				if(CodeIndexNumber != 0x05 || pBuffer[pBuffer_c + 1] != 0xF7 ) {
					DEBUG_LINEOUT_NA("err: HandleMidiPacket: SET_LED_EX_1 expected 0x05 packet with second byte of 0xF7");
				}
				else {
					//DEBUG_LINEOUT("LED Ex state: str:%d fret: %d MRGB:0x%x", g_StateMachineString, g_StateMachineFret, g_StateMachineMRGB);
					SendStringFretLEDStateMRGB(g_StateMachineString, g_StateMachineFret, g_StateMachineMRGB);
				}

				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			case UMPMS_SET_SERIAL_NUMBER_0: {
				for(i = 0; i < 3; i++) {
					g_TempSerial[g_SerialNumberCounter] =  pBuffer[pBuffer_c + i + 1];

					g_SerialNumberCounter++;
					if(g_SerialNumberCounter == 16) {
						// unwrap data
						int32_t  serial_nc = 0;

						int8_t j = 6, k = 1;
						i = 0;

						for(i = 0; i < g_SerialNumberCounter; i++) {
							g_TempSerial[serial_nc] = (g_TempSerial[i] << k) + (g_TempSerial[i + 1] >> j);

							k += 1;
							j -= 1;

							if( i != g_SerialNumberCounter - 1)
								serial_nc++;

							if(k == 8 || j < 0) {
								j = 6;
								k = 1;
								i += 1;
							}
						}

						// Copy over
						memset(g_UserSpace.serial, 0, sizeof(g_UserSpace.serial));

						for(i = 2; i < 16; i++)
							g_UserSpace.serial[i] = g_TempSerial[i - 2];

						// Commit serial number to flash
						OutputSerialToDebug();
						CommitUserSpace();

						g_SerialNumberCounter = 0;
						g_USBMIDIPacketMachineState = UMPMS_INITIAL;
						break;	// ! for(i)
					}
				}
			} break;

			// SET NOTE ACTIVE
			case UMPMS_SET_NA_0: {
				//if(CodeIndexNumber != 0x06 || pBuffer[pBuffer_c + 2] != 0xF7) {
				if(pBuffer[pBuffer_c + 2] != 0xF7) {
					DEBUG_LINEOUT_NA("err: HandleMidiPacket: SET_NA_0 expected 0x06 packet with third byte of 0xF7");
				}
				else {
					SetNoteActiveRGBM(pBuffer[pBuffer_c + 1]);
				}

				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			// SET FRET FOLLOW
			case UMPMS_SET_FW_0: {
				//if(CodeIndexNumber != 0x06 || pBuffer[pBuffer_c + 2] != 0xF7) {
				if(pBuffer[pBuffer_c + 2] != 0xF7) {
					DEBUG_LINEOUT_NA("err: HandleMidiPacket: SET_FW_0 expected third byte of 0xF7");
				}
				else {
					SetFretFollowRGBM(pBuffer[pBuffer_c + 1], 10);
				}

				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			case UMPMS_SET_PIEZO_STATE_0: {
				uint8_t piezoState = pBuffer[pBuffer_c + 1];
				SetPiezoState(piezoState);
				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			case UMPMS_REQ_SERIAL_NUM_0: {
				uint8_t ByteNumber = pBuffer[pBuffer_c + 1];

				// Request serial number
				GTAR_MIDI_EVENT gme_status;
				gme_status.m_gmet = GTAR_SEND_SERIAL_NUMBER;
				gme_status.m_params_n = 1;
				gme_status.m_params[0] = ByteNumber;
				QueueNewMidiEvent(gme_status);

				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			case UMPMS_SET_PIEZO_CT_MATRIX_0: {
				uint8_t row = (pBuffer[pBuffer_c + 1] >> 4) & 0x0F;
				uint8_t col = (pBuffer[pBuffer_c + 1]) & 0x0F;
				uint8_t val = pBuffer[pBuffer_c + 2];

				SetPiezoCrossTalkMatrix(row, col, val);

				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			case UMPMS_SET_PIEZO_SENSITIVITY_0: {
				uint8_t string = pBuffer[pBuffer_c + 1];
				uint8_t val = pBuffer[pBuffer_c + 2];

				SetPiezoSensitivity(string, val);
				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			case UMPMS_SET_PIEZO_WINDOW_0: {
				uint8_t index = pBuffer[pBuffer_c + 1];
				uint8_t val = pBuffer[pBuffer_c + 2];

				SetPiezoWindowValue(index, val);
				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			case UMPMS_CALIBRATE_PIEZO_STRING_0: {
				uint8_t string = pBuffer[pBuffer_c + 1];
				uint8_t peak = (pBuffer[pBuffer_c + 2] >> 4) & 0x0F;
				uint8_t propAdd = pBuffer[pBuffer_c + 2] & 0x0F;

				CalibrateStringAndDownloadToUserspace(string, peak, propAdd);

				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			case UMPMS_GET_PIEZO_CT_MATRIX_0: {
				// Send piezo CT matrix position
				GTAR_MIDI_EVENT gme;
				uint8_t row = (pBuffer[pBuffer_c + 1] >> 4) & 0x0F;
				uint8_t col = (pBuffer[pBuffer_c + 1]) & 0x0F;

				gme.m_gmet = GTAR_SEND_PIEZO_CT_MATRIX;
				gme.m_params[0] = row;
				gme.m_params[1] = col;
				gme.m_params_n = 2;
				QueueNewMidiEvent(gme);

				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			case UMPMS_GET_PIEZO_SENSITIVITY_0: {
				// Get piezo sensitivity
				GTAR_MIDI_EVENT gme;
				gme.m_gmet = GTAR_SEND_PIEZO_SENSITIVITY;
				gme.m_params_n = 1;
				gme.m_params[0] = pBuffer[pBuffer_c + 1];
				QueueNewMidiEvent(gme);

				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			case UMPMS_GET_PIEZO_WINDOW_0: {
				// Get piezo window values
				GTAR_MIDI_EVENT gme;
				gme.m_gmet = GTAR_SEND_PIEZO_WINDOW;
				gme.m_params_n = 1;
				gme.m_params[0] = pBuffer[pBuffer_c + 1];
				QueueNewMidiEvent(gme);

				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			case UMPMS_SET_PIEZO_THRESH_0: {
				uint8_t piezoThresh = pBuffer[pBuffer_c + 1];
				SetPiezoThreshold(piezoThresh);
				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			case UMPMS_PIEZO_CMD: {
				// Figure out what kind of packet this is
				uint8_t CodeIndexNumber = (pBuffer[pBuffer_c + 0] & 0xF);
				if(g_piezoCmd) {
					switch(CodeIndexNumber) {
						case 0x4: {
							if(g_piezoCmdPacketCount * 3 > g_piezoCmd_txBytes) {
								DEBUG_LINEOUT("err: HandleMidiPacket: UMPMS_PIEZO_CMD data exceeded buffer length: 0x%x", g_piezoCmdPacketCount * 3);
								ClearPiezoCmd();
								g_USBMIDIPacketMachineState = UMPMS_INITIAL;
							}
							else {
								memcpy(g_pBuffer_PiezoCmd + (g_piezoCmdPacketCount * 3), pBuffer + (pBuffer_c + 1), 3);
								g_piezoCmdPacketCount++;
							}
						} break;

						case 0x5:
						case 0x6:
						case 0x7: {
							uint8_t remainderCount = CodeIndexNumber - 0x5;
							uint8_t byteCount = g_piezoCmdPacketCount * 3 + remainderCount;

							if(byteCount > g_piezoCmd_txBytes) {
								DEBUG_LINEOUT("err: HandleMidiPacket: UMPMS_PIEZO_CMD data exceeded buffer length: 0x%x", byteCount);
								ClearPiezoCmd();
								g_USBMIDIPacketMachineState = UMPMS_INITIAL;
							}
							else {
								memcpy(g_pBuffer_PiezoCmd + (g_piezoCmdPacketCount * 3), pBuffer + (pBuffer_c + 1), remainderCount);

								UnwrapMIDIBuffer(g_pBuffer_PiezoCmd, &g_piezoCmd_txBytes);

								//TODO: add checksum confirmation here

								uint8_t *p_ucBuffer = malloc(PIEZO_BUFFER_SIZE);
								 int32_t  p_ucBuffer_n = PIEZO_BUFFER_SIZE;

								RESULT result = PiezoInterfaceCommandEx(g_piezoCmd,	g_pBuffer_PiezoCmd,	g_piezoCmd_txBytes,	p_ucBuffer, &p_ucBuffer_n);

								//here we do a double check if the command was successful, but only if no data was returned and commandEx didn't error out
								if(result == R_OK && p_ucBuffer_n == 0) {
									uint8_t CmdArgs[] = {PIEZO_MODULE_BASE, PIEZO_CMD_STATUS_ADDR};
									uint8_t cmdStatus = 0;
									 int32_t  temp_n = 1;
									result = PiezoInterfaceCommandEx(PIEZO_GET_VALUE_CHAR, CmdArgs, 2, &cmdStatus, &temp_n);

									if(result == R_OK && cmdStatus != 1) {
										result = R_ERROR;
										 DEBUG_LINEOUT_NA("err: HandleMidiPacket: UMPMS_PIEZO_CMD command not successful");
									}
								}

								GTAR_MIDI_EVENT gme;
								gme.m_gmet = GTAR_SEND_PIEZO_CMD_ACK;
								gme.m_params_n = 1;
								gme.m_params[0] = (result != R_OK);
								QueueNewMidiEvent(gme);

								if(p_ucBuffer_n && result == R_OK) {
									//calc checksum of unwrapped buffer
									uint8_t checksum = 0;

									// Sum all the bytes / checksum
									int32_t  i;
									for ( i = 0; i < p_ucBuffer_n; i++ )
										checksum += p_ucBuffer[i];
									checksum = (checksum >> 1) & 0x7F;

									result = WrapMIDIBuffer(&p_ucBuffer, &p_ucBuffer_n);

									gme.m_gmet = GTAR_SEND_PIEZO_CMD_RESPONSE;
									gme.m_params_n = 4;

									gme.m_params[0] = 0x04;
									gme.m_params[1] = 0xF0;
									gme.m_params[2] = GTAR_ACK_PIEZO_CMD_RESPONSE;
									gme.m_params[3] = p_ucBuffer_n;

									QueueNewMidiEvent(gme);

									int32_t  packet_c = 0;
									while(p_ucBuffer_n - packet_c*3 >= 3) {
										gme.m_params[0] = 0x4;
										gme.m_params[1] = p_ucBuffer[packet_c*3];
										gme.m_params[2] = p_ucBuffer[packet_c*3 + 1];
										gme.m_params[3] = p_ucBuffer[packet_c*3 + 2];

										QueueNewMidiEvent(gme);
										packet_c++;
									}

									int32_t  lastBytes = p_ucBuffer_n - packet_c*3;
									switch (lastBytes) {
										case 0x0: {
											gme.m_params[0] = 0x6;
											gme.m_params[1] = checksum;
											gme.m_params[2] = 0xF7;
											gme.m_params[3] = 0;

											QueueNewMidiEvent(gme);
										} break;

										case 0x1: {
											gme.m_params[0] = 0x7;
											gme.m_params[1] = p_ucBuffer[packet_c*3];
											gme.m_params[2] = checksum;
											gme.m_params[3] = 0xF7;

											QueueNewMidiEvent(gme);
										} break;

										case 0x2: {
											gme.m_params[0] = 0x4;
											gme.m_params[1] = p_ucBuffer[packet_c*3];
											gme.m_params[2] = p_ucBuffer[packet_c*3 + 1];
											gme.m_params[3] = checksum;

											QueueNewMidiEvent(gme);

											gme.m_params[0] = 0x5;
											gme.m_params[1] = 0xF7;
											gme.m_params[2] = 0;
											gme.m_params[3] = 0;

											QueueNewMidiEvent(gme);
										} break;

										default: {
											r = R_ERROR;
										} break;
									}

									free(p_ucBuffer);
								}

								if(p_ucBuffer != NULL) free(p_ucBuffer);
								ClearPiezoCmd();
								g_USBMIDIPacketMachineState = UMPMS_INITIAL;
							}
						} break;

						default: {
							DEBUG_LINEOUT("err: HandleMidiPacket: UMPMS_PIEZO_CMD expected SysEx CodeIndex, and got: 0x%x", CodeIndexNumber);
							ClearPiezoCmd();
							g_USBMIDIPacketMachineState = UMPMS_INITIAL;
						}
					}
				}
				else if(CodeIndexNumber == 0x4) {
					g_piezoCmd = pBuffer[pBuffer_c + 1];
					g_piezoCmd_txBytes = pBuffer[pBuffer_c + 2];
					g_piezoCmdChecksum = pBuffer[pBuffer_c + 3];

					if(g_piezoCmd && g_piezoCmd_txBytes) {
						g_pBuffer_PiezoCmd = malloc(g_piezoCmd_txBytes*sizeof(uint8_t));

						if(g_pBuffer_PiezoCmd == NULL) {
							DEBUG_LINEOUT_NA("err: HandleMidiPacket: UMPMS_PIEZO_CMD couldn't allocate unwrapping buffers");
							ClearPiezoCmd();
							g_USBMIDIPacketMachineState = UMPMS_INITIAL;
						}
					}
				}
				else {
					DEBUG_LINEOUT_NA("err: HandleMidiPacket: UMPMS_PIEZO_CMD command invalid value: 0.");
					g_USBMIDIPacketMachineState = UMPMS_INITIAL;
				}

			} break;

			case UMPMS_SET_SMART_PICK_THRESH_0: {
				uint8_t spThresh = pBuffer[pBuffer_c + 1];
				SetPickThreshold(spThresh);
				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			case UMPMS_SET_FRETBOARD_THRESH_0: {
				uint8_t uFretBoardThresh = pBuffer[pBuffer_c + 1];
				uint8_t lFretBoardThresh = pBuffer[pBuffer_c + 2];
				uint32_t  thresh = ((uFretBoardThresh & 0x7F) << 7) + (lFretBoardThresh & 0x7F);
				SetFretBoardThreshold(thresh);
				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			case UMPMS_SET_ACCELEROMETER_STATE_0: {
				uint8_t state = pBuffer[pBuffer_c + 1];
				uint8_t speed = pBuffer[pBuffer_c + 2];
				//SetAccelerometerState(state, speed);		// deprecated
				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
			} break;

			// Download Firmware Package
			case UMPMS_DWLD_FW_0: {
				 int32_t  totFWBytes = (pBuffer[pBuffer_c + 1] << 16) + (pBuffer[pBuffer_c + 2] << 8) + pBuffer[pBuffer_c + 3];

				if(g_totFWBytes == 0) {
					// Getting a new firmware package
					DEBUG_LINEOUT("HandleMidiPacket: SET_DWLD_FW_0 fw size: %d", totFWBytes);
					g_totFWBytes = totFWBytes;
					EraseFirmwareUpdateEraseFlashArea();		// erase the region first since flash doesn't do double writes
				}
				else if(g_totFWBytes != totFWBytes) {
					DEBUG_LINEOUT("err: HandleMidiPacket: SET_DWLD_FW_0 fw size mismatch: %d should be %d", totFWBytes, g_totFWBytes);
					DEBUG_LINEOUT("err: c:%d 0x%x 0x%x 0x%x", pBuffer_c, pBuffer[pBuffer_c + 1], pBuffer[pBuffer_c + 2], pBuffer[pBuffer_c + 3]);
					g_USBMIDIPacketMachineState = UMPMS_INITIAL;
					if(g_DownloadedFirmwareIsPiezo)
						g_DownloadedFirmwareIsPiezo = 0;
				}

				// next state
				g_USBMIDIPacketMachineState = UMPMS_DWLD_FW_1;
			} break;

			case UMPMS_DWLD_FW_1: {
				g_totPages = pBuffer[pBuffer_c + 1];
				g_curPage = pBuffer[pBuffer_c + 2];

				DEBUG_LINEOUT("HandleMidiPacket: DWLD_FW_1: totPages: %d curPage: %d", g_totPages, g_curPage);

				if(pBuffer[pBuffer_c + 3] != 0x00) {
					DEBUG_LINEOUT_NA("err: HandleMidiPacket: DWLD_FW_1 expected 0x00 as last packet");
					g_USBMIDIPacketMachineState = UMPMS_INITIAL;
					if(g_DownloadedFirmwareIsPiezo)
						g_DownloadedFirmwareIsPiezo = 0;

					break;
				}
				g_USBMIDIPacketMachineState = UMPMS_DWLD_FW_2;
			} break;

			case UMPMS_DWLD_FW_2: {
				short g_transmittedBytes = (pBuffer[pBuffer_c + 1] << 8) + pBuffer[pBuffer_c + 2];
				DEBUG_LINEOUT("HandleMidiPacket: DWLD_FW_2: transmittedBytes: %d", g_transmittedBytes);

				if(pBuffer[pBuffer_c + 3] != 0x00) {
					DEBUG_LINEOUT("err: HandleMidiPacket: DWLD_FW_2 expected 0x00 as last packet: 0x%x", pBuffer[pBuffer_c + 3]);
					g_USBMIDIPacketMachineState = UMPMS_INITIAL;
					if(g_DownloadedFirmwareIsPiezo)
						g_DownloadedFirmwareIsPiezo = 0;

					break;
				}

				g_pBuffer_n = (int)g_transmittedBytes;
				g_pBuffer_c = 0;
				g_pBuffer = (uint8_t *)malloc(g_pBuffer_n);
				CPRM(g_pBuffer, "HandleMidiPacket: DWLD_FW_2: Failed to allocate %d bytes", g_pBuffer_n);
				memset(g_pBuffer, 0, g_pBuffer_n);

				g_USBMIDIPacketMachineState = UMPMS_DWLD_FW_3;
			} break;

			case UMPMS_DWLD_FW_3: {
				 int32_t  i = 0;
				 int32_t  limit = 4;

				if(pBuffer[pBuffer_c] == 0x04)
					limit = 4;
				else if(pBuffer[pBuffer_c] == 0x05)
					limit = 2;
				else if(pBuffer[pBuffer_c] == 0x06)
					limit = 3;
				else if(pBuffer[pBuffer_c] == 0x07)
					limit = 4;
				else if(pBuffer[pBuffer_c] == 0x0f)
					limit = 2;

				for(i = 1; i < limit; i++) {
					g_pBuffer[g_pBuffer_c] = pBuffer[pBuffer_c + i];

					//DEBUG_LINEOUT("HandleMidiPacket: DWLD_FW_3: rx at position: %d byte: 0x%x in buf:0x%x %d", g_pBuffer_c, pBuffer[pBuffer_c + i], g_pBuffer[g_pBuffer_c], g_pBuffer_n);

					g_pBuffer_c++;
					if(g_pBuffer_c == g_pBuffer_n) {
						if(i == 1) {
							if(pBuffer[pBuffer_c] == 0x0f) {
								g_USBMIDIPacketMachineState = UMPMS_DWLD_FW_DONE_3;
								break;
							}

							// done with download (edge case)
							g_checkSum = pBuffer[pBuffer_c + 2];

							DEBUG_LINEOUT("HandleMidiPacket: DWLD_FW_3: received total of %d bytes, checksum: 0x%x", g_pBuffer_c, g_checkSum);

							if(pBuffer[pBuffer_c + 3] != 0xF7) {
								DEBUG_LINEOUT("err: HandleMidiPacket: DWLD_FW_3 expected 0xF7 as last byte of packet got: 0x%x", pBuffer[pBuffer_c + 3]);
								g_USBMIDIPacketMachineState = UMPMS_INITIAL;
								if(g_DownloadedFirmwareIsPiezo) g_DownloadedFirmwareIsPiezo = 0;
								break;
							}

							DeployFirmwarePage();
							g_USBMIDIPacketMachineState = UMPMS_INITIAL;
							if(g_DownloadedFirmwareIsPiezo) g_DownloadedFirmwareIsPiezo = 0;
							break;
						}
						else if(i == 2) {
							g_checkSum = pBuffer[pBuffer_c + 3];
							g_USBMIDIPacketMachineState = UMPMS_DWLD_FW_DONE_2;
							break;
						}
						else if(i == 3) {
							g_USBMIDIPacketMachineState = UMPMS_DWLD_FW_DONE_3;
							break;
						}
						else {
							DEBUG_LINEOUT_NA("err: HandleMidiPacket: DWLD_FW_2 too many bytes!");
							g_USBMIDIPacketMachineState = UMPMS_INITIAL;
							if(g_DownloadedFirmwareIsPiezo) g_DownloadedFirmwareIsPiezo = 0;
							break;
						}
					}
				}
			} break;

			case UMPMS_DWLD_FW_DONE_2: {
				DEBUG_LINEOUT("HandleMidiPacket: DWLD_FW_DONE_2: received total of %d bytes, checksum: 0x%x", g_pBuffer_c, g_checkSum);

				if(pBuffer[pBuffer_c + 1] != 0xF7) {
					DEBUG_LINEOUT("err: HandleMidiPacket: DWLD_FW_DONE_2 expected 0xF7 as second byte of packet, got: 0x%x", pBuffer[pBuffer_c + 1]);
					g_USBMIDIPacketMachineState = UMPMS_INITIAL;
					if(g_DownloadedFirmwareIsPiezo) g_DownloadedFirmwareIsPiezo = 0;
					break;
				}

				DeployFirmwarePage();
				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
				if(g_DownloadedFirmwareIsPiezo) g_DownloadedFirmwareIsPiezo = 0;
			} break;

			case UMPMS_DWLD_FW_DONE_3: {
				g_checkSum = pBuffer[pBuffer_c + 1];
				DEBUG_LINEOUT("HandleMidiPacket: DWLD_FW_DONE_3: received total of %d bytes, checksum: 0x%x", g_pBuffer_c, g_checkSum);

				if(g_checkSum == 0xF7) {
					// This is a weird edge case that will prevent the final F7 byte to be send so just deploy anyways!
					DeployFirmwarePage();
					g_USBMIDIPacketMachineState = UMPMS_INITIAL;
					if(g_DownloadedFirmwareIsPiezo) g_DownloadedFirmwareIsPiezo = 0;
					break;
				}

				if(pBuffer[pBuffer_c] == 0x0f || pBuffer[pBuffer_c] == 0x05) {
					g_USBMIDIPacketMachineState = UMPMS_DWLD_FW_DONE_2;
					break;
				}

				if(pBuffer[pBuffer_c + 2] != 0xF7) {
					DEBUG_LINEOUT("err: HandleMidiPacket: DWLD_FW_DONE_3 expected 0xF7 as third byte of packet, got: 0x%x", pBuffer[pBuffer_c + 1]);
					g_USBMIDIPacketMachineState = UMPMS_INITIAL;
					if(g_DownloadedFirmwareIsPiezo) g_DownloadedFirmwareIsPiezo = 0;
					break;
				}

				DeployFirmwarePage();
				g_USBMIDIPacketMachineState = UMPMS_INITIAL;
				if(g_DownloadedFirmwareIsPiezo) g_DownloadedFirmwareIsPiezo = 0;
			} break;

			default: {
				CBRM(0, "HandleMidiPacket: invalid state: 0x%x", g_USBMIDIPacketMachineState);
			} break;
		}

		// Move on to next packet
		//DEBUG_LINEOUT("Adding to pBuffer_c 4 from: %d state:%d", pBuffer_c, g_USBMIDIPacketMachineState);
		pBuffer_c += 4;
	} // ! while(pBuffer_c < pBuffer_n)
	
	//DEBUG_LINEOUT_NA("- HandleMidiPacket ");

Error:
	return r;	
}
*/

uint8_t g_USBStatus = 0;
uint8_t g_LastUSBStatus = 0;

void ReadUSBStatus() {
	g_USBStatus = GPIOPinRead(USB_STATUS_PORT, USB_STATUS_PIN) == USB_STATUS_PIN;
}

RESULT USBStatusCallback(void *pContext) {
#ifdef TEST_PIN_LED
	HandleTestPinInterrupt();
#endif

	ReadUSBStatus();

	if(g_OnUSBStatusCallback != NULL)
		g_OnUSBStatusCallback();

#ifdef IPHONE_IAP
	if(g_LastUSBStatus != g_USBStatus) {
		USBDCDTerm(0);

		if(g_USBStatus) {
			USBDCDInit(0, &gc_MidiDevice_iAP, NULL);
			DEBUG_LINEOUT_NA("USB init IAP");
		}
		else {
			USBDCDInit(0, &gc_MidiDevice, NULL);
			DEBUG_LINEOUT_NA("USB init MIDI");

			// iPhone disconnected, need to re-authenticate and ensure no
			// SetiPodPreferences msg sent for line-out
			if(g_fiPhoneAuthenticated)
				g_fiPhoneAuthenticated = 0;
		}

		g_LastUSBStatus = g_USBStatus;
	}
#endif

	DEBUG_LINEOUT("porth: usb:0x%x", g_USBStatus);
	GPIOIntClear(USB_STATUS_PORT, USB_STATUS_PIN);

	return R_OK;
}

// current prototype doesn't have 3V3OUT reroute
#define ENABLE_IAP_MIDI_ISR

USB_PERIPHERAL_INFO *m_pUSBPeripheral = NULL;

RESULT InitializeUSBPeripheralConfiguration(USB_PERIPHERAL_INFO *pUSBPeripheral) {
	m_pUSBPeripheral = pUSBPeripheral;
	return R_OK;
}

// Audio status code injected into here for now, need to separate into own init function
RESULT InitUSBMIDI() {
	RESULT r = R_OK;

#ifdef IPHONE_IAP
	// Set up the handler for port H to handle iPhone connect / disconnect
	CBRM_NA(ROM_SysCtlPeripheralPresent(USB_STATUS_PERIPH), "InitUSBMIDI: USB_STATUS_PERIPH is not present");
	ROM_SysCtlPeripheralEnable(USB_STATUS_PERIPH);

	// Init pin
	GPIOPinTypeGPIOInput(USB_STATUS_PORT, USB_STATUS_PIN);
	ROM_GPIOPadConfigSet(USB_STATUS_PORT, USB_STATUS_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

	CRM(RegisterInterrupt(USB_STATUS_PORT, GPIOPinToInt(USB_STATUS_PIN), GPIO_BOTH_EDGES, USBStatusCallback, NULL),
			"Failed to set interrupt cb for port %x on pin %d", USB_STATUS_PORT, GPIOPinToInt(USB_STATUS_PIN));
#endif

	// For some reason this doesn't seem to work on LM3S3748
	CBRM_NA_WARN(ROM_SysCtlPeripheralPresent(SYSCTL_PERIPH_USB0), "InitUSBMIDI: USB0 is not present");

	if(!g_device.m_fUSB0) g_device.m_fUSB0 = true;
	else return R_NO_EFFECT;

	// Set up the handler for port H to handle iPhone connect / disconnect
	/*
	CBRM_NA(ROM_SysCtlPeripheralPresent(USB_MIDI_PERIPH), "InitUSBMIDI: USBMIDI GPIO port is not present");
	ROM_SysCtlPeripheralEnable(USB_MIDI_PERIPH);
	ROM_GPIOPinTypeUSBAnalog(USB_MIDI_PORT_BASE, USB_MIDI_DP_PIN | USB_MIDI_DM_PIN);
	*/

	CNRM(m_pUSBPeripheral, "USB Peripheral Configuration not set");
	CBRM_NA(ROM_SysCtlPeripheralPresent(m_pUSBPeripheral->gpio.GPIOPeripheral), "InitUSBMIDI: USBMIDI GPIO port is not present");
	ROM_SysCtlPeripheralEnable(m_pUSBPeripheral->gpio.GPIOPeripheral);
	ROM_GPIOPinTypeUSBAnalog(m_pUSBPeripheral->gpio.GPIOPort, m_pUSBPeripheral->gpio.dp_pin | m_pUSBPeripheral->gpio.dm_pin);
	m_pUSBPeripheral->fInitialized = 1;
	m_pUSBPeripheral->gpio.fEnabled = 1;	// TODO: this should be more useful

    // Enable the uDMA controller and set up the control table base.
	// TODO: UDMA?
#ifdef USB_UDMA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    uDMAEnable();
    uDMAControlBaseSet(g_sDMAControlTable);
#endif

	//USBStackModeSet(0, eUSBModeDevice, 0);
    USBStackModeSet(0, eUSBModeForceDevice, 0);		// This forces device regardless of VBUS and USB ID

#ifdef IPHONE_IAP
	ReadUSBStatus();
	g_LastUSBStatus = g_USBStatus;

	// Figure out which mode to use on start up

	int32_t  mode = USB_MIDI;

	if(g_USBStatus)
		mode = IAP_MIDI;
	else
		mode = USB_MIDI;


	switch(mode) {

		case IAP_MIDI: {
			USBDCDInit(0, &gc_MidiDevice_iAP, NULL);
			DEBUG_LINEOUT_NA("USB (iAP) initialized!");
		} break;

		case USB_MIDI: {
			USBDCDInit(0, &gc_MidiDevice, NULL);
			DEBUG_LINEOUT_NA("USB-MIDI initialized!");
		} break;

		default: {
			DEBUG_LINEOUT("USB not initialized, mode %d not supported", mode);
			return R_ERROR;
		//} break;
	}
#else
	USBDCDInit(0, &gc_MidiDevice, NULL);
	DEBUG_LINEOUT_NA("USB-MIDI initialized!");
	ROM_IntMasterEnable();	// just in case
#endif

Error:
	return r;
}

