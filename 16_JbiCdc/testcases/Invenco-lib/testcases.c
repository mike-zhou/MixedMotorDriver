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
static void _004000_tc()
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
 * test that input stage can handle a data packet
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
 * test that input stage can handle successive data packets
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

/**
 * test that input stage filter out data packet with duplicated packet id
 */
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

/**
 * test that input stage can ignore a discontinuous data packet.
 */
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

/**
 * test that input stage can handle crc error.
 */
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

/**
 * test that input stage can handle a data packet with illegal length.
 */
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

/**
 * test that input stage can accept data packet with a non-zero packet id
 */
static void _000010_uart_abruptPacketId()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	_resetTestEnv();

	unsigned char * pChars = "hello world";
	unsigned char buffer[64];
	unsigned char monitorBuffer[64];
	unsigned char appBuffer[64];
	int rc;

	memset(buffer, 0, 64);
	memset(monitorBuffer, 0, 64);
	memset(appBuffer, 0, 64);
	// a data packet with id 5
	_uart_dataPacketArrive(5, pChars, strlen(pChars));
	rc = usbConsumeData(monitorBuffer, 64);
	ASSERT(rc == 16);
	sprintf(buffer, "> D 05\r\n< A 05\r\n");
	ASSERT(strcmp(buffer, monitorBuffer) == 0);
	for(int i=0;;i++)
	{
		unsigned char c = readInputBuffer();
		if(c == 0) {
			break;
		}
		appBuffer[i] = c;
	}
	ASSERT(strcmp(pChars, appBuffer) == 0);

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/**
 * App sends some data to host
 */
static void _001000_uart_sendAppData()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	_resetTestEnv();

	unsigned char buffer[64];
	unsigned char monitorBuffer[64];
	unsigned long crc;
	int rc;

	unsigned char * pChars = "hello world";

	memset(buffer, 0, 64);
	memset(monitorBuffer, 0, 64);
	enableOutputBuffer();
	writeOutputBufferString(pChars);
	pollScsDataExchange(); // output idle -> sending data
	pollScsDataExchange(); // sending data -> waiting ack
	rc = uartConsumeData(buffer, 64);
	ASSERT(rc == (strlen(pChars) + SCS_DATA_PACKET_STAFF_LENGTH));
	crc_set_initial_value(0xffff);
	crc = crc_io_checksum(buffer, rc - 2, 0);
	ASSERT((crc & 0xff) == buffer[rc - 2]);
	ASSERT(((crc >> 8) & 0xff) == buffer[rc - 1]);
	ASSERT(buffer[2] == strlen(pChars));
	for(int i=0; i<strlen(pChars); i++) 
	{
		ASSERT(buffer[3+i] == pChars[i]);
	}
	sprintf(monitorBuffer, "< D 00\r\n");
	memset(buffer, 0, 64);
	rc = usbConsumeData(buffer, 64);
	ASSERT(rc == strlen(monitorBuffer));
	ASSERT(strcmp(monitorBuffer, buffer) == 0);

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/**
 * test that the output stage re-sends the data packet when no ACK in time.
 */
