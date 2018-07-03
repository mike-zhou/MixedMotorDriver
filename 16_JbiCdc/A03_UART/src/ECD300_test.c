/*
 * ECD300_test.c
 *
 * Created: 2014/12/20 1:42:02
 *  Author: user
 */ 

#include <asf.h>
#include "ECD300.h"
#include "ECD300_test.h"
#include "usb_protocol_cdc.h"

void printString(char * pString);
void printHex(unsigned char hex);
unsigned char getChar(void);

void printString(char * pString)
{
	ecd300PutString(ECD300_UART_2, pString);
}

void printHex(unsigned char hex)
{
	ecd300PutHexChar(ECD300_UART_2, hex);
}

unsigned char getChar(void)
{
	unsigned char c;
	char rc;

	for(rc=0;rc!=1;)
	{
		rc=ecd300PollChar(ECD300_UART_2, &c);
	}

	return c;
}

static bool _pollHexChar(unsigned char * p)
{
	char rc;

	if(NULL==p)
	{
		return false;
	}
	
	rc=ecd300PollChar(ECD300_UART_2, p);

	if(rc==1)
		return true;
	else
		return false;
}


/**
 * \brief EBI chip select configuration
 *
 * This struct holds the configuration for the chip select used to set up the
 * SRAM. The example code will use the EBI helper function to setup the
 * contents before writing the configuration using ebi_cs_write_config().
 */
static struct ebi_cs_config     _csConfig;

static void _ecd300ConfigEbi(void)
{
	unsigned long i;
	unsigned short j;
	
	/*
	 * Configure the EBI port with 17 address lines, enable both address
	 * latches, no low pin count mode, and set it in SRAM mode with 3-port
	 * EBI port.
	 */
	ebi_setup_port(17, 2, 0, EBI_PORT_SRAM | EBI_PORT_3PORT
			| EBI_PORT_CS0);

	/*
	 * Configure the EBI chip select for an 128 kB SRAM located at
	 * \ref BOARD_EBI_SDRAM_BASE.
	 */
	ebi_cs_set_mode(&_csConfig, EBI_CS_MODE_SRAM_gc);
	ebi_cs_set_address_size(&_csConfig, EBI_CS_ASPACE_128KB_gc);
	ebi_cs_set_base_address(&_csConfig, BOARD_EBI_SRAM_BASE);

	/*
	 * Refer to 36.2 in xmega manual and datasheet of cy62128e:
	 * In reading, the maximum time from OE# to data valid is 25ns,
	 * the maxium time from address valid to data valid is 55ns.
	 * So OE# should keep 55ns at least.
	 * In writing, the minimum delay from CE# to sampling is 40ns,
	 * the minimum delay from WE# to sampling is 40ns,
	 * So WE# should keep 40ns at least.
	 
	 * The EBI is configured to run at maximum
	 * speed, 64 MHz, which gives a minimum wait state of 16 ns per clock
	 * cycle. 3 additional clock cycles as wait state give enough headroom. (64ns (4 clks) > 55ns > 40ns)
	 */
	ebi_cs_set_sram_wait_states(&_csConfig, EBI_CS_SRWS_3CLK_gc);

	/* Write the chip select configuration into the EBI registers. */
	ebi_cs_write_config(0, &_csConfig);

	ebi_enable_cs(0, &_csConfig);

	/* Enable LED0: EBI is configured and enabled. */
	printString("EBI is configured and enabled\r\n");

#if 0
	//SRAM scan.
	for(i=0;i<0x20000;i+=2)
	{
		hugemem_write16(BOARD_EBI_SRAM_BASE+i, i>>1);
	}
	for(i=0;i<0x20000;i+=2)
	{
		j=hugemem_read16(BOARD_EBI_SRAM_BASE+i);
		if(j!=(i>>1))
			break;
	}
	if(i!=0x20000)
	{
		printString("SRAM scan failed at: ");
		printHex(i>>8);
		printString(", ");
		printHex(i);
		printString("\r\n");
	}
	else
	{
		printString("SRAM scan succeed\r\n");
	}
#endif
}

bool main_cdc_enable(uint8_t port)
{
	printString("Port: ");
	printHex(port);
	printString(" is enabled\r\n");
	
	return true;
}

