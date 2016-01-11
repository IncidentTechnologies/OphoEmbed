#ifndef MEMORYMAP_H_
#define MEMORYMAP_H_

// TODO: Eventually there will be a variety of memory mappings and this should be configurable
// currently putting this here for visibility

#define FLASH_KB_SIZE 0x400

#define SIZE_OF_UPDATE_SPACE_KB 	60

#define BOOTLOADER_ADDRESS 			120 * FLASH_KB_SIZE								// Set bootloader address at 120kb

#define FIRMWARE_DOWNLOAD_ADDRESS 	SIZE_OF_UPDATE_SPACE_KB * FLASH_KB_SIZE 		// Set the firmware update address at 60 Kbytes
#define IPHONE_CERT_ADDRESS 		125 * FLASH_KB_SIZE 							// this is set to the 125KB point in flash
#define USER_SPACE_ADDRESS 			127 * FLASH_KB_SIZE								// this is set to the 127KB point in flash

//#define FIRMWARE_DOWNLOAD_ADDRESS 	0x0DC00		// Set the firmware update address at 55 Kbytes
//#define IPHONE_CERT_ADDRESS 		0x1E000		// this is set to the 120KB point in flash
//#define USER_SPACE_ADDRESS 			0x1E800		// this is set to the 122KB point in flash

#endif /* MEMORYMAP_H_ */