static void _001001_uart_sendAppDataTimeout()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	_resetTestEnv();

	unsigned char buffer[64];
	unsigned char monitorBuffer[64];
	unsigned long crc;
	int rc;
	int len;

	unsigned char * pChars = "hello world";

	memset(buffer, 0, 64);
	memset(monitorBuffer, 0, 64);
	enableOutputBuffer();
	writeOutputBufferString(pChars);
	pollScsDataExchange(); // output idle -> sending data
	pollScsDataExchange(); // sending data -> waiting ack
	rc = uartConsumeData(buffer, 64);
	ASSERT(rc == (strlen(pChars) + SCS_DATA_PACKET_STAFF_LENGTH));
	crc_set_initial_value(0xffff);
	crc = crc_io_checksum(buffer, rc - 2, 0);
	ASSERT((crc & 0xff) == buffer[rc - 2]);
	ASSERT(((crc >> 8) & 0xff) == buffer[rc - 1]);
	ASSERT(buffer[2] == strlen(pChars));
	for(int i=0; i<strlen(pChars); i++) 
	{
		ASSERT(buffer[3+i] == pChars[i]);
	}
	sprintf(monitorBuffer, "< D 00\r\n");
	memset(buffer, 0, 64);
	rc = usbConsumeData(buffer, 64);
	ASSERT(rc == strlen(monitorBuffer));
	ASSERT(strcmp(monitorBuffer, buffer) == 0);

	for(int i=0; i<0xffff; i++)
	{
		tcClocks(outputStageTimeoutValue() + 1);

		memset(buffer, 0, 64);
		memset(monitorBuffer, 0, 64);

		pollScsDataExchange(); // waiting ack -> sending data
		ASSERT(strcmp(monitorBuffer, buffer) == 0);
		pollScsDataExchange(); // sending data -> waiting ack
		pollScsDataExchange(); // send out all data in monitor stage
		rc = uartConsumeData(buffer, 64);
		ASSERT(rc == (strlen(pChars) + SCS_DATA_PACKET_STAFF_LENGTH));
		crc_set_initial_value(0xffff);
		crc = crc_io_checksum(buffer, rc - 2, 0);
		ASSERT((crc & 0xff) == buffer[rc - 2]);
		ASSERT(((crc >> 8) & 0xff) == buffer[rc - 1]);
		ASSERT(buffer[2] == strlen(pChars));
		for(int i=0; i<strlen(pChars); i++) 
		{
			ASSERT(buffer[3+i] == pChars[i]);
		}
		sprintf(monitorBuffer, "ERROR: host ACK time out, 00\r\n< D 00\r\n");
		memset(buffer, 0, 64);
		rc = usbConsumeData(buffer, 64);
		len = strlen(monitorBuffer);
		ASSERT(rc == len);
		ASSERT(strcmp(monitorBuffer, buffer) == 0);
	}

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/**
 * test that App data can be sent out and be acknowledged.
 */
static void _001002_uart_sendAppDataAround()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	_resetTestEnv();

	unsigned char buffer[64];
	unsigned char monitorBuffer[64];
	unsigned long crc;
	int rc;
	int len;

	unsigned char * pChars = "hello world";

	memset(buffer, 0, 64);
	memset(monitorBuffer, 0, 64);
	enableOutputBuffer();
	writeOutputBufferString(pChars);
	pollScsDataExchange(); // output idle -> sending data
	pollScsDataExchange(); // sending data -> waiting ack
	rc = uartConsumeData(buffer, 64);
	ASSERT(rc == (strlen(pChars) + SCS_DATA_PACKET_STAFF_LENGTH));
	crc_set_initial_value(0xffff);
	crc = crc_io_checksum(buffer, rc - 2, 0);
	ASSERT((crc & 0xff) == buffer[rc - 2]);
	ASSERT(((crc >> 8) & 0xff) == buffer[rc - 1]);
	ASSERT(buffer[2] == strlen(pChars));
	for(int i=0; i<strlen(pChars); i++) 
	{
		ASSERT(buffer[3+i] == pChars[i]);
	}
	sprintf(monitorBuffer, "< D 00\r\n");
	memset(buffer, 0, 64);
	rc = usbConsumeData(buffer, 64);
	ASSERT(rc == strlen(monitorBuffer));
	ASSERT(strcmp(monitorBuffer, buffer) == 0);
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_WAIT_ACK);

	tcClocks(outputStageTimeoutValue() / 2);

	rc = _createAckPacket(0, buffer, 64);
	ASSERT(rc == 4);
	uartProduceData(buffer, rc);
	for(int i=0; i<rc; i++) {
		pollScsDataExchange();
	}
	sprintf(buffer, "> A 00\r\n");
	rc = usbConsumeData(monitorBuffer, 64);
	len = strlen(buffer);
	ASSERT(rc == len);
	ASSERT(strcmp(buffer, monitorBuffer) == 0);
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_IDLE);	

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/**
 * test that App data can be sent out continuously
 */
