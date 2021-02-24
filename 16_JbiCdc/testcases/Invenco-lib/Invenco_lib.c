/*
 * ECD300_test.c
 *
 * Created: 2014/12/20 1:42:02
 *  Author: user
 */ 

#include "Invenco_lib.h"
#include "Invenco_lib_internal.h"
#include "crc.h"

//send a buffer to host through USB
// return amount of bytes sent out to host
static inline unsigned char _putCharsUsb(unsigned char * pBuffer, unsigned char size)
{
	iram_size_t freeSpace = udi_cdc_get_free_tx_buffer();
	iram_size_t remaining = 0;
	
	if(freeSpace > size) {
		freeSpace = size;
	}
	if(freeSpace > 0) {
		remaining = udi_cdc_write_buf(pBuffer, freeSpace);
	}
	
	return freeSpace - remaining;
}

//send a buffer to host through UART
// return amount of bytes sent out to host
static inline unsigned char _putCharsUart(unsigned char * pBuffer, unsigned char size)
{
	unsigned char counter;
	char rc;
	
	for(counter = 0; counter < size; counter++)
	{
		rc = ecd300PutChar(ECD300_UART_2, pBuffer[counter]);
		if(rc != 0) {
			break;
		}
	}
	
	return counter;
}

#if DATA_EXCHANGE_THROUGH_USB
//monitor traffic is through UART.
static bool _writeMonitorChar(unsigned char c)
{
	char rc;
	
	rc = ecd300PutChar(ECD300_UART_2, c);
	if(rc == 0) {
		return true;
	}
	else {
		return false;
	}
}

#else
//monitor traffic is through USB
static unsigned char _monitorOutputBuffer[MONITOR_OUTPUT_BUFFER_LENGTH_MASK + 1];
static unsigned short _monitorOutputBufferConsumerIndex;
static unsigned short _monitorOutputBufferProducerIdnex;

static bool _writeMonitorChar(unsigned char c)
{
	unsigned short nextProducerIndex = (_monitorOutputBufferProducerIdnex + 1) & MONITOR_OUTPUT_BUFFER_LENGTH_MASK;
	if(nextProducerIndex == _monitorOutputBufferConsumerIndex) {
		return false; //buffer full
	}
	_monitorOutputBuffer[nextProducerIndex] = c;
	_monitorOutputBufferProducerIdnex = nextProducerIndex;
	return true;
}	
#endif

static void _processMonitorStage()
{
#if DATA_EXCHANGE_THROUGH_USB	
#else
	if(_monitorOutputBufferConsumerIndex == _monitorOutputBufferProducerIdnex) {
		return; // no data to send
	}
	if(_monitorOutputBufferConsumerIndex < _monitorOutputBufferProducerIdnex) 
	{
		unsigned char * pBuffer = _monitorOutputBuffer + _monitorOutputBufferConsumerIndex;
		unsigned short size = _monitorOutputBufferProducerIdnex - _monitorOutputBufferConsumerIndex;
		unsigned char amount;
		
		if(size > 0xff) {
			size = 0xff;
		}			
		amount = _putCharsUsb(pBuffer, size);
		_monitorOutputBufferConsumerIndex += amount;
	}
	else 
	{
		unsigned char * pBuffer = _monitorOutputBuffer + _monitorOutputBufferConsumerIndex;
		unsigned short size = MONITOR_OUTPUT_BUFFER_LENGTH_MASK - _monitorOutputBufferConsumerIndex + 1;
		unsigned char amount;
		
		if(size > 0xff) {
			size = 0xff;
		}
		amount = _putCharsUsb(pBuffer, size);
		_monitorOutputBufferConsumerIndex = (_monitorOutputBufferConsumerIndex + amount) & MONITOR_OUTPUT_BUFFER_LENGTH_MASK;
	}
#endif	
}

