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

static void _testDelay(void)
{
	{
		PORTA_DIR=0xff;
		PORTA_OUT=0xff;
		printString("\r\nPress any key to test delay\r\n");
		getChar();
		PORTA_OUT=0x00;
		ecd300DelayMillisecond(60000);
		PORTA_OUT=0xff;
		printString("Delay test finishes\r\n");
	}
}

static void _testPdiRegister(void)
{
		char status;
		unsigned char buffer[512];
		unsigned short amount;
		buffer[0]=0xf0;
		
		status=xnvm_read_pdi_reset(buffer);
		if(STATUS_OK!=status)
		{
			printString("cannot read PDI reset register, status: ");
			printHex(status);
			printString("\r\n");
		}
		else
		{
			printString("Read PDI reset register successfully: ");
			printHex(buffer[0]);
			printString("\r\n");
		}

		status=xnvm_set_pdi_control(0x01);
		if(STATUS_OK!=status)
		{
			printString("cannot set PDI control register, status: ");
			printHex(status);
			printString("\r\n");
		}
		else
		{
			printString("Set PDI control register successfully: ");
			printString("\r\n");
		}

		buffer[0]=0xf0;
		status=xnvm_read_pdi_control(buffer);
		if(STATUS_OK!=status)
		{
			printString("cannot read PDI control register, status: ");
			printHex(status);
			printString("\r\n");
		}
		else
		{
			printString("Read PDI control register successfully: ");
			printHex(buffer[0]);
			printString("\r\n");
		}

		status=xnvm_put_dev_in_reset();
		if(STATUS_OK!=status)
		{
			printString("cannot put device to reset state, status: ");
			printHex(status);
			printString("\r\n");
		}
		else
		{
			printString("put device to reset state successfully");
			printString("\r\n");
		}

		printString("Press any key to pull device out of reset state\r\n");
		getChar();

		status=xnvm_pull_dev_out_of_reset();
		if(STATUS_OK!=status)
		{
			printString("cannot pull device out of reset state, status: ");
			printHex(status);
			printString("\r\n");
		}
		else
		{
			printString("pull device out of reset state successfully");
			printString("\r\n");
		}

}

static void _testEraseChip(void)
{
		char status;
		unsigned char buffer[512];
		unsigned short amount;
		buffer[0]=0xf0;

		/* Erase the target device */
		status=xnvm_chip_erase();
		if(STATUS_OK!=status)
		{
			printString("cannot erase chip, status: ");
			printHex(status);
			printString("\r\n");
		}
		else
		{
			printString("chip is erased successfully\r\n");
		}
}

static void _testReadFlashInByte(void)
{
		char status;
		unsigned char buffer[512];
		unsigned short amount;
		
		//flash memory is read successfully byte by byte.
		buffer[0]=0;
		buffer[1]=0;
		amount=xnvm_read_memory(XNVM_FLASH_BASE, buffer, 1);
		printString("amount: ");
		printHex(amount);
		printString(", content: ");
		printHex(buffer[0]);
		printString("\r\n");
		buffer[0]=0;
		buffer[1]=0;
		amount=xnvm_read_memory(XNVM_FLASH_BASE+1, buffer, 1);
		printString("amount: ");
		printHex(amount);
		printString(", content: ");
		printHex(buffer[0]);
		printString("\r\n");
		buffer[0]=0;
		buffer[1]=0;
		amount=xnvm_read_memory(XNVM_FLASH_BASE+2, buffer, 1);
		printString("amount: ");
		printHex(amount);
		printString(", content: ");
		printHex(buffer[0]);
		printString("\r\n");
		buffer[0]=0;
		buffer[1]=0;
		amount=xnvm_read_memory(XNVM_FLASH_BASE+3, buffer, 1);
		printString("amount: ");
		printHex(amount);
		printString(", content: ");
		printHex(buffer[0]);
		printString("\r\n");
		buffer[0]=0;
		buffer[1]=0;
		amount=xnvm_read_memory(XNVM_FLASH_BASE+4, buffer, 1);
		printString("amount: ");
		printHex(amount);
		printString(", content: ");
		printHex(buffer[0]);
		printString("\r\n");
		buffer[0]=0;
		buffer[1]=0;
		amount=xnvm_read_memory(XNVM_FLASH_BASE+5, buffer, 1);
		printString("amount: ");
		printHex(amount);
		printString(", content: ");
		printHex(buffer[0]);
		printString("\r\n");
		buffer[0]=0;
		buffer[1]=0;
		amount=xnvm_read_memory(XNVM_FLASH_BASE+6, buffer, 1);
		printString("amount: ");
		printHex(amount);
		printString(", content: ");
		printHex(buffer[0]);
		printString("\r\n");
}

