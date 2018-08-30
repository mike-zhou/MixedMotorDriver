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
#include "tc.h"

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
	 * cycleIndex. 3 additional clock cycleIndexs as wait state give enough headroom. (64ns (4 clks) > 55ns > 40ns)
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
	printString("Host is configuring serial port: ");
	printHex(port);
	printString("\r\n");
}

///////////////////////////////////////////////////////////////////

#define PRODUCT_NAME "ProductName: Smart Card Switch HV1.0 SV1.0"
#define SOLENOID_AMOUNT 4
#define DEFAULT_SOLENOID_DURATION_DIVISON 128

//bufferLength must not exceed 255.
#define BUFFER_LENGTH 0x1FF
static	unsigned char inputBuffer[BUFFER_LENGTH];
static	unsigned char outputBuffer[BUFFER_LENGTH];
static	unsigned short inputProducerIndex=0;
static	unsigned short inputConsumerIndex=0;
static	unsigned short outputProducerIndex=0;
static	unsigned short outputConsumerIndex=0;

enum State
{
	AWAITING_COMMAND = 0,
	STARTING_COMMAND,
	EXECUTING_COMMAND
};
	
static struct CommandState
{
	enum State state;

	unsigned char command;
	unsigned char param;
	unsigned char solenoidDurationDivision;

	union U
	{
		struct 
		{
			unsigned char solenoidIndex;
			unsigned char solenoidPairIndex;
			unsigned char cycleIndex; // a cycle includes an activation period and a deactivation period.
			bool phaseActivationFinished;
			bool phaseDeactivationFinished;
			unsigned short initialCounter;
			unsigned short period;
		} solenoid;
	} u;
	
} commandState;

static struct MachineStatus
{
	//0: no smart card is connected
	//!0: number of the connected smart card
	unsigned short smartCardConnected; 

	bool solenoidActivated[SOLENOID_AMOUNT];
	bool solenoidIsPowered[SOLENOID_AMOUNT];

	bool powerAvailable;
	bool powerFuseBroken;
} status;

inline void clearInputBuffer()
{
	inputProducerIndex = 0;
	inputConsumerIndex = 0;
}

//write a character to input buffer.
//return true if parameter is saved successfully.
//return false if input buffer overflows.
bool writeInputBuffer(unsigned char c)
{
	unsigned short nextProducerIndex = (inputProducerIndex + 1) % BUFFER_LENGTH;
	
	if(nextProducerIndex != inputConsumerIndex) {
		inputBuffer[inputProducerIndex] = c;
		inputProducerIndex = nextProducerIndex;
		return true;
	}
	else {
		//overflow.
		clearInputBuffer();
		return false;
	}
}

// read a character from input buffer.
// return the first character in input buffer,
// return 0 if there is nothing in the input buffer.
unsigned char readInputBuffer()
{
	if(inputConsumerIndex != inputProducerIndex) {
		unsigned char c = inputBuffer[inputConsumerIndex];
		inputConsumerIndex = (inputConsumerIndex + 1) % BUFFER_LENGTH;
		return c;
	}
	else {
		return 0;
	}
}

//write a character to output buffer
//return true if parameter is saved successfully
//return false if output buffer overflows
bool writeOutputBufferChar(unsigned char c)
{
	unsigned short nextProducerIndex;
	
	nextProducerIndex = (outputProducerIndex + 1) % BUFFER_LENGTH;
	if(nextProducerIndex == outputConsumerIndex) {
		//outputBuffer is full, change the last character to * to indicate character loss
		outputBuffer[outputProducerIndex] = '*';
		return false;
	}
	else {
		outputBuffer[outputProducerIndex] = c;
		outputProducerIndex = nextProducerIndex;
		return true;
	}
}

//write a string to output buffer
void writeOutputBufferString(char * pString)
{
	unsigned char c;
	
	for(; ;) {
		c = *pString++;
		if(0 == c) {
			break;
		}
		else {
			writeOutputBufferChar(c);
		}
	}
}

void writeOutputBufferHex(unsigned char n)
{
	if((n >> 4) <= 9) {
		writeOutputBufferChar((n >> 4) + '0');
	}
	else {
		writeOutputBufferChar((n >> 4) - 0xA + 'A');
	}

	if((n & 0x0F) <= 9) {
		writeOutputBufferChar((n & 0x0F) + '0');
	}
	else {
		writeOutputBufferChar((n & 0x0F) - 0xA + 'A');
	}
}

