/*
 * testcases.c
 *
 *  Created on: 2/03/2021
 *      Author: user1
 */

#include <string.h>

#include "Invenco_lib.h"
#include "Invenco_lib_internal.h"
#include "mock_functions.h"

static void _resetTestEnv()
{
	usbClearInputBuffer();
	usbClearOutputBuffer();
	uartReset();
	monitorOutputBufferReset();
	sysclk_init();

	Invenco_init();
}

/**
 * create an ACK packet starting from pPacketBuffer
 * return actual length of packet
 */
static int _createAckPacket(unsigned char packetId, unsigned char * pPacketBuffer, int length)
{
	unsigned long crc;

	ASSERT(length >= 4);
	ASSERT(pPacketBuffer != NULL);

	pPacketBuffer[0] = SCS_ACK_PACKET_TAG;
	pPacketBuffer[1] = packetId;
	crc_set_initial_value(0xFFFF);
	crc = crc_io_checksum(pPacketBuffer, 2, 0);
	pPacketBuffer[2] = crc & 0xff;
	pPacketBuffer[3] = (crc >> 8) & 0xff;

	return 4;
}

/***
 * create a DATA packet starting from pPacketBuffer
 * return actual length of packet.
 */
static int _createDataPacket(unsigned char packetId, unsigned char * pData, unsigned char dataLength, unsigned char * pPacketBuffer, int packetBufferLength)
{
	unsigned long crc;

	ASSERT((dataLength + SCS_DATA_PACKET_STAFF_LENGTH) <= packetBufferLength);
	ASSERT(pPacketBuffer != NULL);
	ASSERT(dataLength <= SCS_DATA_MAX_LENGTH);

	pPacketBuffer[0] = SCS_DATA_PACKET_TAG;
	pPacketBuffer[1] = packetId;
	pPacketBuffer[2] = dataLength;
	if(dataLength > 0) 
	{
		ASSERT(pData != NULL);
		for(int i=0; i<dataLength; i++) {
			pPacketBuffer[3 + i] = pData[i];
		}
	}
	crc_set_initial_value(0xFFFF);
	crc = crc_io_checksum(pPacketBuffer, 3 + dataLength, 0);
	pPacketBuffer[3+dataLength] = crc & 0xff;
	pPacketBuffer[3+dataLength+1] = (crc >> 8) & 0xff;

	return dataLength + SCS_DATA_PACKET_STAFF_LENGTH;
}

static void _templete_testcase_function()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	_resetTestEnv();

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/***
 * Test time and counter functionality.
 */
static void _001000_tc()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	////////////////////////////////////
	// one clock
	////////////////////////////////////
	{
		_resetTestEnv();
		unsigned short curTc = counter_get();
		tcOneClock();
		ASSERT(counter_diff(curTc) == 1);
	}

	///////////////////////////////////
	// more clocks before turning around
	///////////////////////////////////
	{
		_resetTestEnv();
		unsigned short curTc = counter_get();
		tcClocks(0x1FF);
		ASSERT(counter_diff(curTc) == 0x1FF);

		_resetTestEnv();
		curTc = counter_get();
		tcClocks(0xFFFE);
		ASSERT(counter_diff(curTc) == 0xFFFE);

		_resetTestEnv();
		tcClocks(0xFFF0);
		curTc = counter_get();
		ASSERT(curTc == 0xFFF0);
		tcClocks(0xE);
		ASSERT(counter_diff(curTc) == 0xE);
	}
	
	//////////////////////////////////
	// clocks with turning around
	//////////////////////////////////
	{
		unsigned short tc;

		_resetTestEnv();
		tc = counter_get();
		ASSERT(tc == 0);
		tcClocks(0xFFF0);
		tc = counter_get();
		ASSERT(tc == 0xFFF0);
		ASSERT(counter_diff(0) == 0xFFF0);
		tcClocks(0xF);
		ASSERT(counter_diff(tc) == 0xF);
		tcClocks(0xF);
		ASSERT(counter_diff(tc) == (0xF + 0xF));
		tc = counter_get();
		ASSERT(tc == 0xE);
		tcOneClock();
		ASSERT(counter_diff(tc) == 1);
		tcOneClock();
		ASSERT(counter_diff(tc) == 2);
	}

	/////////////////////////////////
	// clocks with more turns
	/////////////////////////////////
	{
		unsigned short tc;

		_resetTestEnv();
		tc = counter_get();
		ASSERT(tc == 0);
		tcOneClock();
		tcClocks(0x10000);
		ASSERT(counter_diff(tc) == 1);
		tcClocks(0x10000);
		ASSERT(counter_diff(tc) == 1);
	}

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/***
 * test whether input stage can handle one incoming byte correctly
 */
