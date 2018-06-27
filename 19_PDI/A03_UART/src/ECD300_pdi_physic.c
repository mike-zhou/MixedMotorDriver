
/**
 * \file
 *
 * \brief Low level PDI driver
 *
 * Copyright (C) 2009 Atmel Corporation. All rights reserved.
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 * Atmel AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */
#include "ECD300_pdi_physic.h"
#include "ECD300_delay.h"

extern void printString(char * pString);
extern void printHex(unsigned char hex);

#define PDI_BAUD_RATE 1000000UL               //!< PDI Baud rate.

#define USART USARTE0
#define PDI_PORT PORTE
#define PDI_RESET_PIN 1
#define PDI_RESET_PIN_CONTROL PIN1CTRL
#define PDI_DATA_PIN 3
#define PDI_DATA_PIN_CONTROL PIN1CTRL

/**
 * \brief Initial the PDI port.
 */
#define pdi_pin_init() do {                               \
				PDI_PORT.OUTCLR= (1 << PDI_DATA_PIN);						\
				PDI_PORT.DIRSET= (1 << PDI_DATA_PIN);							\
				PDI_PORT.OUTSET= (1 << PDI_RESET_PIN);						\
				PDI_PORT.DIRSET= (1 << PDI_RESET_PIN);							\
} while (0)

/**
 * \brief Set the PDI DATA tx pin low.
 */
#define pdi_data_tx_low() do {            \
		PDI_PORT.OUTCLR=(1 << PDI_DATA_PIN);		\
} while (0)

/**
 * \brief Set the PDI DATA tx pin high.
 */
#define pdi_data_tx_high() do {        \
		PDI_PORT.OUTSET= (1 << PDI_DATA_PIN);	\
} while (0)

/**
 * \brief Set the PDI DATA tx as input.
 */
#define pdi_data_tx_input() do {    \
		PDI_PORT.DIRCLR = (1 << PDI_DATA_PIN); \
} while (0)

#define pdi_data_tx_output() do {    \
		PDI_PORT.DIRSET = (1 << PDI_DATA_PIN); \
} while (0)

/**
 * \brief Set the PDI RESET pin low.
 */
#define pdi_reset_low() do {            \
				PDI_PORT.OUTCLR= (1 << PDI_RESET_PIN); \
} while (0)

/**
 * \brief Set the PDI RESET pin high.
 */
#define pdi_reset_high() do {         \
				PDI_PORT.OUTSET= (1 << PDI_RESET_PIN); \
} while (0)

/**
  * \brief Configure USART for PDI application.
  *
  *  The USRT must be configured the following way:
  *  - Double speed
  *  - data change on falling TX
  *  - Character size is 8 bits
  *  - Even parity
  *  - Two stop bits
  *  For more information, please refer to "Program and Debug Interface" section of
  *  XMEGA A MANUAL
  */
static inline void pdi_configure(void)
{
	/* Sample data falling clock edge, 8 bit data, even
	 * parity and 2 stop bits */
	usart_format_set(&USART, USART_CHSIZE_8BIT_gc, USART_PMODE_EVEN_gc, true);
	//usart_set_polarity_bit(&USART, true);
	
	//invert the polarity of PDI_CLOCK, so that data is sampled at the rising edge. According to specification, data is sampled at the rising edge.
	PDI_PORT.PDI_RESET_PIN_CONTROL=PORT_OPC_TOTEM_gc|PORT_INVEN_bm|PORT_SRLEN_bm;
	PDI_PORT.PDI_DATA_PIN_CONTROL=PORT_OPC_TOTEM_gc|PORT_SRLEN_bm;
}


/**
 * \brief USART baud rate set.
 *
 * This function is used to set the PDI baud rate on synchronous mode.
 *
 *  \param  baud_rate the PDI baud rate.
 */
static inline void pdi_set_baud_rate(unsigned long baud_rate)
{
#if 0 //it is strange that the following code will result in the failure of PDI writting later.
	
	unsigned long peripheralClock;

	peripheralClock=sysclk_get_per_hz();

	printString("peripheral clock: ");
	printHex(peripheralClock>>24);
	printString(", ");
	printHex(peripheralClock>>16);
	printString(", ");
	printHex(peripheralClock>>8);
	printString(", ");
	printHex(peripheralClock>>0);
	printString("\r\n");
#endif
	usart_spi_set_baudrate(&USART, baud_rate, sysclk_get_per_hz());
	
}

/**
 * \brief Enable PDI RX.
 */
static inline void pdi_enable_rx(void)
{
	usart_rx_enable(&USART);
}

/**
 * \brief Enable PDI TX.
 *
 *  This function is used to enable PDI TX.
 */
static inline void pdi_enable_tx(void)
{
	usart_tx_enable(&USART);
}

/**
 * \brief Set mode to be synchronuous.
 */
static inline void pdi_enable_clk( void )
{
	usart_set_mode(&USART, USART_CMODE_SYNCHRONOUS_gc);
}

/**
 * \brief The clock is disabled by entering MSPI mode.
 */
