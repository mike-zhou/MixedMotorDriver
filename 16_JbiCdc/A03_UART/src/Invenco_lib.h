/*
 * ECD300_test.h
 *
 * Created: 2014/12/20 1:41:41
 *  Author: user
 */ 

#ifndef ECD300_TEST_H_
#define ECD300_TEST_H_

#include <asf.h>
#include "ECD300.h"
#include "usb_protocol_cdc.h"
#include "tc.h"


// RS232 related
void printString(char * pString);
void printHex(unsigned char hex);
/**
 * read back a character from UART. This function blocks until a character is available
 */
unsigned char getChar(void);
/**
 * Read back a character from UART. This function won't block.
 * Return value:
 *	true: a valid character is put to address pointed by p
 *	false: no valid character in UART.
 */
bool pollChar(unsigned char * p);
/**
 * Output a character to UART. This function won't block.
 * Return value:
 *	true: the character is output to UART
 *	false: the character is not output to UART.
 */
bool putChar(unsigned char c);

// USB related
void clearInputBuffer(void);
bool writeInputBuffer(unsigned char c);
unsigned char readInputBuffer(void);
bool writeOutputBufferChar(unsigned char c);
void writeOutputBufferString(const char * pString);
void writeOutputBufferHex(unsigned char n);
void sendOutputBufferToHost(void);

//////////////////////////////////
// counter related
//////////////////////////////////

//initialize counter
void counter_init(void);
//wait for 1/time second
void counter_wait(unsigned char time);
//return current counter value
unsigned short counter_get(void);
//return length of a clock in microsecond
unsigned short counter_clock_length(void);

enum CommandState
{
	AWAITING_COMMAND = 0,
	STARTING_COMMAND,
	EXECUTING_COMMAND
};

void Invenco_init(void);


#endif /* ECD300_TEST_H_ */