void sendOutputBufferToHost()
{
	if(outputConsumerIndex != outputProducerIndex)
	{
		if(udi_cdc_is_tx_ready()) {
			udi_cdc_putc(outputBuffer[outputConsumerIndex]);
			outputConsumerIndex = (outputConsumerIndex + 1) % BUFFER_LENGTH;
		}
	}
}

static void activate_smart_card_status()
{
	PORTF_DIRSET = 0x01;
	PORTF_OUTSET = 0x01;
}

static void deactivate_smart_card_status() 
{
	PORTF_DIRCLR = 0x01;
	PORTF_OUTCLR = 0x01;
}

static void deactivate_all_solenoids(){
	PORTC_DIR=0x00;
	PORTC_OUT=0x00;
}

static void disconnect_all_smart_card()
{
	PORTA_DIR=0x00;
	PORTK_DIR=0x00;
	PORTA_OUT=0x00;
	PORTK_OUT=0x00;
	deactivate_smart_card_status();
}

//return true if any solenoid is enabled, otherwise return false.
static bool activate_solenoid(unsigned char channel)
{
	deactivate_all_solenoids();
	
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
		default:
			return false;
			break;
	}
	return true;
}

static bool is_solenoid_activated(unsigned char channel)
{
	switch(channel)
	{
		case 1: //pc7
			return PORTC_DIR&0x80;
		case 2: //pc5
			return PORTC_DIR&0x20;
		case 3: //pc3
			return PORTC_DIR&0x08;
		case 4: //pc1
			return PORTC_DIR&0x02;
		default:
			return false;
	}
}

// return true if the designated solenoid is activated
// return false if the designated solenoid is not activated
static bool is_solenoid_powered(unsigned char channel)
{
	switch(channel)
	{
		case 1: //pc6
			if(PORTC_IN & 0x40)
				return false;
			else
				return true;
		case 2: //pc4
			if(PORTC_IN & 0x10)
				return false;
			else
				return true;
		case 3: //pc2
			if(PORTC_IN & 0x04)
				return false;
			else
				return true;
		case 4: //pc0
			if(PORTC_IN & 0x01)
				return false;
			else
				return true;
		default:
			return false;
	}
}

static bool activate_smart_card(unsigned char index)
{
	disconnect_all_smart_card();
	
	switch(index)
	{
		case 1:
			PORTA_OUTSET = 0x80;
			PORTA_DIRSET = 0x80;
			break;
		case 2:
			PORTA_OUTSET = 0x40;
			PORTA_DIRSET = 0x40;
			break;
		case 3:
			PORTA_OUTSET = 0x20;
			PORTA_DIRSET = 0x20;
			break;
		case 4:
			PORTA_OUTSET = 0x10;
			PORTA_DIRSET = 0x10;
			break;
		case 5:
			PORTA_OUTSET = 0x08;
			PORTA_DIRSET = 0x08;
			break;
		case 6:
			PORTA_OUTSET = 0x04;
			PORTA_DIRSET = 0x04;
			break;
		case 7:
			PORTA_OUTSET = 0x02;
			PORTA_DIRSET = 0x02;
			break;
		case 8:
			PORTA_OUTSET = 0x01;
			PORTA_DIRSET = 0x01;
			break;
		case 9:
			PORTK_OUTSET = 0x80;
			PORTK_DIRSET = 0x80;
			break;
		case 10:
			PORTK_OUTSET = 0x40;
			PORTK_DIRSET = 0x40;
			break;
		case 11:
			PORTK_OUTSET = 0x20;
			PORTK_DIRSET = 0x20;
			break;
		case 12:
			PORTK_OUTSET = 0x10;
			PORTK_DIRSET = 0x10;
			break;
		case 13:
			PORTK_OUTSET = 0x08;
			PORTK_DIRSET = 0x08;
			break;
		case 14:
			PORTK_OUTSET = 0x04;
			PORTK_DIRSET = 0x04;
			break;
		case 15:
			PORTK_OUTSET = 0x02;
			PORTK_DIRSET = 0x02;
			break;
		case 16:
			PORTK_OUTSET = 0x01;
			PORTK_DIRSET = 0x01;
			break;		
		default:
			return false;
			break;
	}
	
	activate_smart_card_status();
	return true;
}

