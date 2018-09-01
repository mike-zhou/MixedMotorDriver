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
unsigned char getChar(void);

// USB related
void clearInputBuffer();
bool writeInputBuffer(unsigned char c);
unsigned char readInputBuffer();
bool writeOutputBufferChar(unsigned char c);
void writeOutputBufferString(char * pString);
void writeOutputBufferHex(unsigned char n);
void sendOutputBufferToHost();

// counter related
void counter_init();
void counter_wait(unsigned char time);
unsigned short counter_get();

enum CommandState
{
	AWAITING_COMMAND = 0,
	STARTING_COMMAND,
	EXECUTING_COMMAND
};




#endif /* ECD300_TEST_H_ */