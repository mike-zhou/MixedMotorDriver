/*
 * ECD300_test.h
 *
 * Created: 2014/12/20 1:41:41
 *  Author: user
 */ 


#ifndef ECD300_TEST_H_
#define ECD300_TEST_H_

/**
	Output a string repeatedly through UART0 at 115200, 8N1.a
*/
void ecd300TestUart(void);

/**
	Receive a character from UARTC0, and echo it back.
*/
void ecd300TestUartEcho(void);

void ecd300TestSysClk(void);

void ecd300TestDFU(void);

void ecd300TestBlueTooth(void);

void ecd300TestBulk(void);

void ecd300TestDac(void);

void ecd300TestAdc(void);

void ecd300TestTwi(void);

void ecd300TestMax485(void);

#endif /* ECD300_TEST_H_ */