static void _testReadFlashInBytes(void)
{
		char status;
		unsigned char buffer[512];
		unsigned short amount;

		//read Application
		{
			unsigned short i;

			for(i=0;i<512;i++)
				buffer[i]=0;
		}
		amount=xnvm_read_memory(XNVM_FLASH_BASE, buffer, 512);
		printString("Amount of data read: ");
		printHex(amount>>8);
		printString(", ");
		printHex(amount);
		printString("\r\n");
		if(amount!=0)
		{
			unsigned short i;

			for(i=0;i<amount;i++)
			{
				if(0==(i&0xf))
				{
					printString("\r\n");
				}
				{
					printHex(buffer[i]);
					printString(", ");
				}
			}
		}
}

static void _testProgramFlash(void)
{
	status_code_t status;
	unsigned char buffer[512];
	unsigned short amount;

	unsigned short pageIndex;
	unsigned short i;
	unsigned long counter;
#if 1
	for(pageIndex=0, counter=0; pageIndex<256; pageIndex++)
	{
		for(i=0;i<512;i+=2, counter++)
		{
			buffer[i]=counter>>8;
			buffer[i+1]=counter;
		}

		status=xnvm_erase_program_flash_page(pageIndex*512UL, buffer, 512);
		if(STATUS_OK!=status)
		{
			printString("cannot program application flash page, status: ");
			printHex(status);
			printString(", pageIndex");
			printHex(pageIndex);
			printString("\r\n");
			break;
		}
	}

	for(pageIndex=0, counter=0; pageIndex<16; pageIndex++)
	{
		for(i=0;i<512;i+=2, counter++)
		{
			buffer[i]=counter>>8;
			buffer[i+1]=counter;
		}

		status=xnvm_erase_program_flash_page(pageIndex*512UL+0x20000, buffer, 512);
		if(STATUS_OK!=status)
		{
			printString("cannot program bootloader flash page, status: ");
			printHex(status);
			printString(", pageIndex");
			printHex(pageIndex);
			printString("\r\n");
			break;
		}
	}
#endif
	for(pageIndex=0, counter=0; pageIndex<256; pageIndex++)
	{
		unsigned short temp;
		
		temp=xnvm_read_memory(XNVM_FLASH_BASE+pageIndex*512UL, buffer, 512);
		if(512!=temp)
		{
			printString("cannot read application flash page, status: ");
			printHex(status);
			printString(", pageIndex");
			printHex(pageIndex);
			printString("\r\n");
			break;
		}
		
		for(i=0;i<512;i+=2, counter++)
		{
			temp=buffer[i];
			temp<<=8;
			temp+=buffer[i+1];
			if(temp!=counter)
			{
				break;
			}
		}
		if(512!=i)
		{
			printString("content error at application page: ");
			printHex(pageIndex);
			printString(", position: ");
			printHex(i>>8);
			printString(", ");
			printHex(i);
			printString(", counter: ");
			printHex(counter>>8);
			printString(", ");
			printHex(counter);
			printString(", temp: ");
			printHex(temp>>8);
			printString(", ");
			printHex(temp);
			printString("\r\n");

			for(i=0;i<512;i++)
			{
				if(0==(i&0xf))
				{
					printString("\r\n");
				}
				{
					printHex(buffer[i]);
					printString(", ");
				}
			}
			break;
		}
	}
	
	for(pageIndex=0, counter=0; pageIndex<16; pageIndex++)
	{
		unsigned short temp;
		
		temp=xnvm_read_memory(XNVM_FLASH_BASE+pageIndex*512UL+0x20000, buffer, 512);
		if(512!=temp)
		{
			printString("cannot read bootloader flash page, status: ");
			printHex(status);
			printString(", pageIndex");
			printHex(pageIndex);
			printString("\r\n");
			break;
		}
		
		for(i=0;i<512;i+=2, counter++)
		{
			temp=buffer[i];
			temp<<=8;
			temp+=buffer[i+1];
			if(temp!=counter)
			{
				break;
			}
		}
		if(512!=i)
		{
			printString("content error at bootloader page: ");
			printHex(pageIndex);
			printString(", position: ");
			printHex(i>>8);
			printString(", ");
			printHex(i);
			printString(", counter: ");
			printHex(counter>>8);
			printString(", ");
			printHex(counter);
			printString(", temp: ");
			printHex(temp>>8);
			printString(", ");
			printHex(temp);
			printString("\r\n");

			for(i=0;i<512;i++)
			{
				if(0==(i&0xf))
				{
					printString("\r\n");
				}
				{
					printHex(buffer[i]);
					printString(", ");
				}
			}
			break;
		}
	}

	printString("\r\n_testProgtamFlash finishes\r\n");
}

