/*
 * ECD300_test.c
 *
 * Created: 2014/12/20 1:42:02
 *  Author: user
 */ 

#include <asf.h>
#include "ECD300.h"
#include "..\player\jbi_player.h"

#define ECD300_JTAG_PORT	PORTB
#define ECD300_JTAG_TMS 	PIN7_bm
#define ECD300_JTAG_TDI	PIN6_bm
#define ECD300_JTAG_TCK	PIN5_bm
#define ECD300_JTAG_TDO	PIN4_bm

#define ECD300_JAM_FILE_BASE 0x7f000000
#define ECD300_JAM_FILE_SIZE 0x00100000

#define ECD300_JAM_EXTERNAL_RAM_BASE 0x7f800000
#define ECD300_JAM_EXTERNAL_RAM_SIZE 0x10000

#define ECD300_JAM_INTERNAL_RAM_SIZE 3000

static unsigned long jamExternalRamOffset;
static unsigned long jamInternalRamOffset;

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

static void calibrate_delay(void)
{
	/**
	* configure 32-bit timer.
	*/
	//Use TCC0 as the lower 16-bit of a 32-bit timer.
	tc_enable(&TCC0);
	tc_reset(&TCC0);
	tc_set_wgm(&TCC0, TC_WG_NORMAL);
	tc_write_period(&TCC0, 0xffff);
	//frequency of clock is a bit slower than 4MHz.
	tc_write_clock_source(&TCC0, TC_CLKSEL_DIV8_gc);
	
	//Use TCC1 as the higher 16-bit of a 32-bit timer.
	tc_enable(&TCC1);
	tc_reset(&TCC1);
	tc_set_wgm(&TCC1, TC_WG_NORMAL);
	tc_write_period(&TCC1, 0xffff);
	sysclk_enable_module(SYSCLK_PORT_GEN, SYSCLK_EVSYS);
	EVSYS_CH0MUX=EVSYS_CHMUX_TCC0_OVF_gc;
	tc_write_clock_source(&TCC1, TC_CLKSEL_EVCH0_gc);
	tc_set_input_capture(&TCC1, TC_EVSEL_CH0_gc, TC_EVACT_OFF_gc);

	tc_restart(&TCC0);
	tc_restart(&TCC1);
}

long stubJtagInit(void)
{
	calibrate_delay();
	
	ECD300_JTAG_PORT.OUTSET=ECD300_JTAG_TCK|ECD300_JTAG_TMS|ECD300_JTAG_TDI;
	ECD300_JTAG_PORT.DIRSET=ECD300_JTAG_TCK|ECD300_JTAG_TMS|ECD300_JTAG_TDI;

	jamInternalRamOffset=0;
	jamExternalRamOffset=0;
	
	return 0;
}

long stubJtagRelease(void)
{
	return 0;
}

long stubJtagIo(long tms, long tdi, long bReadTdo)
{
	long tdo = 0;

	if(tms)
	{
		ECD300_JTAG_PORT.OUTSET = ECD300_JTAG_TMS;
	}
	else
	{
		ECD300_JTAG_PORT.OUTCLR = ECD300_JTAG_TMS;
	}

	if(tdi)
	{
		ECD300_JTAG_PORT.OUTSET = ECD300_JTAG_TDI;
	}
	else
	{
		ECD300_JTAG_PORT.OUTCLR = ECD300_JTAG_TDI;
	}

	if(bReadTdo)
	{
		tdo=(ECD300_JTAG_PORT.IN & ECD300_JTAG_TDO)?1:0;
	}
	
	//clock
	ECD300_JTAG_PORT.OUTSET = ECD300_JTAG_TCK;
	stubDelay(0);//the execution of this calling is enough for the delay.
	ECD300_JTAG_PORT.OUTCLR = ECD300_JTAG_TCK;
	
	return (tdo);
}

