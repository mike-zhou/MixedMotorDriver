#ifndef _ECD300_MAX485_H__
#define _ECD300_MAX485_H__

#include "asf.h"

typedef enum
{
	ECD300_MAX485_1,
	ECD300_MAX485_2
} ECD300_MAX485_e;

typedef enum
{
	ECD300_MAX485_INPUT,
	ECD300_MAX485_OUTPUT
} ECD300_MAX485_DIRECTION_e;

/**
* This function initializes the designated MAX485 device.
* Parameter:
* 	deviceNumber: Indicates the MAX485 chip in ECD301.
* Return value:
*	0:	success
*	-1:	the underlying UART cannot be initialized successfully
*	-2:	parameter error
*/
char ecd300InitMax485(ECD300_MAX485_e deviceNumber);

/**
* This function sets whether MAX485 drives the external cable or receives data from external cable.
* Parameter:
* 	deviceNumber: Indicates the MAX485 chip in ECD301.
*	direction:		INPUT:	receive data from external cable.
*				OUTPUT:	drive the external cable.
* Return value:
*	0:	success
*	-1:	un-intialized
*	-2:	parameter error
*/
char ecd300SetMax485Direction(ECD300_MAX485_e deviceNumber, ECD300_MAX485_DIRECTION_e direction);

/**
* This function gets whether MAX485 drives the external cable or receives data from external cable.
* Parameter:
* 	deviceNumber: Indicates the MAX485 chip in ECD301.
*	pDirection:		address where the result will be saved to.
* Return value:
*	0:	success
*	-1:	un-initialized
*	-2:	parameter error
*/
char ecd300GetMax485Direction(ECD300_MAX485_e deviceNumber, ECD300_MAX485_DIRECTION_e * pDirection);

/**
* This function sends out data through designated MAX485 device.
* Parameter:
* 	deviceNumber: Indicates the MAX485 chip in ECD301.
*	pAddress:	address where data is saved.
*	pAmount:	input: variable pointed by this parameter contains the amount of data which is to be sent out.
*				output: variable pointed by this parameter contains the actual amount of data which is sent out.
* Return value:
*	0:	success
*	-1:	un-initialized
*	-2:	parameter error
*	-3:	wrong direction
*	-4:	data cannot be sent out, the MAX485 is receiving data
*/
char ecd300WriteMax485(ECD300_MAX485_e deviceNumber, unsigned char * pAddress, unsigned short * pAmount);

/**
* This function checks whether the data has been sent out completely.
* Parameter:
* 	deviceNumber: Indicates the MAX485 chip in ECD301.
*	pResult:	address where result is saved to.
* Return value:
*	0:	success
*	-1:	un-initialized
*	-2:	parameter error
*	-3: 	wrong direction, the MAX485 is receiving data
*	-4:	data cannot be sent out
*/
char ecd300IsWrittingFinished(ECD300_MAX485_e deviceNumber, bool * pResult);

/**
* This function tries to get the data received from the designaged MAX485 chip.
* Parameter:
* 	deviceNumber: Indicates the MAX485 chip in ECD301.
*	pAddress:	address where result is saved to.
* 	pAmount:	as input, the variable pointed by this parameter indicates the maxium length of data that can be written to pAddress.
*				as output, the variable pointed by this parameter indicates the actual length of data put to pAddress.
* Return value:
*	0:	success
*	-1:	un-initialized
*	-2:	parameter error
*	-3:	wrong direction, the MAX485 is sending data.
*/
char ecd300ReadMax485(ECD300_MAX485_e deviceNumber, unsigned char * pAddress, unsigned short * pAmount);

#endif

