#ifndef MEMORYMAP_H_
#define MEMORYMAP_H_

// TODO: Eventually there will be a variety of memory mappings and this should be configurable
// currently putting this here for visibility

// Physical Memory Layout

// FLASH
#define FLASH_BASE					0x00000000
#define FLASH_KB_SIZE 				0x00000400
#define FLASH_SIZE					0x00040000

// RAM
#define RAM_BASE 					0x20000000
#define RAM_SIZE 					0x00008000

// The starting address of the application.  Normally the interrupt vectors
// must be located at the beginning of the application.

#define APP_START_ADDRESS 			0x00002800
//#define APP_LENGTH					0x00040000
#define APP_LENGTH					0x00030000
#define BOOTLOADER_OFFSET			APP_START_ADDRESS

#define _BUILD_FOR_BOOTLOADER

#ifdef _BUILD_FOR_BOOTLOADER
	#define APP_BASE 					(FLASH_BASE + BOOTLOADER_OFFSET)
#else
	#define APP_BASE 					FLASH_BASE
#endif

#define CRC32_OFFSET				0x00000400
#define CRC32_ADDR					(APP_BASE + CRC32_OFFSET)  // This is the highest location the CRC can reside in, so lets put it there
#define CRC32_LENGTH				0x00000002

//#define RESERVED_BASE USER_BASE + USER_SIZE
//#define RESERVED_SIZE FLASH_SIZE - RESERVED_BASE

#define SIZE_OF_UPDATE_SPACE_KB 	60
#define SIZE_OF_FLASH_KB			256
#define BOOTLOADER_ADDRESS_KB		(SIZE_OF_FLASH_KB - 7)
#define IPHONE_CERT_ADDRESS_KB		(BOOTLOADER_ADDRESS_KB + 5)
#define USERSPACE_ADDRESS_KB		(IPHONE_CERT_ADDRESS_KB + 1)

#define BOOTLOADER_ADDRESS 			(BOOTLOADER_ADDRESS_KB * FLASH_KB_SIZE)								// Set bootloader address at 120kb

#define FIRMWARE_DOWNLOAD_ADDRESS 	(SIZE_OF_UPDATE_SPACE_KB * FLASH_KB_SIZE) 		// Set the firmware update address at 60 Kbytes
#define IPHONE_CERT_ADDRESS 		(IPHONE_CERT_ADDRESS_KB * FLASH_KB_SIZE) 							// this is set to the 125KB point in flash
#define USER_SPACE_ADDRESS 			(USERSPACE_ADDRESS_KB * FLASH_KB_SIZE)								// this is set to the 127KB point in flash

//#define FIRMWARE_DOWNLOAD_ADDRESS 	0x0DC00		// Set the firmware update address at 55 Kbytes
//#define IPHONE_CERT_ADDRESS 		0x1E000		// this is set to the 120KB point in flash
//#define USER_SPACE_ADDRESS 			0x1E800		// this is set to the 122KB point in flash

#endif /* MEMORYMAP_H_ */