void main_cdc_disable(uint8_t port)
{
	printString("Port: ");
	printHex(port);
	printString(" is disabled\r\n");
}

void main_cdc_set_dtr(uint8_t port, bool b_enable)
{
	if (b_enable) 
	{
		printString("Host opened serial port:");
		printHex(port);
		printString("\r\n");
	}
	else
	{
		printString("Host closed serial port:");
		printHex(port);
		printString("\r\n");
	}
}

void main_uart_rx_notify(uint8_t port)
{
	;
}

void main_uart_config(uint8_t port, usb_cdc_line_coding_t * cfg)
{
	printString("Host is configuring seiral port: ");
	printHex(port);
	printString("\r\n");
}


static void disable_all_channels(){
	PORTC_DIR=0x00;
	PORTB_DIR=0x00;
	PORTA_DIR=0x00;
	PORTK_DIR=0x00;
	PORTJ_DIR=0x00;
	PORTH_DIR=0x00;
	PORTF_DIR=0x00;
	PORTE_DIR=0x00;
	PORTC_OUT=0x00;
	PORTB_OUT=0x00;
	PORTA_OUT=0x00;
	PORTK_OUT=0x00;
	PORTJ_OUT=0x00;
	PORTH_OUT=0x00;
	PORTF_OUT=0x00;
	PORTE_OUT=0x00;
}

static void enable_channel(unsigned char channel)
{
	disable_all_channels();
	
	switch(channel)
	{
		case 1: //pc7
			PORTC_OUT=0x80;
			PORTC_DIR=0x80;
			break;
		case 2: //pc5
			PORTC_OUT=0x20;
			PORTC_DIR=0x20;
			break;
		case 3: //pc3
			PORTC_OUT=0x08;
			PORTC_DIR=0x08;
			break;
		case 4: //pc1
			PORTC_OUT=0x02;
			PORTC_DIR=0x02;
			break;
		case 5: //pb7
			PORTB_OUT=0x80;
			PORTB_DIR=0x80;
			break;
		case 6: //pb5
			PORTB_OUT=0x20;
			PORTB_DIR=0x20;
			break;
		case 7: //pb3
			PORTB_OUT=0x08;
			PORTB_DIR=0x08;
			break;
		case 8: //pb1
			PORTB_OUT=0x02;
			PORTB_DIR=0x02;
			break;
		case 9: //pa7
			PORTA_OUT=0x80;
			PORTA_DIR=0x80;
			break;
		case 10: //pa5
			PORTA_OUT=0x20;
			PORTA_DIR=0x20;
			break;
		case 11: //pa3
			PORTA_OUT=0x08;
			PORTA_DIR=0x08;
			break;
		case 12: //pa1
			PORTA_OUT=0x02;
			PORTA_DIR=0x02;
			break;
		case 13: //pk7
			PORTK_OUT=0x80;
			PORTK_DIR=0x80;
			break;
		case 14: //pk5
			PORTK_OUT=0x20;
			PORTK_DIR=0x20;
			break;
		case 15: //pk3
			PORTK_OUT=0x08;
			PORTK_DIR=0x08;
			break;
		case 16: //pk1
			PORTK_OUT=0x02;
			PORTK_DIR=0x02;
			break;
		case 17: //pj7
			PORTJ_OUT=0x80;
			PORTJ_DIR=0x80;
			break;
		case 18: //pj5
			PORTJ_OUT=0x20;
			PORTJ_DIR=0x20;
			break;
		case 19: //pj3
			PORTJ_OUT=0x08;
			PORTJ_DIR=0x08;
			break;
		case 20: //pj1
			PORTJ_OUT=0x02;
			PORTJ_DIR=0x02;
			break;
		case 21: //ph7
			PORTH_OUT=0x80;
			PORTH_DIR=0x80;
			break;
		case 22: //ph5
			PORTH_OUT=0x20;
			PORTH_DIR=0x20;
			break;
		case 23: //ph3
			PORTH_OUT=0x08;
			PORTH_DIR=0x08;
			break;
		case 24: //ph1
			PORTH_OUT=0x02;
			PORTH_DIR=0x02;
			break;
		case 25: //pf7
			PORTF_OUT=0x80;
			PORTF_DIR=0x80;
			break;
		case 26: //pf5
			PORTF_OUT=0x20;
			PORTF_DIR=0x20;
			break;
		case 27: //pf3
			PORTF_OUT=0x08;
			PORTF_DIR=0x08;
			break;
		case 28: //pf1
			PORTF_OUT=0x02;
			PORTF_DIR=0x02;
			break;
		case 29: //pe7
			PORTE_OUT=0x80;
			PORTE_DIR=0x80;
			break;
		case 30: //pe5
			PORTE_OUT=0x20;
			PORTE_DIR=0x20;
			break;
		case 31: //pe3
			PORTE_OUT=0x08;
			PORTE_DIR=0x08;
			break;
		case 32: //pe1
			PORTE_OUT=0x02;
			PORTE_DIR=0x02;
			break;
		default:
			break;
	}
}