static void _001003_uart_multiAppData()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	_resetTestEnv();

	unsigned char buffer[64];
	unsigned char monitorBuffer[64];
	unsigned long crc;
	int rc;
	int len;
	unsigned char packetId;
	unsigned char expectedId = 0;

	unsigned char * pChars = "hello world";

	for(int loop=0; loop<0xffff; loop++)
	{
		memset(buffer, 0, 64);
		memset(monitorBuffer, 0, 64);
		enableOutputBuffer();
		writeOutputBufferString(pChars);
		pollScsDataExchange(); // output idle -> sending data
		pollScsDataExchange(); // sending data -> waiting ack
		rc = uartConsumeData(buffer, 64);
		ASSERT(rc == (strlen(pChars) + SCS_DATA_PACKET_STAFF_LENGTH));
		crc_set_initial_value(0xffff);
		crc = crc_io_checksum(buffer, rc - 2, 0);
		ASSERT((crc & 0xff) == buffer[rc - 2]);
		ASSERT(((crc >> 8) & 0xff) == buffer[rc - 1]);
		ASSERT(buffer[2] == strlen(pChars));
		for(int i=0; i<strlen(pChars); i++) 
		{
			ASSERT(buffer[3+i] == pChars[i]);
		}
		packetId = buffer[1];
		ASSERT(packetId == expectedId);
		sprintf(monitorBuffer, "< D %02X\r\n", packetId);
		memset(buffer, 0, 64);
		rc = usbConsumeData(buffer, 64);
		ASSERT(rc == strlen(monitorBuffer));
		ASSERT(strcmp(monitorBuffer, buffer) == 0);
		ASSERT(inputStageState() == SCS_INPUT_IDLE);
		ASSERT(outputStageState() == SCS_OUTPUT_WAIT_ACK);

		tcClocks(outputStageTimeoutValue() / 2);

		rc = _createAckPacket(packetId, buffer, 64);
		ASSERT(rc == 4);
		uartProduceData(buffer, rc);
		for(int i=0; i<rc; i++) {
			pollScsDataExchange();
		}
		sprintf(buffer, "> A %02X\r\n", packetId);
		rc = usbConsumeData(monitorBuffer, 64);
		len = strlen(buffer);
		ASSERT(rc == len);
		ASSERT(strcmp(buffer, monitorBuffer) == 0);
		ASSERT(inputStageState() == SCS_INPUT_IDLE);
		ASSERT(outputStageState() == SCS_OUTPUT_IDLE);		

		expectedId++;
		if(expectedId == SCS_INVALID_PACKET_ID) {
			expectedId = 1;
		}	
	}

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/**
 * test that input stage and output stage can process data packet at the same time.
 * 	host data packet is coming
 * 	app data is ready
 * 	app data is being sent
 * 	ack packet is to be sent
 * 	app data packet finishes
 * 	ack packet is being sent
 * 	host ack is coming
 */
static void _001004_uart_dataBothWay()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	unsigned char uartInputBuffer[256];
	unsigned char uartOutputBuffer[256];
	unsigned char usbBuffer[256];
	unsigned char buffer[256];
	unsigned char * pAppData = "how are you";
	int rc;
	int length;

	_resetTestEnv();

	memset(uartInputBuffer, 0, sizeof(uartInputBuffer));
	memset(uartOutputBuffer, 0, sizeof(uartOutputBuffer));
	memset(usbBuffer, 0, sizeof(usbBuffer));
	memset(buffer, 0, sizeof(buffer));

	//fill up uart output buffer to make output stage not send out data packet
	uartOutputBufferSetProducerIndex(MOCK_UART_OUTPUT_BUFFER_MASK);
	//create data packet for input stage
	rc = _createDataPacket(0, 0, 0, buffer, 256);
	ASSERT(rc == SCS_DATA_PACKET_STAFF_LENGTH);
	uartProduceData(buffer, rc);
	//create data packet for output stage
	enableOutputBuffer();
	writeOutputBufferString(pAppData);

	//input stage receive data packet.
	pollScsDataExchange();
	ASSERT(inputStageState() == SCS_INPUT_RECEIVING);
	ASSERT(outputStageState() == SCS_OUTPUT_SENDING_DATA);
	pollScsDataExchange();
	pollScsDataExchange();
	pollScsDataExchange();
	pollScsDataExchange();
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_SENDING_DATA_PENDING_ACK);
	//check monitor content
	sprintf(buffer, "> D 00\r\n");
	length = strlen(buffer);
	memset(usbBuffer, 0, 256);
	rc = usbConsumeData(usbBuffer, 256);
	ASSERT(rc == length);
	ASSERT(strcmp(buffer, usbBuffer) == 0);
	//output stage sends out data packet.
	length = strlen(pAppData) + SCS_DATA_PACKET_STAFF_LENGTH;
	for(int i=0; i<length; i++) {
		uartConsumeData(buffer, 1);
		pollScsDataExchange();
	}
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_SENDING_ACK_WAIT_ACK);
	//check monitor content
	sprintf(buffer, "< D 00\r\n");
	length = strlen(buffer);
	memset(usbBuffer, 0, 256);
	rc = usbConsumeData(usbBuffer, 256);
	ASSERT(rc == length);
	ASSERT(strcmp(buffer, usbBuffer) == 0);
	//output stage sends out ack packet
	for(int i=0; i<4; i++) {
		uartConsumeData(buffer, 1);
		pollScsDataExchange();
	}
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_WAIT_ACK);
	//check monitor content
	sprintf(buffer, "< A 00\r\n");
	length = strlen(buffer);
	memset(usbBuffer, 0, 256);
	rc = usbConsumeData(usbBuffer, 256);
	ASSERT(rc == length);
	ASSERT(strcmp(buffer, usbBuffer) == 0);
	//check uart output buffer
	rc = _createDataPacket(0, pAppData, strlen(pAppData), buffer, 256);
	length = _createAckPacket(0, buffer+rc, 256);
	length = rc + length;//total length of data packet and ack packet
	rc = uartConsumeData(uartOutputBuffer, 256);
	ASSERT(rc == MOCK_USB_OUTPUT_BUFFER_MASK);
	for(int i=0; i<length; i++) {
		ASSERT(buffer[i] == uartOutputBuffer[MOCK_USB_OUTPUT_BUFFER_MASK - length + i]);
	}
	//ack app data packet
	rc = _createAckPacket(0, buffer, 256);
	ASSERT(rc == 4);
	uartProduceData(buffer, 4);
	for(int i=0; i<4; i++) {
		pollScsDataExchange();
	}
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_IDLE);
	//check monitor content
	sprintf(buffer, "> A 00\r\n");
	length = strlen(buffer);
	memset(usbBuffer, 0, 256);
	rc = usbConsumeData(usbBuffer, 256);
	ASSERT(rc == length);
	ASSERT(strcmp(buffer, usbBuffer) == 0);

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/**
 * test that App data is postponed until ACK packet is sent out.
 * 	host data packet is coming
 * 	ack packet is being sent out
 * 	App data is ready
 * 	ack packet is sent out
 * 	app data is being sent out
 * 	app data is sent out
 * 	host ack app data
 */
