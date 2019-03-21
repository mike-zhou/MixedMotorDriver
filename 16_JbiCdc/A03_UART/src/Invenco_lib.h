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

//Only 2 communication channels are available, USB and UART.
//If the following is defined, ScsDataExchange interacts with host through USB, 
//otherwise with UART.
#define DATA_EXCHANGE_THROUGH_USB

// functions to send information to the monitor in host
void printString(char * pString);
void printHex(unsigned char hex);

// functions app uses to exchange data with host
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

#define SCS_PACKET_LENGTH 64
#define SCS_DATA_PACKET_TAG 0xDD
/************************************************************************/
/* 
Data packet structure:
	0xDD		// 1 byte
	id			// 1 byte, 0 to 254
	dataLength	// 1 byte
	data bytes	// bytes of dataLength
	padding		// bytes of (SCS_PACKET_LENGTH - 5 - dataLength)
	crcLow		// 1 byte
	crcHigh		// 1 byte                                                                    
*/
/************************************************************************/
#define SCS_ACK_PACKET_TAG 0xAA
/************************************************************************/
/*          
Acknowledge packet structure:
	0xAA		// 1 byte
	id			// 1 byte, 0 to 254
	padding		// bytes of SCS_PACKET_LENGTH
	crcLow      // 1 byte
	crcHigh		// 1 byte                                           
*/
/************************************************************************/
#define SCS_DATA_INPUT_TIMEOUT 20 //milliseconds
#define SCS_DATA_ACK_TIMEOUT 50 //milliseconds
#define SCS_INITIAL_PACKET_ID 0 //this id is used only once at the launch of application
#define SCS_INVALID_PACKET_ID 0xFF
#define SCS_INPUT_STAGE_DATA_BUFFER_LENGTH 0x7f
enum SCS_Input_Packet_State
{
	SCS_INPUT_RECEIVING = 0,
	SCS_INPUT_ACKNOWLEDGING
};
struct SCS_Input_Stage
{
	unsigned char packetBuffer[SCS_PACKET_LENGTH];
	enum SCS_Input_Packet_State state;
	unsigned char packetByteAmount;
	unsigned short timeStamp;
	unsigned char previousId;
	
	unsigned char dataBuffer[SCS_INPUT_STAGE_DATA_BUFFER_LENGTH];
	bool dataBufferOverflow;
	unsigned char dataBufferReadIndex;
	unsigned char dataBufferWriteIndex;
};

enum SCS_Output_Packet_State
{
	SCS_OUTPUT_IDLE = 0,
	SCS_OUTPUT_SENDING,
	SCS_OUTPUT_WAITING_ACK,
	SCS_OUTPUT_WAITING_ACK_AND_SENDING //only ACK packet can be sent in this state
};

struct SCS_Output_Stage
{
	unsigned char packetBuffer[SCS_PACKET_LENGTH];
	unsigned char deliveryBuffer[SCS_PACKET_LENGTH];
	enum SCS_Output_Packet_State state;
	unsigned char deliveryIndex;
	unsigned short timeStamp;
	unsigned char packetId;
};

void initScsDataExchange(void);
void pollScsDataExchange(void);
//read a received byte
bool getScsInputData(unsigned char * pData);

#endif /* ECD300_TEST_H_ */