static void _000001_uart_oneByteInput()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	unsigned char c ;
	int rc;

	/////////////////////
	/// one byte arrive at UART.
	// this byte should be ignored in IDLE state.
	/////////////////////
	_resetTestEnv();
	c = '1';
	rc = uartProduceData(&c, 1);
	ASSERT(rc == 1);
	ASSERT(uartInputBufferConsumerIndex() == 0);
	ASSERT(uartInputBufferProducerIndex() == 1);
	//
	pollScsDataExchange();
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	//
	ASSERT(uartInputBufferProducerIndex() == 1);

	////////////////////////////////////////
	// data tag arrive at UART
	// input stage should change to RECEIVING state
	////////////////////////////////////////
	c = SCS_DATA_PACKET_TAG;
	rc = uartProduceData(&c, 1);
	ASSERT(rc == 1);
	ASSERT(uartInputBufferConsumerIndex() == 1);
	ASSERT(uartInputBufferProducerIndex() == 2);
	//
	pollScsDataExchange();
	ASSERT(inputStageState() == SCS_INPUT_RECEIVING);
	ASSERT(inputStageUsed() == 1);
	//
	ASSERT(uartInputBufferProducerIndex() == 2);

	/////////////////////////////////////
	// ack tag arrive at UART
	// input stage should change to RECEIVING state,
	// then change back to IDLE state after time out
	/////////////////////////////////////
	_resetTestEnv();
	c = SCS_ACK_PACKET_TAG;
	rc = uartProduceData(&c, 1);
	ASSERT(rc == 1);
	ASSERT(uartInputBufferConsumerIndex() == 0);
	ASSERT(uartInputBufferProducerIndex() == 1);
	//
	pollScsDataExchange();
	ASSERT(inputStageState() == SCS_INPUT_RECEIVING);
	//
	ASSERT(uartInputBufferProducerIndex() == 1);
	ASSERT(inputStageUsed() == 1);

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/**
 * test whether input stage can handle timeout correctly
 */
