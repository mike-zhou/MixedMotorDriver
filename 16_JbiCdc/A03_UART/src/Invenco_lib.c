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
void printString(char * pString)
{
#ifdef DATA_EXCHANGE_THROUGH_USB
	unsigned char c;
	char rc;
	
	//send info to UART as much as possible
	for(;;)
	{
		c = *pString;
		if(c == '\0') {
			break;
		}
		rc = ecd300PutChar(ECD300_UART_2, c);
		if(rc != 0) {
			break;
		}
		pString++;
	}
#else
	for(; *pString != '\0'; )
	{
		if(udi_cdc_is_tx_ready()) {
			udi_cdc_putc(*pString);
			pString++;	
		}		
		else {
			break;
		}
	}
#endif
}

void printHex(unsigned char hex)
{
#ifdef DATA_EXCHANGE_THROUGH_USB
	unsigned char c;
	unsigned char rc;
	
	c = (hex >> 4) & 0xf;
	if(c <= 9) {
		rc = ecd300PutChar(ECD300_UART_2, c + '0');
	}
	else {
		rc = ecd300PutChar(ECD300_UART_2, c - 0xA + 'A');
	}
	
	if(rc == 0) 
	{
		c = hex & 0xf;
		if(c <= 9) {
			rc = ecd300PutChar(ECD300_UART_2, c + '0');
		}
		else {
			rc = ecd300PutChar(ECD300_UART_2, c - 0xA + 'A');
		}
	}

#else
	if(udi_cdc_is_tx_ready()) {
		//high 4 bits
		if(((hex >> 4) & 0x0F) <= 9) {
			udi_cdc_putc(((hex >> 4) & 0x0F) + '0');
		}	
		else {
			udi_cdc_putc(((hex >> 4) & 0x0F) - 0xA + 'A');
		}
	}
	if(udi_cdc_is_tx_ready()) {
		//low 4 bits
		if((hex & 0x0F) <= 9) {
			udi_cdc_putc((hex & 0x0F) + '0');
		}
		else {
			udi_cdc_putc((hex & 0x0F) - 0xA + 'A');
		}
	}
#endif
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
	//printString("Port: ");
	//printHex(port);
	//printString(" is enabled\r\n");
	
	return true;
}

void main_cdc_disable(uint8_t port)
{
	//printString("Port: ");
	//printHex(port);
	//printString(" is disabled\r\n");
}

void main_cdc_set_dtr(uint8_t port, bool b_enable)
{
	//if (b_enable) 
	//{
		//printString("Host opened serial port:");
		//printHex(port);
		//printString("\r\n");
	//}
	//else
	//{
		//printString("Host closed serial port:");
		//printHex(port);
		//printString("\r\n");
	//}
}

void main_uart_rx_notify(uint8_t port)
{
	;
}

void main_uart_config(uint8_t port, usb_cdc_line_coding_t * cfg)
{
	//printString("Host is configuring serial port: ");
	//printHex(port);
	//printString("\r\n");
}

//data buffer for upper layer software
#define APP_INPUT_BUFFER_INDEX_MASK 0xFF
#define APP_OUTPUT_BUFFER_INDEX_MASK 0x3FF
static	unsigned char inputBuffer[APP_INPUT_BUFFER_INDEX_MASK + 1]; 
static	unsigned char outputBuffer[APP_OUTPUT_BUFFER_INDEX_MASK + 1];
static	unsigned short inputProducerIndex=0;
static	unsigned short inputConsumerIndex=0;
static	unsigned short outputProducerIndex=0;
static	unsigned short outputConsumerIndex=0;
static  bool outputBufferEnabled = false;
static  bool outputOverflow = false;
static  bool inputOverflow = false;

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
	unsigned short nextProducerIndex = (inputProducerIndex + 1) & APP_INPUT_BUFFER_INDEX_MASK;
	
	if(nextProducerIndex != inputConsumerIndex) {
		inputBuffer[inputProducerIndex] = c;
		inputProducerIndex = nextProducerIndex;
		inputOverflow = false;
		return true;
	}
	else {
		//overflow.
		if(!inputOverflow) {
			inputOverflow = true;
			printChar('!');
		}
		clearInputBuffer();
		return false;
	}
}

