/*
 * ECD300_test.c
 *
 * Created: 2014/12/20 1:42:02
 *  Author: user
 */ 

#include <asf.h>
#include "ECD300.h"
#include "ECD300_test.h"

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

#define ECD300_INFRARED_TCC_RESOLUTION 0x100000

/**
* Actually, 3 bytes is enough for a timestamp. This could be improved later.
* 0x7FFFFF is nearly 2 seconds at 4MHz. The highest bit could contain the level of PORTC_PIN2.
*/
#define MAX_STAMP_POOL_SIZE 0x3ff //1023
static unsigned short _edgeStampTcc0[MAX_STAMP_POOL_SIZE];
static unsigned short _edgeStampTcc1[MAX_STAMP_POOL_SIZE];
static unsigned char _signalValue[MAX_STAMP_POOL_SIZE];
static unsigned short _edgeWriteIndex;
static unsigned short _edgeReadIndex;

#define MAX_SIGNAL_AMOUNT_IN_LIBRARY 0x3fff //16K
static unsigned long _pEdgeLibraryLower16;
static unsigned long _pEdgeLibraryHigher16;
static unsigned long _pSignalLibraryValue;
static unsigned short _edgeTotalAmount;


static void _ecd300InfraRedInputCallback(void)
{
	if(((_edgeWriteIndex+1)&MAX_STAMP_POOL_SIZE)!=_edgeReadIndex)
	{//pool is not full.
		_edgeStampTcc0[_edgeWriteIndex]=tc_read_count(&TCC0);
		_edgeStampTcc1[_edgeWriteIndex]=tc_read_count(&TCC1);
		if(PORTC_IN&0x04)
		{
			//Infrared LED is off.
			_signalValue[_edgeWriteIndex]=1;
			//switch off DA0.
			PORTA_OUTCLR=0x01;
		}
		else
		{
			//Infrared LED is on.
			_signalValue[_edgeWriteIndex]=0;
			//switch on DA0.
			PORTA_OUTSET=0x01;
		}
		
		_edgeWriteIndex=(_edgeWriteIndex+1)&MAX_STAMP_POOL_SIZE;
	}
	else
	{//pool is full.
		PORTA_OUT=0xff;
		//PORTB_OUT=_edgeWriteIndex>>8;
	}
}