static void _001005_uart_appDataPostpone()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	unsigned char uartInputBuffer[256];
	unsigned char uartOutputBuffer[256];
	unsigned char usbBuffer[256];
	unsigned char buffer[256];
	unsigned char * pAppData = "how are you";
	int rc;
	int length;

	_resetTestEnv();

	memset(uartInputBuffer, 0, sizeof(uartInputBuffer));
	memset(uartOutputBuffer, 0, sizeof(uartOutputBuffer));
	memset(usbBuffer, 0, sizeof(usbBuffer));
	memset(buffer, 0, sizeof(buffer));

	//fill up uart output buffer to make output stage not send out data packet
	uartOutputBufferSetProducerIndex(MOCK_UART_OUTPUT_BUFFER_MASK);
	//create data packet for input stage
	rc = _createDataPacket(0, 0, 0, buffer, 256);
	ASSERT(rc == SCS_DATA_PACKET_STAFF_LENGTH);
	uartProduceData(buffer, rc);

	//input stage receive data packet.
	pollScsDataExchange();
	ASSERT(inputStageState() == SCS_INPUT_RECEIVING);
	ASSERT(outputStageState() == SCS_OUTPUT_IDLE);
	pollScsDataExchange();
	pollScsDataExchange();
	pollScsDataExchange();
	pollScsDataExchange();
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_SENDING_ACK);
	//check monitor content
	sprintf(buffer, "> D 00\r\n");
	length = strlen(buffer);
	memset(usbBuffer, 0, 256);
	rc = usbConsumeData(usbBuffer, 256);
	ASSERT(rc == length);
	ASSERT(strcmp(buffer, usbBuffer) == 0);
	//create data packet for output stage
	enableOutputBuffer();
	writeOutputBufferString(pAppData);
	//output stage sends out ACK packet.
	length = 4;
	for(int i=0; i<length; i++) {
		ASSERT(outputStageState() == SCS_OUTPUT_SENDING_ACK);
		uartConsumeData(buffer, 1);
		pollScsDataExchange();
	}
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_IDLE);
	//check monitor content
	sprintf(buffer, "< A 00\r\n");
	length = strlen(buffer);
	memset(usbBuffer, 0, 256);
	rc = usbConsumeData(usbBuffer, 256);
	ASSERT(rc == length);
	ASSERT(strcmp(buffer, usbBuffer) == 0);
	//output stage sends out data packet
	pollScsDataExchange();
	ASSERT(outputStageState() == SCS_OUTPUT_SENDING_DATA);
	length = strlen(pAppData) + SCS_DATA_PACKET_STAFF_LENGTH;
	for(int i=0; i<length; i++) {
		uartConsumeData(buffer, 1);
		pollScsDataExchange();
	}
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_WAIT_ACK);
	//check monitor content
	sprintf(buffer, "< D 00\r\n");
	length = strlen(buffer);
	memset(usbBuffer, 0, 256);
	rc = usbConsumeData(usbBuffer, 256);
	ASSERT(rc == length);
	ASSERT(strcmp(buffer, usbBuffer) == 0);
	//check data and ack packets in uart output buffer
	length = strlen(pAppData);
	rc = _createDataPacket(0, pAppData, length, buffer, 256);
	length = rc;
	rc = uartConsumeData(uartOutputBuffer, 256);
	ASSERT(rc == MOCK_USB_OUTPUT_BUFFER_MASK);
	for(int i=0; i<length; i++) {
		ASSERT(buffer[i] == uartOutputBuffer[MOCK_USB_OUTPUT_BUFFER_MASK - length + i]);
	}
	rc = _createAckPacket(0, buffer, 256);
	ASSERT(rc == 4);
	for(int i=0; i<rc; i++) {
		ASSERT(buffer[i] == uartOutputBuffer[MOCK_USB_OUTPUT_BUFFER_MASK - length - 4 + i]);
	}

	//ack app data packet
	rc = _createAckPacket(0, buffer, 256);
	ASSERT(rc == 4);
	uartProduceData(buffer, 4);
	for(int i=0; i<4; i++) {
		pollScsDataExchange();
	}
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_IDLE);
	//check monitor content
	sprintf(buffer, "> A 00\r\n");
	length = strlen(buffer);
	memset(usbBuffer, 0, 256);
	rc = usbConsumeData(usbBuffer, 256);
	ASSERT(rc == length);
	ASSERT(strcmp(buffer, usbBuffer) == 0);

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/**
 * test that input stage and output stage can process data packet at the same time.
 * 	host data packet is coming
 * 	app data is ready
 * 	app data is being sent
 * 	ack packet is to be sent
 * 	app data packet finishes
 * 	host ack is coming
 * 	ack packet is being sent
 */
