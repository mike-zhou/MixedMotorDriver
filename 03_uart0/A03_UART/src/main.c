/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
#include <asf.h>
#include "ECD300.h"

int main (void)
{
	usart_rs232_options_t uartOption;
	unsigned char character;
	
	board_init();
	disableJtagPort();
	offAllLeds();
	
	uartOption.baudrate=9600;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;

	if(0!=ecd300InitUart(ECD300_UART_0, &uartOption))
		onLed(0);
	else
		onLed(1);

	character='C';
	while(1)
	{
		unsigned long i;
		
		onLed(3);
		if(0==ecd300PutChar(ECD300_UART_0, character))
		{
			onLed(4);
		}
		else
		{
			onLed(5);
		}
		//delay
		for(i=0;i<0x20000;i++)
		{
			;
		}
		offLed(3);
		offLed(4);
		offLed(5);
		//delay
		for(i=0;i<0x40000;i++)
		{
			;
		}

		ecd300PollChar(ECD300_UART_0, &character);
	}
}