static void _testEeprom(void)
{
	status_code_t status;
	unsigned char buffer[32];
	unsigned short amount;

	unsigned short pageIndex;
	unsigned short i;
	unsigned long counter;

	for(pageIndex=0, counter=0; pageIndex<64; pageIndex++)
	{
		for(i=0; i<32; i+=2, counter++)
		{
			buffer[i]=counter>>8;
			buffer[i+1]=counter;
		}

		status=xnvm_erase_program_eeprom_page(pageIndex*32, buffer, 32);
		if(STATUS_OK!=status)
		{
			printString("cannot program EEPROM page, status: ");
			printHex(status);
			printString(", pageIndex");
			printHex(pageIndex);
			printString("\r\n");
			break;
		}
	}

	for(pageIndex=0, counter=0; pageIndex<64; pageIndex++)
	{
		unsigned short temp;
		
		temp=xnvm_read_memory(XNVM_EEPROM_BASE+pageIndex*32UL, buffer, 32);
		if(32!=temp)
		{
			printString("cannot read EEPROM page, status: ");
			printHex(status);
			printString(", pageIndex");
			printHex(pageIndex);
			printString("\r\n");
			break;
		}
		
		for(i=0;i<32;i+=2, counter++)
		{
			temp=buffer[i];
			temp<<=8;
			temp+=buffer[i+1];
			if(temp!=counter)
			{
				break;
			}
		}
		if(32!=i)
		{
			printString("content error at EEPROM page: ");
			printHex(pageIndex);
			printString(", position: ");
			printHex(i>>8);
			printString(", ");
			printHex(i);
			printString(", counter: ");
			printHex(counter>>8);
			printString(", ");
			printHex(counter);
			printString(", temp: ");
			printHex(temp>>8);
			printString(", ");
			printHex(temp);
			printString("\r\n");

			for(i=0;i<32;i++)
			{
				if(0==(i&0xf))
				{
					printString("\r\n");
				}
				{
					printHex(buffer[i]);
					printString(", ");
				}
			}
			break;
		}
	}

	printString("\r\n_testEeprom finishes\r\n");
}