static void _001006_uart_quickHostAck()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	unsigned char uartInputBuffer[256];
	unsigned char uartOutputBuffer[256];
	unsigned char usbBuffer[256];
	unsigned char buffer[256];
	unsigned char * pAppData = "how are you";
	int rc;
	int length;

	_resetTestEnv();

	memset(uartInputBuffer, 0, sizeof(uartInputBuffer));
	memset(uartOutputBuffer, 0, sizeof(uartOutputBuffer));
	memset(usbBuffer, 0, sizeof(usbBuffer));
	memset(buffer, 0, sizeof(buffer));

	//fill up uart output buffer to make output stage not send out data packet
	uartOutputBufferSetProducerIndex(MOCK_UART_OUTPUT_BUFFER_MASK);
	//create data packet for input stage
	rc = _createDataPacket(0, 0, 0, buffer, 256);
	ASSERT(rc == SCS_DATA_PACKET_STAFF_LENGTH);
	uartProduceData(buffer, rc);
	//create data packet for output stage
	enableOutputBuffer();
	writeOutputBufferString(pAppData);

	//input stage receive data packet.
	pollScsDataExchange();
	ASSERT(inputStageState() == SCS_INPUT_RECEIVING);
	ASSERT(outputStageState() == SCS_OUTPUT_SENDING_DATA);
	pollScsDataExchange();
	pollScsDataExchange();
	pollScsDataExchange();
	pollScsDataExchange();
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_SENDING_DATA_PENDING_ACK);
	//check monitor content
	sprintf(buffer, "> D 00\r\n");
	length = strlen(buffer);
	memset(usbBuffer, 0, 256);
	rc = usbConsumeData(usbBuffer, 256);
	ASSERT(rc == length);
	ASSERT(strcmp(buffer, usbBuffer) == 0);
	//output stage sends out data packet.
	length = strlen(pAppData) + SCS_DATA_PACKET_STAFF_LENGTH;
	for(int i=0; i<length; i++) {
		uartConsumeData(buffer, 1);
		pollScsDataExchange();
	}
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_SENDING_ACK_WAIT_ACK);
	//check monitor content
	sprintf(buffer, "< D 00\r\n");
	length = strlen(buffer);
	memset(usbBuffer, 0, 256);
	rc = usbConsumeData(usbBuffer, 256);
	ASSERT(rc == length);
	ASSERT(strcmp(buffer, usbBuffer) == 0);
	//ack app data packet
	rc = _createAckPacket(0, buffer, 256);
	ASSERT(rc == 4);
	uartProduceData(buffer, 4);
	for(int i=0; i<4; i++) {
		pollScsDataExchange();
	}
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_SENDING_ACK_WAIT_ACK);
	//check monitor content
	sprintf(buffer, "> A 00\r\n");
	length = strlen(buffer);
	memset(usbBuffer, 0, 256);
	rc = usbConsumeData(usbBuffer, 256);
	ASSERT(rc == length);
	ASSERT(strcmp(buffer, usbBuffer) == 0);
	//output stage sends out ack packet
	for(int i=0; i<4; i++) {
		uartConsumeData(buffer, 1);
		pollScsDataExchange();
	}
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_WAIT_ACK);
	//check monitor content
	sprintf(buffer, "< A 00\r\n");
	length = strlen(buffer);
	memset(usbBuffer, 0, 256);
	rc = usbConsumeData(usbBuffer, 256);
	ASSERT(rc == length);
	ASSERT(strcmp(buffer, usbBuffer) == 0);
	//check uart output buffer
	rc = _createDataPacket(0, pAppData, strlen(pAppData), buffer, 256);
	length = _createAckPacket(0, buffer+rc, 256);
	length = rc + length;//total length of data packet and ack packet
	rc = uartConsumeData(uartOutputBuffer, 256);
	ASSERT(rc == MOCK_USB_OUTPUT_BUFFER_MASK);
	for(int i=0; i<length; i++) {
		ASSERT(buffer[i] == uartOutputBuffer[MOCK_USB_OUTPUT_BUFFER_MASK - length + i]);
	}
	pollScsDataExchange(); //from wait_ack to idle
	ASSERT(inputStageState() == SCS_INPUT_IDLE);
	ASSERT(outputStageState() == SCS_OUTPUT_IDLE);

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/**
 * test that monitor works well when output is consumed byte by byte
 */