static void _000002_uart_inputStageTimeout()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	unsigned char c ;
	int rc;

	/////////////////////////////////////
	// ack tag arrive at UART
	// input stage should change to RECEIVING state,
	// then change back to IDLE state after time out
	/////////////////////////////////////
	_resetTestEnv();
	c = SCS_ACK_PACKET_TAG;
	rc = uartProduceData(&c, 1);
	ASSERT(rc == 1);
	ASSERT(uartInputBufferConsumerIndex() == 0);
	ASSERT(uartInputBufferProducerIndex() == 1);
	//
	pollScsDataExchange();
	ASSERT(inputStageState() == SCS_INPUT_RECEIVING);
	//
	ASSERT(uartInputBufferProducerIndex() == 1);
	ASSERT(inputStageUsed() == 1);
	// test time out
	tcClocks(inputStageTimeoutValue() + 1);
	pollScsDataExchange();
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	{
		unsigned char buf[64];
		char * pErrStr = "ERROR: input stage timed out\r\n";

		memset(buf, 0, 64);
		usbConsumeData(buf, 64);
		ASSERT(strcmp((char *)buf, pErrStr) == 0);
	}

	////////////////////////////////
	// a partial ACK packet with some noise at the beginning
	// noising bytes should be ignored, and packet content should be saved in input stage.
	// the partial packet should be deleted after time out
	///////////////////////////////
	{
		_resetTestEnv();

		unsigned char buffer[16];
		unsigned char stageBuffer[16];
		unsigned long crc;
		int rc;

		buffer[0] = 0x00;//noise data
		buffer[1] = 0x01;//noise data
		buffer[2] = 0x02;//noise data
		buffer[3] = SCS_ACK_PACKET_TAG;
		buffer[4] = 0x00; //packet id
		crc_set_initial_value(0xFFFF);
		crc = crc_io_checksum(buffer + 3, 2, 0);
		buffer[5] = crc&0xff; //crc low

		rc = uartProduceData(buffer, 6);
		ASSERT(rc == 6);
		ASSERT(uartInputBufferConsumerIndex() == 0);
		ASSERT(uartInputBufferProducerIndex() == 6);
		//noise data
		for(int i=0; i<3; i++) {
			pollScsDataExchange();
			ASSERT(inputStageState() == SCS_INPUT_IDLE);
		}
		//packet data
		for(int i=0; i<3; i++) {
			pollScsDataExchange();
			ASSERT(inputStageState() == SCS_INPUT_RECEIVING);
			ASSERT(inputStageUsed() == (i+1));
		}
		//check data in input stage
		rc = inputStageCopyData(stageBuffer, 16);
		ASSERT(rc == 3);
		for(int i=0; i<3; i++) {
			ASSERT(stageBuffer[i] == buffer[3+i]);
		}
		// test time out
		tcClocks(inputStageTimeoutValue() + 1);
		pollScsDataExchange();
		ASSERT(inputStageState() == SCS_INPUT_IDLE);
		{
			unsigned char buf[64];
			char * pErrStr = "ERROR: input stage timed out\r\n";

			memset(buf, 0, 64);
			usbConsumeData(buf, 64);
			ASSERT(strcmp((char *)buf, pErrStr) == 0);
		}
	}

	////////////////////////////////
	// a partial DATA packet with some noise at the beginning
	// noising bytes should be ignored, and packet content should be saved in input stage.
	// the partial packet should be deleted after time out
	///////////////////////////////
	{
		_resetTestEnv();

		unsigned char buffer[64];
		unsigned char stageBuffer[64];
		unsigned long crc;
		int rc;

		buffer[0] = 0x00;//noise data
		buffer[1] = 0x01;//noise data
		buffer[2] = 0x02;//noise data
		buffer[3] = SCS_DATA_PACKET_TAG;
		buffer[4] = 0x00; //packet id
		buffer[5] = 10; //length
		for(int i=0; i<10; i++) {
			buffer[6+i] = 'a' + i;
		}
		crc_set_initial_value(0xFFFF);
		crc = crc_io_checksum(buffer + 3, 13, 0);
		buffer[16] = crc & 0xff;

		rc = uartProduceData(buffer, 17);
		ASSERT(rc == 17);
		ASSERT(uartInputBufferConsumerIndex() == 0);
		ASSERT(uartInputBufferProducerIndex() == 17);
		//noise data
		for(int i=0; i<3; i++) {
			pollScsDataExchange();
			ASSERT(inputStageState() == SCS_INPUT_IDLE);
		}
		//packet data
		for(int i=0; i<14; i++) {
			pollScsDataExchange();
			ASSERT(inputStageState() == SCS_INPUT_RECEIVING);
			ASSERT(inputStageUsed() == (i+1));
		}
		//check data in input stage
		rc = inputStageCopyData(stageBuffer, 64);
		ASSERT(rc == 14);
		for(int i=0; i<14; i++) {
			ASSERT(stageBuffer[i] == buffer[3+i]);
		}
		// test time out
		tcClocks(inputStageTimeoutValue() + 1);
		pollScsDataExchange();
		ASSERT(inputStageState() == SCS_INPUT_IDLE);
		{
			unsigned char buf[64];
			char * pErrStr = "ERROR: input stage timed out\r\n";

			memset(buf, 0, 64);
			usbConsumeData(buf, 64);
			ASSERT(strcmp((char *)buf, pErrStr) == 0);
		}
	}

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/**
 * test whehter input stage can handle an ACK packet.
 */