static inline void pdi_disable_clk( void )
{
	usart_set_mode(&USART, USART_CMODE_ASYNCHRONOUS_gc);
}

/**
 * \brief Disable the USRT transmitter.
 */
static inline void pdi_disable_tx( void )
{
	usart_tx_disable(&USART);
}

/**
 * \brief Disable the USRT receiver.
 */
static inline void pdi_disable_rx( void )
{
	usart_rx_disable(&USART);
}

/**
 * \brief Initialize PDI.
 *
 * Initialize the peripeherals necessary to communicate with the ATxmega
 *  PDI.
 */
void pdi_init( void )
{
#if 1	
	PDI_PORT.DIRCLR=0xff;

	sysclk_enable_peripheral_clock(&USART);
	/* Enable USRT. */
	pdi_configure();
	pdi_set_baud_rate(PDI_BAUD_RATE);

	/* Enter PDI mode. */
	pdi_pin_init();
	ecd300DelayMicrosecond(100);

	//disable RESET function.
	pdi_data_tx_high();
	pdi_data_tx_output();
	ecd300DelayMicrosecond(1);

	/* Enable the PDI_CLK and let it run for at leat 16 cycles. */
	pdi_enable_rx();
	pdi_enable_tx();
	pdi_enable_clk();
	ecd300DelayMicrosecond(20);
#else
	usart_rs232_options_t uartOption;

	PORTF.DIRCLR=0x04;
	//PORTF.DIRSET=0x08;
	
	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_EVEN_gc;
	uartOption.stopbits=true;

	if(!usart_init_rs232(&USART, &uartOption))
	{
		printString("\r\n!!cannot initialize the underlying USART\r\n");
	}
	else
	{//change the underlying USART to synchronous mode.
	
		PDI_PORT.OUTSET=1<<PDI_RESET_PIN;
		//PORT_OPC_TOTEM_gc|PORT_INVEN_bm|PORT_SRLEN_bm
		PDI_PORT.PDI_RESET_PIN_CONTROL=PORT_OPC_TOTEM_gc|PORT_INVEN_bm;
		PDI_PORT.DIRSET=1<<PDI_RESET_PIN;
		usart_spi_set_baudrate(&USART, PDI_BAUD_RATE, sysclk_get_per_hz());

		//USART is ready.
		ecd300DelayMicrosecond(100);

		//disable the RESET function.
		pdi_data_tx_high();
		pdi_data_tx_output();
		ecd300DelayMicrosecond(1);

		//Enable the PDI_CLK and let it run for at least 16 cycles.
		pdi_enable_clk();
		ecd300DelayMicrosecond(20*1);
	}
#endif
}

/**
 * \brief Write bulk bytes with PDI.
 *
 *  Send bytes over the PDI channel.
 *
 * \param data Pointer to memory where data to be sent is stored.
 * \param length Number of bytes to be sent.
 *
 * \retval STATUS_OK The transmission was successful.
 * \retval ERR_BAD_DATA One of the bytes sent was corrupted during transmission.
 */
enum status_code pdi_write( const unsigned char *data, unsigned short length )
{
	unsigned short i;
	unsigned char dummy_read = 0;
	bool bUnexpectedDataReceived=false;

	//clear the characters received previously.
	while(usart_rx_is_complete(&USART))
	{
		dummy_read = usart_get(&USART);
		bUnexpectedDataReceived=true;
	}
	if(bUnexpectedDataReceived)
	{
		//this makes a BREAK character because of the internal pull-down resistor in PDI_DATA.
		pdi_disable_tx();
		ecd300DelayMicrosecond(14);
		
		//send out two BREAK.
		pdi_data_tx_high();
		pdi_data_tx_output();
		ecd300DelayMicrosecond(5);
		pdi_data_tx_low();
		ecd300DelayMicrosecond(14);
		pdi_data_tx_high();
		//the second BREAK.
		ecd300DelayMicrosecond(5);
		pdi_data_tx_low();
		ecd300DelayMicrosecond(14);
		pdi_data_tx_high();
		ecd300DelayMicrosecond(5);
		
		//enable transmit.
		pdi_enable_tx();

		//empty receiving buffer.
		ecd300DelayMicrosecond(5);
		while(usart_rx_is_complete(&USART))
		{
			dummy_read = usart_get(&USART);
		}
	}
#if 0	
	dummy_read = usart_get(&USART);
	dummy_read = usart_get(&USART);
	dummy_read = usart_get(&USART);
	dummy_read = usart_get(&USART);
	if(USART.STATUS&(USART_FERR_bm|USART_BUFOVF_bm|USART_PERR_bm))
	{
		//signal the error in bit7 and bit6 of PORTB.
		PORTB_DIRSET=0xC0;
		PORTB_OUTSET=0xC0;
		printString("USART status: ");
		printHex(USART.STATUS);
		printString("\r\n");
	}
	else
	{
		PORTB_DIRSET=0xC0;
		PORTB_OUTCLR=0xC0;
	}
#endif		
		