static void _002000_uart_monitorByteByByte()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	_resetTestEnv();

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/**
 * test that large amount of data can be exchanged between host and app
 */
static void _003000_uart_roundsHostAndApp()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	unsigned char hostPktId = 0;
	unsigned char appPktId = 0;
	unsigned char appBuf[256];
	unsigned char monitorBuf[256];
	unsigned char uartBuf[256];
	unsigned char dataBuf[64];
	unsigned char buf[64];
	unsigned char dataLength;
	int rc;

	_resetTestEnv();
	enableOutputBuffer();

	for(int loopCounter = 0; loopCounter<0x3ffff; loopCounter++)
	{
		//data from host to app
		dataLength = loopCounter % (SCS_DATA_MAX_LENGTH + 1);
		memset(dataBuf, 0, sizeof(dataBuf));
		for(int i=0; i<dataLength; i++) {
			dataBuf[i] = dataLength;
		}
		rc = _createDataPacket(hostPktId, dataBuf, dataLength, uartBuf, 256);
		ASSERT(rc == (dataLength + SCS_DATA_PACKET_STAFF_LENGTH));
		uartProduceData(uartBuf, rc);
		for(int i=0; i<rc; i++) {
			pollScsDataExchange();
		}
		ASSERT(inputBufferUsed() == dataLength);
		//check ack packet.
		rc = _createAckPacket(hostPktId, buf, 64);
		ASSERT(rc == 4);
		memset(uartBuf, 0, sizeof(uartBuf));
		rc = uartConsumeData(uartBuf, 256);
		ASSERT(rc == 4);
		for(int i=0; i<rc; i++) {
			ASSERT(buf[i] == uartBuf[i]);
		}
		//check monitor content
		sprintf(buf, "> D %02X\r\n< A %02X\r\n", hostPktId, hostPktId);
		memset(monitorBuf, 0, sizeof(monitorBuf));
		rc = usbConsumeData(monitorBuf, 256);
		ASSERT(rc == strlen(buf));
		ASSERT(strcmp(buf, monitorBuf) == 0);
		//stage state
		ASSERT(inputStageState() == SCS_INPUT_IDLE);
		ASSERT(outputStageState() == SCS_OUTPUT_IDLE);

		if(dataLength > 0)  
		{
			unsigned char c;
			int counter;

			//data from app to host
			memset(appBuf, 0, sizeof(appBuf));
			for(counter= 0; ; counter++)
			{
				c = readInputBuffer();
				if(c == 0) {
					break;
				}
				ASSERT(c == dataLength);
				ASSERT(counter <= dataLength);
				writeOutputBufferChar(c);
			}
			ASSERT(counter == dataLength);
			ASSERT(counter == outputBufferUsed());

			for(int i=0; i<(dataLength + SCS_DATA_PACKET_STAFF_LENGTH); i++) {
				pollScsDataExchange();
			}
			//check monitor content
			sprintf(buf, "< D %02X\r\n", appPktId);
			memset(monitorBuf, 0, 256);
			rc = usbConsumeData(monitorBuf, 256);
			ASSERT(rc == strlen(buf));
			ASSERT(strcmp(buf, monitorBuf) == 0);
			//check data pkt content
			memset(buf, dataLength, 64);
			rc = _createDataPacket(appPktId, buf, dataLength, appBuf, 256);
			ASSERT(rc == (dataLength + SCS_DATA_PACKET_STAFF_LENGTH));
			rc = uartConsumeData(uartBuf, 256);
			ASSERT(rc == (dataLength + SCS_DATA_PACKET_STAFF_LENGTH));
			for(int i=0; i<rc; i++) {
				ASSERT(appBuf[i] == uartBuf[i]);
			}
			//ack app pkt
			rc = _createAckPacket(appPktId, buf, 64);
			ASSERT(rc == 4);
			uartProduceData(buf, 4);
			for(int i=0; i<4; i++) {
				pollScsDataExchange();
			}
			//check monitor content
			sprintf(buf, "> A %02X\r\n", appPktId);
			rc = usbConsumeData(monitorBuf, 256);
			ASSERT(rc == strlen(buf));
			ASSERT(strcmp(buf, monitorBuf) == 0);
			//stage state
			ASSERT(inputStageState() == SCS_INPUT_IDLE);
			ASSERT(outputStageState() == SCS_OUTPUT_IDLE);

			appPktId++;
			if(appPktId == SCS_INVALID_PACKET_ID) {
				appPktId = 1;
			}
		}

		hostPktId++;
		if(hostPktId == SCS_INVALID_PACKET_ID) {
			hostPktId = 1;
		}
	}

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