static void _000003_uart_completeAckPacket()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	_resetTestEnv();

	unsigned char buffer[16];
	unsigned char stageBuffer[16];
	unsigned char usbBuffer[64];
	unsigned long crc;
	int rc;

	buffer[0] = 0x00;//noise data
	buffer[1] = 0x01;//noise data
	buffer[2] = 0x02;//noise data
	buffer[3] = SCS_ACK_PACKET_TAG;
	buffer[4] = 0x0A; //packet id
	crc_set_initial_value(0xFFFF);
	crc = crc_io_checksum(buffer + 3, 2, 0);
	buffer[5] = crc&0xff; //crc low
	buffer[6] = (crc >> 8) & 0xff; //crc high
	buffer[7] = 0x03;//noise data
	buffer[8] = 0x04;//noise data
	buffer[9] = 0x05;//noise data

	rc = uartProduceData(buffer, 10);
	ASSERT(rc == 10);
	ASSERT(uartInputBufferConsumerIndex() == 0);
	ASSERT(uartInputBufferProducerIndex() == 10);
	//noise data
	for(int i=0; i<3; i++) {
		pollScsDataExchange();
		ASSERT(inputStageState() == SCS_INPUT_IDLE);
	}
	//packet data
	for(int i=0; i<3; i++) {
		pollScsDataExchange();
		ASSERT(inputStageState() == SCS_INPUT_RECEIVING);
		ASSERT(inputStageUsed() == (i+1));
	}
	//check data in input stage
	rc = inputStageCopyData(stageBuffer, 16);
	ASSERT(rc == 3);
	for(int i=0; i<3; i++) {
		ASSERT(stageBuffer[i] == buffer[3+i]);
	}
	//process byte of crc high
	pollScsDataExchange();
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageAckedDataPktId() == 0x0A);
	ASSERT(outputStageState() == SCS_OUTPUT_IDLE);
	//noise data at tail.
	for(int i=0; i<3;i++) {
		pollScsDataExchange();
	}
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_IDLE);
	memset(usbBuffer, 0, 64);
	usbConsumeData(usbBuffer, 64);
	ASSERT(strcmp(usbBuffer, "> A 0A\r\n") == 0);

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/**
 * send a data packet with noise to input stage.
 */
static void _uart_completeDataPacket()
{
	unsigned char buffer[64];
	unsigned char uartOutputBuffer[64];
	unsigned char monitorBuffer[64];
	unsigned char ackPktBuf[4];
	unsigned char appBuffer[64];
	int rc, length;

	memset(buffer, 0, 64);
	memset(uartOutputBuffer, 0, 64);
	memset(monitorBuffer, 0, 64);
	memset(appBuffer, 0, 64);
	//some noise data at the beginning
	buffer[0] = 'a';
	buffer[1] = 'b';
	//data packet
	char * pChars = "hello world";
	length = _createDataPacket(1, pChars, strlen(pChars), buffer + 2, 62);
	//some noise data at the tail
	buffer[2 + length] = 'c';
	buffer[2 + length + 1] = 'd';
	//data packet
	uartProduceData(buffer, 2 + length + 2);
	pollScsDataExchange(); //first byte
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	pollScsDataExchange(); //second byte
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	for(int i=0; i<(length-1); i++) {
		pollScsDataExchange();
		ASSERT(inputStageState() == SCS_INPUT_RECEIVING);
	}
	pollScsDataExchange();//crc high
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_IDLE);
	//check ACK packet
	rc = uartConsumeData(uartOutputBuffer, 64);
	ASSERT(rc == 4);
	_createAckPacket(1, ackPktBuf, 4);
	for(int i=0; i<4; i++) {
		ASSERT(uartOutputBuffer[i] == ackPktBuf[i]);
	}
	//check monitor information
	rc = usbConsumeData(monitorBuffer, 64);
	ASSERT(rc == strlen("> D 01\r\n< A 01\r\n"));
	ASSERT(strcmp(monitorBuffer, "> D 01\r\n< A 01\r\n") == 0);
	//check APP buffer
	for(int i=0; ; i++)
	{
		unsigned char c = readInputBuffer();
		if(c == 0) {
			break;
		}
		appBuffer[i] = c;
	}
	ASSERT(strcmp(appBuffer, pChars) == 0);
	//tail noise
	pollScsDataExchange();//tail 1
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	pollScsDataExchange();//tail 2
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
}

