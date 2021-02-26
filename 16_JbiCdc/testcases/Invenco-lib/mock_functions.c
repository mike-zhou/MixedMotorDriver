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

unsigned char mockUsbInputBuffer[MOCK_USB_INPUT_BUFFER_MASK + 1];
unsigned short mockUsbInputBufferConsumerIndex = 0;
unsigned short mockUsbInputBufferProducerIndex = 0;
unsigned char mockUsbOutputBuffer[MOCK_USB_OUTPUT_BUFFER_MASK];
unsigned short mockUsbOutputBufferConsumerIndex = 0;
unsigned short mockUsbOutputBufferProducerIndex = 0;

void udc_start()
{

}

iram_size_t udi_cdc_get_free_tx_buffer(void)
{
	int used;

	if(mockUsbOutputBufferConsumerIndex <= mockUsbOutputBufferProducerIndex) {
		used = mockUsbOutputBufferProducerIndex - mockUsbOutputBufferConsumerIndex;
	}
	else {
		used = (MOCK_USB_OUTPUT_BUFFER_MASK - mockUsbOutputBufferConsumerIndex) + mockUsbOutputBufferProducerIndex;
	}

	return MOCK_USB_INPUT_BUFFER_MASK - used;
}

iram_size_t udi_cdc_get_nb_received_data()
{
	int used;

	if(mockUsbInputBufferConsumerIndex <= mockUsbInputBufferProducerIndex) {
		used = mockUsbInputBufferProducerIndex - mockUsbInputBufferConsumerIndex;
	}
	else {
		used = (MOCK_USB_INPUT_BUFFER_MASK - mockUsbInputBufferConsumerIndex) + mockUsbInputBufferProducerIndex;
	}

	return used;
}

iram_size_t udi_cdc_write_buf(const void * buf, iram_size_t size)
{
	unsigned char * pSource = (unsigned char *)buf;
	unsigned int count;

	for(count = 0; count < size; count++) 
	{
		int nextIndex = (mockUsbOutputBufferProducerIndex + 1) & MOCK_USB_OUTPUT_BUFFER_MASK;
		
		if(nextIndex == mockUsbOutputBufferConsumerIndex) {
			break;
		}
		mockUsbOutputBuffer[mockUsbOutputBufferProducerIndex] = pSource[count];
		mockUsbOutputBufferProducerIndex = nextIndex;
	}

	return count;
}

iram_size_t udi_cdc_read_buf(void * buf, iram_size_t size)
{
	unsigned int count;
	unsigned char * pBuf = (unsigned char * )buf;

	for(count =0; count < size; count++)
	{
		if(mockUsbInputBufferConsumerIndex == mockUsbInputBufferProducerIndex) {
			break;
		}
		pBuf[count] = mockUsbInputBuffer[mockUsbInputBufferConsumerIndex];
		mockUsbInputBufferConsumerIndex = (mockUsbInputBufferConsumerIndex + 1) & MOCK_USB_INPUT_BUFFER_MASK;
	}

	return count;
}

bool udi_cdc_is_tx_ready(void)
{
	bool bFull;
	unsigned int nextIndex = (mockUsbOutputBufferProducerIndex + 1) & MOCK_USB_OUTPUT_BUFFER_MASK;

	if(nextIndex == mockUsbOutputBufferConsumerIndex) {
		bFull = true;
	}
	else {
		bFull = false;
	}

	return bFull;
}

int udi_cdc_putc(int value)
{
	unsigned int nextIndex = (mockUsbOutputBufferProducerIndex + 1) & MOCK_USB_OUTPUT_BUFFER_MASK;

	if(nextIndex == mockUsbOutputBufferConsumerIndex) {
		return 0;
	}
	else {
		mockUsbOutputBuffer[mockUsbOutputBufferProducerIndex] = value;
		mockUsbOutputBufferProducerIndex = nextIndex;
		return 1;
	}
}


/*******************************************
 * UART
 *******************************************/
char ecd300InitUart(unsigned char devIndex, usart_rs232_options_t * pOptions)
{

}

char ecd300PutChar(unsigned char devIndex, unsigned char character)
{

}

char ecd300PollChar(unsigned char devIndex, unsigned char * pChar)
{

}

/*************************************************
 * Timer Counter
 *************************************************/
void sysclk_init()
{

}

void tc_enable(void * pTc)
{

}


void tc_set_resolution(void * pTc, unsigned char r)
{

}


unsigned long tc_get_resolution(void * pTc)
{

}


unsigned short tc_read_count(void * pTc)
{

}

/*****************************************
 * CRC
 *****************************************/
void crc_set_initial_value(unsigned long v)
{

}

unsigned long crc_io_checksum(void * pBuf, unsigned short len, unsigned char type)
{

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


