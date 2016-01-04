#ifndef DEVICE_H_
#define DEVICE_H_

// OphoEmbed/Device/Device.h
// The OPHO Device
// This contains the common interfaces for an OphoEmbed device
// and allows for custom services as needed

#include "../Common/EHM.h"

typedef bool (*fnbool)();
typedef uint8_t (*fnuint8)();
typedef uint8_t* (*fnpuint8)();
typedef uint8_t (*fnuint8_v)(int);
typedef int8_t (*fnint8)();
typedef uint8_t* (*fnpsz)();	// null terminated string
typedef uint16_t (*fnuint16)();
typedef int16_t (*fnint16)();
typedef uint32_t (*fnuint32)();
typedef int32_t (*fnint32)();

typedef void* (*fnpvoid)();

// TODO: Use custom service arch?
#define MAX_DEVICE_SERVICES 10

typedef enum {
	DEVICE_SERVICE_BOOL,
	DEVICE_SERVICE_UINT8,
	DEVICE_SERVICE_INT8,
	DEVICE_SERVICE_PSZ,
	DEVICE_SERVICE_UINT16,
	DEVICE_SERVICE_INT16,
	DEVICE_SERVICE_UINT32,
	DEVICE_SERVICE_INT32,
	DEVICE_SERVICE_PVOID,
	DEVICE_SERVICE_INVALID
} DEVICE_SERVICE_TYPE;


// Service call backs
typedef RESULT (*fncbService) (int argc, ...);

typedef struct {
	bool m_fInitialized;			// Is service initialized?
	uint8_t m_serviceID;			// The service identifier
	DEVICE_SERVICE_TYPE type;		// The service type
	void *pfncbService;				// The service call back
} DEVICE_SERVICE;

typedef struct {
	unsigned m_fInitialized: 1;

	unsigned m_fSysTick :1;

	// Bus Interfaces
	unsigned m_fSPI0 :1;
	unsigned m_fSPI1 :1;
	unsigned m_fI2C0 :1;
	unsigned m_fI2C1 :1;
	unsigned m_fUART0 :1;
	unsigned m_fUSB0 :1;

	unsigned m_fVelocityEnabled :1;

	fnuint32 cbDeviceID;
	fnuint8 cbFirmwareVersion;
	fnbool cbIsCharging;
	fnuint8 cbBatteryPercentage;
	fnuint8_v cbSerialNumber;
	fnpvoid cbGetUserspaceAddress;
	fnpuint8 cbGetUserspaceSerialAddress;
	fnuint8 cbGetUserspaceSerialLength;

	// TODO: Put all of the standard services into generic services arch?
	DEVICE_SERVICE m_services[MAX_DEVICE_SERVICES];
} DEVICE;

typedef struct {
	unsigned major: 4;
	unsigned minor: 4;
} DEVICE_FIRMWARE_VERSION;

extern DEVICE g_device;

// Firmware Update
extern uint32_t  g_FirmwareDownloadAddress;
extern uint32_t  g_DownloadedFirmwareBytes;
extern uint32_t  g_DownloadedFirmwarePages;
extern uint32_t  g_DownloadedFirmwareIsPiezo;
extern uint32_t  g_DownloadedPiezoFirmware_size;
RESULT EraseFirmwareUpdateEraseFlashArea();
RESULT PrintFirmwareDownloadedBytes();

RESULT InitDevice(DEVICE device);
bool IsDeviceInitialized();

uint8_t GetDeviceID();
DEVICE_FIRMWARE_VERSION GetDeviceFirmwareVersion();
bool IsDeviceCharging();
uint8_t GetDeviceBatteryPercentage();
uint8_t GetDeviceSerialNumber(uint8_t byteNum);

uint8_t GetDeviceUSBStatus();

void *GetDeviceUserspaceAddress();
uint8_t *GetDeviceUserspaceSerialAddress();
uint8_t GetDeviceUserspaceSerialLength();


#endif // ! DEVICE_H_
