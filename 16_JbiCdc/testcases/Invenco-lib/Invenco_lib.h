/*
 * ECD300_test.h
 *
 * Created: 2014/12/20 1:41:41
 *  Author: user
 */ 

#ifndef ECD300_TEST_H_
#define ECD300_TEST_H_

#include "asf.h"
#include "ECD300.h"
#include "usb_protocol_cdc.h"
#include "tc.h"

//Only 2 communication channels are available, USB and UART.
//If the following is defined as 1, ScsDataExchange interacts with host through USB, 
//otherwise with UART.
#define DATA_EXCHANGE_THROUGH_USB 0

// API for APPs to send information to the monitor in host
void printString(char * pString);
void printHex(unsigned char hex);
void inline printChar(unsigned char c);

// API for APPs to exchange data with host
void clearInputBuffer(void);
unsigned char readInputBuffer(void);
bool writeOutputBufferChar(unsigned char c);
void writeOutputBufferString(const char * pString);
void writeOutputBufferHex(unsigned char n);
void enableOutputBuffer(void);

//APP call this function to initialize underlying data exchange mechanism
void initScsDataExchange(void);
//APP call this function periodically to drive the underlying data exchange
void pollScsDataExchange(void);



//////////////////////////////////
// counter related
//////////////////////////////////

//initialize counter
void counter_init(void);
//wait for 1/time second
void counter_wait(unsigned char time);
//return current counter value
unsigned short counter_get(void);
//return difference between current counter and parameter
unsigned short counter_diff(unsigned short prevCounter);
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