//////////////////////////////////////////////////
//// public functions
//////////////////////////////////////////////////
void printString(char * pString)
{
	for(; *pString != '\0'; )
	{
		if(_writeMonitorChar(*pString)) {
			pString++;	
		}		
		else {
			break;
		}
	}
}

void printHex(unsigned char hex)
{
	unsigned char c;
	
	c = (hex >> 4) & 0xf;
	if(c <= 9) {
		_writeMonitorChar(c + '0');
	}
	else {
		_writeMonitorChar(c - 0xA + 'A');
	}
	
	c = hex & 0xf;
	if(c <= 9) {
		_writeMonitorChar(c + '0');
	}
	else {
		_writeMonitorChar(c - 0xA + 'A');
	}
}

void inline printChar(unsigned char c)
{
	_writeMonitorChar(c);
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
static	unsigned char inputBuffer[APP_INPUT_BUFFER_LENGTH_MASK + 1]; 
static	unsigned char outputBuffer[APP_OUTPUT_BUFFER_LENGTH_MASK + 1];
// the way to check whether buffer is full:
//	if ((producer + 1) & mask) == consumer, then buffer is full
static	unsigned short inputProducerIndex=0;
static	unsigned short inputConsumerIndex=0;
static	unsigned short outputProducerIndex=0;
static	unsigned short outputConsumerIndex=0;
static  bool outputBufferEnabled = false;
static  bool outputOverflow = false;

// return free space in APP's input buffer
static inline unsigned short _getAppInputBufferAvailable()
{
	if(inputProducerIndex >= inputConsumerIndex) {
		return APP_INPUT_BUFFER_LENGTH_MASK - (inputProducerIndex - inputConsumerIndex);
	}
	else {
		return inputConsumerIndex - inputProducerIndex - 1;
	}
}

// write to APP's input buffer
// return amount of byte written to APP's input buffer. 
static inline unsigned short _writeAppInputBuffer(unsigned char * pBuffer, unsigned short length)
{
	unsigned short i;
	unsigned short nextWritingIndex;
	
	for(i=0; i<length; i++) 
	{
		nextWritingIndex = (inputProducerIndex + 1) & APP_INPUT_BUFFER_LENGTH_MASK;
		if(nextWritingIndex == inputConsumerIndex) {
			//input buffer is full
			break;
		}
		
		inputBuffer[inputProducerIndex] = pBuffer[i];
		inputProducerIndex = nextWritingIndex;
	}
	
	return i;
}

void clearInputBuffer(void)
{
	inputProducerIndex = 0;
	inputConsumerIndex = 0;
}

// read a character from input buffer.
// return the first character in input buffer,
// return 0 if there is nothing in the input buffer.
unsigned char readInputBuffer(void)
{
	unsigned char c;
	
	if(inputConsumerIndex != inputProducerIndex) {
		c = inputBuffer[inputConsumerIndex];
		inputConsumerIndex = (inputConsumerIndex + 1) & APP_INPUT_BUFFER_LENGTH_MASK;
	}
	else {
		c = 0;
	}
	
	return c;
}

//return amount of bytes in output buffer
unsigned short _getOutputBufferUsed()
{
	if(outputProducerIndex >= outputConsumerIndex) {
		return outputProducerIndex - outputConsumerIndex;
	}
	else {
		return outputProducerIndex + APP_OUTPUT_BUFFER_LENGTH_MASK + 1 - outputConsumerIndex;
	}
}

//read size of bytes from output buffer to pBuffer
// return actual size of bytes written to pBuffer.
unsigned short _readOutputBuffer(unsigned char * pBuffer, unsigned short size)
{
	unsigned short counter;
	
	for(counter=0; counter<size; counter++) 
	{
		if(outputConsumerIndex == outputProducerIndex) {
			break;
		}
		*pBuffer = outputBuffer[outputConsumerIndex];
		pBuffer++;
		outputConsumerIndex = (outputConsumerIndex + 1) & APP_OUTPUT_BUFFER_LENGTH_MASK;
	}
	
	return counter;
}

//write a character to output buffer
//return true if parameter is saved successfully
//return false if output buffer overflows
bool writeOutputBufferChar(unsigned char c)
{
	if(!outputBufferEnabled) {
		return false;
	}
	
	unsigned short nextProducerIndex = (outputProducerIndex + 1) & APP_OUTPUT_BUFFER_LENGTH_MASK;
	
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

void enableOutputBuffer(void)
{
	outputBufferEnabled = true;
}


/*******************************************************
* Counter
*******************************************************/
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
static unsigned short _scsOutputTimeout;

#if DATA_EXCHANGE_THROUGH_USB

	static unsigned char _usbInputBuffer[USB_INPUT_BUFFER_SIZE];
	static unsigned char _usbInputBufferConsumerIndex;
	static unsigned char _usbInputBufferUsed;
	static void _initUsbInputBuffer()
	{
		_usbInputBufferConsumerIndex = 0;
		_usbInputBufferUsed = 0;
	}
	
#endif

//receive a character from host.
// return true if a character is received.
static bool _getChar(unsigned char * p)
{
#if DATA_EXCHANGE_THROUGH_USB
	if(_usbInputBufferConsumerIndex == _usbInputBufferUsed) 
	{
		//read data from USB device
		iram_size_t available = udi_cdc_get_nb_received_data();
		iram_size_t remaining;
		
		if(available != 0) 
		{
			if(available > USB_INPUT_BUFFER_SIZE) {
				available = USB_INPUT_BUFFER_SIZE;
			}	
			remaining = udi_cdc_read_buf(_usbInputBuffer, available);
			_usbInputBufferConsumerIndex = 0;
			_usbInputBufferUsed = available - remaining;
		}
	}
	if(_usbInputBufferConsumerIndex == _usbInputBufferUsed) {
		return false; //no data to read
	}
	
	*p = _usbInputBuffer[_usbInputBufferConsumerIndex];
	_usbInputBufferConsumerIndex++;
	return true;
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

//send a character to host
static bool _putChar(unsigned char c)
{
#if DATA_EXCHANGE_THROUGH_USB
	if(udi_cdc_is_tx_ready()) {
		udi_cdc_putc(c);
		return true;		
	}	
	else {
		return false;
	}
#else
	char rc = ecd300PutChar(ECD300_UART_2, c);
	if(rc == 0) {
		return true;
	}
	else {
		return false;
	}
#endif
}

//send a buffer to host
// return amount of bytes sent out to host
static unsigned char _putChars(unsigned char * pBuffer, unsigned char size)
{
#if DATA_EXCHANGE_THROUGH_USB
	return _putCharsUsb(pBuffer, size);
#else
	return _putCharsUart(pBuffer, size);
#endif
}

static bool _calculateCrc16(unsigned char * pData, unsigned char length, unsigned char * pCrcLow, unsigned char * pCrcHigh)
{
	uint32_t crc;
	
	if((pData == NULL) || (pCrcLow == NULL) || (pCrcHigh == NULL)) {
		printString("ERROR: NULL in _calculateCrc16\r\n");
		return false;
	}
	
	crc_set_initial_value(0xffffffff);
	crc = crc_io_checksum(pData, length, CRC_16BIT);
	
	*pCrcLow = crc&0xff;
	*pCrcHigh = (crc>>8)&0xff;
	
	return true;
}

static inline void _initOutputStageAckPacket(unsigned char packetId)
{
	unsigned char crcLow, crcHigh;
	unsigned char * pBuffer = _scsOutputStage.ackPktBuffer;
	
	pBuffer[0] = SCS_ACK_PACKET_TAG;
	pBuffer[1] = packetId;
	_calculateCrc16(pBuffer, 2, &crcLow, &crcHigh);
	pBuffer[2] = crcLow;
	pBuffer[3] = crcHigh;
}

// acknowledge data packet received in input stage
static inline void _ackInputStageDataPacket(unsigned char packetId) 
{
	switch(_scsOutputStage.state)
	{
		case SCS_OUTPUT_IDLE:
		{
			_initOutputStageAckPacket(packetId);
			_scsOutputStage.ackPktSendingIndex = 0;
			_scsOutputStage.state = SCS_OUTPUT_SENDING_ACK;
		}
		break;
		case SCS_OUTPUT_SENDING_DATA:
		{
			_initOutputStageAckPacket(packetId);
			_scsOutputStage.ackPktSendingIndex = 0;
			_scsOutputStage.state = SCS_OUTPUT_SENDING_DATA_PENDING_ACK;
		}
		break;
		case SCS_OUTPUT_WAIT_ACK:
		{
			_initOutputStageAckPacket(packetId);
			_scsOutputStage.ackPktSendingIndex = 0;
			_scsOutputStage.state = SCS_OUTPUT_SENDING_ACK_WAIT_ACK;
		}
		break;
		default:
		{
			//cannot acknowledge Data packet.
		}
		break;
	}
}

// be called when a ACK packet is received in input stage
static inline void _on_inputStageAckPacketComplete(unsigned char packetId)
{
	_scsOutputStage.ackedDataPktId = packetId;
}

// be called when a Data packet is received in input stage
static inline void _on_inputStageDataPacketComplete()
{
	unsigned char packetId = _scsInputStage.packetBuffer[1];
	
	if(packetId == SCS_INVALID_PACKET_ID) {
		//illegal packet Id.
		printString("ERROR: invalid DATA packet Id\r\n");
		return;
	}
	if(packetId == _scsInputStage.prevDataPktId) {
		//this packet has been received successfully, discard content of this packet.
		_ackInputStageDataPacket(packetId);
		return;
	}
	
	//check continuation of packetId
	if(_scsInputStage.prevDataPktId == SCS_INVALID_PACKET_ID) {
		//this device just starts, any valid packet id is OK
		// do nothing 
	}
	else 
	{
		if(packetId == 0) {
			//this is the first packet from host
			//do nothing
		}
		else 
		{
			unsigned char expectedPacketId = _scsInputStage.prevDataPktId + 1;
				
			if(expectedPacketId == SCS_INVALID_PACKET_ID) {
				expectedPacketId = 1;
			}
			if(packetId != expectedPacketId) {
				printString("ERROR: unexpected host packetId "); 
				printHex(packetId);
				printString(" expect: ");
				printHex(expectedPacketId);
				printString("\r\n");
				return; //ignore this packet.
			}
		}
	}
	
	unsigned char dataLength = _scsInputStage.packetBuffer[2];
	if(dataLength > _getAppInputBufferAvailable()) {
		//not enough capacity in APP's inputBuffer
		//do not send ACK so that this packet is re-sent by host later
		printString("ERROR: not enough APP input buffer\r\n");
		return;
	}

	_writeAppInputBuffer(_scsInputStage.packetBuffer + 3, dataLength);
	_scsInputStage.prevDataPktId = packetId;
	
	_ackInputStageDataPacket(packetId);
}

// check if a packet from host is complete
static void _processScsInputStage(void)
{
	unsigned char c;
	
	if(_scsInputStage.state == SCS_INPUT_IDLE) 
	{
		if(_getChar(&c)) 
		{
			if((c == SCS_DATA_PACKET_TAG) || (c == SCS_ACK_PACKET_TAG))
			{
				_scsInputStage.state = SCS_INPUT_RECEIVING;
				_scsInputStage.timeStamp = counter_get();
				_scsInputStage.packetBuffer[0] = c;
				_scsInputStage.byteAmount = 1;
			}
			else
			{
				//do nothing, ignore this character
			}
		}
	}
	else if(_scsInputStage.state == SCS_INPUT_RECEIVING) 
	{
		if(_getChar(&c)) 
		{
			unsigned char pktType = _scsInputStage.packetBuffer[0];
			
			_scsInputStage.packetBuffer[_scsInputStage.byteAmount] = c;
			_scsInputStage.byteAmount++;
			
			if(pktType == SCS_DATA_PACKET_TAG)
			{
				unsigned char byteAmount = _scsInputStage.byteAmount;
				unsigned char * pBuffer = _scsInputStage.packetBuffer;
				
				if(byteAmount > 3)
				{
					unsigned char dataLength = pBuffer[2];
					
					if(dataLength > SCS_DATA_MAX_LENGTH) 
					{
						printString("ERROR: corrupted input data packet\r\n");
						_scsInputStage.state = SCS_INPUT_IDLE;
					}
					else if(byteAmount == (dataLength + SCS_DATA_PACKET_STAFF_LENGTH))
					{
						// a complete data packet is received
						unsigned char crcLow, crcHigh;
						
						_calculateCrc16(pBuffer, byteAmount - 2, &crcLow, &crcHigh);
						if((crcLow == pBuffer[byteAmount - 2]) && (crcHigh == pBuffer[byteAmount - 1]))
						{
							printString("> D "); printHex(pBuffer[1]); printString("\r\n");
							_on_inputStageDataPacketComplete();							
						}
						else {
							printString("ERROR: corrupted input data packet\r\n");
						}
						_scsInputStage.state = SCS_INPUT_IDLE;
					}
					else if(byteAmount > SCS_PACKET_MAX_LENGTH)
					{
						//shouldn't occur
						printString("ERROR: data packet overflow: "); printHex(byteAmount); printString("\r\n");
						_scsInputStage.state = SCS_INPUT_IDLE;
					}
					else {
						//do nothing, continue to receive byte.
					}
				}
				else {
					//do nothing, continue to receive byte
				}
			}
			else if(pktType == SCS_ACK_PACKET_TAG)
			{
				if(_scsInputStage.byteAmount == SCS_ACK_PACKET_LENGTH) 
				{
					//a complete ACK packet is received
					unsigned char crcLow, crcHigh;
					
					_calculateCrc16(_scsInputStage.packetBuffer, 2, &crcLow, &crcHigh);
					
					if((crcLow == _scsInputStage.packetBuffer[2]) && (crcHigh == _scsInputStage.packetBuffer[3]))
					{
						unsigned char packetId = _scsInputStage.packetBuffer[1];
						_on_inputStageAckPacketComplete(packetId);
						printString("> A "); printHex(packetId); printString("\r\n");
					}
					else
					{
						printString("ERROR: corrupted input ACK packet\r\n");
					}
					_scsInputStage.state = SCS_INPUT_IDLE; //change to IDLE state.
				}
				else if(_scsInputStage.byteAmount > SCS_ACK_PACKET_LENGTH) 
				{
					// shouldn't occur
					printString("ERROR: wrong input ACK packet length ");
					printHex(_scsInputStage.byteAmount);
					printString("\r\n");
					_scsInputStage.state = SCS_INPUT_IDLE; //change to IDLE state.
				}
				else {
					//do nothing, wait until complete packet is received.
				}
			}
			else
			{
				//shouldn't occur
				printString("ERROR: corrupted input stage\r\n");
				_scsInputStage.state = SCS_INPUT_IDLE;
			}
		}
		else
		{
			//check timeout
			if(counter_diff(_scsInputStage.timeStamp) > _scsInputTimeOut) 
			{
				_scsInputStage.state = SCS_INPUT_IDLE; //change to IDLE state.
				printString("ERROR: input stage timed out");
			}
		}
	}
	else 
	{
		//shouldn't occur
		printString("ERROR: wrong input stage state ");
		printHex(_scsInputStage.state);
		printString("\r\n");
		
		_scsInputStage.state = SCS_INPUT_IDLE;
	}
}

//handle SCS_OUTPUT_IDLE.
// read data to host from APP's outputBuffer.
static void _processScsOutputStageIdle()
{
	unsigned char * pPacket = _scsOutputStage.dataPktBuffer;
	unsigned short size = _readOutputBuffer(pPacket + 3, SCS_DATA_MAX_LENGTH);
	unsigned char crcLow, crcHigh;
	
	if(size == 0) {
		return; //no data need to be sent to host
	}
	if(size > SCS_DATA_MAX_LENGTH) {
		//shouldn't occur
		printString("ERROR: too much data read from APP's output buffer\r\n");
		return;
	}
	
	//fill the packet staff
	pPacket[0] = SCS_DATA_PACKET_TAG; //tag
	//packet id
	if(_scsOutputStage.currentDataPktId == SCS_INVALID_PACKET_ID) {
		_scsOutputStage.currentDataPktId = 0;
		pPacket[1] = 0;
	}
	else {
		_scsOutputStage.currentDataPktId++;
		if(_scsOutputStage.currentDataPktId == SCS_INVALID_PACKET_ID) {
			_scsOutputStage.currentDataPktId = 1; //turn around
		}
		pPacket[1] = _scsOutputStage.currentDataPktId;
	}
	//length
	pPacket[2] = (unsigned char)size;
	//CRC
	_calculateCrc16(pPacket, size + 3, &crcLow, &crcHigh);
	pPacket[3+size] = crcLow;
	pPacket[4+size] = crcHigh;
	
	_scsOutputStage.dataPktSendingIndex = 0;
	_scsOutputStage.ackedDataPktId = SCS_INVALID_PACKET_ID;
	_scsOutputStage.dataPktTimeStamp = counter_get();
	_scsOutputStage.state = SCS_OUTPUT_SENDING_DATA;	
}

//send out data in output stage as much as possible
static void _processScsOutputStage(void)
{
	switch(_scsOutputStage.state)
	{
		case SCS_OUTPUT_IDLE:
		{
			_processScsOutputStageIdle();
		}
		break;
		case SCS_OUTPUT_SENDING_DATA:
		{
			unsigned char packetLength = _scsOutputStage.dataPktBuffer[2] + SCS_DATA_PACKET_STAFF_LENGTH;
			unsigned char * pStart = _scsOutputStage.dataPktBuffer + _scsOutputStage.dataPktSendingIndex;
			unsigned char remaining = packetLength - _scsOutputStage.dataPktSendingIndex;
			unsigned char size = _putChars(pStart, remaining);
			
			_scsOutputStage.dataPktSendingIndex += size;
			if(size < remaining) {
				//do nothing
			}
			else if(size == remaining) {
				//data packet is sent out
				_scsOutputStage.state = SCS_OUTPUT_WAIT_ACK; 
				printString("< D "); printHex(_scsOutputStage.dataPktBuffer[1]); printString("\r\n");
			}
			else {
				//shouldn't happen
				printString("ERROR: too much data sent out\r\n");
				_scsOutputStage.state = SCS_OUTPUT_WAIT_ACK;
			}
		}
		break;
		case SCS_OUTPUT_SENDING_DATA_PENDING_ACK:
		{
			unsigned char packetLength = _scsOutputStage.dataPktBuffer[2] + SCS_DATA_PACKET_STAFF_LENGTH;
			unsigned char * pStart = _scsOutputStage.dataPktBuffer + _scsOutputStage.dataPktSendingIndex;
			unsigned char remaining = packetLength - _scsOutputStage.dataPktSendingIndex;
			unsigned char size = _putChars(pStart, remaining);
			
			_scsOutputStage.dataPktSendingIndex += size;
			if(size < remaining) {
				//do nothing
			}
			else if(size == remaining) {
				//data packet is sent out
				_scsOutputStage.state = SCS_OUTPUT_SENDING_ACK_WAIT_ACK; 
				printString("< D "); printHex(_scsOutputStage.dataPktBuffer[1]); printString("\r\n");
			}
			else {
				//shouldn't happen
				printString("ERROR: too much data sent out\r\n");
				_scsOutputStage.state = SCS_OUTPUT_SENDING_ACK_WAIT_ACK;
			}
		}
		break;
		case SCS_OUTPUT_WAIT_ACK:
		{
			unsigned char packetId = _scsOutputStage.dataPktBuffer[1];
			
			if(packetId == _scsOutputStage.ackedDataPktId) {
				_scsOutputStage.state = SCS_OUTPUT_IDLE;
			}
			else if(counter_diff(_scsOutputStage.dataPktTimeStamp) > _scsOutputTimeout)
			{
				//time out, send data packet again
				_scsOutputStage.dataPktSendingIndex = 0;
				_scsOutputStage.dataPktTimeStamp = counter_get();
				_scsOutputStage.state = SCS_OUTPUT_SENDING_DATA;
				printString("ERROR: host ACK time out, "); printHex(packetId); printString("\r\n");
			}
		}
		break;
		case SCS_OUTPUT_SENDING_ACK_WAIT_ACK:
		{
			unsigned char * pStart = _scsOutputStage.ackPktBuffer + _scsOutputStage.ackPktSendingIndex;
			unsigned char remaining = SCS_ACK_PACKET_LENGTH - _scsOutputStage.dataPktSendingIndex;
			unsigned char size = _putChars(pStart, remaining);
			
			_scsOutputStage.ackPktSendingIndex += size;
			if(size < remaining) {
				//do nothing
			}
			else if(size == remaining) {
				//ACK packet is sent out
				_scsOutputStage.state = SCS_OUTPUT_WAIT_ACK;
				printString("< A "); printHex(_scsOutputStage.ackPktBuffer[1]); printString("\r\n");
			}
			else {
				//shouldn't happen
				printString("ERROR: too much ACK sent out\r\n");
				_scsOutputStage.state = SCS_OUTPUT_WAIT_ACK;
			}
		}
		break;
		case SCS_OUTPUT_SENDING_ACK:
		{
			unsigned char * pStart = _scsOutputStage.ackPktBuffer + _scsOutputStage.ackPktSendingIndex;
			unsigned char remaining = SCS_ACK_PACKET_LENGTH - _scsOutputStage.dataPktSendingIndex;
			unsigned char size = _putChars(pStart, remaining);
			
			_scsOutputStage.ackPktSendingIndex += size;
			if(size < remaining) {
				//do nothing
			}
			else if(size == remaining) {
				//ACK packet is sent out
				_scsOutputStage.state = SCS_OUTPUT_IDLE;
				printString("< A "); printHex(_scsOutputStage.ackPktBuffer[1]); printString("\r\n");
			}
			else {
				//shouldn't happen
				printString("ERROR: too much ACK sent out\r\n");
				_scsOutputStage.state = SCS_OUTPUT_IDLE;
			}
		}
		break;
		default:
		{
			printString("ERROR: wrong output stage state: "); printHex(_scsOutputStage.state);printString("\r\n");
			_scsOutputStage.state = SCS_OUTPUT_IDLE;
		}
		break;
	}
}

void pollScsDataExchange(void)
{
	_processScsInputStage();
	_processScsOutputStage();
	_processMonitorStage();
}

void initScsDataExchange(void)
{
#if DATA_EXCHANGE_THROUGH_USB
	_initUsbInputBuffer();
#endif

	//input stage
	_scsInputTimeOut = ((uint32_t)SCS_DATA_INPUT_TIMEOUT * 1000)/counter_clock_length();
	_scsInputStage.state = SCS_INPUT_IDLE;
	_scsInputStage.prevDataPktId = SCS_INVALID_PACKET_ID;
	
	//output stage
	_scsOutputTimeout = ((uint32_t)SCS_DATA_OUTPUT_TIMEOUT * 1000)/counter_clock_length();
	_scsOutputStage.state = SCS_OUTPUT_IDLE;
	_scsOutputStage.currentDataPktId = SCS_INVALID_PACKET_ID;
	_scsOutputStage.ackedDataPktId = SCS_INVALID_PACKET_ID;
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

	//UART
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
