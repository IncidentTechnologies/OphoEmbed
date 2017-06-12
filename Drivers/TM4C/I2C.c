#include "I2C.h"
#include "SysTickController.h"

// Using I2C0 for the auth chip which runs at 50Khz bus speed
RESULT InitI2C0(uint8_t fAuthChipOn) {
	RESULT r = R_OK;

	CBRM(ROM_SysCtlPeripheralPresent(SYSCTL_PERIPH_GPIOB), "GPIO port B not present");
	CBRM(ROM_SysCtlPeripheralPresent(SYSCTL_PERIPH_I2C0), "I2C0 not present");

	if(!g_device.m_fI2C0) g_device.m_fI2C0 = true;
	else return R_NO_EFFECT;

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3);

	//set pin type as I2C using special I2CSCL funtion
    ROM_GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);   //   I2CSCL special function
    ROM_GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);


	I2CMasterInitExpClk(I2C0_BASE, ROM_SysCtlClockGet(), false);	// not fast mode
	
	// Slow down the I2C0 to 50KHz since we're since we're using 2.0B chip (!!)
	// TODO: Remove this and speed up our ACCEL
    if(fAuthChipOn != 0) {
    	HWREG(I2C0_BASE + I2C_O_MTPR) = 0x32;
    	DEBUG_LINEOUT("I2C0 set for 50Mhz operation");
    }
    else {
    	DEBUG_LINEOUT("I2C0 set for normal operation");
    }
	
	DEBUG_LINEOUT("I2C0 initialized");
	
Error:
	return r;	
}

RESULT InitI2C1() {
	RESULT r = R_OK;
	
	CBRM(ROM_SysCtlPeripheralPresent(SYSCTL_PERIPH_GPIOA), "GPIO port A not present");
	CBRM(ROM_SysCtlPeripheralPresent(SYSCTL_PERIPH_I2C1), "I2C1 not present");
	
	if(!g_device.m_fI2C1) g_device.m_fI2C1 = true;
	else return R_NO_EFFECT;

	//Enable I2C1 Peripheral
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    ROM_SysCtlPeripheralReset(SYSCTL_PERIPH_I2C1);

	//Enable GPIO that handles I2C1
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	//set pin type as I2C using special I2CSCL funtion
    ROM_GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);   //   I2CSCL special function
    ROM_GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);
    //Link GPIOA pins 6&7 to I2C1,
    ROM_GPIOPinConfigure(GPIO_PA6_I2C1SCL);
    ROM_GPIOPinConfigure(GPIO_PA7_I2C1SDA);

    //for some reason pad settings need to be as follows
    ROM_GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);   //<<<<<<<<<<<  general NOT right
//    GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_7, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_OD_WPU);

    
    I2CMasterInitExpClk(I2C1_BASE, ROM_SysCtlClockGet(), false);	// not fast mode

	DEBUG_LINEOUT("I2C1 initialized");

Error:
	return r;	
}

#define DELAY_DIV_FACTOR 300

RESULT ReadI2C(uint32_t  ulI2CBase,			//	I2C Base (I2CX_MASTER_BASE)
			   uint8_t addr,				// I2C Bus address
			   uint8_t *pBuffer,			// Buffer to read to
			   int32_t pBuffer_n,					// size of buffer
			   uint8_t reg,				// Register to read to
			   int32_t *pLength					// Length of bytes to read, then returns the number of bytes read
			  )
{
	RESULT r = R_OK;
	int32_t  bytesRead = 0;
	uint32_t  err = I2C_MASTER_ERR_NONE;
	
	CBRM((*pLength >= pBuffer_n), "ReadI2C: Cannot read %d bytes into %d byte buffer", *pLength, pBuffer_n);
	
	// Write the address of the register to the bus
    I2CMasterSlaveAddrSet(ulI2CBase, addr, false);       
    I2CMasterDataPut(ulI2CBase, reg);    
    I2CMasterControl(ulI2CBase, I2C_MASTER_CMD_SINGLE_SEND);		
    while(I2CMasterBusy(ulI2CBase));
    
    CBRM(((err = I2CMasterErr(ulI2CBase)) == I2C_MASTER_ERR_NONE), "WriteI2C: I2C Send error 0x%x", err);

    if(ulI2CBase == I2C0_BASE)
		SysCtlDelay(ROM_SysCtlClockGet() / DELAY_DIV_FACTOR);
	else
		SysCtlDelay(ROM_SysCtlClockGet() / 1000);
    
    // Set the address for read
    I2CMasterSlaveAddrSet(ulI2CBase, addr, true);
    for(bytesRead = 0; bytesRead < (*pLength); bytesRead++)
    {
    	// Read the register	    
	    if(*pLength == 1)
	    	I2CMasterControl(ulI2CBase, I2C_MASTER_CMD_SINGLE_RECEIVE);		// First and only byte
	    else	    
	    	if(bytesRead == 0)
	   			I2CMasterControl(ulI2CBase,  I2C_MASTER_CMD_BURST_RECEIVE_START);		// First byte
	   		else if(bytesRead == *pLength - 1)
	   			I2CMasterControl(ulI2CBase,  I2C_MASTER_CMD_BURST_RECEIVE_FINISH);		// Last byte
	   		else
	       		I2CMasterControl(ulI2CBase,  I2C_MASTER_CMD_BURST_RECEIVE_CONT);		// Continued Rx
	       		    	   	
	   	SysCtlDelay(ROM_SysCtlClockGet() / DELAY_DIV_FACTOR);
	    pBuffer[bytesRead] = I2CMasterDataGet(ulI2CBase);	// rx
	    
	    CBRM(((err = I2CMasterErr(ulI2CBase)) == I2C_MASTER_ERR_NONE), "WriteI2C: I2C Read error 0x%x", err);    	
    } 
    
    while(I2CMasterBusy(ulI2CBase));
	SysCtlDelay(ROM_SysCtlClockGet() / DELAY_DIV_FACTOR);
	
Error:
	*pLength = bytesRead;
	return r;	
}

