/*
 * mock_functions.cpp
 *
 *  Created on: 25/02/2021
 *      Author: user1
 */

#include "asf.h"
#include "crc.h"
#include "ECD300.h"
#include "tc.h"
#include "usb_protocol_cdc.h"

#include "mock_functions.h"

unsigned char PORTA_DIR;
unsigned char PORTB_DIR;
unsigned char PORTC_DIR;
unsigned char PORTD_DIR;
unsigned char PORTE_DIR;
unsigned char PORTF_DIR;
unsigned char PORTH_DIR;
unsigned char PORTJ_DIR;
unsigned char PORTK_DIR;



/*****************************************
 * EBI
 *****************************************/
void ebi_setup_port(unsigned char addr, unsigned char sram, unsigned char lpc, unsigned char flags)
{

}

void ebi_cs_set_mode(struct ebi_cs_config * p, unsigned char mode)
{

}


void ebi_cs_set_address_size(struct ebi_cs_config * p, unsigned char mode)
{

}


void ebi_cs_set_base_address(struct ebi_cs_config * p, unsigned char mode)
{

}


void ebi_cs_set_sram_wait_states(struct ebi_cs_config * p, unsigned char mode)
{

}


void ebi_cs_write_config(unsigned char mode, struct ebi_cs_config * p)
{

}


void ebi_enable_cs(unsigned char mode, struct ebi_cs_config * p)
{

}

/************************************************
 * USB
 ************************************************/

unsigned char _mockUsbInputBuffer[MOCK_USB_INPUT_BUFFER_MASK + 1];
unsigned short _mockUsbInputBufferConsumerIndex = 0;
unsigned short _mockUsbInputBufferProducerIndex = 0;
unsigned char _mockUsbOutputBuffer[MOCK_USB_OUTPUT_BUFFER_MASK + 1];
unsigned short _mockUsbOutputBufferConsumerIndex = 0;
unsigned short _mockUsbOutputBufferProducerIndex = 0;

void udc_start()
{

}

iram_size_t udi_cdc_get_free_tx_buffer(void)
{
	int used;

	if(_mockUsbOutputBufferConsumerIndex <= _mockUsbOutputBufferProducerIndex) {
		used = _mockUsbOutputBufferProducerIndex - _mockUsbOutputBufferConsumerIndex;
	}
	else {
		used = (MOCK_USB_OUTPUT_BUFFER_MASK - _mockUsbOutputBufferConsumerIndex) + _mockUsbOutputBufferProducerIndex;
	}

	return MOCK_USB_INPUT_BUFFER_MASK - used;
}

iram_size_t udi_cdc_get_nb_received_data()
{
	int used;

	if(_mockUsbInputBufferConsumerIndex <= _mockUsbInputBufferProducerIndex) {
		used = _mockUsbInputBufferProducerIndex - _mockUsbInputBufferConsumerIndex;
	}
	else {
		used = (MOCK_USB_INPUT_BUFFER_MASK - _mockUsbInputBufferConsumerIndex) + _mockUsbInputBufferProducerIndex;
	}

	return used;
}

iram_size_t udi_cdc_write_buf(const void * buf, iram_size_t size)
{
	unsigned char * pSource = (unsigned char *)buf;
	unsigned int count;

	for(count = 0; count < size; count++) 
	{
		int nextIndex = (_mockUsbOutputBufferProducerIndex + 1) & MOCK_USB_OUTPUT_BUFFER_MASK;
		
		if(nextIndex == _mockUsbOutputBufferConsumerIndex) {
			break;
		}
		_mockUsbOutputBuffer[_mockUsbOutputBufferProducerIndex] = pSource[count];
		_mockUsbOutputBufferProducerIndex = nextIndex;
	}

	return size - count;
}

iram_size_t udi_cdc_read_buf(void * buf, iram_size_t size)
{
	unsigned int count;
	unsigned char * pBuf = (unsigned char * )buf;

	for(count =0; count < size; count++)
	{
		if(_mockUsbInputBufferConsumerIndex == _mockUsbInputBufferProducerIndex) {
			break;
		}
		pBuf[count] = _mockUsbInputBuffer[_mockUsbInputBufferConsumerIndex];
		_mockUsbInputBufferConsumerIndex = (_mockUsbInputBufferConsumerIndex + 1) & MOCK_USB_INPUT_BUFFER_MASK;
	}

	return size - count;
}

bool udi_cdc_is_tx_ready(void)
{
	bool bFull;
	unsigned int nextIndex = (_mockUsbOutputBufferProducerIndex + 1) & MOCK_USB_OUTPUT_BUFFER_MASK;

	if(nextIndex == _mockUsbOutputBufferConsumerIndex) {
		bFull = true;
	}
	else {
		bFull = false;
	}

	return bFull;
}

int udi_cdc_putc(int value)
{
	unsigned int nextIndex = (_mockUsbOutputBufferProducerIndex + 1) & MOCK_USB_OUTPUT_BUFFER_MASK;

	if(nextIndex == _mockUsbOutputBufferConsumerIndex) {
		return 0;
	}
	else {
		_mockUsbOutputBuffer[_mockUsbOutputBufferProducerIndex] = value;
		_mockUsbOutputBufferProducerIndex = nextIndex;
		return 1;
	}
}

