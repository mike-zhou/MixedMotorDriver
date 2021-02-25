/*
 * ECD300.h
 *
 *  Created on: 25/02/2021
 *      Author: user1
 */

#ifndef ECD300_H_
#define ECD300_H_

#define ECD300_UART_2 2

extern unsigned char PORTA_DIR;
extern unsigned char	PORTB_DIR;
extern unsigned char	PORTC_DIR;
extern unsigned char	PORTD_DIR;
extern unsigned char	PORTE_DIR;
extern unsigned char	PORTF_DIR;
extern unsigned char	PORTH_DIR;
extern unsigned char	PORTJ_DIR;
extern unsigned char	PORTK_DIR;

char ecd300InitUart(unsigned char devIndex, usart_rs232_options_t * pOptions);
char ecd300PutChar(unsigned char devIndex, unsigned char character);
char ecd300PollChar(unsigned char devIndex, unsigned char * pChar);

#endif /* ECD300_H_ */
