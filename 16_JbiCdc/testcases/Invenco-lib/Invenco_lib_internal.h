/*
 * Invenco_lib_internal.h
 *
 * Created: 2/23/2021 10:46:40 AM
 *  Author: MikeZ
 */ 


#ifndef INVENCO_LIB_INTERNAL_H_
#define INVENCO_LIB_INTERNAL_H_

#define MONITOR_OUTPUT_BUFFER_LENGTH_MASK 0xFF
#define APP_INPUT_BUFFER_LENGTH_MASK 0xFF
#define APP_OUTPUT_BUFFER_LENGTH_MASK 0xFF

/*********************************************************
* Data exchange stages
**********************************************************/
#define SCS_PACKET_MAX_LENGTH 64
#define SCS_DATA_PACKET_TAG 0xDD
/************************************************************************/
/* 
Data packet structure:
	0xDD		// 1 byte
	id			// 1 byte: 0, 1 to 0xFE
	dataLength	// 1 byte
	data bytes	// bytes of dataLength
	crcLow		// 1 byte
	crcHigh		// 1 byte                                                                    
*/
/************************************************************************/
#define SCS_ACK_PACKET_TAG 0xAA
/************************************************************************/
/*          
Acknowledge packet structure:
	0xAA		// 1 byte
	id			// 1 byte: 0, 1 to 0xFE
	crcLow      // 1 byte
	crcHigh		// 1 byte                                           
*/
/************************************************************************/
#define SCS_DATA_PACKET_STAFF_LENGTH 5 //tag, id, dataLength, crcLow, crcHigh
#define SCS_DATA_MAX_LENGTH (SCS_PACKET_MAX_LENGTH - SCS_DATA_PACKET_STAFF_LENGTH)
#define SCS_ACK_PACKET_LENGTH 4
#define SCS_DATA_INPUT_TIMEOUT 50 //milliseconds
#define SCS_DATA_OUTPUT_TIMEOUT 200 //milliseconds
#define SCS_INITIAL_PACKET_ID 0 //this id is used only once at the launch of application
#define SCS_INVALID_PACKET_ID 0xFF
enum SCS_Input_Stage_State
{
	SCS_INPUT_IDLE = 0,
	SCS_INPUT_RECEIVING
};
struct SCS_Input_Stage
{
	enum SCS_Input_Stage_State state;
	unsigned char packetBuffer[SCS_PACKET_MAX_LENGTH];
	unsigned char byteAmount;
	unsigned short timeStamp;
	unsigned char prevDataPktId;
};

enum SCS_Output_Packet_State
{
	SCS_OUTPUT_IDLE = 0,
	SCS_OUTPUT_SENDING_DATA,
	SCS_OUTPUT_SENDING_ACK,
	SCS_OUTPUT_SENDING_DATA_PENDING_ACK,
	SCS_OUTPUT_WAIT_ACK,
	SCS_OUTPUT_SENDING_ACK_WAIT_ACK
};

struct SCS_Output_Stage
{
	enum SCS_Output_Packet_State state;
	//data packet
	unsigned char dataPktBuffer[SCS_PACKET_MAX_LENGTH];
	unsigned char dataPktSendingIndex; //index of byte to be sent
	unsigned char currentDataPktId;
	unsigned char ackedDataPktId;
	unsigned short dataPktTimeStamp;
	//acknowledge packet
	unsigned char ackPktBuffer[SCS_ACK_PACKET_LENGTH];
	unsigned char ackPktSendingIndex; //index of byte to be sent
};

#define USB_INPUT_BUFFER_SIZE 64


#endif /* INVENCO_LIB_INTERNAL_H_ */