/**
 * empty the mockUsbOutputBuffer
 */
void usbClearOutputBuffer()
{
	_mockUsbOutputBufferProducerIndex = 0;
	_mockUsbOutputBufferConsumerIndex = 0;
}
/**
 * return amount of available data in the mockUsbOutputBuffer
 */
int usbOutputBufferUsed()
{
	if(_mockUsbOutputBufferConsumerIndex <= _mockUsbOutputBufferProducerIndex) {
		return _mockUsbOutputBufferProducerIndex - _mockUsbOutputBufferConsumerIndex;
	}
	else {
		return MOCK_UART_OUTPUT_BUFFER_MASK - _mockUsbOutputBufferConsumerIndex + _mockUsbOutputBufferProducerIndex;
	}
}

/**
 * read data from mockOutputData, 
 * return the amount of data actually read
 */
int usbConsumeData(unsigned char * pBuffer, int size)
{
	int count = 0;

	for(; count < size; count++) 
	{
		if(_mockUsbOutputBufferConsumerIndex == _mockUsbOutputBufferProducerIndex) {
			break;
		}
		pBuffer[count] = _mockUsbOutputBuffer[_mockUsbOutputBufferConsumerIndex];
		_mockUsbOutputBufferConsumerIndex = (_mockUsbOutputBufferConsumerIndex + 1) & MOCK_UART_OUTPUT_BUFFER_MASK;
	}

	return count;
}

/**
 * clear mockUsbInputBuffer
 */
void usbClearInputBuffer()
{
	_mockUsbInputBufferProducerIndex = 0;
	_mockUsbInputBufferConsumerIndex = 0;
}

/**
 * return the amount of byte available in the mockUsbInputBuffer
 */
int usbGetInputBufferUsed()
{
	if(_mockUsbInputBufferConsumerIndex <= _mockUsbInputBufferProducerIndex) {
		return _mockUsbInputBufferProducerIndex - _mockUsbInputBufferConsumerIndex;
	}
	else {
		return MOCK_USB_INPUT_BUFFER_MASK - _mockUsbInputBufferConsumerIndex + _mockUsbInputBufferProducerIndex;
	}
}

/*
 * write data to mockUsbInputBuffer,
 * return the amount of data actually written
 */
int usbProduceData(unsigned char * pBuffer, int size)
{
	int count = 0;

	for(; count < size; count++) 
	{
		int nextIndex = (_mockUsbInputBufferProducerIndex + 1) & MOCK_USB_INPUT_BUFFER_MASK;
		
		if(nextIndex == _mockUsbInputBufferConsumerIndex) {
			break;
		}
		_mockUsbInputBuffer[_mockUsbInputBufferProducerIndex] = pBuffer[count];
		_mockUsbInputBufferProducerIndex = nextIndex;
	}

	return count;
}

int usbInputBufferConsumerIndex()
{
	return _mockUsbInputBufferConsumerIndex;
}

int usbInputBufferProducerIndex()
{
	return _mockUsbInputBufferProducerIndex;
}

int usbOutputBufferConsumerIndex()
{
	return _mockUsbOutputBufferConsumerIndex;
}

bool usbOutputBufferSetConsumerIndex(unsigned short index)
{
	if(index > MOCK_USB_OUTPUT_BUFFER_MASK) {
		return false;
	}

	_mockUsbOutputBufferConsumerIndex = index;
	return true;
}

int usbOutputBufferProducerIndex()
{
	return _mockUsbOutputBufferProducerIndex;
}

bool usbOutputBufferSetProducerIndex(unsigned short index)
{
	if(index > MOCK_USB_OUTPUT_BUFFER_MASK) {
		return false;
	}

	_mockUsbOutputBufferProducerIndex = index;
	return true;
}

/*******************************************
 * UART
 *******************************************/
static unsigned char _mockUartInputBuffer[MOCK_UART_INPUT_BUFFER_MASK + 1];
static unsigned short _mockUartInputBufferConsumerIndex = 0;
static unsigned short _mockUartInputBufferProducerIndex = 0;
static unsigned char _mockUartIOutputBuffer[MOCK_UART_OUTPUT_BUFFER_MASK + 1];
static unsigned short _mockUartOutputBufferConsumerIndex = 0;
static unsigned short _mockUartOutputBufferProducerIndex = 0;

char ecd300InitUart(unsigned char devIndex, usart_rs232_options_t * pOptions)
{
	return 0;
}

char ecd300PutChar(unsigned char devIndex, unsigned char character)
{
	unsigned short nextIndex = (_mockUartOutputBufferProducerIndex + 1) & MOCK_UART_OUTPUT_BUFFER_MASK;

	if(nextIndex == _mockUartOutputBufferConsumerIndex) {
		return -1;
	}
	_mockUartIOutputBuffer[_mockUartOutputBufferProducerIndex] = character;
	_mockUartOutputBufferProducerIndex = nextIndex;

	return 0;
}