// read a character from input buffer.
// return the first character in input buffer,
// return 0 if there is nothing in the input buffer.
unsigned char readInputBuffer(void)
{
	unsigned char c;
	
	if(inputConsumerIndex != inputProducerIndex) {
		c = inputBuffer[inputConsumerIndex];
		inputConsumerIndex = (inputConsumerIndex + 1) & APP_INPUT_BUFFER_INDEX_MASK;
	}
	else {
		c = 0;
	}
	
	return c;
}

//write a character to output buffer
//return true if parameter is saved successfully
//return false if output buffer overflows
bool writeOutputBufferChar(unsigned char c)
{
	if(!outputBufferEnabled) {
		return false;
	}
	
	unsigned short nextProducerIndex;
	
	nextProducerIndex = (outputProducerIndex + 1) & APP_OUTPUT_BUFFER_INDEX_MASK;
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
	if(!outputBufferEnabled) {
		return;
	}
	
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
	if(!outputBufferEnabled) {
		return;
	}
	
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
			//outputConsumerIndex = (outputConsumerIndex + 1) & BUFFER_LENGTH;
		//}
		//else {
			//break;
		//}
	//}
}

void enableOutputBuffer(void)
{
	outputBufferEnabled = true;
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
	
	if(curCounter >= prevCounter) {
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

static inline void _uCharToHex(unsigned char c, unsigned char * pLow4Bits, unsigned char * pHigh4Bits) 
{
	unsigned char low, high;
	
	low = c & 0xf;
	if(low <= 9) {
		*pLow4Bits = low + '0';
	}
	else {
		*pLow4Bits = low - 0xA + 'A';
	}
	
	high = (c >> 4) & 0xf;
	if(high <= 9) {
		*pHigh4Bits = high + '0';
	}
	else {
		*pHigh4Bits = high - 0xA + 'A';
	}
}

static inline unsigned char _uCharFromHex(unsigned char low4Bits, unsigned char high4Bits)
{
	unsigned char c;
	
	if((high4Bits >= '0') && (high4Bits <= '9')) {
		c = high4Bits - '0';
	}
	else if((high4Bits >= 'A') && (high4Bits <= 'F')) {
		c = high4Bits - 'A' + 0xA;
	}
	else { //take for granted that the high4Bits is between 'a' and 'f'
		c = high4Bits - 'a' + 0xA;
	}
	
	c = (c << 4) & 0xF0;

	if((low4Bits >= '0') && (low4Bits <= '9')) {
		c += low4Bits - '0';
	}
	else if((low4Bits >= 'A') && (low4Bits <= 'F')) {
		c += low4Bits - 'A' + 0xA;
	}
	else { //take for granted that the low4Bits is between 'a' and 'f'
		c += low4Bits - 'a' + 0xA;
	}
	
	return c;
}

static bool _getChar(unsigned char * p)
{
#ifdef DATA_EXCHANGE_THROUGH_USB
	if(udi_cdc_is_rx_ready()) {
		*p = udi_cdc_getc();
		return true;
	}
	else {
		return false;
	}
#else
	char rc = ecd300PollChar(ECD300_UART_2, p);
	if(rc==1) {
		return true;
	}
	else {
		return false;
	}
#endif
}

static bool _putChar(unsigned char c)
{
#ifdef DATA_EXCHANGE_THROUGH_USB
	if(udi_cdc_is_tx_ready()) {
		udi_cdc_putc(c);
		return true;		
	}	
	else {
		return false;
	}
#else
	int rc = ecd300PutChar(ECD300_UART_2, c);
	if(rc == 0) {
		return true;
	}
	else {
		return false;
	}
#endif
}

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
	unsigned char lowHex, highHex;
	
	//construct the ACK packet.	
	//tag
	_uCharToHex(SCS_ACK_PACKET_TAG, &lowHex, &highHex);
	pPacket[0] = highHex;
	pPacket[1] = lowHex;
	//packetId
	_uCharToHex(packetId, &lowHex, &highHex);
	pPacket[2] = highHex;
	pPacket[3] = lowHex;
	//paddings
	for(unsigned char i=4; i<(SCS_PACKET_LENGTH -4); i++) {
		pPacket[i] = '0';
	}
	//crc
	_calculateCrc16(pPacket, SCS_PACKET_LENGTH - 4, &crcLow, &crcHigh);
	_uCharToHex(crcLow, &lowHex, &highHex);
	pPacket[SCS_PACKET_LENGTH - 4] = highHex;
	pPacket[SCS_PACKET_LENGTH - 3] = lowHex;
	_uCharToHex(crcHigh, &lowHex, &highHex);
	pPacket[SCS_PACKET_LENGTH - 2] = highHex;
	pPacket[SCS_PACKET_LENGTH - 1] = lowHex;
	
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

	unsigned char tag = _uCharFromHex(_scsOutputStage.packetBuffer[1], _scsOutputStage.packetBuffer[0]);
	
	if(tag == SCS_DATA_PACKET_TAG) 
	{
		unsigned char curPacketId = _uCharFromHex(_scsOutputStage.packetBuffer[3], _scsOutputStage.packetBuffer[2]);
		if(curPacketId == packetId) 
		{
			if(_scsOutputStage.state == SCS_OUTPUT_WAITING_ACK) {
				_scsOutputStage.state = SCS_OUTPUT_IDLE; //ready for the next block of data
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
		_scsInputStage.dataBufferWriteIndex = (_scsInputStage.dataBufferWriteIndex + 1) & SCS_INPUT_STAGE_DATA_BUFFER_INDEX_MASK;
		
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
		_scsInputStage.dataBufferReadIndex = (_scsInputStage.dataBufferReadIndex + 1) & SCS_INPUT_STAGE_DATA_BUFFER_INDEX_MASK;
		return true;
	}
}

static void _processScsInputStage(void)
{
	bool dataReceived = false;
	
	if(_scsInputStage.packetByteAmount > 0)
	{
		//partial packet 
		if(counter_diff(_scsInputStage.timeStamp) >= _scsInputTimeOut) 
		{
			printString("ERROR: - ");
			printHex(_scsInputStage.packetByteAmount);
			printString(":");
			for(unsigned char i=0; i<_scsInputStage.packetByteAmount; i++) {
				printHex(_scsInputStage.packetBuffer[i]);
			}
			printString("\r\n");
			_scsInputStage.packetByteAmount = 0; //discard packet data
			printString("ERROR: discard data\r\n");
		}
	}
	
	for(;;)
	{
		if(_getChar(_scsInputStage.packetBuffer + _scsInputStage.packetByteAmount))
		{
			dataReceived = true;
			_scsInputStage.packetByteAmount++;
			
			if(_scsInputStage.packetByteAmount < SCS_PACKET_LENGTH) {
				continue; //incomplete packet, continue receiving
			}
			
			//enough bytes for a packet, check CRC
			unsigned char crcLow, crcHigh;
			unsigned char * pPacket =_scsInputStage.packetBuffer;
			unsigned char tmpChar;
			bool packetError = false;
			
			for(unsigned char i=0; i<SCS_PACKET_LENGTH; i++) 
			{
				tmpChar = pPacket[i];
				
				if((tmpChar >= '0') && (tmpChar <= '9')) continue;
				if((tmpChar >= 'A') && (tmpChar <= 'F')) continue;
				if((tmpChar >= 'a') && (tmpChar <= 'f')) continue;
				
				packetError = true;
				break;	
			}
			if(packetError)
			{
				printString("ERROR: non-hex\r\n");
				_scsInputStage.packetByteAmount = 0; //discard packet
				break;
			}
							
			_calculateCrc16(pPacket, SCS_PACKET_LENGTH - 4, &crcLow, &crcHigh);
			if((crcLow != _uCharFromHex(pPacket[SCS_PACKET_LENGTH - 3], pPacket[SCS_PACKET_LENGTH - 4])) || 
				(crcHigh != _uCharFromHex(pPacket[SCS_PACKET_LENGTH - 1], pPacket[SCS_PACKET_LENGTH - 2]))) 
			{
				//invalid packet
				printString("ERROR: crc mismatch:");
				for(unsigned char i=0; i<SCS_PACKET_LENGTH; i++) {
					printHex(_scsInputStage.packetBuffer[i]);
				}
				printString("\r\n");
				
				_scsInputStage.packetByteAmount = 0; //discard packet
				break;
			}
			
			//a valid packet
			unsigned char tag = _uCharFromHex(pPacket[1], pPacket[0]);
			unsigned char curPacketId = _uCharFromHex(pPacket[3], pPacket[2]); 
			
			if(tag == SCS_DATA_PACKET_TAG)
			{
				if(curPacketId != _scsInputStage.previousId) 
				{
					//new packet, the host has received the acknowledgment for previous packet.
					_scsInputStage.previousId = curPacketId;
							
					//send data to application
					unsigned char length = _uCharFromHex(pPacket[5], pPacket[4]);
					length = length << 1; //convert to length of HEX bytes.
					for(unsigned char i=0; i<length; i+=2) 
					{
						if(_saveInputData(_uCharFromHex(pPacket[6 + i + 1], pPacket[6 + i])) == false) {
							break;									
						}
					}
				}
				printString("> D "); 
				printHex(curPacketId); 
				printString("\r\n");
				_scsInputStage.state = SCS_INPUT_ACKNOWLEDGING;
			}
			else if(tag == SCS_ACK_PACKET_TAG)
			{
				printString("> A "); 
				printHex(curPacketId); 
				printString("\r\n");
				_acknowledgeScsOutputPacket(curPacketId);
			}
			else
			{
				printString("ERROR: unknown packet type ");
				printHex(tag);
				printString(" ");
			}
			
			_scsInputStage.packetByteAmount = 0;	
			break; //finished a packet, process further input in next round to keep state correct
		}
		else
		{
			if(dataReceived) {
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
			if(_putChar(_scsOutputStage.deliveryBuffer[_scsOutputStage.deliveryIndex]))
			{
				_scsOutputStage.deliveryIndex++;
				if(_scsOutputStage.deliveryIndex < SCS_PACKET_LENGTH) {
					continue;
				}
				
				//a packet has been sent out
				unsigned char tag = _uCharFromHex(_scsOutputStage.deliveryBuffer[1], _scsOutputStage.deliveryBuffer[0]);
				unsigned char curPacketId = _uCharFromHex(_scsOutputStage.deliveryBuffer[3], _scsOutputStage.deliveryBuffer[2]);

				if(tag == SCS_ACK_PACKET_TAG) 
				{
					_scsOutputStage.state = SCS_OUTPUT_IDLE; //no acknowledgment is needed
					printString("< A ");
					printHex(curPacketId);
					printString("\r\n");
				}
				else if(tag == SCS_DATA_PACKET_TAG) 
				{
					_scsOutputStage.state = SCS_OUTPUT_WAITING_ACK; //wait for the acknowledgment
					_scsOutputStage.timeStamp = counter_get();
					printString("< D ");
					printHex(curPacketId);
					printString("\r\n");
				}
				else 
				{
					//unknown packet type
					printString("ERROR: unknown packet was sent\r\n");
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
			if(_putChar(_scsOutputStage.deliveryBuffer[_scsOutputStage.deliveryIndex]))
			{
				_scsOutputStage.deliveryIndex++;
				if(_scsOutputStage.deliveryIndex < SCS_PACKET_LENGTH) {
					continue;
				}
				
				//a packet has been sent out
				unsigned char tag = _uCharFromHex(_scsOutputStage.deliveryBuffer[1], _scsOutputStage.deliveryBuffer[0]);
				unsigned char curPacketId = _uCharFromHex(_scsOutputStage.deliveryBuffer[3], _scsOutputStage.deliveryBuffer[2]);
				
				if(tag == SCS_ACK_PACKET_TAG) 
				{
					printString("< A ");
					printHex(curPacketId);
					printString("\r\n");
				}
				else if(tag == SCS_DATA_PACKET_TAG) 
				{
					//no data packet should be sent when a data packet is waiting for ACK.
					printString("ERROR: wrong data packet was sent\r\n");
				}
				else 
				{
					//unknown packet type
					printString("ERROR: unknown packet was sent\r\n");
				}
				
				_scsOutputStage.state = SCS_OUTPUT_WAITING_ACK; //wait for the acknowledgment
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
	unsigned char lowHex, highHex;
	
	if(_scsOutputStage.state != SCS_OUTPUT_IDLE) {
		return;
	}
	
	if(outputConsumerIndex == outputProducerIndex) {
		return; //no application data
	}
	
	//get a block of data from outputBuffer 
	pPacket = _scsOutputStage.packetBuffer;
	//tag
	_uCharToHex(SCS_DATA_PACKET_TAG, &lowHex, &highHex);
	pPacket[0] = highHex;
	pPacket[1] = lowHex;
	//packetId
	_uCharToHex(_scsOutputStage.packetId, &lowHex, &highHex);
	pPacket[2] = highHex;
	pPacket[3] = lowHex;
	//data
	for(dataAmount = 0; outputConsumerIndex != outputProducerIndex; )
	{
		_uCharToHex(outputBuffer[outputConsumerIndex], &lowHex, &highHex);
		pPacket[6 + dataAmount] = highHex;
		pPacket[6 + dataAmount + 1] = lowHex;
		
		outputConsumerIndex = (outputConsumerIndex + 1) & APP_OUTPUT_BUFFER_LENGTH;
		dataAmount += 2;
		if(dataAmount == (SCS_PACKET_LENGTH - 10)) {
			break; //packet is full
		}
	}
	//length
	_uCharToHex((dataAmount >> 1), &lowHex, &highHex); //convert to length of actual data
	pPacket[4] = highHex;
	pPacket[5] = lowHex;
	//padding
	for(; dataAmount < (SCS_PACKET_LENGTH - 10); dataAmount++) {
		pPacket[6 + dataAmount] = '0'; //padding
	}
	//calculate CRC
	_calculateCrc16(pPacket, SCS_PACKET_LENGTH - 4, &crcLow, &crcHigh);
	_uCharToHex(crcLow, &lowHex, &highHex);
	pPacket[SCS_PACKET_LENGTH - 4] = highHex;
	pPacket[SCS_PACKET_LENGTH - 3] = lowHex;
	_uCharToHex(crcHigh, &lowHex, &highHex);
	pPacket[SCS_PACKET_LENGTH - 2] = highHex;
	pPacket[SCS_PACKET_LENGTH - 1] = lowHex;
	
	//copy packet content
	for(unsigned char i=0; i<SCS_PACKET_LENGTH; i++) {
		_scsOutputStage.deliveryBuffer[i] = _scsOutputStage.packetBuffer[i];
	}
	
	//get ready for being sent.
	_scsOutputStage.state = SCS_OUTPUT_SENDING;
	_scsOutputStage.deliveryIndex = 0;
	
	//increase packet id
	_scsOutputStage.packetId++;
	if(_scsOutputStage.packetId == SCS_INVALID_PACKET_ID) {
		_scsOutputStage.packetId++; //jump over the invalid packet id
	}
	if(_scsOutputStage.packetId == SCS_INITIAL_PACKET_ID) {
		_scsOutputStage.packetId++; //jump over the initial packet id
	}
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
