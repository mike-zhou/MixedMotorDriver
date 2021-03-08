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

static void _templete_testcase_function()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);

	_resetTestEnv();

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

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

void startTestCases()
{
	_000001_uart_oneByteInput();
	_000002_uart_inputStageTimeout();
	_000003_uart_completeAckPacket();
}