	for (i = 0; i < length; i++) {
		while (!usart_data_register_is_empty(&USART))
		{
			;
		}
		usart_clear_tx_complete(&USART);
		//send a character.
		usart_put(&USART, data[i]);
		//wait the finish of transmition.
		while(!usart_tx_is_complete(&USART))
		{
			;
		}

		//wait until the data is echoed.
		while (!usart_rx_is_complete(&USART))
		{
			;
		}
		if(USART.STATUS&(USART_FERR_bm|USART_BUFOVF_bm|USART_PERR_bm))
		{
			//signal the error in bit7 of PORTB.
			PORTB_DIRSET=0x80;
			PORTB_OUTSET=0x80;
		}
		dummy_read = usart_get(&USART);

		/* Check that the transmission was successful. */
		if (dummy_read != data[i]) {
			// Set TX to tri-state
			pdi_data_tx_input();
			
			//this makes a BREAK character because of the internal pull-down resistor in PDI_DATA.
			pdi_disable_tx();
			ecd300DelayMicrosecond(14);
			
			//send out two BREAK.
			pdi_data_tx_high();
			pdi_data_tx_output();
			ecd300DelayMicrosecond(5);
			pdi_data_tx_low();
			ecd300DelayMicrosecond(14);
			pdi_data_tx_high();
			//the second BREAK.
			ecd300DelayMicrosecond(5);
			pdi_data_tx_low();
			ecd300DelayMicrosecond(14);
			pdi_data_tx_high();
			ecd300DelayMicrosecond(5);
			
			//enable transmit.
			pdi_enable_tx();

printString("write error, index: ");
printHex(i);
printString(", data sent: ");
printHex(data[i]);
printString(", data echoed: ");
printHex(dummy_read);
printString(", status: ");
printHex(USART.STATUS);
printString(", ctrlC: ");
printHex(USART.CTRLC);
printString(", ba: ");
printHex(USART.BAUDCTRLA);
printString(", bb: ");
printHex(USART.BAUDCTRLB);
printString("\r\n");

			return ERR_IO_ERROR;
		}
	}

	return STATUS_OK;
}

/**
 * \brief Read bulk bytes from PDI.
 *
 * \param data Pointer to memory where data to be stored.
 * \param length Number of bytes to be read.
 * \param retries the retry count.
 *
 * \retval non-zero the length of data.
 * \retval zero read fail.
 */
unsigned short pdi_read( unsigned char *data, unsigned short length, unsigned long retries )
{
	unsigned long count;
	unsigned short bytes_read = 0;
	unsigned short i;

	pdi_data_tx_input();//this is necessary. It seems that this PIN is not change to input when TX of USART is disabled.
	pdi_disable_tx();

	for (i = 0; i < length; i++) {
		count = retries;
		while (count != 0) {
			if (usart_rx_is_complete(&USART)) {
				if(USART.STATUS&(USART_FERR_bm|USART_BUFOVF_bm|USART_PERR_bm))
				{
					//signal the error at bit6 of PORTB
					PORTB_DIRSET=0x40;
					PORTB_OUTSET=0x40;
				}
				
				*(data + i) = usart_get(&USART);
				bytes_read++;
				break;
			}
			ecd300DelayMicrosecond(1);//delay 1 us.
			--count;
		}
		/* Read fail error */
		if (count == 0) {
			break;
		}
	}

	pdi_data_tx_high();
	pdi_data_tx_output();
	pdi_enable_tx();
	
	return bytes_read;
}

/**
 * \brief Read a byte from PDI.
 *
 * \param ret Pointer to buffer memory where data to be stored.
 * \param retries the retry count.
 *
 * \retval STATUS_OK read successfully.
 * \retval ERR_TIMEOUT read fail.
 */
enum status_code pdi_get_byte( unsigned char *ret, unsigned long retries )
{
	status_code_t rc=ERR_IO_ERROR;

	pdi_data_tx_input();//this is necessary. It seems that this PIN is not change to input when TX of USART is disabled.
	pdi_disable_tx();

	while (retries != 0) {
		if (usart_rx_is_complete(&USART)) {
			if(USART.STATUS&(USART_FERR_bm|USART_BUFOVF_bm|USART_PERR_bm))
			{
				//signal the error at bit6 of PORTB.
				PORTB_DIRSET=0x40;
				PORTB_OUTSET=0x40;
			}
				
			*ret = usart_get(&USART);
			rc= STATUS_OK;
			break;
		}
		ecd300DelayMicrosecond(1);//delay 1 us.
		--retries;
	}
	
	pdi_data_tx_high();
	pdi_data_tx_output();
	pdi_enable_tx();
	
	return rc;
}

/**
 * \brief This function disables the PDI port.
 */
void pdi_deinit( void )
{
	pdi_disable_clk();
	pdi_disable_rx();
	pdi_disable_tx();
	pdi_data_tx_input();
	pdi_data_tx_low();
	ecd300DelayMicrosecond(300);
	pdi_reset_high();
	sysclk_disable_peripheral_clock(&USART);
}


