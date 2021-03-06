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
	sysclk_init();

	Invenco_init();
}

static void _templete_testcase_function()
{
	printf("\r\n===================================\r\n");
	printf("%s started\r\n", __FUNCTION__);


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
		ASSERT(strcmp(buf, pErrStr) == 0);
	}

	printf("%s stopped\r\n", __FUNCTION__);
	printf("-------------------------------------\r\n");
}

void startTestCases()
{
	_000001_uart_oneByteInput();
	_000002_uart_inputStageTimeout();
}