static unsigned char get_activated_smart_card()
{
	if(PORTA_DIR & 0x80)
		return 1;
	if(PORTA_DIR & 0x40)
		return 2;
	if(PORTA_DIR & 0x20)
		return 3;
	if(PORTA_DIR & 0x10)
		return 4;
	if(PORTA_DIR & 0x08)
		return 5;
	if(PORTA_DIR & 0x04)
		return 6;
	if(PORTA_DIR & 0x02)
		return 7;
	if(PORTA_DIR & 0x01)
		return 8;
	if(PORTK_DIR & 0x80)
		return 9;
	if(PORTK_DIR & 0x40)
		return 10;
	if(PORTK_DIR & 0x20)
		return 11;
	if(PORTK_DIR & 0x10)
		return 12;
	if(PORTK_DIR & 0x08)
		return 13;
	if(PORTK_DIR & 0x04)
		return 14;
	if(PORTK_DIR & 0x02)
		return 15;
	if(PORTK_DIR & 0x01)
		return 16;
	
	return 0;
}

static bool test_smart_card_connection(void)
{
	unsigned char tmp;
	
	unsigned short initialCounter;
	unsigned short currentCounter;
	//wait for 1/8 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)/8) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)/8) {
				break;
			}
		}
	}
	
	PORTB_DIR = 0x0F;
	
	PORTB_OUT = 0x00;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}
	
	PORTB_OUT = 0x0F;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0xF0) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}
	
	PORTB_OUT = 0x01;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0x10) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	PORTB_OUT = 0x02;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0x20) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	PORTB_OUT = 0x04;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0x40) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	PORTB_OUT = 0x08;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0x80) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	PORTB_OUT = 0x0E;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0xE0) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	PORTB_OUT = 0x0D;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0xD0) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	PORTB_OUT = 0x0B;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0xB0) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	PORTB_OUT = 0x07;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0x70) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	return true;
}

static bool test_smart_card_connection_slow(void)
{
	unsigned short initialCounter;
	unsigned short currentCounter;
			
	PORTB_DIR = 0x0F;
	
	PORTB_OUT = 0x00;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0) {
		return false;
	}
	
	PORTB_OUT = 0x0F;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0xF0) {
		return false;
	}
	
	PORTB_OUT = 0x01;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0x10) {
		return false;
	}

	PORTB_OUT = 0x02;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0x20) {
		return false;
	}

	PORTB_OUT = 0x04;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0x40) {
		return false;
	}

	PORTB_OUT = 0x08;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0x80) {
		return false;
	}

	PORTB_OUT = 0x0E;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0xE0) {
		return false;
	}

	PORTB_OUT = 0x0D;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0xD0) {
		return false;
	}

	PORTB_OUT = 0x0B;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0xB0) {
		return false;
	}

	PORTB_OUT = 0x07;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0x70) {
		return false;
	}

	return true;
}

// is power available
bool is_power_ok()
{
	//PD5
	if(PORTD_IN	& 0x20) {
		return false;
	}
	else {
		return true;
	}
}

bool is_power_fuse_ok() 
{
	//PD4
	if(PORTD_IN & 0x10)
		return true;
	else 
		return false;
}

static void counter_init()
{
	//set counter. The resolution should be 31MHz/1024.
	tc_enable(&TCC0);
	tc_set_resolution(&TCC0, 1);
}

//wait for 1/time second
static void counter_wait(unsigned char time)
{
	uint32_t resolution;
	unsigned short activationLength;
	unsigned short initialCounter;

	if(0==time)
		return;
		
	resolution=tc_get_resolution(&TCC0);
	activationLength = resolution/time; 

	// wait for timeout
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		unsigned short currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > activationLength) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > activationLength) {
				break;
			}
		}
	}
}

static inline unsigned short counter_get()
{
	return tc_read_count(&TCC0);
}

static void init_status()
{
	unsigned char solenoidIndex;

	status.smartCardConnected = 0;
	
	for(solenoidIndex = 1; solenoidIndex <= SOLENOID_AMOUNT; solenoidIndex++){
		status.solenoidActivated[solenoidIndex - 1] = false;
		status.solenoidIsPowered[solenoidIndex - 1] = false;
	}
	
	status.powerAvailable = false;
	status.powerFuseBroken = false;
}

