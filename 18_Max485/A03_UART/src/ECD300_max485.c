#include "ECD300_max485.h"
#include "ECD300_max485_config.h"
#include "ECD300_usart.h"

static bool _bMax1Initialized=false;
static bool _bMax2Initialized=false;

/**
* This function initializes the designated MAX485 device.
* Parameter:
* 	deviceNumber: Indicates the MAX485 chip in ECD301.
* Return value:
*	0:	success
*	-1:	the underlying UART cannot be initialized successfully
*	-2:	parameter error
*/
char ecd300InitMax485(ECD300_MAX485_e deviceNumber)
{
	char rc;
	usart_rs232_options_t uartOption;

	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;
	
	switch(deviceNumber)
	{
		case ECD300_MAX485_1:
			//set the pull-up of UART input
			PORTC.PIN6CTRL=PORT_OPC_PULLUP_gc;
			PORTC.PIN7CTRL=PORT_OPC_PULLUP_gc;
			//set the data direction.
			PORTC.OUTCLR=PIN5_bm;
			PORTC.DIRSET=PIN5_bm;
			//initialize the underlying UART.
			uartOption.baudrate=ECD300_MAX485_1_BAUDRATE;
			if(0==ecd300InitUart(ECD300_UART_1, &uartOption))
			{
				_bMax1Initialized=true;
				rc=0;
			}
			else
			{
				rc=-1;
			}
			break;
			
		case ECD300_MAX485_2:
			//set the pull-up of UART input
			PORTD.PIN2CTRL=PORT_OPC_PULLUP_gc;
			PORTD.PIN3CTRL=PORT_OPC_PULLUP_gc;
			//set the data direction.
			PORTD.OUTCLR=PIN1_bm;
			PORTD.DIRSET=PIN1_bm;
			//initialize the underlying UART.
			uartOption.baudrate=ECD300_MAX485_2_BAUDRATE;
			if(0==ecd300InitUart(ECD300_UART_2, &uartOption))
			{
				_bMax2Initialized=true;
				rc=0;
			}
			else
			{
				rc=-1;
			}
			break;
			
		default:
			rc=-2;
			break;
	}

	return rc;
}

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
char ecd300SetMax485Direction(ECD300_MAX485_e deviceNumber, ECD300_MAX485_DIRECTION_e direction)
{
	char rc;
	
	switch(deviceNumber)
	{
		case ECD300_MAX485_1:
			if(!_bMax1Initialized)
			{
				rc=-1;
				break;
			}
			//set the data direction.
			if(ECD300_MAX485_INPUT==direction)
			{
				PORTC.OUTCLR=PIN5_bm;
			}
			else
			{
				PORTC.OUTSET=PIN5_bm;
			}
			rc=0;
			break;
			
		case ECD300_MAX485_2:
			if(!_bMax2Initialized)
			{
				rc=-1;
				break;
			}
			//set the data direction.
			if(ECD300_MAX485_INPUT==direction)
			{
				PORTD.OUTCLR=PIN1_bm;
			}
			else
			{
				PORTD.OUTSET=PIN1_bm;
			}
			rc=0;
			break;
			
		default:
			rc=-2;
			break;
	}

	return rc;
}

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
char ecd300GetMax485Direction(ECD300_MAX485_e deviceNumber, ECD300_MAX485_DIRECTION_e * pDirection)
{
	char rc;

	if(NULL==pDirection)
	{
		return -2;
	}
	
	switch(deviceNumber)
	{
		case ECD300_MAX485_1:
			if(!_bMax1Initialized)
			{
				rc=-1;
				break;
			}
			//check the data direction.
			if(PORTC.IN&PIN5_bm)
			{//high level
				*pDirection=ECD300_MAX485_OUTPUT;
			}
			else
			{//low level
				*pDirection=ECD300_MAX485_INPUT;
			}
			rc=0;
			break;
			
		case ECD300_MAX485_2:
			if(!_bMax2Initialized)
			{
				rc=-1;
				break;
			}
			//check the data direction.
			if(PORTD.IN&PIN1_bm)
			{//high level
				*pDirection=ECD300_MAX485_OUTPUT;
			}
			else
			{//low level
				*pDirection=ECD300_MAX485_INPUT;
			}
			rc=0;
			break;
			
		default:
			rc=-2;
			break;
	}

	return rc;
}

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
char ecd300WriteMax485(ECD300_MAX485_e deviceNumber, unsigned char * pAddress, unsigned short * pAmount)
{
	char rc;

	if(NULL==pAddress)
	{
		return -2;
	}
	
	switch(deviceNumber)
	{
		case ECD300_MAX485_1:
			if(!_bMax1Initialized)
			{
				rc=-1;
				break;
			}
			//check the data direction.
			if(0==(PORTC.IN&PIN5_bm))
			{//low level, is receiving data
				rc=-3;
			}
			else
			{
				unsigned short i;

				rc=0;
				for(i=0;i<*pAmount;i++)
				{
					if(0!=ecd300PutChar(ECD300_UART_1, pAddress[i]))
					{
						rc=-4;
						break;
					}
				}
				*pAmount=i;
			}
			break;
			
		case ECD300_MAX485_2:
			if(!_bMax2Initialized)
			{
				rc=-1;
				break;
			}
			//check the data direction.
			if(0==(PORTD.IN&PIN1_bm))
			{//is receiving data
				rc=-3;
			}
			else
			{
				unsigned short i;

				rc=0;
				for(i=0;i<*pAmount;i++)
				{
					if(0!=ecd300PutChar(ECD300_UART_2, pAddress[i]))
					{
						rc=-4;
						break;
					}
				}
				*pAmount=i;
			}
			break;
			
		default:
			rc=-2;
			break;
	}

	return rc;
}

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
*/
char ecd300IsWrittingFinished(ECD300_MAX485_e deviceNumber, bool * pResult)
{
	char rc;

	if(NULL==pResult)
	{
		return -2;
	}
	
	switch(deviceNumber)
	{
		case ECD300_MAX485_1:
			if(!_bMax1Initialized)
			{
				rc=-1;
				break;
			}
			//check the data direction.
			if(0==(PORTC.IN&PIN5_bm))
			{//low level, is receiving data
				rc=-3;
			}
			else
			{
				if(usart_data_register_is_empty(&USARTC1)&&usart_tx_is_complete(&USARTC1))
				{
					*pResult=true;
				}
				else
				{
					*pResult=false;
				}
				rc=0;
			}
			break;
			
		case ECD300_MAX485_2:
			if(!_bMax2Initialized)
			{
				rc=-1;
				break;
			}
			//check the data direction.
			if(0==(PORTD.IN&PIN1_bm))
			{//low level, is receiving data.
				rc=-3;
			}
			else
			{
				if(usart_data_register_is_empty(&USARTD0)&&usart_tx_is_complete(&USARTD0))
				{
					*pResult=true;
				}
				else
				{
					*pResult=false;
				}
				rc=0;
			}
			break;
			
		default:
			rc=-2;
			break;
	}

	return rc;
}

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
*	-3:	wrong direction, the MAX485 is sending data
*/
char ecd300ReadMax485(ECD300_MAX485_e deviceNumber, unsigned char * pAddress, unsigned short * pAmount)
{
	char rc;

	if((NULL==pAddress)||(NULL==pAmount))
	{
		return -2;
	}
	
	switch(deviceNumber)
	{
		case ECD300_MAX485_1:
			if(!_bMax1Initialized)
			{
				rc=-1;
				break;
			}
			//check the data direction.
			if(PORTC.IN&PIN5_bm)
			{//high level, is sending data.
				rc=-3;
			}
			else
			{
				unsigned char c;
				unsigned short i;

				for(i=0;i<*pAmount;i++)
				{
					if(1==ecd300PollChar(ECD300_UART_1, &c))
					{
						pAddress[i]=c;
					}
					else
					{
						*pAmount=i;
						break;
					}
				}
				rc=0;
			}
			break;
			
		case ECD300_MAX485_2:
			if(!_bMax2Initialized)
			{
				rc=-1;
				break;
			}
			//check the data direction.
			if(PORTD.IN&PIN1_bm)
			{//high level, is sending data.
				rc=-3;
			}
			else
			{
				unsigned char c;
				unsigned short i;

				for(i=0;i<*pAmount;i++)
				{
					if(1==ecd300PollChar(ECD300_UART_2, &c))
					{
						pAddress[i]=c;
					}
					else
					{
						*pAmount=i;
						break;
					}
				}
				rc=0;
			}
			break;
			
		default:
			rc=-2;
			break;
	}

	return rc;
}

