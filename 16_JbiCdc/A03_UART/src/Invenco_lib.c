/*
 * ECD300_test.c
 *
 * Created: 2014/12/20 1:42:02
 *  Author: user
 */ 

#include "Invenco_lib.h"
#include "crc.h"

//////////////////////////////////////////////////
//// public functions
//////////////////////////////////////////////////

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


#define BUFFER_LENGTH 0x1FF
static	unsigned char inputBuffer[BUFFER_LENGTH];
static	unsigned char outputBuffer[BUFFER_LENGTH];
static	unsigned short inputProducerIndex=0;
static	unsigned short inputConsumerIndex=0;
static	unsigned short outputProducerIndex=0;
static	unsigned short outputConsumerIndex=0;
static  bool outputOverflow = false;

void clearInputBuffer(void)
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
unsigned char readInputBuffer(void)
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
		if(outputOverflow == false) {
			outputOverflow = true;
			printString("*");
		}
		return false;
	}
	else {
		outputOverflow = false;
		outputBuffer[outputProducerIndex] = c;
		outputProducerIndex = nextProducerIndex;
		return true;
	}
}

//write a string to output buffer
void writeOutputBufferString(const char * pString)
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

void sendOutputBufferToHost(void)
{
	//for(;outputConsumerIndex != outputProducerIndex;)
	//{
		//if(udi_cdc_is_tx_ready()) {
			//udi_cdc_putc(outputBuffer[outputConsumerIndex]);
			//outputConsumerIndex = (outputConsumerIndex + 1) % BUFFER_LENGTH;
		//}
		//else {
			//break;
		//}
	//}
}

void counter_init(void)
{
	//set counter. The resolution should be 31MHz/1024.
	tc_enable(&TCC0);
	tc_set_resolution(&TCC0, 1);
}

//wait for 1/time second
void counter_wait(unsigned char time)
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

unsigned short counter_get(void)
{
	return tc_read_count(&TCC0);
}

unsigned short counter_diff(unsigned short prevCounter)
{
	unsigned short curCounter = counter_get();
	
	if(curCounter > prevCounter) {
		return (curCounter - prevCounter);
	}
	else {
		return (0xFFFF - prevCounter + curCounter);
	}
}

unsigned short counter_clock_length(void)
{
	return 1000000/tc_get_resolution(&TCC0);
}



static struct SCS_Input_Stage _scsInputStage;
static unsigned short _scsInputTimeOut;
static struct SCS_Output_Stage _scsOutputStage;
static unsigned short _scsDataAckTimeout;

static bool _calculateCrc16(unsigned char * pData, unsigned char length, unsigned char * pCrcLow, unsigned char * pCrcHigh)
{
	uint32_t crc;
	
	if((pData == NULL) || (pCrcLow == NULL) || (pCrcHigh == NULL)) {
		printString("ERR: NULL in _calculateCrc16\r\n");
		return false;
	}
	
	crc_set_initial_value(0xffffffff);
	crc = crc_io_checksum(pData, length, CRC_16BIT);
	
	*pCrcLow = crc&0xff;
	*pCrcHigh = (crc>>8)&0xff;
	
	return true;
}

//Send acknowledge of input data packet
// return true if acknowledge is put to output stage
// return false if acknowledge cannot be put to output stage
static bool _sendInputAcknowledgment(unsigned char packetId)
{
	if((_scsOutputStage.state != SCS_OUTPUT_IDLE) && (_scsOutputStage.state != SCS_OUTPUT_WAITING_ACK)) {
		return false;
	}
	
	unsigned char * pPacket = _scsOutputStage.deliveryBuffer;
	unsigned char crcLow, crcHigh;
	
	//construct the ACK packet.	
	pPacket[0] = SCS_ACK_PACKET_TAG;
	pPacket[1] = packetId;
	for(unsigned char i=2; i<(SCS_PACKET_LENGTH -2); i++) {
		pPacket[i] = 0;
	}
	_calculateCrc16(pPacket, SCS_PACKET_LENGTH - 2, &crcLow, &crcHigh);
	pPacket[SCS_PACKET_LENGTH - 2] = crcLow;
	pPacket[SCS_PACKET_LENGTH - 1] = crcHigh;
	
	//change state
	if(_scsOutputStage.state == SCS_OUTPUT_IDLE) {
		_scsOutputStage.state = SCS_OUTPUT_SENDING;
	}
	else { // in SCS_OUTPUT_WAITING_ACK
		_scsOutputStage.state = SCS_OUTPUT_WAITING_ACK_AND_SENDING;
	}
	_scsOutputStage.deliveryIndex = 0;
		
	return true;
}