static void check_status()
{
	unsigned char activatedSmartCard;
	bool powerAvailable;
	bool powerFuseBroken;

	//check which smart card is connected.
	activatedSmartCard = get_activated_smart_card();
	if(status.smartCardConnected != activatedSmartCard) {
		//smart card status changed.
		if(status.smartCardConnected != 0) {
			writeOutputBufferString("SC 0x"); 
			writeOutputBufferHex(status.smartCardConnected); 
			writeOutputBufferString(" disconnected\r\n");
		}
		status.smartCardConnected = activatedSmartCard;
		if(status.smartCardConnected != 0) {
			writeOutputBufferString("SC 0x"); 
			writeOutputBufferHex(status.smartCardConnected); 
			writeOutputBufferString(" connected\r\n");
		}
	}

	//check 24V and its fuse
	powerAvailable = is_power_ok();
	if(status.powerAvailable != powerAvailable)
	{
		status.powerAvailable = powerAvailable;
		if(powerAvailable) {
			writeOutputBufferString("Power connected\r\n");
		}
		else {
			writeOutputBufferString("Power disconnected\r\n");
		}
	}
	powerFuseBroken = !is_power_fuse_ok();
	if(status.powerFuseBroken != powerFuseBroken) {
		status.powerFuseBroken = powerFuseBroken;
		if(powerFuseBroken) {
			writeOutputBufferString("PowerFuse is broken\r\n");
		}
		else {
			writeOutputBufferString("PowerFuse is OK\r\n");
		}
	}
	
	//check solenoids
	for(unsigned char solenoidIndex = 1; solenoidIndex <= SOLENOID_AMOUNT; solenoidIndex++) 
	{
		if(status.solenoidActivated[solenoidIndex - 1] != is_solenoid_activated(solenoidIndex)) {
			status.solenoidActivated[solenoidIndex - 1] = is_solenoid_activated(solenoidIndex);
			if(status.solenoidActivated[solenoidIndex - 1]) {
				writeOutputBufferString("Solenoid 0x");
				writeOutputBufferHex(solenoidIndex);
				writeOutputBufferString(" is activated\r\n");
			}
			else {
				writeOutputBufferString("Solenoid 0x");
				writeOutputBufferHex(solenoidIndex);
				writeOutputBufferString(" is not activated\r\n");
			}
		}

		if(status.solenoidIsPowered[solenoidIndex - 1] != is_solenoid_powered(solenoidIndex)) {
			status.solenoidIsPowered[solenoidIndex - 1] = is_solenoid_powered(solenoidIndex);
			if(status.solenoidIsPowered[solenoidIndex - 1]) {
				writeOutputBufferString("Solenoid 0x");
				writeOutputBufferHex(solenoidIndex);
				writeOutputBufferString(" is powered\r\n");
			}
			else {
				writeOutputBufferString("Solenoid 0x");
				writeOutputBufferHex(solenoidIndex);
				writeOutputBufferString(" is not powered\r\n");
			}
		}
	}
}

static void write_status()
{
	//Power
	if(status.powerAvailable) {
		writeOutputBufferString("Power connected\r\n");
	}
	else {
		writeOutputBufferString("Power disconnected\r\n");
	}
	if(status.powerFuseBroken) {
		writeOutputBufferString("PowerFuse is broken\r\n");
	}
	else {
		writeOutputBufferString("PowerFuse is OK\r\n");
	}		

	//smart card
	if(status.smartCardConnected != 0) {
		writeOutputBufferString("SC 0x"); 
		writeOutputBufferHex(status.smartCardConnected); 
		writeOutputBufferString(" connected\r\n");
	}
	else {
		writeOutputBufferString("No smart card connected\r\n");
	}

	//solenoids
	for(unsigned char solenoidIndex = 1; solenoidIndex <= SOLENOID_AMOUNT; solenoidIndex++) 
	{
		writeOutputBufferString("Solenoid 0x");
		writeOutputBufferHex(solenoidIndex);
		if(status.solenoidActivated[solenoidIndex - 1]) {
			writeOutputBufferString(" is activated\r\n");
		}
		else {
			writeOutputBufferString(" is not activated\r\n");
		}

		writeOutputBufferString("Solenoid 0x");
		writeOutputBufferHex(solenoidIndex);
		if(status.solenoidIsPowered[solenoidIndex - 1]) {
			writeOutputBufferString(" is powered\r\n");
		}
		else {
			writeOutputBufferString(" is not powered\r\n");
		}
	}
}