void stubDelay(unsigned long microSeconds)
{
	unsigned long preStamp, curStamp, duration;
	unsigned short stampLow, stampHigh;

	stampLow=tc_read_count(&TCC0);
	stampHigh=tc_read_count(&TCC1);
	preStamp=stampHigh;
	preStamp=preStamp<<16;
	preStamp+=stampLow;
	if(stampLow>tc_read_count(&TCC0))
	{
		//overflow occur.
		stampLow=tc_read_count(&TCC0);
		stampHigh=tc_read_count(&TCC1);
		preStamp=stampHigh;
		preStamp=preStamp<<16;
		preStamp+=stampLow;
	}
	

	for(;;)
	{
		stampLow=tc_read_count(&TCC0);
		stampHigh=tc_read_count(&TCC1);
		curStamp=stampHigh;
		curStamp=curStamp<<16;
		curStamp+=stampLow;
		if(stampLow>tc_read_count(&TCC0))
		{
			//overflow occur.
			stampLow=tc_read_count(&TCC0);
			stampHigh=tc_read_count(&TCC1);
			curStamp=stampHigh;
			curStamp=curStamp<<16;
			curStamp+=stampLow;
		}
		
		if(curStamp>preStamp)
		{
			duration=curStamp-preStamp;
		}
		else
		{
			duration=0xffffffff-preStamp;
			duration+=curStamp+1;
		}
		
		//timer at 4MHz.
		if((duration>>2)>=microSeconds)
		{
			break;
		}
	}
}

#if 0
static unsigned char jamInternalRamPool[ECD300_JAM_INTERNAL_RAM_SIZE];
#endif

void * stubMalloc(unsigned long size)
{
	void * rc=NULL;
#if 0
	size=(size+3)&(~3L);
	if((size+jamInternalRamOffset)<=ECD300_JAM_INTERNAL_RAM_SIZE)
	{
		rc=(void *)(jamInternalRamPool+jamInternalRamOffset);
		jamInternalRamOffset+=size;
	}
	printString("stubMalloc: ");
	printHex(jamInternalRamOffset>>8);
	printString(", ");
	printHex(jamInternalRamOffset>>0);
	printString("\r\n");
#endif	
	return rc;
}

unsigned long stubMallocExternalRam(unsigned long size)
{
	unsigned long rc=0;

	size=(size+3)&(~3L);
	if((size+jamExternalRamOffset)<=ECD300_JAM_EXTERNAL_RAM_SIZE)
	{
		rc=ECD300_JAM_EXTERNAL_RAM_BASE+jamExternalRamOffset;
		jamExternalRamOffset+=size;
	}
	printString("stubMallocExt: ");
	printHex(jamExternalRamOffset>>8);
	printString(", ");
	printHex(jamExternalRamOffset>>0);
	printString("\r\n");

	return rc;
}

void stubFree(unsigned long address)
{

}

void stubPrintString(char * pString)
{
	printString(pString);
}

void stubPrintHex(unsigned char c)
{
	printHex(c);
}

char getInt8(unsigned long address)
{
	return (char)getUint8Array(address, 0);
}

char	getInt8Array(unsigned long address, long index)
{
	return (char)getUint8Array(address, index);
}

void setInt8(unsigned long address, char value)
{
	setUint8Array(address, 0, value);
}

void setInt8Array(unsigned long address, long index, char value)
{
	setUint8Array(address, index, value);
}

unsigned char getUint8(unsigned long address)
{
	return getUint8Array(address, 0);
}

unsigned char getUint8Array(unsigned long address, long index)
{
	if((address>=ECD300_JAM_EXTERNAL_RAM_BASE)&&
		(address<(ECD300_JAM_EXTERNAL_RAM_BASE+ECD300_JAM_EXTERNAL_RAM_SIZE)))
	{
		//external SRAM
		return hugemem_read8(BOARD_EBI_SRAM_BASE+0x10000+(address-ECD300_JAM_EXTERNAL_RAM_BASE)+index);
	}
	else if((address>=ECD300_JAM_FILE_BASE)&&
		(address<(ECD300_JAM_FILE_BASE+ECD300_JAM_FILE_SIZE)))
	{
		//JAM file buffer
		return hugemem_read8(BOARD_EBI_SRAM_BASE+(address-ECD300_JAM_FILE_BASE)+index);
	}
	else
	{
		unsigned char * p=(unsigned char *)(address+index);
		
		return p[0];
	}
}

void setUint8(unsigned long address, unsigned char value)
{
	setUint8Array(address, 0, value);
}