static void _testUserSignature(void)
{
	status_code_t status;
	unsigned char buffer[512];
	unsigned short amount;

	unsigned short pageIndex;
	unsigned short i;
	unsigned long counter;

	for(i=0;i<512;i++)
	{
		buffer[i]=i;
	}

	status=xnvm_erase_program_user_sign(0, buffer, 512);
	if(STATUS_OK!=status)
	{
		printString("cannot program user signature, status: ");
		printHex(status);
		printString("\r\n");
	}

	for(i=0;i<512;i++)
	{
		buffer[i]=0;
	}
	i=xnvm_read_memory(XNVM_SIGNATURE_BASE, buffer, 512);
	if(512!=i)
	{
		printString("cannot read user signature\r\n");
	}
	else
	{
		for(i=0;i<512;i++)
		{
			if(buffer[i]!=(i&0xff))
			{
				break;
			}
		}
		if(512!=i)
		{
			printString("error in user signature, index: ");
			printHex(i>>8);
			printString(", ");
			printHex(i);
			printString("\r\n");
		}
	}

	printString("\r\n_testUserSignature finishes\r\n");
}

static void _testFuse(void)
{
	status_code_t status;
	unsigned char buffer[8];
	unsigned short amount;

	unsigned short pageIndex;
	unsigned short i;
	unsigned long counter;

	_testEraseChip();
	
	i=xnvm_read_memory(XNVM_FUSE_BASE, buffer, 8);
	if(8!=i)
	{
		printString("cannot read fuse and lock, status: ");
		printHex(status);
		printString("\r\n");
	}
	else
	{
		printString("Fuse and Lock after erasing chip:\r\n    ");
		for(i=0;i<8;i++)
		{
			printHex(buffer[i]);
			printString(", ");
		}
		printString("\r\n");
	}

	status=xnvm_write_fuse_bit(4, 0xfe, 1000);
	if(STATUS_OK!=status)
	{
		printString("cannot program user signature, status: ");
		printHex(status);
		printString("\r\n");
	}
	else
	{
		for(i=0;i<8;i++)
		{
			buffer[i]=0;
		}
		i=xnvm_read_memory(XNVM_FUSE_BASE, buffer, 8);
		if(8!=i)
		{
			printString("cannot read fuse and lock, status: ");
			printHex(status);
			printString("\r\n");
		}
		else
		{
			printString("Fuse and Lock:\r\n    ");
			for(i=0;i<8;i++)
			{
				printHex(buffer[i]);
				printString(", ");
			}
			printString("\r\n");
		}
	}
	
	printString("\r\n_testFuse finishes\r\n");
}

static void _testLockBit(void)
{
	status_code_t status;
	unsigned char buffer[8];
	unsigned short amount;

	unsigned short pageIndex;
	unsigned short i;
	unsigned long counter;

	_testEraseChip();
	
	i=xnvm_read_memory(XNVM_FUSE_BASE, buffer, 8);
	if(8!=i)
	{
		printString("cannot read fuse and lock, status: ");
		printHex(status);
		printString("\r\n");
	}
	else
	{
		printString("Fuse and Lock after erasing chip:\r\n    ");
		for(i=0;i<8;i++)
		{
			printHex(buffer[i]);
			printString(", ");
		}
		printString("\r\n");
	}

	status=xnvm_write_lock_bit(0xfc, 1000);
	if(STATUS_OK!=status)
	{
		printString("cannot program user signature, status: ");
		printHex(status);
		printString("\r\n");
	}
	else
	{
		for(i=0;i<8;i++)
		{
			buffer[i]=0;
		}
		i=xnvm_read_memory(XNVM_FUSE_BASE, buffer, 8);
		if(8!=i)
		{
			printString("cannot read fuse and lock, status: ");
			printHex(status);
			printString("\r\n");
		}
		else
		{
			printString("Fuse and Lock:\r\n    ");
			for(i=0;i<8;i++)
			{
				printHex(buffer[i]);
				printString(", ");
			}
			printString("\r\n");
		}
	}

	printString("\r\n_testLockBit finishes\r\n");
}