static void _acknowledgeScsOutputPacket(unsigned char packetId)
{
	if((_scsOutputStage.state != SCS_OUTPUT_WAITING_ACK) && (_scsOutputStage.state != SCS_OUTPUT_WAITING_ACK_AND_SENDING)) {
		return; //do not expect an ACK.
	}
	
	if(_scsOutputStage.packetBuffer[0] == SCS_DATA_PACKET_TAG) 
	{
		if(_scsOutputStage.packetBuffer[1] == packetId) 
		{
			if(_scsOutputStage.state == SCS_OUTPUT_WAITING_ACK) {
				_scsOutputStage.state = SCS_OUTPUT_IDLE; //ready for the next block of data
				_scsOutputStage.deliveryIndex = 0;
			}
			else {
				_scsOutputStage.state = SCS_OUTPUT_SENDING;
			}
		}
	}
}

static bool _saveInputData(unsigned char data)
{
	if((_scsInputStage.dataBufferWriteIndex + 1) == _scsInputStage.dataBufferReadIndex)
	{
		if(_scsInputStage.dataBufferOverflow == false)
		{
			printString("input overflow\r\n");
			_scsInputStage.dataBufferOverflow = true;
		}
		return false;
	}
	else
	{
		_scsInputStage.dataBufferOverflow = false;
		_scsInputStage.dataBuffer[_scsInputStage.dataBufferWriteIndex] = data;
		_scsInputStage.dataBufferWriteIndex = (_scsInputStage.dataBufferWriteIndex + 1) & SCS_INPUT_STAGE_DATA_BUFFER_LENGTH;
		return true;
	}
}

bool getScsInputData(unsigned char * pData)
{
	if(pData == NULL) {
		return false;
	}
	
	if(_scsInputStage.dataBufferReadIndex == _scsInputStage.dataBufferWriteIndex) {
		return false;
	}
	else
	{
		*pData = _scsInputStage.dataBuffer[_scsInputStage.dataBufferReadIndex];
		_scsInputStage.dataBufferReadIndex = (_scsInputStage.dataBufferReadIndex + 1) & SCS_INPUT_STAGE_DATA_BUFFER_LENGTH;
		return true;
	}
}

static void _processScsInputStage(void)
{
	if(_scsInputStage.packetByteAmount > 0)
	{
		//partial packet 
		if(counter_diff(_scsInputStage.timeStamp) >= _scsInputTimeOut) {
			_scsInputStage.packetByteAmount = 0; //discard packet data
		}
	}
	
	for(;;)
	{
		if(udi_cdc_is_rx_ready())
		{
			_scsInputStage.packetBuffer[_scsInputStage.packetByteAmount] = udi_cdc_getc();
			_scsInputStage.packetByteAmount++;
			
			if(_scsInputStage.packetByteAmount < SCS_PACKET_LENGTH) {
				continue; //incomplete packet, continue receiving
			}
			
			//enough bytes for a packet, check CRC
			unsigned char crcLow, crcHigh;
			unsigned char * pPacket =_scsInputStage.packetBuffer;
							
			_calculateCrc16(pPacket, SCS_PACKET_LENGTH - 2, &crcLow, &crcHigh);
			if((crcLow != pPacket[SCS_PACKET_LENGTH -2]) || (crcHigh != pPacket[SCS_PACKET_LENGTH -1])) 
			{
				//invalid packet
				printString("ERROR: crc mismatch\r\n");
				_scsInputStage.packetByteAmount = 0; //discard packet
				break;
			}
			
			//a valid packet
			if(pPacket[0] == SCS_DATA_PACKET_TAG)
			{
				if(pPacket[1] != _scsInputStage.previousId) 
				{
					//new packet, the host has received the acknowledgment for previous packet.
					_scsInputStage.previousId = pPacket[1];
							
					//send data to application
					for(unsigned char i=0; i<pPacket[2]; i++) 
					{
						if(_saveInputData(pPacket[3 + i]) == false) {
							break;									
						}
					}
				}
				_scsInputStage.state = SCS_INPUT_ACKNOWLEDGING;
				printString("> D "); printHex(pPacket[1]); printString("\r\n");
			}
			else if(pPacket[0] == SCS_ACK_PACKET_TAG)
			{
				_acknowledgeScsOutputPacket(pPacket[1]);
				printString("> A "); printHex(pPacket[1]); printString("\r\n");
			}
			else
			{
				printString("ERROR: unknown packet type ");
				printHex(pPacket[0]);
				printString("\r\n");
			}
			
			_scsInputStage.packetByteAmount = 0;	
			break; //finished a packet, process further input in next round to keep state correct
		}
		else
		{
			if(_scsInputStage.packetByteAmount > 0) {
				_scsInputStage.timeStamp = counter_get(); //partial packet
			}
			
			break;
		}
	}
}