RESULT WriteI2C(uint32_t  ulI2CBase,		// I2C Base (I2CX_MASTER_BASE) 
				uint8_t addr, 			// I2C Bus address
				uint8_t *pBuffer, 		// Buffer to write data in
				int32_t  pBuffer_n, 					// Buffer length (prevent buffer overloads)
				uint8_t reg, 				// Register to manipulate 
				int32_t  *pBytesWritten				// Bytes Written
			   )		
{
	RESULT r = R_OK;	
	uint32_t  err = I2C_MASTER_ERR_NONE;
	 int32_t  i = 0;
	 int32_t  bytesWritten = 0;
	
	// First write the address
	I2CMasterSlaveAddrSet(ulI2CBase, addr, false);       
    I2CMasterDataPut(ulI2CBase, reg);    
    I2CMasterControl(ulI2CBase, I2C_MASTER_CMD_BURST_SEND_START);
    
    CBRM(((err = I2CMasterErr(ulI2CBase)) == I2C_MASTER_ERR_NONE), "WriteI2C: I2C Send error 0x%x", err);
	
	for(i = 0; i < pBuffer_n; i++)
	{
		if(ulI2CBase == I2C0_BASE)
			SysCtlDelay(ROM_SysCtlClockGet() / 1000);
		else
			SysCtlDelay(ROM_SysCtlClockGet() / 10000);

		I2CMasterDataPut(ulI2CBase, pBuffer[i]);

		if(i == pBuffer_n - 1)
			I2CMasterControl(ulI2CBase, I2C_MASTER_CMD_BURST_SEND_FINISH);		// send the cmd
		else
			I2CMasterControl(ulI2CBase, I2C_MASTER_CMD_BURST_SEND_CONT);		// send the cmd
			
		CBRM(((err = I2CMasterErr(ulI2CBase)) == I2C_MASTER_ERR_NONE), "WriteI2C: I2C Send error 0x%x", err);		
		bytesWritten++;

	}
	
	while(I2CMasterBusy(ulI2CBase));
	if(ulI2CBase == I2C0_BASE)
		SysCtlDelay(ROM_SysCtlClockGet() / 1000);
	else
		SysCtlDelay(ROM_SysCtlClockGet() / 10000);

Error:
	*pBytesWritten = bytesWritten;
	return r;	
}