void	setUint8Array(unsigned long address, long index, unsigned char value)
{
	if((address>=ECD300_JAM_EXTERNAL_RAM_BASE)&&
		(address<(ECD300_JAM_EXTERNAL_RAM_BASE+ECD300_JAM_EXTERNAL_RAM_SIZE)))
	{
		//external SRAM
		hugemem_write8(BOARD_EBI_SRAM_BASE+0x10000+(address-ECD300_JAM_EXTERNAL_RAM_BASE)+index, value);
	}
	else if((address>=ECD300_JAM_FILE_BASE)&&
		(address<(ECD300_JAM_FILE_BASE+ECD300_JAM_FILE_SIZE)))
	{
		//JAM file buffer
		hugemem_write8(BOARD_EBI_SRAM_BASE+(address-ECD300_JAM_FILE_BASE)+index, value);
	}
	else
	{
		unsigned char * p=(unsigned char *)(address+index);
		
		p[0]=value;
	}
}

long getInt32(unsigned long address)
{
	return (long)getUint32Array(address, 0);
}

long getInt32Array(unsigned long address, long index)
{
	return (long)getUint32Array(address, index);
}

void setInt32(unsigned long address, long value)
{
	setUint32Array(address, 0, value);
}

void setInt32Array(unsigned long address, long index, long value)
{
	setUint32Array(address, index, value);
}

unsigned long 	getUint32(unsigned long address)
{
	return getUint32Array(address, 0);
}

unsigned long getUint32Array(unsigned long address, long index)
{
	if((address>=ECD300_JAM_EXTERNAL_RAM_BASE)&&
		(address<(ECD300_JAM_EXTERNAL_RAM_BASE+ECD300_JAM_EXTERNAL_RAM_SIZE)))
	{
		//external SRAM
		return hugemem_read32(BOARD_EBI_SRAM_BASE+0x10000+(address-ECD300_JAM_EXTERNAL_RAM_BASE)+(index<<2));
	}
	else if((address>=ECD300_JAM_FILE_BASE)&&
		(address<(ECD300_JAM_FILE_BASE+ECD300_JAM_FILE_SIZE)))
	{
		//JAM file buffer
		return hugemem_read32(BOARD_EBI_SRAM_BASE+(address-ECD300_JAM_FILE_BASE)+(index<<2));
	}
	else
	{
		unsigned long * p=(unsigned long *)(address+(index<<2));
		
		return p[0];
	}
}

void setUint32(unsigned long address, unsigned long value)
{
	setUint32Array(address, 0, value);
}

void	setUint32Array(unsigned long address, long index, unsigned long value)
{
	if((address>=ECD300_JAM_EXTERNAL_RAM_BASE)&&
		(address<(ECD300_JAM_EXTERNAL_RAM_BASE+ECD300_JAM_EXTERNAL_RAM_SIZE)))
	{
		//external SRAM
		hugemem_write32(BOARD_EBI_SRAM_BASE+0x10000+(address-ECD300_JAM_EXTERNAL_RAM_BASE)+(index<<2), value);
	}
	else if((address>=ECD300_JAM_FILE_BASE)&&
		(address<(ECD300_JAM_FILE_BASE+ECD300_JAM_FILE_SIZE)))
	{
		//JAM file buffer
		hugemem_write32(BOARD_EBI_SRAM_BASE+(address-ECD300_JAM_FILE_BASE)+(index<<2), value);
	}
	else
	{
		unsigned long * p=(unsigned long *)(address+(index<<2));
		
		p[0]=value;
	}
}

static void _receiveJamFile(void)
{
	unsigned char buffer[512];
	unsigned long index;
	unsigned long i;
	bool bJbcTransferStart;
	unsigned char c;
	unsigned long jbcAmount;
	unsigned long pJbc;
		
	udc_start();
	pJbc=BOARD_EBI_SRAM_BASE;
	jbcAmount=0;

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

void ecd300TestJbi(void)
{
	usart_rs232_options_t uartOption;
	unsigned long jbcAmount;
	unsigned long pJbc;

	PORTD.PIN3CTRL=PORT_OPC_PULLUP_gc;

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

	_ecd300ConfigEbi();

	if(0==(PORTD.IN&0x08))
	{
		_receiveJamFile();
	}
	else
	{
		pJbc=BOARD_EBI_SRAM_BASE;
		jbcAmount=0;

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

		jp_main(ECD300_JAM_FILE_BASE, jbcAmount, "PROGRAM", 1);
	}
	
	while(1)
	{
		;
	}
}