void ecd300TestJbi(void)
{
	const unsigned char bufferLength=255;
	unsigned char inputBuffer[bufferLength];
	unsigned char outputBuffer[bufferLength];
	unsigned char inputProducerIndex=0;
	unsigned char inputConsumerIndex=0;
	unsigned char outputProducerIndex=0;
	unsigned char outputConsumerIndex=0;
	
	usart_rs232_options_t uartOption;
	unsigned char c;

	PORTA_DIR=0x00;
	PORTB_DIR=0x00;
	PORTC_DIR=0x00;
	PORTD_DIR=0x00;
	PORTE_DIR=0x00;
	PORTF_DIR=0x00;
	PORTH_DIR=0x00;
	PORTJ_DIR=0x00;
	PORTK_DIR=0x00;
	

	disableJtagPort();
	sysclk_init();
	sleepmgr_init();
	irq_initialize_vectors(); //enable LOW, MED and HIGH level interrupt in PMIC.
	cpu_irq_enable();

	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;
	ecd300InitUart(ECD300_UART_2, &uartOption);
	printString("Serial Port in Power Allocator was initialized\r\n");

	//PD0 works as indicator of host output
	PORTD_DIRSET = 0x01;

#if 1
	//cooperate with the programmer to program EPM1270.
	udc_start();
		
	while(1)
	{
		unsigned char key, tdo;
		
		
		if (udi_cdc_is_rx_ready()) 
		{
			//read a command from USB buffer.
			key = (unsigned char)udi_cdc_getc();
			
			// !!! buffer overflow is not checked for reason of simplicity.
			inputBuffer[inputProducerIndex] = key;
			outputBuffer[outputProducerIndex] = key;
			inputProducerIndex = (inputProducerIndex + 1) % bufferLength;
			outputProducerIndex = (outputProducerIndex + 1) % bufferLength;
			if(key == 0x0D) {
				outputBuffer[outputProducerIndex] = 0x0A;// Line Feed.
				outputProducerIndex = (outputProducerIndex + 1) % bufferLength;
			}
			
			printHex(key);
			printString("\r\n");
			
			//toggle PD0 to indicate character reception.
			if(PORTD_IN&0x01) {
				PORTD_OUTCLR = 0x01;
			}
			else {
				PORTD_OUTSET = 0x01;
			}
			
			if(key == 0x0D) { // \n
				unsigned char channel = 0;
				if(inputConsumerIndex != inputProducerIndex) {
					while(inputConsumerIndex != inputProducerIndex) {
						unsigned char c = inputBuffer[inputConsumerIndex];
						if((c >= '0') && (c <= '9')) {
							channel =  channel * 10 + c - '0';
							inputConsumerIndex = (inputConsumerIndex + 1) % bufferLength;
						}
						else {
							if(c != 0x0D) {
								// illegal character
								printString("Illegal command\r\n");
								channel = 0;
							}
							inputConsumerIndex = inputProducerIndex; //discard all content in inputBuffer.
							break;
						}
					}
				}
				enable_channel(channel);
			}
		}
		
		if(outputConsumerIndex != outputProducerIndex)
		{
			if(udi_cdc_is_tx_ready()) {
				udi_cdc_putc(outputBuffer[outputConsumerIndex]);
				outputConsumerIndex = (outputConsumerIndex + 1) % bufferLength;
			}
		}
	}
#endif

	while(1)
	{
		;
	}
}