RESULT I2CSend( uint32_t   ulI2CBase,			// I2C Base (I2CX_MASTER_BASE)
				uint8_t  ucAddr, 				// I2C Bus address
				uint8_t *p_ucBuffer, 			// Buffer to send data from
				uint32_t    p_ucBuffer_n, 		// Buffer length (prevent buffer overloads)
				uint32_t   *p_uiBytesWritten,	// Bytes Written
				uint32_t    uiDelay,				// optional delay after each byte sent
				uint32_t    uiTimeout			// optional timeout
			   )
{
	RESULT r = R_OK;
	uint32_t  ulError = I2C_MASTER_ERR_NONE;
	uint32_t  uiSystickStamp, uiBytesWritten = 0;
	CNR(p_ucBuffer);
	CNR(p_uiBytesWritten);

	// First set the slave address
	I2CMasterSlaveAddrSet(ulI2CBase, ucAddr, false);

	int32_t  i = 0;
	for(i = 0; i < p_ucBuffer_n; i++) {
	    I2CMasterDataPut(ulI2CBase, p_ucBuffer[i]);

	    if(i == 0) { //if it's the first byte start the transfer

			if(p_ucBuffer_n == 1)
				I2CMasterControl(ulI2CBase, I2C_MASTER_CMD_SINGLE_SEND);
			 else
				I2CMasterControl(ulI2CBase, I2C_MASTER_CMD_BURST_SEND_START);

	    } else { //if we're past the first byte, check for last byte - otherwise continue

			if(i == p_ucBuffer_n - 1)
				I2CMasterControl(ulI2CBase, I2C_MASTER_CMD_BURST_SEND_FINISH);
			else
				I2CMasterControl(ulI2CBase, I2C_MASTER_CMD_BURST_SEND_CONT);
	    }

	    uiSystickStamp = SystemTickCount();
		while(I2CMasterBusy(ulI2CBase)){
			//check timeout
			if(uiTimeout) CBRM((SystemTickCount() - uiSystickStamp > uiTimeout) == 0, "I2CSend: timeout on bus after %d ticks", SystemTickCount() - uiSystickStamp);
		}

		CBRM(((ulError = I2CMasterErr(ulI2CBase)) == I2C_MASTER_ERR_NONE), "I2CSend: I2C Send error 0x%x", ulError);
		uiBytesWritten++;
		SysCtlDelay(uiDelay);
	}

Error:
	if(ulError && p_ucBuffer_n > 1 && i < p_ucBuffer_n - 1) I2CMasterControl(ulI2CBase, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
	*p_uiBytesWritten = uiBytesWritten;
	return r;
}

RESULT I2CReceive(uint32_t   ulI2CBase,			// I2C Base (I2CX_MASTER_BASE)
				  uint8_t  ucAddr, 			// I2C Bus address
				  uint8_t *p_ucBuffer, 		// Buffer to write data to
				  uint32_t    p_ucBuffer_n, 		// Buffer length (prevent buffer overloads)
				  uint32_t   *p_uiBytesRead,		// Bytes To Read
				  uint32_t    uiDelay,			// optional delay after each byte sent
				  uint32_t    uiTimeout			// optional timeout
			   )
{
	RESULT r = R_OK;
	uint32_t  ulError = I2C_MASTER_ERR_NONE;
	uint32_t  uiSystickStamp, uiBytesRead = 0, uiBytesToRead = 0;
	CNR(p_ucBuffer);
	CNR(p_uiBytesRead);

	uiBytesToRead = *p_uiBytesRead;

	CBRM((p_ucBuffer_n >= uiBytesToRead), "I2CReceive: Cannot read %d bytes into %d byte buffer", uiBytesToRead, p_ucBuffer_n);

    // Set the address for read
    I2CMasterSlaveAddrSet(ulI2CBase, ucAddr, true);

    int32_t i;
    for(i = 0; i < (uiBytesToRead); i++)
    {
	    if(i == 0) { //if it's the first byte start the transfer

			if(uiBytesToRead == 1)
		    	I2CMasterControl(ulI2CBase, I2C_MASTER_CMD_SINGLE_RECEIVE);		// First and only byte
		    else
		   		I2CMasterControl(ulI2CBase,  I2C_MASTER_CMD_BURST_RECEIVE_START);	// First byte

	    } else { //if we're past the first byte, check for last byte - otherwise continue

			if(i == uiBytesToRead - 1)
	   			I2CMasterControl(ulI2CBase,  I2C_MASTER_CMD_BURST_RECEIVE_FINISH);		// Last byte
	   		else
	       		I2CMasterControl(ulI2CBase,  I2C_MASTER_CMD_BURST_RECEIVE_CONT);		// Continued Rx
	    }

	    uiSystickStamp = SystemTickCount();
		while(I2CMasterBusy(ulI2CBase)){
			//check timeout
			if(uiTimeout) CBRM((SystemTickCount() - uiSystickStamp > uiTimeout) == 0, "I2CReceive: timeout on bus after %d ticks", SystemTickCount() - uiSystickStamp);
		}
	    CBRM(((ulError = I2CMasterErr(ulI2CBase)) == I2C_MASTER_ERR_NONE), "I2CReceive: I2C Send error 0x%x", ulError);

	   	p_ucBuffer[i] = I2CMasterDataGet(ulI2CBase);

	    uiBytesRead++;
	    SysCtlDelay(uiDelay);
    }

Error:
	if(ulError && p_ucBuffer_n > 1 && i < p_ucBuffer_n - 1) I2CMasterControl(ulI2CBase, I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);
	*p_uiBytesRead = uiBytesRead;
	return r;
}