void run_command()
{
	if(commandState.state == STARTING_COMMAND)
	{
		switch(commandState.command)
		{
			case 'I'://insert smart card
				commandState.u.solenoid.solenoidIndex = 1;
				commandState.u.solenoid.solenoidPairIndex = 2;
				commandState.u.solenoid.cycleIndex = 0;
				commandState.u.solenoid.phaseActivationFinished = false;
				commandState.u.solenoid.phaseDeactivationFinished = false;
				commandState.u.solenoid.initialCounter = counter_get();
				commandState.u.solenoid.period = tc_get_resolution(&TCC0)/commandState.solenoidDurationDivision;
				activate_solenoid(commandState.u.solenoid.solenoidIndex);
				commandState.state = EXECUTING_COMMAND;
				break;

			case 'P'://pull out smart card
				commandState.u.solenoid.solenoidIndex = 3;
				commandState.u.solenoid.solenoidPairIndex = 0;
				commandState.u.solenoid.cycleIndex = 0;
				commandState.u.solenoid.phaseActivationFinished = false;
				commandState.u.solenoid.phaseDeactivationFinished = false;
				commandState.u.solenoid.initialCounter = counter_get();
				commandState.u.solenoid.period = tc_get_resolution(&TCC0)/commandState.solenoidDurationDivision;
				activate_solenoid(commandState.u.solenoid.solenoidIndex);
				commandState.state = EXECUTING_COMMAND;
				break;

			case 'C'://connect smart card
				activate_smart_card(commandState.param);
				commandState.state = AWAITING_COMMAND;
				break;

			case 'Q'://query
				writeOutputBufferString(PRODUCT_NAME);
				writeOutputBufferString("\r\n");
				writeOutputBufferString("SolenoidDuration 1/0x");
				writeOutputBufferHex(commandState.solenoidDurationDivision);
				writeOutputBufferString(" second\r\n");
				write_status();
				commandState.state = AWAITING_COMMAND;
				break;

			case 'D'://division
				if(commandState.param > 0) {
					commandState.solenoidDurationDivision = commandState.param;
				}
				else {
					writeOutputBufferString("Invalid parameter of 'D' command. Restore division to default value\r\n");
					commandState.solenoidDurationDivision = DEFAULT_SOLENOID_DURATION_DIVISON;
				}
				commandState.state = AWAITING_COMMAND;
				break;
				
			default:
				writeOutputBufferString("Unknown command at STARTING_COMMAND\r\n");
				deactivate_all_solenoids();
				disconnect_all_smart_card();
				commandState.state = AWAITING_COMMAND;
				break;
		}
	}

	if(commandState.state == EXECUTING_COMMAND)
	{
		switch(commandState.command)
		{
			case 'I': // fall through
			case 'P':
			{
				unsigned short currentCounter;
				unsigned short initialCounter = commandState.u.solenoid.initialCounter;
				bool periodExpired = false;

				currentCounter = counter_get();
				if(currentCounter > commandState.u.solenoid.initialCounter) {
					if((currentCounter - commandState.u.solenoid.initialCounter) > commandState.u.solenoid.period) {
						periodExpired = true;
					}
				}
				else if(currentCounter < commandState.u.solenoid.initialCounter) {
					// a wrap around
					if(((0xffff - commandState.u.solenoid.initialCounter) + currentCounter) > commandState.u.solenoid.period) {
						periodExpired = true;
					}
				}
				
				if(periodExpired) {
					if(commandState.u.solenoid.phaseActivationFinished == false) {
						//activation phase finished
						commandState.u.solenoid.phaseActivationFinished = true;
						deactivate_all_solenoids();
						if(commandState.u.solenoid.solenoidPairIndex != 0) {
							//activate the other solenoid
							activate_solenoid(commandState.u.solenoid.solenoidPairIndex);
						}
						commandState.u.solenoid.initialCounter = currentCounter;
					}
					else if(commandState.u.solenoid.phaseDeactivationFinished == false) {
						//deactivation phase finished
						deactivate_all_solenoids();
						commandState.u.solenoid.phaseDeactivationFinished = true;
					}
				}

				if((commandState.u.solenoid.phaseActivationFinished == true) &&
					(commandState.u.solenoid.phaseDeactivationFinished == true)) 
				{
					//one cycleIndex finished
					commandState.u.solenoid.cycleIndex++;
					if(commandState.u.solenoid.cycleIndex < commandState.param)
					{
						//start a new cycleIndex
						commandState.u.solenoid.phaseActivationFinished = false;
						commandState.u.solenoid.phaseDeactivationFinished = false;
						commandState.u.solenoid.initialCounter = currentCounter;
						activate_solenoid(commandState.u.solenoid.solenoidIndex);
					}
					else 
					{
						//all cycleIndexs have finished.
						commandState.state = AWAITING_COMMAND;
					}
				}
			}
			break;

			default:
			{
				writeOutputBufferString("Unknown command at EXECUTING_COMMAND\r\n");
				deactivate_all_solenoids();
				disconnect_all_smart_card();
				commandState.state = AWAITING_COMMAND;
			}
			break;
		}
	}
}

