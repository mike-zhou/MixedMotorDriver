/*
 * ECD300.h
 *
 * Created: 2014/12/17 17:26:10
 *  Author: user
 */ 


#ifndef ECD300_H_
#define ECD300_H_

#include <asf.h> //compiler.h can find the iox128a1u.h

typedef enum ECD300_UART
{
	ECD300_UART_0=0,
	ECD300_UART_1,
	ECD300_UART_2,
	ECD300_UART_3,
	ECD300_UART_4,
	ECD300_UART_5,
	ECD300_UART_6,
	ECD300_UART_MAX
} ECD300_UART_e;

/**
	Initialize the designated UART port. The pins for this UART will be configured in this function.
	return value:
		0: 	success
		-1:	failure
*/
char ecd300InitUart(ECD300_UART_e uart, usart_rs232_options_t * pOptions);

/**
	Output a character through the designated UART.
	return value:
		0:	success
		-1:	failure
*/
char ecd300PutChar(ECD300_UART_e uart, unsigned char character);

/**
	Check whether there is a character received in the designated UART. 
	If there is a character received, it is copied to the address pointed by pCharacter.
	return value:
		1:	a character is copied to the address passed by pCharacter.
		0:	no data received in the designated UART.
		-1:	error.
*/
char ecd300PollChar(ECD300_UART_e uart, unsigned char * pCharacter);

/**
	Wait until a character is read back from the designated UART. 
	The character data will be copied to the address pointed by pCharacter.
	return value:
		1:	a character is copied to the address passed by pCharacter.
		-1:	error.
*/
char ecd300GetChar(ECD300_UART_e uart, unsigned char * pCharacter);

#endif /* ECD300_H_ */