static void _testInitNvram(void)
{
		char status;
		unsigned char buffer[512];
		unsigned short amount;
		
		/* Initialize the PDI interface */
		status=xnvm_init();
		if(STATUS_OK!=status)
		{
			printString("cannot initialize XNVM driver, status: ");
			printHex(status);
			printString("\r\n");
		}
		else
		{
			printString("XNVM is intialized successfully\r\n");
		}

}

static unsigned char _getCdcChar(void)
{
	unsigned char c;

	for(;;)
	{
		if (udi_cdc_is_rx_ready()) 
		{
			//read a character from USB buffer.
			c = (unsigned char)udi_cdc_getc();
			break;
		}
	}
	
	return c;
}

static void _sendCdcChar(unsigned char c)
{
	for(;;)
	{
		if(udi_cdc_is_tx_ready())
		{
			udi_cdc_putc(c);
			break;
		}
	}
}

#define MAX_LENGTH_OF_HEX_LINE 128
#define FLASH_PAGE_BUFFER_SIZE 512
//define as global variables so that SRAM usage can be estimated easily.
static unsigned char pageBuffer[FLASH_PAGE_BUFFER_SIZE];
static unsigned char hexLine[MAX_LENGTH_OF_HEX_LINE+1];
static unsigned char commandLine[MAX_LENGTH_OF_HEX_LINE/2+1];
	