//enable pull down resistors in all ports.
static void shortcircute_pull_down_ports()
{
	PORTA.PIN0CTRL=(PORTA.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTA.PIN1CTRL=(PORTA.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTA.PIN2CTRL=(PORTA.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTA.PIN3CTRL=(PORTA.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTA.PIN4CTRL=(PORTA.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTA.PIN5CTRL=(PORTA.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTA.PIN6CTRL=(PORTA.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTA.PIN7CTRL=(PORTA.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	
	PORTB.PIN0CTRL=(PORTB.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTB.PIN1CTRL=(PORTB.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTB.PIN2CTRL=(PORTB.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTB.PIN3CTRL=(PORTB.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTB.PIN4CTRL=(PORTB.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTB.PIN5CTRL=(PORTB.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTB.PIN6CTRL=(PORTB.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTB.PIN7CTRL=(PORTB.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTC.PIN0CTRL=(PORTC.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTC.PIN1CTRL=(PORTC.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTC.PIN2CTRL=(PORTC.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTC.PIN3CTRL=(PORTC.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTC.PIN4CTRL=(PORTC.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTC.PIN5CTRL=(PORTC.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTC.PIN6CTRL=(PORTC.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTC.PIN7CTRL=(PORTC.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTD.PIN0CTRL=(PORTD.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTD.PIN1CTRL=(PORTD.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTD.PIN4CTRL=(PORTD.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTD.PIN5CTRL=(PORTD.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	
	PORTE.PIN0CTRL=(PORTE.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTE.PIN1CTRL=(PORTE.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTE.PIN2CTRL=(PORTE.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTE.PIN3CTRL=(PORTE.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTE.PIN4CTRL=(PORTE.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTE.PIN5CTRL=(PORTE.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTE.PIN6CTRL=(PORTE.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTE.PIN7CTRL=(PORTE.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTF.PIN0CTRL=(PORTF.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTF.PIN1CTRL=(PORTF.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTF.PIN2CTRL=(PORTF.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTF.PIN3CTRL=(PORTF.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTF.PIN4CTRL=(PORTF.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTF.PIN5CTRL=(PORTF.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTF.PIN6CTRL=(PORTF.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTF.PIN7CTRL=(PORTF.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTH.PIN0CTRL=(PORTH.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTH.PIN1CTRL=(PORTH.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTH.PIN2CTRL=(PORTH.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTH.PIN3CTRL=(PORTH.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTH.PIN4CTRL=(PORTH.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTH.PIN5CTRL=(PORTH.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTH.PIN6CTRL=(PORTH.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTH.PIN7CTRL=(PORTH.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTJ.PIN0CTRL=(PORTJ.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTJ.PIN1CTRL=(PORTJ.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTJ.PIN2CTRL=(PORTJ.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTJ.PIN3CTRL=(PORTJ.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTJ.PIN4CTRL=(PORTJ.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTJ.PIN5CTRL=(PORTJ.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTJ.PIN6CTRL=(PORTJ.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTJ.PIN7CTRL=(PORTJ.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTK.PIN0CTRL=(PORTK.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTK.PIN1CTRL=(PORTK.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTK.PIN2CTRL=(PORTK.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTK.PIN3CTRL=(PORTK.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTK.PIN4CTRL=(PORTK.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTK.PIN5CTRL=(PORTK.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTK.PIN6CTRL=(PORTK.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTK.PIN7CTRL=(PORTK.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTQ.PIN0CTRL=(PORTQ.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTQ.PIN1CTRL=(PORTQ.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTQ.PIN2CTRL=(PORTQ.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTQ.PIN3CTRL=(PORTQ.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTR.PIN0CTRL=(PORTR.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTR.PIN1CTRL=(PORTR.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
}

// return true if short circute
// return false if no short circute
static bool shortcircute_check_pins()
{
	if((PORTA_DIR & PORTA_OUT) != (PORTA_IN & PORTA_OUT)) {
		printString("PORTA: ");printHex(PORTA_DIR & PORTA_OUT); printString(":");printHex(PORTA_IN & PORTA_OUT);printString("\r\n");
		return true;
	}
	if((PORTB_DIR & PORTB_OUT) != (PORTB_IN & PORTB_OUT)) {
		printString("PORTB: ");printHex(PORTB_DIR & PORTB_OUT); printString(":");printHex(PORTB_IN & PORTB_OUT);printString("\r\n");
		return true;
	}
	if((PORTC_DIR & PORTC_OUT) != (PORTC_IN & PORTC_OUT)) {
		printString("PORTC: ");printHex(PORTC_DIR & PORTC_OUT); printString(":");printHex(PORTC_IN & PORTC_OUT);printString("\r\n");
		return true;
	}
	if((PORTD_DIR & 0x33) != (PORTD_IN & 0x33)) {
		printString("PORTD: ");printHex(PORTD_DIR & PORTD_OUT); printString(":");printHex(PORTD_IN & PORTD_OUT);printString("\r\n");
		return true;
	}
	if((PORTE_DIR & PORTE_OUT) != (PORTE_IN & PORTE_OUT)) {
		printString("PORTE: ");printHex(PORTE_DIR & PORTE_OUT); printString(":");printHex(PORTE_IN & PORTE_OUT);printString("\r\n");
		return true;
	}
	if((PORTF_DIR & PORTF_OUT) != (PORTF_IN & PORTF_OUT)) {
		printString("PORTF: ");printHex(PORTF_DIR & PORTF_OUT); printString(":");printHex(PORTF_IN & PORTF_OUT);printString("\r\n");
		return true;
	}
	if((PORTH_DIR & PORTH_OUT) != (PORTH_IN & PORTH_OUT)) {
		printString("PORTH: ");printHex(PORTH_DIR & PORTH_OUT); printString(":");printHex(PORTH_IN & PORTH_OUT);printString("\r\n");
		return true;
	}
	if((PORTJ_DIR & PORTJ_OUT) != (PORTJ_IN & PORTJ_OUT)) {
		printString("PORTJ: ");printHex(PORTJ_DIR & PORTJ_OUT); printString(":");printHex(PORTJ_IN & PORTJ_OUT);printString("\r\n");
		return true;
	}
	if((PORTK_DIR & PORTK_OUT) != (PORTK_IN & PORTK_OUT)) {
		printString("PORTK: ");printHex(PORTK_DIR & PORTK_OUT); printString(":");printHex(PORTK_IN & PORTK_OUT);printString("\r\n");
		return true;
	}
	if((PORTQ_DIR & PORTQ_OUT) != (PORTQ_IN & PORTQ_OUT)) {
		printString("PORTQ: ");printHex(PORTQ_DIR & PORTQ_OUT); printString(":");printHex(PORTQ_IN & PORTQ_OUT);printString("\r\n");
		return true;
	}
	if((PORTR_DIR & PORTR_OUT) != (PORTR_IN & PORTR_OUT)) {
		printString("PORTR: ");printHex(PORTR_DIR & PORTR_OUT); printString(":");printHex(PORTR_IN & PORTR_OUT);printString("\r\n");
		return true;
	}
	
	return false;
}

// check if there short circute between pins.
// return true if all ports are ok
// return false if short circute in any ports.
bool shortcircute_test()
{
	shortcircute_pull_down_ports();
	
	PORTA_DIR = 0x00;
	PORTA_OUT = 0xff;
	PORTB_DIR = 0x00;
	PORTB_OUT = 0xff;
	PORTC_DIR = 0x00;
	PORTC_OUT = 0xff;
	PORTD_DIRCLR = 0x33;
	PORTD_OUTSET = 0x33;
	PORTE_DIR = 0x00;
	PORTE_OUT = 0xff;
	PORTF_DIR = 0x00;
	PORTF_OUT = 0xff;
	PORTH_DIR = 0x00;
	PORTH_OUT = 0xff;
	PORTJ_DIR = 0x00;
	PORTJ_OUT = 0xff;
	PORTK_DIR = 0x00;
	PORTK_OUT = 0xff;
	PORTQ_DIR = 0x00;
	PORTQ_OUT = 0xff;
	PORTR_DIR = 0x00;
	PORTR_OUT = 0xff;

	PORTA_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x00;

	PORTB_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x00;

	PORTC_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x00;
	

	PORTD_DIRSET = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false; PORTD_DIRCLR = 0x01;
	PORTD_DIRSET = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false; PORTD_DIRCLR = 0x02;
	PORTD_DIRSET = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false; PORTD_DIRCLR = 0x10;
	PORTD_DIRSET = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false; PORTD_DIRCLR = 0x20;

	PORTE_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x00;

	PORTF_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x00;

	PORTH_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x00;

	PORTJ_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x00;

	PORTK_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x00;

	PORTQ_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTQ_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTQ_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTQ_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTQ_DIR = 0x00;

	PORTR_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTR_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTR_DIR = 0x00;
	
	return true;
}

void ecd300TestJbi(void)
{
	
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
	
	counter_init();
	//wait for 5 seconds
	for(c=0; c<30; c++){
		counter_wait(1);
	}

	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;
	ecd300InitUart(ECD300_UART_2, &uartOption);
	printString("Serial Port in Power Allocator was initialized\r\n");

	if(shortcircute_test()) {
		printString("No short circute in ports\r\n");
	}
	else {
		printString("Short circute in a port\r\n");
	}
	
	while(1);
	
	commandState.solenoidDurationDivision = DEFAULT_SOLENOID_DURATION_DIVISON; 
	
	//PD0 works as indicator of host output
	PORTD_DIRSET = 0x01;
	udc_start();
	
	while(1)
	{
		unsigned char key, tdo;
		
		if (udi_cdc_is_rx_ready()) 
		{
			//read a command from USB buffer.
			key = (unsigned char)udi_cdc_getc();
			
			writeInputBuffer(key);
			writeOutputBufferChar(key);
			if(key == 0x0D) {
				writeOutputBufferChar(0x0A); //append a line feed.
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
			
			// 0x0D is command terminator
			if((key == 0x0D) && (commandState.state == AWAITING_COMMAND)) 
			{ 
				unsigned char cmd;
				unsigned char param = 0;
				bool validCmd = true;
				
				cmd = readInputBuffer();
				printString("CMD:");printHex(cmd);printString("\r\n");
				if(0 == cmd) {
					validCmd = false;
				}
				else if(0x0D == cmd) {
					validCmd = false;
				}
				else {
					//read parameter of the command
					for(;;) {
						unsigned char c;
						c = readInputBuffer();
						
						if((c >= '0') && (c <= '9')) {
							param =  param * 10 + c - '0';
						}
						else if(0x0D == c) {
							break; //end of a command
						}
						else {
							//illegal character in parameter
							writeOutputBufferString("Illegal parameters\r\n");
							clearInputBuffer();
							validCmd = false;
							break;
						}
					}
				}
				
				printString("param:");printHex(param);printString("\r\n");
				if(!validCmd) {
					deactivate_all_solenoids();
					disconnect_all_smart_card();
				}
				else 
				{
					switch(cmd)
					{
					case 'I':
					case 'i':
						// Insert smart card.
						commandState.state = STARTING_COMMAND;
						commandState.command = 'I';
						commandState.param = param;
						break;
						
					case 'P':
					case 'p':
						// Pullout smart card.
						commandState.state = STARTING_COMMAND;
						commandState.command = 'P';
						commandState.param = param;
						break;
						
					case 'C':
					case 'c':
						//connect smart card
						commandState.state = STARTING_COMMAND;
						commandState.command = 'C';
						commandState.param = param;
						break;
						
					case 'D':
					case 'd':
						//solenoid duration division
						commandState.state = STARTING_COMMAND;
						commandState.command = 'D';
						commandState.param = param;
						break;
						
					case 'Q':
					case 'q':
						// Query
						commandState.state = STARTING_COMMAND;
						commandState.command = 'Q';
						break;						
					default:
						writeOutputBufferString("Invalid command\r\n");
						deactivate_all_solenoids();
						disconnect_all_smart_card();
						break;
					}	
				}
			}
		}

		run_command();
		
		check_status();

		sendOutputBufferToHost();
	}

	while(1)
	{
		;
	}
}

