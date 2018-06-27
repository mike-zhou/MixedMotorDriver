/*
 * ECD300_test.c
 *
 * Created: 2014/12/20 1:42:02
 *  Author: user
 */ 

#include <asf.h>
#include "ECD300.h"
#include "ECD300_test.h"

void ecd300TestUart(void)
 {
	usart_rs232_options_t uartOption;
	
	board_init();
	disableJtagPort();
	offAllLeds();
	
	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;

	if(0!=ecd300InitUart(ECD300_UART_0, &uartOption))
		onLed(0);
	else
		onLed(1);

	if(0!=ecd300InitUart(ECD300_UART_1, &uartOption))
		onLed(0);
	else
		onLed(1);

	//make CPU respond to interrupts.
	cpu_irq_enable();

	while(1)
	{
		onLed(3);

		if(0==ecd300PutString(ECD300_UART_0, "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"))
		{
			onLed(4);
		}
		else
		{
			onLed(5);
		}
		offLed(3);
		offLed(4);
		offLed(5);

		//ecd300PollChar(ECD300_UART_0, &character);
	}
}

void ecd300TestUartEcho(void)
{
	usart_rs232_options_t uartOption;
	unsigned char character;
	
	board_init();
	disableJtagPort();
	offAllLeds();
	
	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;

	if(0!=ecd300InitUart(ECD300_UART_6, &uartOption))
		onLed(0);
	else
		onLed(1);

	//make CPU respond to interrupts.
	cpu_irq_enable();

	character='c';
	while(1)
	{
		onLed(3);

		if(0==ecd300GetChar(ECD300_UART_6, &character))
		{
			onLed(4);
		}
		else
		{
			onLed(5);
		}
		offLed(3);
		offLed(4);
		offLed(5);

		ecd300PutChar(ECD300_UART_6, character);
	}
}