/**
 * test whether input stage can handle a data packet
 */
static void _000004_uart_oneCompleteDataPacket()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	_resetTestEnv();
	_uart_completeDataPacket();

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/**
 * simulate the arrival of a data packet
 */
static void _uart_dataPacketArrive(unsigned char packetId, unsigned char * pData, int dataLength)
{
	unsigned char buffer[64];

	memset(buffer, 0, 64);
	_createDataPacket(packetId, pData, dataLength, buffer, 64);

	uartProduceData(buffer, 64);
	for(int i=0; i<64; i++) {
		pollScsDataExchange();
	}
}

/**
 * test whether input stage can handle a data packet
 */
static void _000005_uart_repeatedCompleteDataPacket()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	unsigned char buffer[SCS_DATA_MAX_LENGTH + 1];
	unsigned char pId = 0;
	unsigned char outputBuffer[64];
	unsigned char appBuffer[64];
	int rc;
	char monitorStr[64];
	unsigned char ackBuffer[64];

	_resetTestEnv();
	
	for(int i=0; i<0xFFFF; i++) 
	{
		unsigned char len = i % (SCS_DATA_MAX_LENGTH + 1);

		memset(buffer, 0, sizeof(buffer));
		memset(outputBuffer, 0, 64);
		memset(appBuffer, 0, 64);
		memset(monitorStr, 0, 64);

		for(int j=0; j<len; j++) {
			buffer[j] = len;
		}

		_uart_dataPacketArrive(pId, buffer, len);

		//check ACK packet
		rc = _createAckPacket(pId, ackBuffer, 64);
		ASSERT(rc == 4);
		rc = uartConsumeData(outputBuffer, 64);
		ASSERT(rc == 4);
		for(int j=0; j<4; j++) {
			ASSERT(ackBuffer[j] == outputBuffer[j]);
		}
		//check monitor information
		rc = usbConsumeData(outputBuffer, 64);
		ASSERT(rc == 16);
		sprintf(monitorStr, "> D %02X\r\n< A %02X\r\n", pId, pId);
		ASSERT(strcmp(outputBuffer, monitorStr) == 0);
		//check APP data
		for(int j=0; ;j++) 
		{
			unsigned char c = readInputBuffer();
			if(c == 0) {
				break;
			}
			appBuffer[j] = c;
		} 
		for(int j=0; j<len; j++) {
			ASSERT(buffer[j] == appBuffer[j]);
		}

		//increase packet id
		pId++;
		if(pId == SCS_INVALID_PACKET_ID) {
			pId = 1;
		}
	}

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

