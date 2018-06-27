#ifndef __JBI_PLAYER_
#define __JBI_PLAYER_

/**



=======================================
                        jp_main
JBI Player     ---------------------------------------

                        implementation of player


=======================================
                       driver APIs
jbi driver       ---------------------------------------

                       implementation of driver APIs



*/

/*
Initialize the JTAG hardware. 
Return value:
    0:  success
    -1: failed
Implemented by the driver.
*/
long stubJtagInit(void);

/*
Release the resource for JTAG operation. 
Implemented by the driver.
*/
long stubJtagRelease(void);


/*
Access to the jtag pins.
The following is done in sequence:
	1. if(tms==0) then set TMS of jtag to 0, else to 1.
		if(tdi==0) then set TDI of jtag to 0, else to 1.
	2. if(bReadTdo!=0) then read the level of TDO of jtag.
	3. set CLK of jtag to 1.
	4. set CLK of jtag to 0.
	5. return value of TDO.
Implemented by the driver.
*/
long stubJtagIo(long tms, long tdi, long bReadTdo);

/*
This function is invoked to suspend the caller for the indicated micro seconds. 
Implemented by the driver.
*/
long stubDelay(unsigned long microSeconds);

/*
This function is invoked to make the driver notified of the maxium frequency of the clock pin of the JTAG.
Implemented by the driver.
*/
long stubSetClockFrequency(unsigned long maxFrequencyHz);

/*
This function is invoked to allocate the indicated amount of memory.
Implemented by the driver.
*/
void * stubMalloc(unsigned long size);

/**
This function is invoked to allocate the designated amoount of memory in the external RAM.
Implmemented by the driver.
*/
unsigned long stubMallocExternalRam(unsigned long size);

/*
This function is invoked to free the memory.
Implemented by the driver.
*/
void stubFree(void * address);

/*
This function is invoked to show a string to the user.
Implemented by the driver.
*/
void stubPrintString(char * pString);
void stubPrintHex(unsigned char c);


char 	getInt8(unsigned long address);
char		getInt8Array(unsigned long address, long index);
void 	setInt8(unsigned long address, char value);
void 	setInt8Array(unsigned long address, long index, char value);

unsigned char getUint8(unsigned long address);
unsigned char getUint8Array(unsigned long address, long index);
void 		setUint8(unsigned long address, unsigned char value);
void			setUint8Array(unsigned long address, long index, unsigned char value);

short 	getInt16(unsigned long address);
short 	getInt16Array(unsigned long address, long index);
void 	setInt16(unsigned long address, short value);
void 	setInt16Array(unsigned long address, long index, short value);

unsigned short 	getUint16(unsigned long address);
unsigned short	getUint16Array(unsigned long address, long index);
void 			setUint16(unsigned long address, unsigned short value);
void				setUint16Array(unsigned long address, long index, unsigned short value);

long 	getInt32(unsigned long address);
long 	getInt32Array(unsigned long address, long index);
void 	setInt32(unsigned long address, long value);
void 	setInt32Array(unsigned long address, long index, long value);

unsigned long 	getUint32(unsigned long address);
unsigned long getUint32Array(unsigned long address, long index);
void 		setUint32(unsigned long address, unsigned long value);
void			setUint32Array(unsigned long address, long index, unsigned long value);


/*
This function is invoked to execute the JAM file. 
    fileBuffer: address of JAM file content in the memory
    fileLength: amount of JAM file content, in bytes
    actionCommand: what to do with the JAM file. "PROGRAM" - program the JAM file to the CPLD through JTAG
                                                 "VERIFY" - verify whether the content in the CPLD is same as JAM file.
    bOutputInfo: 1 - output information when the programing or verifying is going on.
                 0 - no information is output.
This funciton is provided by the JAM player.
*/
long jp_main(unsigned long fileBufferAddress, unsigned long fileLength, unsigned char * actionCommand, long bOutputInfo);


#endif