static void _processOneHexFile(void)
{
	unsigned char c;
	
	unsigned long segmentAddress;
	unsigned long pageOffsetInSegment;
	unsigned long pageBufferIndex;
	unsigned char hexLineLength;
	bool hexLineQualified;
	bool endOfFileProcessed;
	bool deviceErased;
	
	unsigned char commandLineLength;

	segmentAddress=0;
	pageOffsetInSegment=0;
	endOfFileProcessed=false;
	deviceErased=false;
	//initialize page buffer.
	for(pageBufferIndex=0;pageBufferIndex<FLASH_PAGE_BUFFER_SIZE;pageBufferIndex++)
	{
		pageBuffer[pageBufferIndex]=0xff;
	}
	//process the hex file line by line.
	for(;;)
	{
		for(hexLineLength=0; hexLineLength<MAX_LENGTH_OF_HEX_LINE; hexLineLength++)
		{
			hexLine[hexLineLength]=0;
		}

		//find the start of a HEX line.
		for(hexLineLength=0;;)
		{
			c=_getCdcChar();
			if(':'==c)
			{
				if(false==deviceErased)
				{
					_testInitNvram();
					_testEraseChip();
					deviceErased=true;
				}
				hexLine[0]=c;
				hexLineLength++;
				break;
			}
		}
		//receive the reminder of this line.
		for(;hexLineLength<MAX_LENGTH_OF_HEX_LINE;hexLineLength++)
		{
			c=_getCdcChar();
			if((c>='0')&&(c<='9'))
			{
				hexLine[hexLineLength]=c;
			}
			else if((c>='a')&&(c<='f'))
			{
				hexLine[hexLineLength]=c;
			}
			else if((c>='A')&&(c<='F'))
			{
				hexLine[hexLineLength]=c;
			}
			else
			{
				break;
			}
		}
		//a hex line is received.
		if(MAX_LENGTH_OF_HEX_LINE==hexLineLength)
		{
			//over flow
			hexLine[MAX_LENGTH_OF_HEX_LINE]=0;
			hexLineQualified=false;
		}
		else
		{
			//check the hex line.
			if(hexLineLength&0x1)
			{
				unsigned char i;

				//translate the hex line to command line
				for(i=1, commandLineLength=0;i<hexLineLength;i+=2, commandLineLength++)
				{
					//the first byte.
					if((hexLine[i]>='0')&&(hexLine[i]<='9'))
					{
						commandLine[commandLineLength]=hexLine[i]-'0';
					}
					if((hexLine[i]>='a')&&(hexLine[i]<='f'))
					{
						commandLine[commandLineLength]=hexLine[i]-'a'+10;
					}
					if((hexLine[i]>='A')&&(hexLine[i]<='F'))
					{
						commandLine[commandLineLength]=hexLine[i]-'A'+10;
					}
					//the second byte.
					commandLine[commandLineLength]<<=4;
					if((hexLine[i+1]>='0')&&(hexLine[i+1]<='9'))
					{
						commandLine[commandLineLength]+=hexLine[i+1]-'0';
					}
					if((hexLine[i+1]>='a')&&(hexLine[i+1]<='f'))
					{
						commandLine[commandLineLength]+=hexLine[i+1]-'a'+10;
					}
					if((hexLine[i+1]>='A')&&(hexLine[i+1]<='F'))
					{
						commandLine[commandLineLength]+=hexLine[i+1]-'A'+10;
					}
				}

				//check sum
				{
					unsigned char sum;
					unsigned char i;

					for(i=0, sum=0; i<commandLineLength; i++)
					{
						sum+=commandLine[i];
					}
					if(0!=sum)
					{
						hexLineQualified=false;
					}
					else
					{
						hexLineQualified=true;
					}
				}
			}
			else
			{
				//the correct value of hexLineLength should be odd.
				hexLineQualified=false;
			}
		}
		//echo back this line
		{
			unsigned char i;

			for(i=0;i<hexLineLength;i++)
			{
				_sendCdcChar(hexLine[i]);
			}
			hexLine[hexLineLength]=0;
			_sendCdcChar(hexLine[hexLineLength]);
		}

		if(hexLineQualified)
		{
			unsigned char count;
			unsigned long address;
			unsigned char type;
			unsigned char * pData;

			count=commandLine[0];
			address=commandLine[1];
			address<<=8;
			address+=commandLine[2];
			type=commandLine[3];
			pData=&commandLine[4];

			switch(type)
			{
				case 0x00: //data
				{
					if(address<pageOffsetInSegment)
					{
						printString("Error, address in data record should be increasing\r\n"); 
					}
					else
					{
						status_code_t status;
						unsigned short dataIndex;

						//save the data in current page.
						for(dataIndex=0; 
							(dataIndex<count)&&((address+dataIndex-pageOffsetInSegment)<FLASH_PAGE_BUFFER_SIZE); 
							dataIndex++)
						{
							pageBuffer[address+dataIndex-pageOffsetInSegment]=pData[dataIndex];
						}

						if(dataIndex<count)
						{
							//a new page is needed.
							unsigned short i;
							
							//save current page.
							status=xnvm_erase_program_flash_page(segmentAddress+pageOffsetInSegment, pageBuffer, FLASH_PAGE_BUFFER_SIZE);
							printString("program page: ");
							printHex((segmentAddress+pageOffsetInSegment)>>16);
							printString(", ");
							printHex((segmentAddress+pageOffsetInSegment)>>8);
							printString(", ");
							printHex((segmentAddress+pageOffsetInSegment)>>0);
							printString("\r\n");
							if(STATUS_OK!=status)
							{
								printString("Error when current page is saved, segmentAddress: ");
								printHex(segmentAddress>>16);
								printString(", ");
								printHex(segmentAddress>>8);
								printString(", ");
								printHex(segmentAddress);
								printString(", pageOffset: ");
								printHex(pageOffsetInSegment>>8);
								printString(", ");
								printHex(pageOffsetInSegment);
								printString("\r\n");
							}
							//refresh page buffer.	
							for(i=0;i<FLASH_PAGE_BUFFER_SIZE; i++)
							{
								pageBuffer[i]=0xff;
							}
							//update page offset.
							pageOffsetInSegment=(address+dataIndex)-((address+dataIndex)&(FLASH_PAGE_BUFFER_SIZE-1));

							//save the left data.
							for(; (dataIndex<count)&&((address+dataIndex-pageOffsetInSegment)<FLASH_PAGE_BUFFER_SIZE); 
								dataIndex++)
							{
								pageBuffer[address+dataIndex-pageOffsetInSegment]=pData[dataIndex];
							}
							if(dataIndex<count)
							{
								printString("Error, data address in a segment is not continuous, segmentAddress: ");
								printHex(segmentAddress>>16);
								printString(", ");
								printHex(segmentAddress>>8);
								printString(", ");
								printHex(segmentAddress);
								printString(", data address: ");
								printHex(address>>8);
								printString(", ");
								printHex(address);
								printString("\r\n");
							}
						}
					}
					
					break;
				}
				case 0x02: //extended segment address
				{
					status_code_t status;

					//save flash page.
					status=xnvm_erase_program_flash_page(segmentAddress+pageOffsetInSegment, pageBuffer, FLASH_PAGE_BUFFER_SIZE);
					printString("program page: ");
					printHex((segmentAddress+pageOffsetInSegment)>>16);
					printString(", ");
					printHex((segmentAddress+pageOffsetInSegment)>>8);
					printString(", ");
					printHex((segmentAddress+pageOffsetInSegment)>>0);
					printString("\r\n");
					if(STATUS_OK!=status)
					{
						printString("Error when new segment, segmentAddress: ");
						printHex(segmentAddress>>16);
						printString(", ");
						printHex(segmentAddress>>8);
						printString(", ");
						printHex(segmentAddress);
						printString(", pageOffset: ");
						printHex(pageOffsetInSegment>>8);
						printString(", ");
						printHex(pageOffsetInSegment);
						printString("\r\n");
					}

						
					//update segment address
					segmentAddress=pData[0];
					segmentAddress<<=8;
					segmentAddress+=pData[1];
					segmentAddress<<=4;
					//update page offset
					pageOffsetInSegment=0;
						
					{
						unsigned short temp;
						
						for(temp=0;temp<FLASH_PAGE_BUFFER_SIZE; temp++)
						{
							pageBuffer[temp]=0xff;
						}
					}
					
					break;
				}
				case 0x01: //end of file
				{
					status_code_t status;

					//save flash page.
					status=xnvm_erase_program_flash_page(segmentAddress+pageOffsetInSegment, pageBuffer, FLASH_PAGE_BUFFER_SIZE);
					printString("program page: ");
					printHex((segmentAddress+pageOffsetInSegment)>>16);
					printString(", ");
					printHex((segmentAddress+pageOffsetInSegment)>>8);
					printString(", ");
					printHex((segmentAddress+pageOffsetInSegment)>>0);
					printString("\r\n");
					if(STATUS_OK!=status)
					{
						printString("Error when new segment, segmentAddress: ");
						printHex(segmentAddress>>16);
						printString(", ");
						printHex(segmentAddress>>8);
						printString(", ");
						printHex(segmentAddress);
						printString(", pageOffset: ");
						printHex(pageOffsetInSegment>>8);
						printString(", ");
						printHex(pageOffsetInSegment);
						printString("\r\n");
					}
					endOfFileProcessed=true;
					
					break;
				}
				default:
				{
					printString("Error, unknown type of Hex line\r\n");
					break;
				}
			}
		}
		else
		{
			printString("\"");
			printString(hexLine);
			printString("\" is not qualified\r\n");
		}

		if(endOfFileProcessed)
		{
			break;
		}
	}
	
	//reset the device.
	xnvm_put_dev_in_reset();
	xnvm_pull_dev_out_of_reset();
	
	xnvm_deinit();
}

void ecd300TestPdi(void)
{
	usart_rs232_options_t uartOption;
	

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
	printString("UART1 was initialized 3\r\n");

	ecd300DelayInit();
	udc_start();

	while(1)
	{
		_processOneHexFile();
	}
}

