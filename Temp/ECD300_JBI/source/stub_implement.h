#ifndef __STUB_IMPLEMENT_H_
#define __STUB_IMPLEMENT_H_

#define ECD300

#ifdef ECD_9200
#include "stdio.h"

#define STUB_PRINT_1(a) printf(a)
#define STUB_PRINT_2(a, b) printf(a, b)
#define STUB_PRINT_3(a, b, c) printf(a, b, c)
#define STUB_PRINT_4(a, b, c, d) printf(a, b, c, d)
#endif 

#ifdef ECD200
#include "stdio.h"

#define STUB_MAX_STRING_LENGTH 200

#define STUB_PRINT_1(a) {char buffer[STUB_MAX_STRING_LENGTH]; sprintf(buffer, "%s", a); stubPrintString(buffer);}
#define STUB_PRINT_2(a, b) {char buffer[STUB_MAX_STRING_LENGTH]; sprintf(buffer, a, b); stubPrintString(buffer);}
#define STUB_PRINT_3(a, b, c) {char buffer[STUB_MAX_STRING_LENGTH]; sprintf(buffer, a, b, c); stubPrintString(buffer);}
#define STUB_PRINT_4(a, b, c, d) {char buffer[STUB_MAX_STRING_LENGTH]; sprintf(buffer, a, b, c, d); stubPrintString(buffer);}
#endif

#ifdef ECD300
#include "stdio.h"

#define STUB_MAX_STRING_LENGTH 200

#define STUB_PRINT_1(a) {char buffer[STUB_MAX_STRING_LENGTH]; sprintf(buffer, "%s", a); stubPrintString(buffer);}
#define STUB_PRINT_2(a, b) {char buffer[STUB_MAX_STRING_LENGTH]; sprintf(buffer, a, b); stubPrintString(buffer);}
#define STUB_PRINT_3(a, b, c) {char buffer[STUB_MAX_STRING_LENGTH]; sprintf(buffer, a, b, c); stubPrintString(buffer);}
#define STUB_PRINT_4(a, b, c, d) {char buffer[STUB_MAX_STRING_LENGTH]; sprintf(buffer, a, b, c, d); stubPrintString(buffer);}
#endif

/*
Initialize the JTAG hardware. 
Return value:
    0:  success
    -1: failed
Implemented by the driver.
*/
int jtagInit(void);

/*
Release the resource for JTAG operation. 
Implemented by the driver.
*/
int jtagRelease(void);


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
int jtagIo(int tms, int tdi, int bReadTdo);

/*
This function is invoked to suspend the caller for the indicated micro seconds. 
Implemented by the driver.
*/
int stubDelay(unsigned long microSeconds);

/*
This function is invoked to make the driver notified of the maxium frequency of the clock pin of the JTAG.
Implemented by the driver.
*/
int stubSetClockFrequency(unsigned long maxFrequencyHz);

/*
This function is invoked to allocate the indicated amount of memory.
Implemented by the driver.
*/
void * stubMalloc(unsigned long size);

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
int jp_main(unsigned char * fileBuffer, unsigned long fileLength, unsigned char * actionCommand, int bOutputInfo);


#endif