/**
* This function launches the process of infrared signal catching.
* ISR puts the stamp of signal edges to a pool in the internal RAM, 
* then the data in the pool is transferred to external RAM.
* If there is no further signal in a certain time, it is considered as the end of signal.
* The blinking DB7 indicates that infrared signal is being waitted.
* If LEDs of PORTA are turned on all, it means part of signal is lost.
* DB6 turns on when the signal pool for ISR is empty since the first edge of infrared signal is received.
*/
static void _learnInfraRedInputWithIsr(void)
{
	unsigned short index;
	unsigned long i;
	unsigned long curStamp, nextStamp;

	printString("in _learnInfraRedInputWithIsr, the blinking DB7 indicates that infrared signal is being waitted\r\n");

	/**
	* configure pool and library.
	*/
	//signal pool
	_edgeWriteIndex=0;
	_edgeReadIndex=0;
	//signal library
	_edgeTotalAmount=0;
	_pEdgeLibraryLower16=BOARD_EBI_SRAM_BASE;
	_pEdgeLibraryHigher16=_pEdgeLibraryLower16	+ 2*MAX_SIGNAL_AMOUNT_IN_LIBRARY;
	_pSignalLibraryValue=_pEdgeLibraryHigher16	+ 2*MAX_SIGNAL_AMOUNT_IN_LIBRARY;

	PORTA_OUT=0x00;
	PORTB_OUT=0x00;
	
	/**
	* configure 32-bit timer.
	*/
	//Use TCC0 as the lower 16-bit of a 32-bit timer.
	tc_enable(&TCC0);
	tc_reset(&TCC0);
	tc_set_wgm(&TCC0, TC_WG_NORMAL);
	tc_write_period(&TCC0, 0xffff);
	//the actual resolution is about 4MHz, so it takes the timer about 16ms to overflow.
	//this time is much enough for two bits which take about 1.7ms at 36KHz. 
	//In other words, this time is much more than the longest gap in the signals at 36KHz.
	tc_set_resolution(&TCC0, ECD300_INFRARED_TCC_RESOLUTION);
	//Use TCC1 as the higher 16-bit of a 32-bit timer.
	tc_enable(&TCC1);
	tc_reset(&TCC1);
	tc_set_wgm(&TCC1, TC_WG_NORMAL);
	tc_write_period(&TCC1, 0xffff);
	sysclk_enable_module(SYSCLK_PORT_GEN, SYSCLK_EVSYS);
	EVSYS_CH0MUX=EVSYS_CHMUX_TCC0_OVF_gc;
	tc_write_clock_source(&TCC1, TC_CLKSEL_EVCH0_gc);
	tc_set_input_capture(&TCC1, TC_EVSEL_CH0_gc, TC_EVACT_OFF_gc);
	
#if 1
	/**
	* configure interrupt.
	*/
	//configure PIN edge
	ecd300IoPortConfigPin(&PORTC, ECD300_PORT_PIN2, false, false, PORT_OPC_TOTEM_gc, PORT_ISC_BOTHEDGES_gc);
	//select PIN2 as a source of PORTC's INT0.
	ecd300IoPortInt0SelectPin(&PORTC, ECD300_PORT_PIN2);
	//clear the possible INT0IF.
	ecd300IoPortClearInt0Flag(&PORTC);
	//register ISR function.
	ecd300IoPortRegisterInt0(&PORTC, _ecd300InfraRedInputCallback);
	//set the priority of INT0.
	ecd300IoPortSetInt0Level(&PORTC, PORT_INT0LVL_HI_gc);
#else
	//mimic the interrupt.
	for(index=0; index<2048; index++)
	{
		ecd300InfraRedInputCallback();
	}
	printString("Read index: ");
	printHex(_edgeReadIndex>>8);
	printString(", ");
	printHex(_edgeReadIndex);
	printString("\r\n");
	printString("Write index: ");
	printHex(_edgeWriteIndex>>8);
	printString(", ");
	printHex(_edgeWriteIndex);
	printString("\r\n");
#endif	

	//wait for the the first edge of infrared signal.
	for(i=0; _edgeWriteIndex==0; i++)
	{
#if 1	
		//blink DB7 when waiting for the signal.
		//some code is needed here, otherwise it seems that the program traps here.
		//perehaps this is caused by the optimization.
		if((i&0x100000)==0)
		{
			PORTB_OUTCLR=0x80;
		}
		else
		{
			PORTB_OUTSET=0x80;
		}
#else
		;
#endif
	}
	PORTB_OUTCLR=0x80;

	//transfer signal stamps from pool to library.
	for(i=0;;i++)
	{
		if(_edgeReadIndex!=_edgeWriteIndex)
		{// a signal edge
			if(_edgeTotalAmount<MAX_SIGNAL_AMOUNT_IN_LIBRARY)
			{
				hugemem_write16(_pEdgeLibraryLower16+(_edgeTotalAmount<<1), _edgeStampTcc0[_edgeReadIndex]);
				hugemem_write16(_pEdgeLibraryHigher16+(_edgeTotalAmount<<1), _edgeStampTcc1[_edgeReadIndex]);
				hugemem_write8(_pSignalLibraryValue+_edgeTotalAmount, _signalValue[_edgeReadIndex]);

				_edgeTotalAmount++;
			}
			_edgeReadIndex=(_edgeReadIndex+1)&MAX_STAMP_POOL_SIZE;
			i=0; //restart the counter of idle signal.
			PORTB_OUTCLR=0x40;
		}
		else
		{//pool is empty
			//It was found that 3 signals were sent out when the remote controller was presssed,
			//and the gap between two signal was about 64ms. So this pool should be kept more 
			//than 64ms to prevent any signal lost.
			if(i>0xfffff)
			{
				// pool is empty for a certain time, signal finishes.
				break;
			}
			PORTB_OUTSET=0x40;
		}
	}
	PORTB_OUTCLR=0x40;

	//print the signal data.
	printString("Catch a key:\r\n");
#if 0
	for(index=0;(index<(_edgeTotalAmount-1))&&(index<(MAX_SIGNAL_AMOUNT_IN_LIBRARY-1));index++)
	{
		printString("level: ");
		printHex(hugemem_read8(_pSignalLibraryValue+index));
		
		printString(", curStamp: ");
		curStamp=hugemem_read16(_pEdgeLibraryHigher16+(index<<1));
		curStamp<<=16;
		curStamp+=hugemem_read16(_pEdgeLibraryLower16+(index<<1));
		printHex(curStamp>>24);
		printString(", ");
		printHex(curStamp>>16);
		printString(", ");
		printHex(curStamp>>8);
		printString(", ");
		printHex(curStamp);
		
		printString(", nextStamp: ");
		nextStamp=hugemem_read16(_pEdgeLibraryHigher16+((index+1)<<1));
		nextStamp<<=16;
		nextStamp+=hugemem_read16(_pEdgeLibraryLower16+((index+1)<<1));
		printHex(nextStamp>>24);
		printString(", ");
		printHex(nextStamp>>16);
		printString(", ");
		printHex(nextStamp>>8);
		printString(", ");
		printHex(nextStamp);

		printString(", duration: ");
		if(curStamp<nextStamp)
		{ 
			i=nextStamp-curStamp;
		}
		else
		{
			i=0xffffffff-curStamp;
			i+=nextStamp+1;
		}
		printHex(i>>24);
		printString(", ");
		printHex(i>>16);
		printString(", ");
		printHex(i>>8);
		printString(", ");
		printHex(i);
		
		printString("\r\n");
	}
#endif
	printString("Signal data ends, edge amount: ");
	printHex(_edgeTotalAmount>>8);
	printString(", ");
	printHex(_edgeTotalAmount);
	printString("\r\n");
	
	//disable INT0
	PORTC.INTCTRL=PORT_INT0LVL_OFF_gc;
	//disable the counter
	tc_disable(&TCC0);
	tc_disable(&TCC1);
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

static bool _allSignalIsInPool;
static bool _allSignalIsSent;

static void _comparatorCallback(void)
{
	if(_edgeReadIndex!=_edgeWriteIndex)
	{//signal pool is not empty.
		if((_signalValue[_edgeReadIndex]&0x7f)==0)
		{
			//turn on or off the emiter.
			if(_signalValue[_edgeReadIndex]&0x80)
			{//emiter should be off.
				PORTC_OUTCLR=0x08;
				//turn off DA0
				PORTA_OUTCLR=0x01;
			}
			else
			{//emiter shoud be on.
				PORTC_OUTSET=0x08;
				//turn on DA0
				PORTA_OUTSET=0x01;
			}
			//write the next stamp to CCA.
			tc_write_cc_buffer(&TCC0, TC_CCA, _edgeStampTcc0[_edgeReadIndex]);//prevent other value is written to CCA when UPDATE occurs.
			tc_write_cc(&TCC0, TC_CCA, _edgeStampTcc0[_edgeReadIndex]);

			_edgeReadIndex=(_edgeReadIndex+1)&MAX_STAMP_POOL_SIZE;
		}
		else
		{
			_signalValue[_edgeReadIndex]-=1;
		}
	}
	else
	{//signal pool is empty.
		if(false==_allSignalIsInPool)
		{//signal pool is not filled soon enough which results in the wrong signal timing.
			//ALARM!
			PORTB_OUTSET=_edgeReadIndex>>8;
			PORTA_OUTSET=_edgeReadIndex;
		}
		else
		{//all edges have been processed.
			_allSignalIsSent=true;
		}

		//turn off the emiter.
		PORTC_OUTCLR=0x08;
		PORTA_OUTCLR=0x01;
	}
}

static void _sendCatchedKey(void)
{
	unsigned long i;
	unsigned short edgeLibraryIndex;
	unsigned char value;

	unsigned long curStamp, nextStamp;
	unsigned short firstEdgeStamp;

	printString("\r\nin _sendCatchedKey\r\n");
	
	PORTA_OUT=0x00;
	PORTB_OUT=0x00;

#if 0
	for(edgeLibraryIndex=0;edgeLibraryIndex<(_edgeTotalAmount-1);edgeLibraryIndex++)

	{
		//if(((_edgeWriteIndex+1)&MAX_STAMP_POOL_SIZE)!=_edgeReadIndex)
		{
			curStamp=hugemem_read16(_pEdgeLibraryHigher16+(edgeLibraryIndex<<1));
			curStamp<<=16;
			curStamp+=hugemem_read16(_pEdgeLibraryLower16+(edgeLibraryIndex<<1));

			nextStamp=hugemem_read16(_pEdgeLibraryHigher16+((edgeLibraryIndex+1)<<1));
			nextStamp<<=16;
			nextStamp+=hugemem_read16(_pEdgeLibraryLower16+((edgeLibraryIndex+1)<<1));

			printString("curStamp: ");
			printHex(curStamp>>24);
			printString(", ");
			printHex(curStamp>>16);
			printString(", ");
			printHex(curStamp>>8);
			printString(", ");
			printHex(curStamp);
			printString(", nextStamp: ");
			printHex(nextStamp>>24);
			printString(", ");
			printHex(nextStamp>>16);
			printString(", ");
			printHex(nextStamp>>8);
			printString(", ");
			printHex(nextStamp);
			printString(", value: ");

			//the period of this signal since this edge.
			if(nextStamp>curStamp)
			{
				nextStamp-=curStamp;
			}
			else
			{//wrap
				curStamp=0xffffffff-curStamp;
				nextStamp+=curStamp+1;
			}
			nextStamp>>=16; //nextStamp now contains the number of complete TCC cycles.
			nextStamp=nextStamp&0x7f;

			//_edgeStampTcc0[_edgeWriteIndex]=hugemem_read16(_pEdgeLibraryLower16+((edgeLibraryIndex+1)<<1));
			//TCC0 runs at 4MHz. 127 cycles of TCC0 are nearly 2 seconds, which is enough for a pulse.
			value=hugemem_read8(_pSignalLibraryValue+edgeLibraryIndex);
			if(value)
			{
				_signalValue[0]=0x80+nextStamp;
			}
			else
			{
				_signalValue[0]=nextStamp;
			}

			printHex(_signalValue[0]);
			printString("\r\n");
			
			//_edgeWriteIndex=(_edgeWriteIndex+1)&MAX_STAMP_POOL_SIZE;
		}
	}
	printString("press 1 to emit the signal, other key to return\r\n");
	if('1'!=getChar())
	{
		return;
	}
#endif	
	/**
	* fill the pool.
	* bit7 of _signalValue[X] contains the level of signal,
	* bit6 to bit0 of _signalValue[X] contains the period of signal, 
	* and _edgeStampTcc0[X] contains the stamp of next edge.
	*/
	_allSignalIsInPool=false;
	_allSignalIsSent=false;
	_edgeReadIndex=0;
	_edgeWriteIndex=0;
	//first edge stamp.
	firstEdgeStamp=hugemem_read16(_pEdgeLibraryLower16);
	for(edgeLibraryIndex=0;edgeLibraryIndex<(_edgeTotalAmount-1);edgeLibraryIndex++)
	{
		if(((_edgeWriteIndex+1)&MAX_STAMP_POOL_SIZE)!=_edgeReadIndex)
		{
			curStamp=hugemem_read16(_pEdgeLibraryHigher16+(edgeLibraryIndex<<1));
			curStamp<<=16;
			curStamp+=hugemem_read16(_pEdgeLibraryLower16+(edgeLibraryIndex<<1));

			nextStamp=hugemem_read16(_pEdgeLibraryHigher16+((edgeLibraryIndex+1)<<1));
			nextStamp<<=16;
			nextStamp+=hugemem_read16(_pEdgeLibraryLower16+((edgeLibraryIndex+1)<<1));
#if 0
			printString("curStamp: ");
			printHex(curStamp>>24);
			printString(", ");
			printHex(curStamp>>16);
			printString(", ");
			printHex(curStamp>>8);
			printString(", ");
			printHex(curStamp);
			printString(", nextStamp: ");
			printHex(nextStamp>>24);
			printString(", ");
			printHex(nextStamp>>16);
			printString(", ");
			printHex(nextStamp>>8);
			printString(", ");
			printHex(nextStamp);
			printString(", value: ");
#endif
			//the period of this signal since this edge.
			if(nextStamp>curStamp)
			{
				nextStamp-=curStamp;
			}
			else
			{//wrap
				curStamp=0xffffffff-curStamp;
				nextStamp+=curStamp+1;
			}
			nextStamp>>=16; //nextStamp now contains the number of complete TCC cycles.
			nextStamp=nextStamp&0x7f;

			_edgeStampTcc0[_edgeWriteIndex]=hugemem_read16(_pEdgeLibraryLower16+((edgeLibraryIndex+1)<<1));
			//TCC0 runs at 4MHz. 127 cycles of TCC0 are nearly 2 seconds, which is enough for a pulse.
			value=hugemem_read8(_pSignalLibraryValue+edgeLibraryIndex);
			if(value)
			{
				_signalValue[_edgeWriteIndex]=0x80+nextStamp;
			}
			else
			{
				_signalValue[_edgeWriteIndex]=nextStamp;
			}
#if 0
			printHex(_signalValue[_edgeWriteIndex]);
			printString("\r\n");
#endif			
			_edgeWriteIndex=(_edgeWriteIndex+1)&MAX_STAMP_POOL_SIZE;
		}
		else
		{// the pool is full.
			break;
		}
	}
	
	//Use TCC0.
	tc_enable(&TCC0);
	tc_reset(&TCC0);
	tc_set_wgm(&TCC0, TC_WG_NORMAL);
	tc_write_period(&TCC0, 0xffff);
	//the actual resolution is about 4MHz, so it takes the timer about 16ms to overflow.
	//this time is much enough for two bits which take about 1.7ms at 36KHz. 
	//In other words, this time is much more than the longest gap in the signals at 36KHz.
	tc_set_resolution(&TCC0, ECD300_INFRARED_TCC_RESOLUTION);
	//set the interrupt callback function and level.
	tc_set_cca_interrupt_callback(&TCC0, _comparatorCallback);
	//enable CC channel.
	tc_enable_cc_channels(&TCC0, TC_CCAEN);
	//set the comparator's value.
	tc_write_cc_buffer(&TCC0, TC_CCA, firstEdgeStamp);
	tc_write_cc(&TCC0, TC_CCA, firstEdgeStamp);
	//restart the counter so that there is enough time for the following actions.
	tc_restart(&TCC0);
	//clear the possible pending interrupt flag.
	tc_clear_cc_interrupt(&TCC0, TC_CCA);
	//enable the interrupt.
	tc_set_cca_interrupt_level(&TCC0, TC_INT_LVL_HI);


	//go on filling the pool
	for(;edgeLibraryIndex<(_edgeTotalAmount-1);)
	{
		if(((_edgeWriteIndex+1)&MAX_STAMP_POOL_SIZE)!=_edgeReadIndex)
		{
			PORTB_OUTCLR=0x80;
			
			curStamp=hugemem_read16(_pEdgeLibraryHigher16+(edgeLibraryIndex<<1));
			curStamp<<=16;
			curStamp+=hugemem_read16(_pEdgeLibraryLower16+(edgeLibraryIndex<<1));

			nextStamp=hugemem_read16(_pEdgeLibraryHigher16+((edgeLibraryIndex+1)<<1));
			nextStamp<<=16;
			nextStamp+=hugemem_read16(_pEdgeLibraryLower16+((edgeLibraryIndex+1)<<1));

			//the period of this signal since this edge.
			if(nextStamp>curStamp)
			{
				nextStamp-=curStamp;
			}
			else
			{//wrap
				curStamp=0xffffffff-curStamp;
				nextStamp+=curStamp+1;
			}
			nextStamp>>=16; //nextStamp now contains the number of complete TCC cycles.

			_edgeStampTcc0[_edgeWriteIndex]=hugemem_read16(_pEdgeLibraryLower16+((edgeLibraryIndex+1)<<1));
			//TCC0 runs at 4MHz. 127 cycles of TCC0 are nearly 2 seconds, which is enough for a pulse.
			value=hugemem_read8(_pSignalLibraryValue+edgeLibraryIndex);
			if(value)
			{
				_signalValue[_edgeWriteIndex]=0x80+nextStamp;
			}
			else
			{
				_signalValue[_edgeWriteIndex]=nextStamp;
			}

			_edgeWriteIndex=(_edgeWriteIndex+1)&MAX_STAMP_POOL_SIZE;
			edgeLibraryIndex++;
		}
		else
		{// the pool is full.
			PORTB_OUTSET=0x80;
			continue;
		}
	}
	_allSignalIsInPool=true;
	

	for(i=0;_allSignalIsSent==false;i++)
	{
		if((i&0x1fffff)==0x100000)
		{
			printString("Wait for the end of emitting\r\n");
		}
	}
	
	tc_set_cca_interrupt_level(&TCC0, TC_INT_LVL_OFF);
	tc_reset(&TCC0);
	tc_disable(&TCC0);
	
	printString("The signal is sent\r\n");
}


static void _testInfraRed(void)
{
	while(1)
	{
		while(1)
		{
			printString("Learn signal ...\r\n");
			//_catchInfraRedInput();
			_learnInfraRedInputWithIsr();
			printString("Press 1 to send the key catched, other key to learn the input again\r\n");
			if('1'==getChar())
			{
				break;
			}
		}

		while(1)
		{
			printString("Send signal learnt ...\r\n");
			_sendCatchedKey();
			printString("Press 2 to send again, other key to learn the input\r\n");
			if('2'==getChar())
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}	
}

void ecd300TestInfraRed(void)
{
	usart_rs232_options_t uartOption;
	unsigned long i;

	//turn off the emiter.
	PORTC_OUTCLR=0x08;
	PORTC_DIRSET=0x08;

	PORTA_DIR=0xff;
	PORTB_DIR=0xff;
	
	PORTA_OUT=0x55;
	for(i=0;i<0x40000;i++)
	{
		if(i&0x10000)
			PORTA_OUT=0xff;
		else
			PORTA_OUT=0x55;
	}
	PORTA_OUT=0x00;
	PORTB_OUT=0x00;
	
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
	
	_testInfraRed();
	
	while(1)
	{
		;
	}
}
