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

int jbi_main(unsigned long file_buffer_address, long file_length, char *action);
int jbi_jtag_io(int tms, int tdi, int read_tdo);

#define ECD300_JTAG_CMD_TMS_BIT 0x02
#define ECD300_JTAG_CMD_TDI_BIT 0x01
#define ECD300_JTAG_CMD_TDO_BIT 0x80

void printString(char * pString);
void printHex(unsigned char hex);
unsigned char getChar(void);

void printString(char * pString)
{
	ecd300PutString(ECD300_UART_1, pString);
}

void printHex(unsigned char hex)
{
	ecd300PutHexChar(ECD300_UART_1, hex);
}

unsigned char getChar(void)
{
	unsigned char c;
	char rc;

	for(rc=0;rc!=1;)
	{
		rc=ecd300PollChar(ECD300_UART_1, &c);
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
	
	rc=ecd300PollChar(ECD300_UART_1, p);

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

void ecd300TestJbi(void)
{
	usart_rs232_options_t uartOption;
	unsigned long jbcAmount;
	unsigned long pJbc;
	unsigned char c;

	{
		unsigned long i;

		PORTA_DIR=0xff;
		for(i=0;i<0x100000; i++)
		{
			if(i&0x10000)
			{
				PORTA_OUT=0xff;
			}
			else
			{
				PORTA_OUT=0x00;
			}
		}
	}

	PORTA_DIR=0x00;
	PORTB_DIR=0x00;
	PORTD_DIR=0x00;
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
	ecd300InitUart(ECD300_UART_1, &uartOption);
	printString("UART1 was initialized 9\r\n");


#if 1
	//cooperate with the programmer to program EPM1270.
	udc_start();
	
	//PORTB works as a counter of characters coming from host.
	PORTB_OUT = 0;
	PORTB_DIR = 0xff;
		
	while(1)
	{
		unsigned char c, tdo;
		
		
		if (udi_cdc_is_rx_ready()) 
		{
			//read a command from USB buffer.
			c = (unsigned char)udi_cdc_getc();
			
			printHex(c);
			printString("\r\n");
			PORTB_OUT = PORTB_IN + 1;
		}
	}
#endif

	while(1)
	{
		;
	}
}