static void _000006_uart_duplicatedDataPacket()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	unsigned char buffer[128];
	unsigned char ackBuffer[128];
	unsigned char monitorBuffer[128];
	char monitorStr[128];
	unsigned char appBuffer[128];
	int rc;
	unsigned char packetId = 0;
	
	_resetTestEnv();

	for(int loop=0; loop<0x1ffff; loop++)
	{
		char * pData = "hello world, how are you?";
		_uart_dataPacketArrive(packetId, pData, strlen(pData));
		rc = _createAckPacket(packetId, ackBuffer, 64);
		ASSERT(rc == 4);
		rc = uartConsumeData(buffer, 64);
		ASSERT(rc == 4);
		for(int j=0; j<4; j++) {
			ASSERT(ackBuffer[j] == buffer[j]);
		}
		//check monitor information
		memset(buffer, 0, 64);
		rc = usbConsumeData(buffer, 64);
		ASSERT(rc == 16);
		sprintf(monitorStr, "> D %02X\r\n< A %02X\r\n", packetId, packetId);
		ASSERT(strcmp(buffer, monitorStr) == 0);
		//check APP data
		memset(appBuffer, 0, sizeof(appBuffer));
		for(int j=0; ;j++) 
		{
			unsigned char c = readInputBuffer();
			if(c == 0) {
				break;
			}
			appBuffer[j] = c;
		} 
		for(int j=0; ; j++) {
			if(appBuffer[j] == 0) {
				break;
			}
			ASSERT(pData[j] == appBuffer[j]);
		}
		///////////  second packet ///////////////////
		_uart_dataPacketArrive(packetId, pData, strlen(pData));
		rc = _createAckPacket(packetId, ackBuffer, 64);
		ASSERT(rc == 4);
		rc = uartConsumeData(buffer, 64);
		ASSERT(rc == 4);
		for(int j=0; j<4; j++) {
			ASSERT(ackBuffer[j] == buffer[j]);
		}
		//check monitor information
		memset(buffer, 0, 64);
		rc = usbConsumeData(buffer, 64);
		ASSERT(rc == 16);
		sprintf(monitorStr, "> D %02X\r\n< A %02X\r\n", packetId, packetId);
		ASSERT(strcmp(buffer, monitorStr) == 0);
		//check APP data
		ASSERT(readInputBuffer() == 0);
		///////////  3rd packet ///////////////////
		_uart_dataPacketArrive(packetId, pData, strlen(pData));
		rc = _createAckPacket(packetId, ackBuffer, 64);
		ASSERT(rc == 4);
		rc = uartConsumeData(buffer, 64);
		ASSERT(rc == 4);
		for(int j=0; j<4; j++) {
			ASSERT(ackBuffer[j] == buffer[j]);
		}
		//check monitor information
		memset(buffer, 0, 64);
		rc = usbConsumeData(buffer, 64);
		ASSERT(rc == 16);
		sprintf(monitorStr, "> D %02X\r\n< A %02X\r\n", packetId, packetId);
		ASSERT(strcmp(buffer, monitorStr) == 0);
		//check APP data
		ASSERT(readInputBuffer() == 0);
		///////////  4th packet ///////////////////
		_uart_dataPacketArrive(packetId, pData, strlen(pData));
		rc = _createAckPacket(packetId, ackBuffer, 64);
		ASSERT(rc == 4);
		rc = uartConsumeData(buffer, 64);
		ASSERT(rc == 4);
		for(int j=0; j<4; j++) {
			ASSERT(ackBuffer[j] == buffer[j]);
		}
		//check monitor information
		memset(buffer, 0, 64);
		rc = usbConsumeData(buffer, 64);
		ASSERT(rc == 16);
		sprintf(monitorStr, "> D %02X\r\n< A %02X\r\n", packetId, packetId);
		ASSERT(strcmp(buffer, monitorStr) == 0);
		//check APP data
		ASSERT(readInputBuffer() == 0);

		packetId++;
		if(packetId == SCS_INVALID_PACKET_ID) {
			packetId = 1;
		}
	}

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

static void _000007_uart_discontinuousPakcetId()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	unsigned char buffer[128];
	unsigned char ackBuffer[128];
	unsigned char monitorBuffer[128];
	char monitorStr[128];
	unsigned char appBuffer[128];
	int rc;
	unsigned char packetId = 0;

	_resetTestEnv();

	const char * pData = "hello world, how are you?";
	_uart_dataPacketArrive(packetId, pData, strlen(pData));
	rc = _createAckPacket(packetId, ackBuffer, 64);
	ASSERT(rc == 4);
	rc = uartConsumeData(buffer, 64);
	ASSERT(rc == 4);
	for(int j=0; j<4; j++) {
		ASSERT(ackBuffer[j] == buffer[j]);
	}
	//check monitor information
	memset(buffer, 0, 64);
	rc = usbConsumeData(buffer, 64);
	ASSERT(rc == 16);
	sprintf(monitorStr, "> D %02X\r\n< A %02X\r\n", packetId, packetId);
	ASSERT(strcmp(buffer, monitorStr) == 0);
	//check APP data
	memset(appBuffer, 0, sizeof(appBuffer));
	for(int j=0; ;j++) 
	{
		unsigned char c = readInputBuffer();
		if(c == 0) {
			break;
		}
		appBuffer[j] = c;
	} 
	for(int j=0; ; j++) {
		if(appBuffer[j] == 0) {
			break;
		}
		ASSERT(pData[j] == appBuffer[j]);
	}

	packetId = 2;
	_uart_dataPacketArrive(packetId, pData, strlen(pData));
	//no ACK for discontinuous packet id
	//check monitor information
	sprintf(monitorStr, "> D 02\r\nERROR: unexpected host packetId 02 expect: 01\r\n");
	memset(buffer, 0, 64);
	rc = usbConsumeData(buffer, 64);
	ASSERT(rc == strlen(monitorStr));
	ASSERT(strcmp(buffer, monitorStr) == 0);

	packetId = 1;
	_uart_dataPacketArrive(packetId, pData, strlen(pData));
	rc = _createAckPacket(packetId, ackBuffer, 64);
	ASSERT(rc == 4);
	rc = uartConsumeData(buffer, 64);
	ASSERT(rc == 4);
	for(int j=0; j<4; j++) {
		ASSERT(ackBuffer[j] == buffer[j]);
	}
	//check monitor information
	memset(buffer, 0, 64);
	rc = usbConsumeData(buffer, 64);
	ASSERT(rc == 16);
	sprintf(monitorStr, "> D %02X\r\n< A %02X\r\n", packetId, packetId);
	ASSERT(strcmp(buffer, monitorStr) == 0);
	//check APP data
	memset(appBuffer, 0, sizeof(appBuffer));
	for(int j=0; ;j++) 
	{
		unsigned char c = readInputBuffer();
		if(c == 0) {
			break;
		}
		appBuffer[j] = c;
	} 
	for(int j=0; ; j++) {
		if(appBuffer[j] == 0) {
			break;
		}
		ASSERT(pData[j] == appBuffer[j]);
	}

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