//send out data in output stage as much as possible
static void _processScsOutputStage(void)
{
	if(_scsOutputStage.state == SCS_OUTPUT_SENDING) 
	{
		for(;;) 
		{
			if(udi_cdc_is_tx_ready())
			{
				udi_cdc_putc(_scsOutputStage.deliveryBuffer[_scsOutputStage.deliveryIndex]);
				_scsOutputStage.deliveryIndex++;
				if(_scsOutputStage.deliveryIndex < SCS_PACKET_LENGTH) {
					continue;
				}
				
				//a packet has been sent out
				if(_scsOutputStage.deliveryBuffer[0] == SCS_ACK_PACKET_TAG) 
				{
					_scsOutputStage.deliveryIndex = 0;
					_scsOutputStage.state = SCS_OUTPUT_IDLE; //no acknowledgment is needed
					printString("< A ");printHex(_scsOutputStage.deliveryBuffer[1]);printString("\r\n");
				}
				else if(_scsOutputStage.deliveryBuffer[0] == SCS_DATA_PACKET_TAG) 
				{
					_scsOutputStage.state = SCS_OUTPUT_WAITING_ACK; //wait for the acknowledgment
					_scsOutputStage.timeStamp = counter_get();
					printString("< D ");printHex(_scsOutputStage.deliveryBuffer[1]);printString("\r\n");
				}
				else 
				{
					//unknown packet type
					printString("ERROR: unknown packet was sent\r\n");
					_scsOutputStage.deliveryIndex = 0;
					_scsOutputStage.state = SCS_OUTPUT_IDLE;
				}
				break;
			}
			else {
				break;//no free space in USB end point
			}
		}
	}
	else if(_scsOutputStage.state == SCS_OUTPUT_WAITING_ACK_AND_SENDING) 
	{
		for(;;)
		{
			if(udi_cdc_is_tx_ready())
			{
				udi_cdc_putc(_scsOutputStage.deliveryBuffer[_scsOutputStage.deliveryIndex]);
				_scsOutputStage.deliveryIndex++;
				if(_scsOutputStage.deliveryIndex < SCS_PACKET_LENGTH) {
					continue;
				}
				
				//a packet has been sent out
				if(_scsOutputStage.deliveryBuffer[0] == SCS_ACK_PACKET_TAG) 
				{
					_scsOutputStage.state = SCS_OUTPUT_WAITING_ACK; //no acknowledgment is needed
					printString("< A ");printHex(_scsOutputStage.deliveryBuffer[1]);printString("\r\n");
				}
				else if(_scsOutputStage.deliveryBuffer[0] == SCS_DATA_PACKET_TAG) 
				{
					//no data packet should be sent when a data packet is waiting for ACK.
					printString("ERROR: wrong data packet was sent\r\n");
					_scsOutputStage.state = SCS_OUTPUT_WAITING_ACK; //wait for the acknowledgment
				}
				else 
				{
					//unknown packet type
					printString("ERROR: unknown packet was sent\r\n");
					_scsOutputStage.deliveryIndex = 0;
					_scsOutputStage.state = SCS_OUTPUT_IDLE;
				}
				break;
			}
			else {
				break;//no free space in USB end point
			}
		}
	}
	
	if(_scsOutputStage.state == SCS_OUTPUT_WAITING_ACK) 
	{
		if(counter_diff(_scsOutputStage.timeStamp) >= _scsDataAckTimeout) 
		{
			//re-send this data packet
			for(unsigned char i=0; i<SCS_PACKET_LENGTH; i++) {
				_scsOutputStage.deliveryBuffer[i] = _scsOutputStage.packetBuffer[i];
			}
			_scsOutputStage.state = SCS_OUTPUT_SENDING;
			_scsOutputStage.deliveryIndex = 0;
		}
	}
	
	if(_scsInputStage.state == SCS_INPUT_ACKNOWLEDGING)
	{
		if(_sendInputAcknowledgment(_scsInputStage.previousId)) {
			_scsInputStage.state = SCS_INPUT_RECEIVING; 
		}
	}
}

