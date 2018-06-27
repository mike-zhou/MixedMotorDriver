/*
 * ECD300.c
 *
 * Created: 2014/12/17 17:26:28
 *  Author: user
 */ 
#include "ECD300.h"

char ecd300InitUart(ECD300_UART_e uart, usart_rs232_options_t * pOptions)
{
	char rc;
	
	switch(uart)
	{
		case ECD300_UART_0:
			PORTC.DIRCLR=PIN2_bm;//set this pin as input.
			PORTC.DIRSET=PIN3_bm;//set this pin as output.
			if(usart_init_rs232(&USARTC0, pOptions))
				rc=0;
			else
				rc=-1;
			break;

		case ECD300_UART_1:
			PORTC.DIRCLR=PIN6_bm;//set this pin as input.
			PORTC.DIRSET=PIN7_bm;//set this pin as output.
			if(usart_init_rs232(&USARTC1, pOptions))
				rc=0;
			else
				rc=-1;
			break;

		case ECD300_UART_2:
			PORTD.DIRCLR=PIN2_bm;//set this pin as input.
			PORTD.DIRSET=PIN3_bm;//set this pin as output.
			if(usart_init_rs232(&USARTD0, pOptions))
				rc=0;
			else
				rc=-1;
			break;

		case ECD300_UART_3:
			PORTE.DIRCLR=PIN2_bm;//set this pin as input.
			PORTE.DIRSET=PIN3_bm;//set this pin as output.
			if(usart_init_rs232(&USARTE0, pOptions))
				rc=0;
			else
				rc=-1;
			break;

		case ECD300_UART_4:
			PORTE.DIRCLR=PIN6_bm;//set this pin as input.
			PORTE.DIRSET=PIN7_bm;//set this pin as output.
			if(usart_init_rs232(&USARTE1, pOptions))
				rc=0;
			else
				rc=-1;
			break;

		case ECD300_UART_5:
			PORTF.DIRCLR=PIN2_bm;//set this pin as input.
			PORTF.DIRSET=PIN3_bm;//set this pin as output.
			if(usart_init_rs232(&USARTF0, pOptions))
				rc=0;
			else
				rc=-1;
			break;

		case ECD300_UART_6:
			PORTF.DIRCLR=PIN6_bm;//set this pin as input.
			PORTF.DIRSET=PIN7_bm;//set this pin as output.
			if(usart_init_rs232(&USARTF1, pOptions))
				rc=0;
			else
				rc=-1;
			break;

		default:
			rc=-1;
	}

	return rc;
}

char ecd300PutChar(ECD300_UART_e uart, unsigned char character)
{
	int rc=-1;
	USART_t * pUART=NULL;
	
	switch(uart)
	{
		case ECD300_UART_0:
			pUART=&USARTC0;
			break;

		case ECD300_UART_1:
			pUART=&USARTC1;
			break;

		case ECD300_UART_2:
			pUART=&USARTD0;
			break;

		case ECD300_UART_3:
			pUART=&USARTE0;
			break;

		case ECD300_UART_4:
			pUART=&USARTE1;
			break;

		case ECD300_UART_5:
			pUART=&USARTF0;
			break;

		case ECD300_UART_6:
			pUART=&USARTF1;
			break;

		default:
			rc=-1;
	}

	if(pUART!=NULL)
	{
		if(STATUS_OK==usart_putchar(pUART, character))
			rc=0;
		else
			rc=-1;
	}

	return rc;
}

char ecd300PollChar(ECD300_UART_e uart, unsigned char * pCharacter)
{
	int rc=-1;
	USART_t * pUART;
	
	switch(uart)
	{
		case ECD300_UART_0:
			pUART=&USARTC0;
			break;

		case ECD300_UART_1:
			pUART=&USARTC1;
			break;

		case ECD300_UART_2:
			pUART=&USARTD0;
			break;

		case ECD300_UART_3:
			pUART=&USARTE0;
			break;

		case ECD300_UART_4:
			pUART=&USARTE1;
			break;

		case ECD300_UART_5:
			pUART=&USARTF0;
			break;

		case ECD300_UART_6:
			pUART=&USARTF1;
			break;

		default:
			pUART=NULL;
	}

	if(pUART!=NULL)
	{
		if(pUART->STATUS & USART_RXCIF_bm)
		{//data available
			*pCharacter=pUART->DATA;
			rc=1;
		}
		else
		{//no data
			rc=0;
		}
	}

	return rc;
}

char ecd300GetChar(ECD300_UART_e uart, unsigned char * pCharacter)
{
	int rc=-1;
	USART_t * pUART;
	
	switch(uart)
	{
		case ECD300_UART_0:
			pUART=&USARTC0;
			break;

		case ECD300_UART_1:
			pUART=&USARTC1;
			break;

		case ECD300_UART_2:
			pUART=&USARTD0;
			break;

		case ECD300_UART_3:
			pUART=&USARTE0;
			break;

		case ECD300_UART_4:
			pUART=&USARTE1;
			break;

		case ECD300_UART_5:
			pUART=&USARTF0;
			break;

		case ECD300_UART_6:
			pUART=&USARTF1;
			break;

		default:
			pUART=NULL;
	}

	if(pUART!=NULL)
	{
		while(!(pUART->STATUS & USART_RXCIF_bm))
		{
			;
		}
		*pCharacter=pUART->DATA;
		rc=1;
	}

	return rc;
}
 