static void _000008_uart_crcError()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	_resetTestEnv();

	unsigned char buffer[64];
	unsigned char monitorStr[64];
	char * pStr = "hello world";
	int rc;

	/////// ACK packet ////////////////
	memset(buffer, 0, 64);
	memset(monitorStr, 0, 64);
	rc = _createAckPacket(5, buffer, 64);
	ASSERT(rc == 4);
	buffer[3] = 0; //make a crc error
	uartProduceData(buffer, 64);
	for(int i=0; i<64; i++) {
		pollScsDataExchange();
	}
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	sprintf(monitorStr, "ERROR: corrupted input ACK packet\r\n");
	rc = usbConsumeData(buffer, 64);
	ASSERT(rc == strlen(monitorStr));
	ASSERT(strcmp(buffer, monitorStr) == 0);

	////// data packet ////////////////
	memset(buffer, 0, 64);
	memset(monitorStr, 0, 64);
	rc = _createDataPacket(5, pStr, strlen(pStr), buffer, 64);
	buffer[rc - 1] = 0; //make a crc error
	uartProduceData(buffer, 64);
	for(int i=0; i<64; i++) {
		pollScsDataExchange();
	}
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	sprintf(monitorStr, "ERROR: corrupted input data packet\r\n");
	rc = usbConsumeData(buffer, 64);
	ASSERT(rc == strlen(monitorStr));
	ASSERT(strcmp(buffer, monitorStr) == 0);

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

static void _000009_uart_illegalPacketLength()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	unsigned char buffer[64];
	unsigned char monitorBuffer[64];
	int rc;

	_resetTestEnv();

	memset(buffer, 0, 64);
	memset(monitorBuffer, 0, 64);
	rc = _createDataPacket(5, 0, 0, buffer, 64);
	ASSERT(rc == 5);
	buffer[2] = 0x80;
	uartProduceData(buffer, 64);
	for(int i=0; i<64; i++) {
		pollScsDataExchange();
	}
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	sprintf(buffer, "ERROR: illegal input data packet length 80\r\n");
	rc = usbConsumeData(monitorBuffer, 64);
	ASSERT(rc == strlen(buffer));
	ASSERT(strcmp(buffer, monitorBuffer) == 0);

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}



void startTestCases()
{
	_001000_tc();
	_000001_uart_oneByteInput();
	_000002_uart_inputStageTimeout();
	_000003_uart_completeAckPacket();
	_000004_uart_oneCompleteDataPacket();
	_000005_uart_repeatedCompleteDataPacket();
	_000006_uart_duplicatedDataPacket();
	_000007_uart_discontinuousPakcetId();
	_000008_uart_crcError();
	_000009_uart_illegalPacketLength();
}
