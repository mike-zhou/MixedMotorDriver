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
	PORTD_DIRCLR=0x10;
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
	
#if 0
	printString("size of unsigned char *:");
	printHex(sizeof(unsigned char *));
	printString("\r\n");
	printString("size of void *:");
	printHex(sizeof(void *));
	printString("\r\n");
	printString("size of unsigned long:");
	printHex(sizeof(unsigned long));
	printString("\r\n");
	printString("size of unsigned long *:");
	printHex(sizeof(unsigned long *));
	printString("\r\n");
	printString("size of int:");
	printHex(sizeof(int));
	printString("\r\n");
#endif

	_ecd300ConfigEbi();
#if 1
	//cooperate with the programmer to program EPM1270.
	udc_start();
	while(1)
	{
		unsigned char c, tdo;
		
		if (udi_cdc_is_rx_ready()) 
		{
			//read a command from USB buffer.
			c = (unsigned char)udi_cdc_getc();
			
			tdo=jbi_jtag_io(c&ECD300_JTAG_CMD_TMS_BIT, c&ECD300_JTAG_CMD_TDI_BIT, c&ECD300_JTAG_CMD_TDO_BIT);
			
			if(c&ECD300_JTAG_CMD_TDO_BIT)
			{// the programmer is waiting for the TDO.
				while(1)
				{
					if(udi_cdc_is_tx_ready())
					{
						udi_cdc_putc(tdo);
						break;
					}
				}
			}
		}
	}
#endif
	
	pJbc=BOARD_EBI_SRAM_BASE;
	jbcAmount=0;

#if 0
	udc_start();

	printString("\r\n Press 1 to receive the jbc file, other key to Program the EPM1270\r\n");
	c=getChar();
	if('1'==c)
	{
		unsigned char buffer[512];
		unsigned long index;
		unsigned long i;
		bool bJbcTransferStart;
		
		bJbcTransferStart=false;
		printString("Wait for the jbc file ... \r\n");
		while(1)
		{
			if (udi_cdc_is_rx_ready()) 
			{
				//read a command from USB buffer.
				c = (unsigned char)udi_cdc_getc();
				
				hugemem_write8(pJbc+jbcAmount, c);
				jbcAmount++;
				bJbcTransferStart=true;
				i=0;
			}
			if(bJbcTransferStart)
			{
				i++;
				if(i>=0x100000)
				{
					break;
				}
			}
		}

		printString("\r\n");
		printHex(jbcAmount>>8);
		printString(", ");
		printHex(jbcAmount);
		printString(" bytes recieved\r\n");

		printString("Save jbc file to Nand ...\r\n");
		//init NAND driver
		ecd300InitNand();
		//erase the block of NAND.
		ecd300EraseNandBlockEx(0, 1, true);
		//save the length of jbc content to page 0 of block 1.
		buffer[0]=jbcAmount;
		buffer[1]=jbcAmount>>8;
		buffer[2]=jbcAmount>>16;
		buffer[3]=jbcAmount>>24;
		ecd300ProgramNandPageEx(0, 1, 0, 0, buffer, 4, true);
		//save the content of jbc since page 1 of block 1.
		for(index=0; index<=(jbcAmount+511); index+=512)
		{
			for(i=0;i<512;i++)
			{
				buffer[i]=hugemem_read8(pJbc+index+i);
			}
			ecd300ProgramNandPageEx(0, 			//plane
									1, 				//block
									(index>>11)+1, 	//page	
									index&0x7ff,		//col
									buffer,
									512,
									true);
		}
		printString("jbc content is saved to block 1 of NAND chip\r\n");
	}

#else
	//tranfer jbc content from NAND to external SRAM
	{
		unsigned char buffer[512];
		unsigned long index;
		unsigned long i;

		//init NAND driver
		ecd300InitNand();
		//read the length of jbc
		ecd300ReadNandPageEx(0, 1, 0, 0, buffer, 4, true);
		jbcAmount=buffer[3];
		jbcAmount<<=8;
		jbcAmount+=buffer[2];
		jbcAmount<<=8;
		jbcAmount+=buffer[1];
		jbcAmount<<=8;
		jbcAmount+=buffer[0];
		//read the content of jbc.
		for(index=0; index<=(jbcAmount+511); index+=512)
		{
			ecd300ReadNandPageEx(0,
									1,
									(index>>11)+1, 	//page	
									index&0x7ff,		//col
									buffer,
									512,
									true);
			for(i=0; i<512; i++)
			{
				hugemem_write8(pJbc+index+i, buffer[i]);
			}
									
		}
	}

	jbi_main(pJbc, jbcAmount, "PROGRAM");
#endif

	while(1)
	{
		;
	}
}