char ecd300PollChar(unsigned char devIndex, unsigned char * pChar)
{
	if(_mockUartInputBufferConsumerIndex == _mockUartInputBufferProducerIndex) {
		return 0;
	}
	*pChar = _mockUartInputBuffer[_mockUartInputBufferConsumerIndex];
	_mockUartInputBufferConsumerIndex = (_mockUartInputBufferConsumerIndex + 1) & MOCK_USB_INPUT_BUFFER_MASK;

	return 1;
}

void uartReset()
{
	_mockUartInputBufferConsumerIndex = 0;
	_mockUartInputBufferProducerIndex = 0;
	_mockUartOutputBufferConsumerIndex = 0;
	_mockUartOutputBufferProducerIndex = 0;
}

int uartInputBufferConsumerIndex()
{
	return _mockUartInputBufferConsumerIndex;
}

int uartInputBufferProducerIndex()
{
	return _mockUartInputBufferProducerIndex;
}

int uartOutputBufferConsumerIndex()
{
	return _mockUartOutputBufferConsumerIndex;
}

int uartOutputBufferProducerIndex()
{
	return _mockUartOutputBufferProducerIndex;
}

/***
 * simulate UART bytes arrival
 * 	return amount of bytes really received.
 */
int uartProduceData(unsigned char * pBuffer, int size)
{
	int count = 0;

	for(; count < size; count++)
	{
		int nextIndex = (_mockUartInputBufferProducerIndex + 1) & MOCK_UART_INPUT_BUFFER_MASK;

		if(nextIndex == _mockUartInputBufferConsumerIndex) {
			break;
		}
		_mockUartInputBuffer[_mockUartInputBufferProducerIndex] = pBuffer[count];
		_mockUartInputBufferProducerIndex = nextIndex;
	}

	return count;
}

int uartConsumeData(unsigned char * pBuffer, int size)
{
	int count = 0;

	for(; count < size; count++)
	{
		if(_mockUartOutputBufferConsumerIndex == _mockUartOutputBufferProducerIndex) {
			break;
		}
		pBuffer[count] = _mockUartIOutputBuffer[_mockUartOutputBufferConsumerIndex];
		_mockUartOutputBufferConsumerIndex = (_mockUartOutputBufferConsumerIndex + 1) & MOCK_UART_OUTPUT_BUFFER_MASK;
	}

	return count;
}

/*************************************************
 * Timer Counter
 *************************************************/
static unsigned short _timerCounter;

void sysclk_init()
{
	_timerCounter = 0;
}

void tc_enable(void * pTc)
{

}


void tc_set_resolution(void * pTc, unsigned char r)
{

}


unsigned long tc_get_resolution(void * pTc)
{
	return 0x8000; //clokcs per HZ
}


unsigned short tc_read_count(void * pTc)
{
	return _timerCounter;
}

/***
 * increase one clock
 */
void tcOneClock()
{
	_timerCounter++;
}

/***
 * increase designated clocks
 */
void tcClocks(unsigned int ticks)
{
	for(int i=0; i<ticks; i++) {
		tcOneClock();
	}
}

/*****************************************
 * CRC
 *****************************************/
static const unsigned short _CRC_POLY_CCITT	= 0x1021;
static unsigned short _crc_tabccitt[256];
static unsigned short _crc_initial_value;

void _init_crcccitt_tab( void )
{
	unsigned short i;
	unsigned short j;
	unsigned short crc;
	unsigned short c;

	for (i=0; i<256; i++) {

		crc = 0;
		c   = i << 8;

		for (j=0; j<8; j++) {

			if ( (crc ^ c) & 0x8000 ) crc = ( crc << 1 ) ^ _CRC_POLY_CCITT;
			else                      crc =   crc << 1;

			c = c << 1;
		}

		_crc_tabccitt[i] = crc;
	}
}

static unsigned short _crc_ccitt_generic( const unsigned char *input_str, int num_bytes, unsigned short start_value )
{
	unsigned short crc;
	unsigned short tmp;
	unsigned short short_c;
	const unsigned char *ptr;
	int a;

	_init_crcccitt_tab();

	crc = start_value;
	ptr = input_str;

	if ( ptr != NULL ) for (a=0; a<num_bytes; a++) {

		short_c = 0x00ff & (unsigned short) *ptr;
		tmp     = (crc >> 8) ^ short_c;
		crc     = (crc << 8) ^ _crc_tabccitt[tmp];

		ptr++;
	}

	return crc;
}

void crc_set_initial_value(unsigned long v)
{
	_crc_initial_value = (unsigned short)v;
}

unsigned long crc_io_checksum(void * pBuf, unsigned short len, unsigned char type)
{
	unsigned short crc;

	crc = _crc_ccitt_generic(pBuf, len, _crc_initial_value);
	return crc;
}


void disableJtagPort()
{

}

/****************************************
 * Miscellaneous
 ****************************************/
void sleepmgr_init()
{

}


void irq_initialize_vectors()
{

}


void cpu_irq_enable()
{

}