/**
 * test that large amount of data can be exchanged between host and app byte by byte in hungry mode
 * simulate the situation that transfer speed is slow
 */
static void _003001_uart_roundsHostAndAppHungry()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	unsigned char hostPktId = 0;
	unsigned char appPktId = 0;
	unsigned char appBuf[256];
	unsigned char monitorBuf[256];
	unsigned char uartBuf[256];
	unsigned char dataBuf[64];
	unsigned char buf[64];
	unsigned char dataLength;
	int rc;

	_resetTestEnv();
	enableOutputBuffer();
	uartOutputBufferSetProducerIndex(MOCK_UART_OUTPUT_BUFFER_MASK);

	for(int loopCounter = 0; loopCounter<0x3ffff; loopCounter++)
	{
		//data from host to app
		dataLength = loopCounter % (SCS_DATA_MAX_LENGTH + 1);
		memset(dataBuf, 0, sizeof(dataBuf));
		for(int i=0; i<dataLength; i++) {
			dataBuf[i] = dataLength;
		}
		rc = _createDataPacket(hostPktId, dataBuf, dataLength, uartBuf, 256);
		ASSERT(rc == (dataLength + SCS_DATA_PACKET_STAFF_LENGTH));
		for(int i=0; i<rc; i++) 
		{
			uartProduceData(uartBuf + i, 1);
			tcOneClock();
			pollScsDataExchange();
			tcOneClock();
			pollScsDataExchange();
			tcOneClock();
			pollScsDataExchange();
		}
		ASSERT(inputBufferUsed() == dataLength);
		ASSERT(inputStageState() == SCS_INPUT_IDLE);
		ASSERT(outputStageState() == SCS_OUTPUT_SENDING_ACK);
		//check ack packet.
		rc = _createAckPacket(hostPktId, buf, 64);
		ASSERT(rc == 4);
		memset(uartBuf, 0, sizeof(uartBuf));
		for(int i=0; i<4; i++) 
		{
			unsigned char c;

			uartConsumeData(&c, 1);
			tcOneClock();
			pollScsDataExchange();
			tcOneClock();
			pollScsDataExchange();
		}
		rc = uartOutputBufferCopyLastBytes(uartBuf, 4);
		ASSERT(rc == 4);
		for(int i=0; i<rc; i++) {
			ASSERT(buf[i] == uartBuf[i]);
		}
		//check monitor content
		sprintf(buf, "> D %02X\r\n< A %02X\r\n", hostPktId, hostPktId);
		memset(monitorBuf, 0, sizeof(monitorBuf));
		rc = usbConsumeData(monitorBuf, 256);
		ASSERT(rc == strlen(buf));
		ASSERT(strcmp(buf, monitorBuf) == 0);
		//stage state
		ASSERT(inputStageState() == SCS_INPUT_IDLE);
		ASSERT(outputStageState() == SCS_OUTPUT_IDLE);

		if(dataLength > 0)  
		{
			unsigned char c;
			int counter;

			//data from app to host
			memset(appBuf, 0, sizeof(appBuf));
			for(counter= 0; ; counter++)
			{
				c = readInputBuffer();
				if(c == 0) {
					break;
				}
				ASSERT(c == dataLength);
				ASSERT(counter <= dataLength);
				writeOutputBufferChar(c);
			}
			ASSERT(counter == dataLength);
			ASSERT(counter == outputBufferUsed());


			for(int i=0; i<(dataLength + SCS_DATA_PACKET_STAFF_LENGTH); i++) 
			{
				unsigned char c;

				uartConsumeData(&c, 1);
				pollScsDataExchange();
				pollScsDataExchange();
				pollScsDataExchange();
			}
			//check monitor content
			sprintf(buf, "< D %02X\r\n", appPktId);
			memset(monitorBuf, 0, 256);
			rc = usbConsumeData(monitorBuf, 256);
			ASSERT(rc == strlen(buf));
			ASSERT(strcmp(buf, monitorBuf) == 0);
			//check data pkt content
			memset(buf, dataLength, 64);
			rc = _createDataPacket(appPktId, buf, dataLength, appBuf, 256);
			ASSERT(rc == (dataLength + SCS_DATA_PACKET_STAFF_LENGTH));
			rc = uartOutputBufferCopyLastBytes(uartBuf, rc);
			ASSERT(rc == (dataLength + SCS_DATA_PACKET_STAFF_LENGTH));
			for(int i=0; i<rc; i++) {
				ASSERT(appBuf[i] == uartBuf[i]);
			}
			//check output stage state
			ASSERT(outputStageState() == SCS_OUTPUT_WAIT_ACK);
			//ack app pkt
			rc = _createAckPacket(appPktId, buf, 64);
			ASSERT(rc == 4);
			for(int i=0; i<4; i++) 
			{
				uartProduceData(buf+i, 1);
				pollScsDataExchange();
				pollScsDataExchange();
				pollScsDataExchange();
			}
			//check monitor content
			sprintf(buf, "> A %02X\r\n", appPktId);
			rc = usbConsumeData(monitorBuf, 256);
			ASSERT(rc == strlen(buf));
			ASSERT(strcmp(buf, monitorBuf) == 0);
			//stage state
			ASSERT(inputStageState() == SCS_INPUT_IDLE);
			ASSERT(outputStageState() == SCS_OUTPUT_IDLE);

			appPktId++;
			if(appPktId == SCS_INVALID_PACKET_ID) {
				appPktId = 1;
			}
		}

		hostPktId++;
		if(hostPktId == SCS_INVALID_PACKET_ID) {
			hostPktId = 1;
		}
	}

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

void startTestCases()
{
	printf("startTestCases started\r\n");

	// _004000_tc();
	// _000001_uart_oneByteInput();
	// _000002_uart_inputStageTimeout();
	// _000003_uart_completeAckPacket();
	// _000004_uart_oneCompleteDataPacket();
	// _000005_uart_repeatedCompleteDataPacket();
	// _000006_uart_duplicatedDataPacket();
	// _000007_uart_discontinuousPakcetId();
	// _000008_uart_crcError();
	// _000009_uart_illegalPacketLength();
	// _000010_uart_abruptPacketId();
	// _001000_uart_sendAppData();
	// _001001_uart_sendAppDataTimeout();
	// _001002_uart_sendAppDataAround();
	// _001003_uart_multiAppData();
	// _001004_uart_dataBothWay();
	// _001005_uart_appDataPostpone();
	// _001006_uart_quickHostAck();
	// _003000_uart_roundsHostAndApp();
	_003001_uart_roundsHostAndAppHungry();

	printf("startTestCases finished\r\n");
}
