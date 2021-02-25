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

unsigned char PORTA_DIR;
unsigned char	PORTB_DIR;
unsigned char	PORTC_DIR;
unsigned char	PORTD_DIR;
unsigned char	PORTE_DIR;
unsigned char	PORTF_DIR;
unsigned char	PORTH_DIR;
unsigned char	PORTJ_DIR;
unsigned char	PORTK_DIR;

/**
 * EBI
 */
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

/**
 * USB
 */
void udc_start()
{

}

iram_size_t udi_cdc_get_free_tx_buffer(void)
{

}

iram_size_t udi_cdc_write_buf(const void * buf, iram_size_t size)
{

}

/**
 * UART
 */
char ecd300InitUart(unsigned char devIndex, usart_rs232_options_t * pOptions)
{

}

char ecd300PutChar(unsigned char devIndex, unsigned char character)
{

}

char ecd300PollChar(unsigned char devIndex, unsigned char * pChar)
{

}

/**
 * Timer Counter
 */
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

/**
 * CRC
 */
void crc_set_initial_value(unsigned long v)
{

}

unsigned long crc_io_checksum(void * pBuf, unsigned short len, unsigned char type)
{

}


void disableJtagPort()
{

}

/**
 * Miscellaneous
 */
void sleepmgr_init()
{

}


void irq_initialize_vectors()
{

}


void cpu_irq_enable()
{

}