static void _fillScsOutputStage(void)
{
	unsigned char * pPacket;
	unsigned char dataAmount;
	unsigned char crcLow, crcHigh;
	
	if(_scsOutputStage.state != SCS_OUTPUT_IDLE) {
		return;
	}
	
	if(outputConsumerIndex == outputProducerIndex) {
		return; //no application data
	}
	
	//get a block of data from outputBuffer 
	pPacket = _scsOutputStage.packetBuffer;
	pPacket[0] = SCS_DATA_PACKET_TAG;
	pPacket[1] = _scsOutputStage.packetId;
	for(dataAmount = 0; outputConsumerIndex != outputProducerIndex; )
	{
		pPacket[3 + dataAmount] = outputBuffer[outputConsumerIndex];
		dataAmount++;
		outputConsumerIndex = (outputConsumerIndex + 1) % BUFFER_LENGTH;
		if(dataAmount == (SCS_PACKET_LENGTH - 5)) {
			break; //packet is full
		}
	}
	pPacket[2] = dataAmount;
	for(; dataAmount < (SCS_PACKET_LENGTH - 5); dataAmount++) {
		pPacket[3 + dataAmount] = 0; //padding
	}
	//calculate CRC
	_calculateCrc16(pPacket, SCS_PACKET_LENGTH - 2, &crcLow, &crcHigh);
	pPacket[SCS_PACKET_LENGTH - 2] = crcLow;
	pPacket[SCS_PACKET_LENGTH - 1] = crcHigh;
	
	//copy packet content
	for(unsigned char i=0; i<SCS_PACKET_LENGTH; i++) {
		_scsOutputStage.deliveryBuffer[i] = _scsOutputStage.packetBuffer[i];
	}
	
	//increase packet id
	_scsOutputStage.packetId++;
	if(_scsOutputStage.packetId == SCS_INVALID_PACKET_ID) {
		_scsOutputStage.packetId++; //jump over the invalid packet id
	}
	if(_scsOutputStage.packetId == SCS_INITIAL_PACKET_ID) {
		_scsOutputStage.packetId++; //jump over the initial packet id
	}
	
	//get ready for being sent.
	_scsOutputStage.state = SCS_OUTPUT_SENDING;
	_scsOutputStage.deliveryIndex = 0;
}

void pollScsDataExchange(void)
{
	_processScsOutputStage();
	_processScsInputStage();
	_fillScsOutputStage();
}

void initScsDataExchange(void)
{
	_scsInputTimeOut = ((uint32_t)SCS_DATA_INPUT_TIMEOUT * 1000)/counter_clock_length();
	
	_scsInputStage.state = SCS_INPUT_RECEIVING;
	_scsInputStage.packetByteAmount = 0;
	_scsInputStage.previousId = SCS_INVALID_PACKET_ID;
	_scsInputStage.dataBufferOverflow = false;
	_scsInputStage.dataBufferReadIndex = 0;
	_scsInputStage.dataBufferWriteIndex = 0;
	
	_scsDataAckTimeout = ((uint32_t)SCS_DATA_ACK_TIMEOUT * 1000)/counter_clock_length();
	_scsOutputStage.deliveryIndex = 0;
	_scsOutputStage.state = SCS_OUTPUT_IDLE;
	_scsOutputStage.packetId = SCS_INITIAL_PACKET_ID;
}

void Invenco_init(void)
{
	usart_rs232_options_t uartOption;
	
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
	
	//counter
	counter_init();

	//uart
	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;
	ecd300InitUart(ECD300_UART_2, &uartOption);
	printString("UART is initialized successfully\r\n");
	
	//data exchange
	udc_start();
	initScsDataExchange();
}
