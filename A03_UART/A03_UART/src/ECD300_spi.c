#include "ECD300_spi.h"
#include "spi_master.h"

#ifdef ECD300_SPIC_ENABLED_INT

/************************************************************************
 SPI master or SPI slave are supported by this driver.
 SPIC has a receiving buffer with a default size of 256 bytes. 
 This buffer facilitates data receiving in SPI slave, and is not used in SPI master.
 
 If SPIC works as master, PIN4 is SS by default.
 If SPIC works as slave, the falling edge on PIN2 indicates that master can write one byte to slave;
 with the falling edge on PIN2, low level on PIN3 indicates that slave has a byte to be read by master, 
 high level on PIN3 indicates that slave has no data to be read. The falling edge is sent out in ISR or
 when data is read from the receiving buffer.
************************************************************************/

static bool _spicInitialized = false;
static unsigned char _spicReceiveBuffer[256];
static unsigned char _spicReceiveBufferHead;
static unsigned char _spicReceiveBufferTail;
static unsigned char _spicSendBufferLength;
static unsigned char _spicCurrentSendIndex;
static unsigned char * _pSpicSendBuffer;
static bool _spicSendFinished = true;
static bool _spicReceiveBufferFull = false;
static bool _spicReceiveBufferOverflow = false;

/************************************************************************
Initialize SPI module in PORTC, including PINs.
If master, PIN 4, 5, 6 and 7 are configured; if slave, PIN 2, 3, 4, 5, 6 and 7 are configured.

Parameters:
	is_master:	SPI module is initialized as master if true; as slave if false
	frequency:	frequency of SCK if SPI master
	priority:	priority of SPIC interrupt, must be 1, 2, or 3 with 3 the highest priority
	mode:		0, 1, 2, 3, refer to chapter 22.5 in XMEGA manual.
Return value:
	0:	success
	-1:	mode is not supported
	-2:	frequency is out of scope
	-3: interrupt priority is out of range
************************************************************************/
char spi_c_init(bool is_master, unsigned long frequency, unsigned char priority, unsigned char mode)
{
	_spicReceiveBufferHead = 0;
	_spicReceiveBufferTail = 0;
	_spicSendBufferLength = 0;
	_spicCurrentSendIndex = 0;
	
	switch(mode)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			break;
		default:
			return -1;
	}
	switch(priority)
	{
		case 1:
		case 2:
		case 3:
			break;
		default:
			return -3;
	}

	if(is_master)
	{
		//SS
		ioport_configure_port_pin(&PORTC, PIN4_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//MOSI
		ioport_configure_port_pin(&PORTC, PIN5_bm, IOPORT_DIR_OUTPUT);
		//MISO
		ioport_configure_port_pin(&PORTC, PIN6_bm, IOPORT_DIR_INPUT);
		//CLK
		ioport_configure_port_pin(&PORTC, PIN7_bm, IOPORT_DIR_OUTPUT);
	}
	else
	{
		//SLAVE READY
		ioport_configure_port_pin(&PORTC, PIN2_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//SLAVE DATA AVAILABLE
		ioport_configure_port_pin(&PORTC, PIN3_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//SS
		ioport_configure_port_pin(&PORTC, PIN4_bm, IOPORT_PULL_UP	| IOPORT_DIR_INPUT);
		//MOSI
		ioport_configure_port_pin(&PORTC, PIN5_bm, IOPORT_DIR_INPUT);
		//MISO
		ioport_configure_port_pin(&PORTC, PIN6_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//CLK
		ioport_configure_port_pin(&PORTC, PIN7_bm, IOPORT_DIR_INPUT);
	}

	sysclk_enable_module(SYSCLK_PORT_C, PR_SPI_bm);

	if(is_master){
		spi_enable_master_mode(&SPIC);
		if (spi_xmega_set_baud_div(&SPIC, frequency, sysclk_get_cpu_hz()) < 0) {
			return -2;
		}
	}
	
	/* Clear any set SPI mode flags and set them to the user-specified mode */
	SPIC_CTRL = (SPIC_CTRL & ~SPI_MODE_gm) | ((mode << SPI_MODE_gp) & SPI_MODE_gm);
	
	SPIC_INTCTRL = priority;
	
	spi_enable(&SPIC);
		
	_spicReceiveBufferFull = false;
	_spicReceiveBufferOverflow = false;
	_spicSendFinished = true;
	_spicInitialized = true;
	
	return 0;
}

static inline void _spi_c_slave_data_available(bool bDataReady)
{
	if(bDataReady)
		PORTC_OUTCLR = PIN3_bm;
	else
		PORTC_OUTSET = PIN3_bm;
}

static inline void _spi_c_slave_singal_ready()
{
	if((_spicReceiveBufferTail + 1) != _spicReceiveBufferHead)
	{
		//create a falling edge.
		PORTC_OUTSET = PIN2_bm;
		PORTC_OUTCLR = PIN2_bm;
	}
}

/************************************************************************
Send a block of bytes out of SPI module.
If SPI module works as master, data read back from MISO is saved to address pointed by pBuffer.
If slave, data read from MOSI is saved to receiving buffer, more data can be received from MOSI
if there are free space in the receiving buffer.

Parameters:
	pBuffer:	address of bytes to be sent out.
	length:		amount of data in byte.
Return value:
	0: data exchange starts
	-1: value of pBuffer is not qualified
	-2:	previous block of bytes hasn't been sent out completely
************************************************************************/
char spi_c_send_data(unsigned char * pBuffer, unsigned char length)
{
	if(NULL == pBuffer)
	{
		return -1;	
	}
	if(!_spicSendFinished)
	{
		return -2;
	}
	if(0 != length)
	{
		_pSpicSendBuffer = pBuffer;
		_spicSendBufferLength = length;
		_spicCurrentSendIndex = 0;
		_spicSendFinished = false;
		
		if(SPIC_CTRL & SPI_MASTER_bm)
		{
			//master, assert default SS pin.
			PORTC_OUTCLR = PIN4_bm;
			SPIC_DATA = pBuffer[0];
		}
		else
		{
			SPIC_DATA = pBuffer[0];
			_spi_c_slave_data_available(true);
			_spi_c_slave_singal_ready();
		}
	}
	
	return 0;
}


/************************************************************************
Check status of data sending.

Return value:
	true:	data specified in spi_c_send_data( ) is sent out.
	false:	data specified in spi_c_send_data( ) hasn't been sent out.
************************************************************************/
bool spi_c_send_finished( )
{
	return _spicSendFinished;
}

/************************************************************************
Get received data from the receiving buffer when SPI module works as slave.

Parameter:
	pBuffer:	buffer address where the received data should be written to
	maxLength:	length of buffer pointed by pBuffer
Return value:
	amount of bytes actually written to the buffer.
************************************************************************/
unsigned char spi_c_slave_receive_data(unsigned char * pBuffer, unsigned char maxLength)
{
	unsigned char counter;
	unsigned char head, tail;
	
	if(maxLength == 0)
	{
		return 0;
	}
	if(pBuffer == NULL)
	{
		return 0;
	}
	
	for(counter = 0; 
		(counter < maxLength) && (_spicReceiveBufferHead != _spicReceiveBufferTail); 
		counter++, _spicReceiveBufferHead++)
	{
		pBuffer[counter] = _spicReceiveBuffer[_spicReceiveBufferHead];		
	}
	
	if(_spicReceiveBufferFull)
	{
		//ISR has stopped signaling readiness to master.
		if(counter > 0)
		{
			_spicReceiveBufferFull = false;
			if(!_spicSendFinished)
			{
				SPIC_DATA = _pSpicSendBuffer[_spicCurrentSendIndex];
				_spi_c_slave_data_available(true);
			}
			else
			{
				SPIC_DATA = 0x00;
				_spi_c_slave_data_available(false);
			}
			_spi_c_slave_singal_ready();
		}
	}
	else 
	{
		//there is free space before this reading.
		if(_spicSendFinished)
		{
			//notify master of free space in slave
			SPIC_DATA = 0x00;
			_spi_c_slave_data_available(false);
			_spi_c_slave_singal_ready();	
		}
		else
		{
			//current block of bytes should be being sent, so do nothing.
			;
		}
	}
	
	return counter;
}


/************************************************************************
Get status about overflow in slave receiving buffer.

Return value:
	true:	overflow happened in receiving buffer
	false:	overflow didn't happen in receiving buffer
************************************************************************/
bool is_spi_c_slave_receive_buffer_overflow()
{
	return _spicReceiveBufferOverflow;
}

/************************************************************************
Clear the overflow flag.
This flag can be cleared if there is free space in slave receiving buffer.

Return value:
	true:	overflow flag is cleared
	false:	overflow flag cannot be cleared
************************************************************************/
bool spi_c_slave_clear_receive_buffer_overflow()
{
	if(_spicReceiveBufferOverflow)
	{
		if(!_spicReceiveBufferFull)
		{
			//free space available
			_spicReceiveBufferOverflow = false;
		}
		else
		{
			//no free space in receiving buffer, cannot clear overflow flag.
			return false;
		}
	}
	
	return true;
}

ISR(SPIC_INT_vect)
{
	if(SPIC_CTRL & SPI_MASTER_bm)
	{
		//master
		_pSpicSendBuffer[_spicCurrentSendIndex] = SPIC_DATA;
		_spicCurrentSendIndex++;
		if(!_spicSendFinished)
		{
			if(_spicCurrentSendIndex < _spicSendBufferLength)
			{
				SPIC_DATA = _pSpicSendBuffer[_spicCurrentSendIndex];
			}
			else
			{
				PORTC_OUTSET = PIN4_bm;
				_spicSendFinished = true;
			}
		}
	}
	else
	{
		//slave
		if(_spicReceiveBufferFull)
		{
			//overflow
			_spicReceiveBufferOverflow = true;
		}
		else
		{
			_spicReceiveBuffer[_spicReceiveBufferTail] = SPIC_DATA;
			if((_spicReceiveBufferTail + 1) == _spicReceiveBufferHead)
			{
				_spicReceiveBufferFull = true;
			}
			else
			{
				//further data can be received.
				_spicReceiveBufferTail++;
				if(!_spicSendFinished)
				{
					_spicCurrentSendIndex++;
					if(_spicCurrentSendIndex < _spicSendBufferLength)
					{
						SPIC_DATA = _pSpicSendBuffer[_spicCurrentSendIndex];
						_spi_c_slave_data_available(true);
					}
					else
					{
						_spicSendFinished = true;
						SPIC_DATA = 0x00;
						_spi_c_slave_data_available(false);
					}
				}
				else
				{
					SPIC_DATA = 0x00;
					_spi_c_slave_data_available(false);
				}
				_spi_c_slave_singal_ready();
			}
		}
	}
}

#endif

#ifdef ECD300_SPID_ENABLED_INT

/************************************************************************
 SPI master or SPI slave are supported by this driver.
 SPID has a receiving buffer with a default size of 256 bytes. 
 This buffer facilitates data receiving in SPI slave, and is not used in SPI master.
 
 If SPID works as master, PIN4 is SS by default.
 If SPID works as slave, the falling edge on PIN2 indicates that master can write one byte to slave;
 with the falling edge on PIN2, low level on PIN3 indicates that slave has a byte to be read by master, 
 high level on PIN3 indicates that slave has no data to be read. The falling edge is sent out in ISR or
 when data is read from the receiving buffer.
************************************************************************/

static bool _spidInitialized = false;
static unsigned char _spidReceiveBuffer[256];
static unsigned char _spidReceiveBufferHead;
static unsigned char _spidReceiveBufferTail;
static unsigned char _spidSendBufferLength;
static unsigned char _spidCurrentSendIndex;
static unsigned char * _pSpidSendBuffer;
static bool _spidSendFinished = true;
static bool _spidReceiveBufferFull = false;
static bool _spidReceiveBufferOverflow = false;

/************************************************************************
Initialize SPI module in PORTD, including PINs.
If master, PIN 4, 5, 6 and 7 are configured; if slave, PIN 2, 3, 4, 5, 6 and 7 are configured.

Parameters:
	is_master:	SPI module is initialized as master if true; as slave if false
	frequency:	frequency of SCK if SPI master
	priority:	priority of SPID interrupt, must be 1, 2, or 3 with 3 the highest priority
	mode:		0, 1, 2, 3, refer to chapter 22.5 in XMEGA manual.
Return value:
	0:	success
	-1:	mode is not supported
	-2:	frequency is out of scope
	-3: interrupt priority is out of range
************************************************************************/
char spi_d_init(bool is_master, unsigned long frequency, unsigned char priority, unsigned char mode)
{
	_spidReceiveBufferHead = 0;
	_spidReceiveBufferTail = 0;
	_spidSendBufferLength = 0;
	_spidCurrentSendIndex = 0;
	
	switch(mode)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			break;
		default:
			return -1;
	}
	switch(priority)
	{
		case 1:
		case 2:
		case 3:
			break;
		default:
			return -3;
	}
	
	if(is_master)
	{
		//SS
		ioport_configure_port_pin(&PORTD, PIN4_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//MOSI
		ioport_configure_port_pin(&PORTD, PIN5_bm, IOPORT_DIR_OUTPUT);
		//MISO
		ioport_configure_port_pin(&PORTD, PIN6_bm, IOPORT_DIR_INPUT);
		//CLK
		ioport_configure_port_pin(&PORTD, PIN7_bm, IOPORT_DIR_OUTPUT);
	}
	else
	{
		//SLAVE READY
		ioport_configure_port_pin(&PORTD, PIN2_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//SLAVE DATA AVAILABLE
		ioport_configure_port_pin(&PORTD, PIN3_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//SS
		ioport_configure_port_pin(&PORTD, PIN4_bm, IOPORT_PULL_UP	| IOPORT_DIR_INPUT);
		//MOSI
		ioport_configure_port_pin(&PORTD, PIN5_bm, IOPORT_DIR_INPUT);
		//MISO
		ioport_configure_port_pin(&PORTD, PIN6_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//CLK
		ioport_configure_port_pin(&PORTD, PIN7_bm, IOPORT_DIR_INPUT);
	}

	sysclk_enable_module(SYSCLK_PORT_D, PR_SPI_bm);

	if(is_master){
		spi_enable_master_mode(&SPID);
		if (spi_xmega_set_baud_div(&SPID, frequency, sysclk_get_cpu_hz()) < 0) {
			return -2;
		}
	}
	
	/* Clear any set SPI mode flags and set them to the user-specified mode */
	SPID_CTRL = (SPID_CTRL & ~SPI_MODE_gm) | ((mode << SPI_MODE_gp) & SPI_MODE_gm);
	
	SPID_INTCTRL = priority;
	
	spi_enable(&SPID);
		
	_spidReceiveBufferFull = false;
	_spidReceiveBufferOverflow = false;
	_spidSendFinished = true;
	_spidInitialized = true;
	
	return 0;
}

static inline void _spi_d_slave_data_available(bool bDataReady)
{
	if(bDataReady)
		PORTD_OUTCLR = PIN3_bm;
	else
		PORTD_OUTSET = PIN3_bm;
}

static inline void _spi_d_slave_singal_ready()
{
	if((_spidReceiveBufferTail + 1) != _spidReceiveBufferHead)
	{
		//create a falling edge.
		PORTD_OUTSET = PIN2_bm;
		PORTD_OUTCLR = PIN2_bm;
	}
}

/************************************************************************
Send a block of bytes out of SPI module.
If SPI module works as master, data read back from MISO is saved to address pointed by pBuffer.
If slave, data read from MOSI is saved to receiving buffer, more data can be received from MOSI
if there are free space in the receiving buffer.

Parameters:
	pBuffer:	address of bytes to be sent out.
	length:		amount of data in byte.
Return value:
	0: data exchange starts
	-1: value of pBuffer is not qualified
	-2:	previous block of bytes hasn't been sent out completely
************************************************************************/
char spi_d_send_data(unsigned char * pBuffer, unsigned char length)
{
	if(NULL == pBuffer)
	{
		return -1;	
	}
	if(!_spidSendFinished)
	{
		return -2;
	}
	if(0 != length)
	{
		_pSpidSendBuffer = pBuffer;
		_spidSendBufferLength = length;
		_spidCurrentSendIndex = 0;
		_spidSendFinished = false;
		
		if(SPID_CTRL & SPI_MASTER_bm)
		{
			//master, assert default SS pin.
			PORTD_OUTCLR = PIN4_bm;
			SPID_DATA = pBuffer[0];
		}
		else
		{
			SPID_DATA = pBuffer[0];
			_spi_d_slave_data_available(true);
			_spi_d_slave_singal_ready();
		}
	}
	
	return 0;
}


/************************************************************************
Check status of data sending.

Return value:
	true:	data specified in spi_d_send_data( ) is sent out.
	false:	data specified in spi_d_send_data( ) hasn't been sent out.
************************************************************************/
bool spi_d_send_finished( )
{
	return _spidSendFinished;
}

/************************************************************************
Get received data from the receiving buffer when SPI module works as slave.

Parameter:
	pBuffer:	buffer address where the received data should be written to
	maxLength:	length of buffer pointed by pBuffer
Return value:
	amount of bytes actually written to the buffer.
************************************************************************/
unsigned char spi_d_slave_receive_data(unsigned char * pBuffer, unsigned char maxLength)
{
	unsigned char counter;
	unsigned char head, tail;
	
	if(maxLength == 0)
	{
		return 0;
	}
	if(pBuffer == NULL)
	{
		return 0;
	}
	
	for(counter = 0; 
		(counter < maxLength) && (_spidReceiveBufferHead != _spidReceiveBufferTail); 
		counter++, _spidReceiveBufferHead++)
	{
		pBuffer[counter] = _spidReceiveBuffer[_spidReceiveBufferHead];		
	}
	
	if(_spidReceiveBufferFull)
	{
		//ISR has stopped signaling readiness to master.
		if(counter > 0)
		{
			_spidReceiveBufferFull = false;
			if(!_spidSendFinished)
			{
				SPID_DATA = _pSpidSendBuffer[_spidCurrentSendIndex];
				_spi_d_slave_data_available(true);
			}
			else
			{
				SPID_DATA = 0x00;
				_spi_d_slave_data_available(false);
			}
			_spi_d_slave_singal_ready();
		}
	}
	else 
	{
		//there is free space before this reading.
		if(_spidSendFinished)
		{
			//notify master of free space in slave
			SPID_DATA = 0x00;
			_spi_d_slave_data_available(false);
			_spi_d_slave_singal_ready();	
		}
		else
		{
			//current block of bytes should be being sent, so do nothing.
			;
		}
	}
	
	return counter;
}


/************************************************************************
Get status about overflow in slave receiving buffer.

Return value:
	true:	overflow happened in receiving buffer
	false:	overflow didn't happen in receiving buffer
************************************************************************/
bool is_spi_d_slave_receive_buffer_overflow()
{
	return _spidReceiveBufferOverflow;
}

/************************************************************************
Clear the overflow flag.
This flag can be cleared if there is free space in slave receiving buffer.

Return value:
	true:	overflow flag is cleared
	false:	overflow flag cannot be cleared
************************************************************************/
bool spi_d_slave_clear_receive_buffer_overflow()
{
	if(_spidReceiveBufferOverflow)
	{
		if(!_spidReceiveBufferFull)
		{
			//free space available
			_spidReceiveBufferOverflow = false;
		}
		else
		{
			//no free space in receiving buffer, cannot clear overflow flag.
			return false;
		}
	}
	
	return true;
}

ISR(SPID_INT_vect)
{
	if(SPID_CTRL & SPI_MASTER_bm)
	{
		//master
		_pSpidSendBuffer[_spidCurrentSendIndex] = SPID_DATA;
		_spidCurrentSendIndex++;
		if(!_spidSendFinished)
		{
			if(_spidCurrentSendIndex < _spidSendBufferLength)
			{
				SPID_DATA = _pSpidSendBuffer[_spidCurrentSendIndex];
			}
			else
			{
				PORTD_OUTSET = PIN4_bm;
				_spidSendFinished = true;
			}
		}
	}
	else
	{
		//slave
		if(_spidReceiveBufferFull)
		{
			//overflow
			_spidReceiveBufferOverflow = true;
		}
		else
		{
			_spidReceiveBuffer[_spidReceiveBufferTail] = SPID_DATA;
			if((_spidReceiveBufferTail + 1) == _spidReceiveBufferHead)
			{
				_spidReceiveBufferFull = true;
			}
			else
			{
				//further data can be received.
				_spidReceiveBufferTail++;
				if(!_spidSendFinished)
				{
					_spidCurrentSendIndex++;
					if(_spidCurrentSendIndex < _spidSendBufferLength)
					{
						SPID_DATA = _pSpidSendBuffer[_spidCurrentSendIndex];
						_spi_d_slave_data_available(true);
					}
					else
					{
						_spidSendFinished = true;
						SPID_DATA = 0x00;
						_spi_d_slave_data_available(false);
					}
				}
				else
				{
					SPID_DATA = 0x00;
					_spi_d_slave_data_available(false);
				}
				_spi_d_slave_singal_ready();
			}
		}
	}
}

#endif

#ifdef ECD300_SPIE_ENABLED_INT

/************************************************************************
 SPI master or SPI slave are supported by this driver.
 SPIE has a receiving buffer with a default size of 256 bytes. 
 This buffer facilitates data receiving in SPI slave, and is not used in SPI master.
 
 If SPIE works as master, PIN4 is SS by default.
 If SPIE works as slave, the falling edge on PIN2 indicates that master can write one byte to slave;
 with the falling edge on PIN2, low level on PIN3 indicates that slave has a byte to be read by master, 
 high level on PIN3 indicates that slave has no data to be read. The falling edge is sent out in ISR or
 when data is read from the receiving buffer.
************************************************************************/

static bool _spieInitialized = false;
static unsigned char _spieReceiveBuffer[256];
static unsigned char _spieReceiveBufferHead;
static unsigned char _spieReceiveBufferTail;
static unsigned char _spieSendBufferLength;
static unsigned char _spieCurrentSendIndex;
static unsigned char * _pSpieSendBuffer;
static bool _spieSendFinished = true;
static bool _spieReceiveBufferFull = false;
static bool _spieReceiveBufferOverflow = false;

/************************************************************************
Initialize SPI module in PORTE, including PINs.
If master, PIN 4, 5, 6 and 7 are configured; if slave, PIN 2, 3, 4, 5, 6 and 7 are configured.

Parameters:
	is_master:	SPI module is initialized as master if true; as slave if false
	frequency:	frequency of SCK if SPI master
	priority:	priority of SPIE interrupt, must be 1, 2, or 3 with 3 the highest priority
	mode:		0, 1, 2, 3, refer to chapter 22.5 in XMEGA manual.
Return value:
	0:	success
	-1:	mode is not supported
	-2:	frequency is out of scope
	-3: interrupt priority is out of range
************************************************************************/
char spi_e_init(bool is_master, unsigned long frequency, unsigned char priority, unsigned char mode)
{
	_spieReceiveBufferHead = 0;
	_spieReceiveBufferTail = 0;
	_spieSendBufferLength = 0;
	_spieCurrentSendIndex = 0;
	
	switch(mode)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			break;
		default:
			return -1;
	}
	switch(priority)
	{
		case 1:
		case 2:
		case 3:
			break;
		default:
			return -3;
	}

	if(is_master)
	{
		//SS
		ioport_configure_port_pin(&PORTE, PIN4_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//MOSI
		ioport_configure_port_pin(&PORTE, PIN5_bm, IOPORT_DIR_OUTPUT);
		//MISO
		ioport_configure_port_pin(&PORTE, PIN6_bm, IOPORT_DIR_INPUT);
		//CLK
		ioport_configure_port_pin(&PORTE, PIN7_bm, IOPORT_DIR_OUTPUT);
	}
	else
	{
		//SLAVE READY
		ioport_configure_port_pin(&PORTE, PIN2_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//SLAVE DATA AVAILABLE
		ioport_configure_port_pin(&PORTE, PIN3_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//SS
		ioport_configure_port_pin(&PORTE, PIN4_bm, IOPORT_PULL_UP	| IOPORT_DIR_INPUT);
		//MOSI
		ioport_configure_port_pin(&PORTE, PIN5_bm, IOPORT_DIR_INPUT);
		//MISO
		ioport_configure_port_pin(&PORTE, PIN6_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//CLK
		ioport_configure_port_pin(&PORTE, PIN7_bm, IOPORT_DIR_INPUT);
	}

	sysclk_enable_module(SYSCLK_PORT_E, PR_SPI_bm);

	if(is_master){
		spi_enable_master_mode(&SPIE);
		if (spi_xmega_set_baud_div(&SPIE, frequency, sysclk_get_cpu_hz()) < 0) {
			return -2;
		}
	}
	
	/* Clear any set SPI mode flags and set them to the user-specified mode */
	SPIE_CTRL = (SPIE_CTRL & ~SPI_MODE_gm) | ((mode << SPI_MODE_gp) & SPI_MODE_gm);
	
	SPIE_INTCTRL = priority;
	
	spi_enable(&SPIE);
		
	_spieReceiveBufferFull = false;
	_spieReceiveBufferOverflow = false;
	_spieSendFinished = true;
	_spieInitialized = true;
	
	return 0;
}

static inline void _spi_e_slave_data_available(bool bDataReady)
{
	if(bDataReady)
		PORTE_OUTCLR = PIN3_bm;
	else
		PORTE_OUTSET = PIN3_bm;
}

static inline void _spi_e_slave_singal_ready()
{
	if((_spieReceiveBufferTail + 1) != _spieReceiveBufferHead)
	{
		//create a falling edge.
		PORTE_OUTSET = PIN2_bm;
		PORTE_OUTCLR = PIN2_bm;
	}
}

/************************************************************************
Send a block of bytes out of SPI module.
If SPI module works as master, data read back from MISO is saved to address pointed by pBuffer.
If slave, data read from MOSI is saved to receiving buffer, more data can be received from MOSI
if there are free space in the receiving buffer.

Parameters:
	pBuffer:	address of bytes to be sent out.
	length:		amount of data in byte.
Return value:
	0: data exchange starts
	-1: value of pBuffer is not qualified
	-2:	previous block of bytes hasn't been sent out completely
************************************************************************/
char spi_e_send_data(unsigned char * pBuffer, unsigned char length)
{
	if(NULL == pBuffer)
	{
		return -1;	
	}
	if(!_spieSendFinished)
	{
		return -2;
	}
	if(0 != length)
	{
		_pSpieSendBuffer = pBuffer;
		_spieSendBufferLength = length;
		_spieCurrentSendIndex = 0;
		_spieSendFinished = false;
		
		if(SPIE_CTRL & SPI_MASTER_bm)
		{
			//master, assert default SS pin.
			PORTE_OUTCLR = PIN4_bm;
			SPIE_DATA = pBuffer[0];
		}
		else
		{
			SPIE_DATA = pBuffer[0];
			_spi_e_slave_data_available(true);
			_spi_e_slave_singal_ready();
		}
	}
	
	return 0;
}


/************************************************************************
Check status of data sending.

Return value:
	true:	data specified in spi_e_send_data( ) is sent out.
	false:	data specified in spi_e_send_data( ) hasn't been sent out.
************************************************************************/
bool spi_e_send_finished( )
{
	return _spieSendFinished;
}

/************************************************************************
Get received data from the receiving buffer when SPI module works as slave.

Parameter:
	pBuffer:	buffer address where the received data should be written to
	maxLength:	length of buffer pointed by pBuffer
Return value:
	amount of bytes actually written to the buffer.
************************************************************************/
unsigned char spi_e_slave_receive_data(unsigned char * pBuffer, unsigned char maxLength)
{
	unsigned char counter;
	unsigned char head, tail;
	
	if(maxLength == 0)
	{
		return 0;
	}
	if(pBuffer == NULL)
	{
		return 0;
	}
	
	for(counter = 0; 
		(counter < maxLength) && (_spieReceiveBufferHead != _spieReceiveBufferTail); 
		counter++, _spieReceiveBufferHead++)
	{
		pBuffer[counter] = _spieReceiveBuffer[_spieReceiveBufferHead];		
	}
	
	if(_spieReceiveBufferFull)
	{
		//ISR has stopped signaling readiness to master.
		if(counter > 0)
		{
			_spieReceiveBufferFull = false;
			if(!_spieSendFinished)
			{
				SPIE_DATA = _pSpieSendBuffer[_spieCurrentSendIndex];
				_spi_e_slave_data_available(true);
			}
			else
			{
				SPIE_DATA = 0x00;
				_spi_e_slave_data_available(false);
			}
			_spi_e_slave_singal_ready();
		}
	}
	else 
	{
		//there is free space before this reading.
		if(_spieSendFinished)
		{
			//notify master of free space in slave
			SPIE_DATA = 0x00;
			_spi_e_slave_data_available(false);
			_spi_e_slave_singal_ready();	
		}
		else
		{
			//current block of bytes should be being sent, so do nothing.
			;
		}
	}
	
	return counter;
}


/************************************************************************
Get status about overflow in slave receiving buffer.

Return value:
	true:	overflow happened in receiving buffer
	false:	overflow didn't happen in receiving buffer
************************************************************************/
bool is_spi_e_slave_receive_buffer_overflow()
{
	return _spieReceiveBufferOverflow;
}

/************************************************************************
Clear the overflow flag.
This flag can be cleared if there is free space in slave receiving buffer.

Return value:
	true:	overflow flag is cleared
	false:	overflow flag cannot be cleared
************************************************************************/
bool spi_e_slave_clear_receive_buffer_overflow()
{
	if(_spieReceiveBufferOverflow)
	{
		if(!_spieReceiveBufferFull)
		{
			//free space available
			_spieReceiveBufferOverflow = false;
		}
		else
		{
			//no free space in receiving buffer, cannot clear overflow flag.
			return false;
		}
	}
	
	return true;
}

ISR(SPIE_INT_vect)
{
	if(SPIE_CTRL & SPI_MASTER_bm)
	{
		//master
		_pSpieSendBuffer[_spieCurrentSendIndex] = SPIE_DATA;
		_spieCurrentSendIndex++;
		if(!_spieSendFinished)
		{
			if(_spieCurrentSendIndex < _spieSendBufferLength)
			{
				SPIE_DATA = _pSpieSendBuffer[_spieCurrentSendIndex];
			}
			else
			{
				PORTE_OUTSET = PIN4_bm;
				_spieSendFinished = true;
			}
		}
	}
	else
	{
		//slave
		if(_spieReceiveBufferFull)
		{
			//overflow
			_spieReceiveBufferOverflow = true;
		}
		else
		{
			_spieReceiveBuffer[_spieReceiveBufferTail] = SPIE_DATA;
			if((_spieReceiveBufferTail + 1) == _spieReceiveBufferHead)
			{
				_spieReceiveBufferFull = true;
			}
			else
			{
				//further data can be received.
				_spieReceiveBufferTail++;
				if(!_spieSendFinished)
				{
					_spieCurrentSendIndex++;
					if(_spieCurrentSendIndex < _spieSendBufferLength)
					{
						SPIE_DATA = _pSpieSendBuffer[_spieCurrentSendIndex];
						_spi_e_slave_data_available(true);
					}
					else
					{
						_spieSendFinished = true;
						SPIE_DATA = 0x00;
						_spi_e_slave_data_available(false);
					}
				}
				else
				{
					SPIE_DATA = 0x00;
					_spi_e_slave_data_available(false);
				}
				_spi_e_slave_singal_ready();
			}
		}
	}
}

#endif

#ifdef ECD300_SPIF_ENABLED_INT

/************************************************************************
 SPI master or SPI slave are supported by this driver.
 SPIF has a receiving buffer with a default size of 256 bytes. 
 This buffer facilitates data receiving in SPI slave, and is not used in SPI master.
 
 If SPIF works as master, PIN4 is SS by default.
 If SPIF works as slave, the falling edge on PIN2 indicates that master can write one byte to slave;
 with the falling edge on PIN2, low level on PIN3 indicates that slave has a byte to be read by master, 
 high level on PIN3 indicates that slave has no data to be read. The falling edge is sent out in ISR or
 when data is read from the receiving buffer.
************************************************************************/

static bool _spifInitialized = false;
static unsigned char _spifReceiveBuffer[256];
static unsigned char _spifReceiveBufferHead;
static unsigned char _spifReceiveBufferTail;
static unsigned char _spifSendBufferLength;
static unsigned char _spifCurrentSendIndex;
static unsigned char * _pSpifSendBuffer;
static bool _spifSendFinished = true;
static bool _spifReceiveBufferFull = false;
static bool _spifReceiveBufferOverflow = false;

/************************************************************************
Initialize SPI module in PORTF, including PINs.
If master, PIN 4, 5, 6 and 7 are configured; if slave, PIN 2, 3, 4, 5, 6 and 7 are configured.

Parameters:
	is_master:	SPI module is initialized as master if true; as slave if false
	frequency:	frequency of SCK if SPI master
	priority:	priority of SPIF interrupt, must be 1, 2, or 3 with 3 the highest priority
	mode:		0, 1, 2, 3, refer to chapter 22.5 in XMEGA manual.
Return value:
	0:	success
	-1:	mode is not supported
	-2:	frequency is out of scope
	-3: interrupt priority is out of range
************************************************************************/
char spi_f_init(bool is_master, unsigned long frequency, unsigned char priority, unsigned char mode)
{
	_spifReceiveBufferHead = 0;
	_spifReceiveBufferTail = 0;
	_spifSendBufferLength = 0;
	_spifCurrentSendIndex = 0;
	
	switch(mode)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			break;
		default:
			return -1;
	}
	switch(priority)
	{
		case 1:
		case 2:
		case 3:
			break;
		default:
			return -3;
	}

	if(is_master)
	{
		//SS
		ioport_configure_port_pin(&PORTF, PIN4_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//MOSI
		ioport_configure_port_pin(&PORTF, PIN5_bm, IOPORT_DIR_OUTPUT);
		//MISO
		ioport_configure_port_pin(&PORTF, PIN6_bm, IOPORT_DIR_INPUT);
		//CLK
		ioport_configure_port_pin(&PORTF, PIN7_bm, IOPORT_DIR_OUTPUT);
	}
	else
	{
		//SLAVE READY
		ioport_configure_port_pin(&PORTF, PIN2_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//SLAVE DATA AVAILABLE
		ioport_configure_port_pin(&PORTF, PIN3_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//SS
		ioport_configure_port_pin(&PORTF, PIN4_bm, IOPORT_PULL_UP	| IOPORT_DIR_INPUT);
		//MOSI
		ioport_configure_port_pin(&PORTF, PIN5_bm, IOPORT_DIR_INPUT);
		//MISO
		ioport_configure_port_pin(&PORTF, PIN6_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);
		//CLK
		ioport_configure_port_pin(&PORTF, PIN7_bm, IOPORT_DIR_INPUT);
	}

	sysclk_enable_module(SYSCLK_PORT_F, PR_SPI_bm);

	if(is_master){
		spi_enable_master_mode(&SPIF);
		if (spi_xmega_set_baud_div(&SPIF, frequency, sysclk_get_cpu_hz()) < 0) {
			return -2;
		}
	}
	
	/* Clear any set SPI mode flags and set them to the user-specified mode */
	SPIF_CTRL = (SPIF_CTRL & ~SPI_MODE_gm) | ((mode << SPI_MODE_gp) & SPI_MODE_gm);
	
	SPIF_INTCTRL = priority;
	
	spi_enable(&SPIF);
		
	_spifReceiveBufferFull = false;
	_spifReceiveBufferOverflow = false;
	_spifSendFinished = true;
	_spifInitialized = true;
	
	return 0;
}

static inline void _spi_f_slave_data_available(bool bDataReady)
{
	if(bDataReady)
		PORTF_OUTCLR = PIN3_bm;
	else
		PORTF_OUTSET = PIN3_bm;
}

static inline void _spi_f_slave_singal_ready()
{
	if((_spifReceiveBufferTail + 1) != _spifReceiveBufferHead)
	{
		//create a falling edge.
		PORTF_OUTSET = PIN2_bm;
		PORTF_OUTCLR = PIN2_bm;
	}
}

/************************************************************************
Send a block of bytes out of SPI module.
If SPI module works as master, data read back from MISO is saved to address pointed by pBuffer.
If slave, data read from MOSI is saved to receiving buffer, more data can be received from MOSI
if there are free space in the receiving buffer.

Parameters:
	pBuffer:	address of bytes to be sent out.
	length:		amount of data in byte.
Return value:
	0: data exchange starts
	-1: value of pBuffer is not qualified
	-2:	previous block of bytes hasn't been sent out completely
************************************************************************/
char spi_f_send_data(unsigned char * pBuffer, unsigned char length)
{
	if(NULL == pBuffer)
	{
		return -1;	
	}
	if(!_spifSendFinished)
	{
		return -2;
	}
	if(0 != length)
	{
		_pSpifSendBuffer = pBuffer;
		_spifSendBufferLength = length;
		_spifCurrentSendIndex = 0;
		_spifSendFinished = false;
		
		if(SPIF_CTRL & SPI_MASTER_bm)
		{
			//master, assert default SS pin.
			PORTF_OUTCLR = PIN4_bm;
			SPIF_DATA = pBuffer[0];
		}
		else
		{
			SPIF_DATA = pBuffer[0];
			_spi_f_slave_data_available(true);
			_spi_f_slave_singal_ready();
		}
	}
	
	return 0;
}


/************************************************************************
Check status of data sending.

Return value:
	true:	data specified in spi_f_send_data( ) is sent out.
	false:	data specified in spi_f_send_data( ) hasn't been sent out.
************************************************************************/
bool spi_f_send_finished( )
{
	return _spifSendFinished;
}

/************************************************************************
Get received data from the receiving buffer when SPI module works as slave.

Parameter:
	pBuffer:	buffer address where the received data should be written to
	maxLength:	length of buffer pointed by pBuffer
Return value:
	amount of bytes actually written to the buffer.
************************************************************************/
unsigned char spi_f_slave_receive_data(unsigned char * pBuffer, unsigned char maxLength)
{
	unsigned char counter;
	unsigned char head, tail;
	
	if(maxLength == 0)
	{
		return 0;
	}
	if(pBuffer == NULL)
	{
		return 0;
	}
	
	for(counter = 0; 
		(counter < maxLength) && (_spifReceiveBufferHead != _spifReceiveBufferTail); 
		counter++, _spifReceiveBufferHead++)
	{
		pBuffer[counter] = _spifReceiveBuffer[_spifReceiveBufferHead];		
	}
	
	if(_spifReceiveBufferFull)
	{
		//ISR has stopped signaling readiness to master.
		if(counter > 0)
		{
			_spifReceiveBufferFull = false;
			if(!_spifSendFinished)
			{
				SPIF_DATA = _pSpifSendBuffer[_spifCurrentSendIndex];
				_spi_f_slave_data_available(true);
			}
			else
			{
				SPIF_DATA = 0x00;
				_spi_f_slave_data_available(false);
			}
			_spi_f_slave_singal_ready();
		}
	}
	else 
	{
		//there is free space before this reading.
		if(_spifSendFinished)
		{
			//notify master of free space in slave
			SPIF_DATA = 0x00;
			_spi_f_slave_data_available(false);
			_spi_f_slave_singal_ready();	
		}
		else
		{
			//current block of bytes should be being sent, so do nothing.
			;
		}
	}
	
	return counter;
}


/************************************************************************
Get status about overflow in slave receiving buffer.

Return value:
	true:	overflow happened in receiving buffer
	false:	overflow didn't happen in receiving buffer
************************************************************************/
bool is_spi_f_slave_receive_buffer_overflow()
{
	return _spifReceiveBufferOverflow;
}

/************************************************************************
Clear the overflow flag.
This flag can be cleared if there is free space in slave receiving buffer.

Return value:
	true:	overflow flag is cleared
	false:	overflow flag cannot be cleared
************************************************************************/
bool spi_f_slave_clear_receive_buffer_overflow()
{
	if(_spifReceiveBufferOverflow)
	{
		if(!_spifReceiveBufferFull)
		{
			//free space available
			_spifReceiveBufferOverflow = false;
		}
		else
		{
			//no free space in receiving buffer, cannot clear overflow flag.
			return false;
		}
	}
	
	return true;
}

ISR(SPIF_INT_vect)
{
	if(SPIF_CTRL & SPI_MASTER_bm)
	{
		//master
		_pSpifSendBuffer[_spifCurrentSendIndex] = SPIF_DATA;
		_spifCurrentSendIndex++;
		if(!_spifSendFinished)
		{
			if(_spifCurrentSendIndex < _spifSendBufferLength)
			{
				SPIF_DATA = _pSpifSendBuffer[_spifCurrentSendIndex];
			}
			else
			{
				PORTF_OUTSET = PIN4_bm;
				_spifSendFinished = true;
			}
		}
	}
	else
	{
		//slave
		if(_spifReceiveBufferFull)
		{
			//overflow
			_spifReceiveBufferOverflow = true;
		}
		else
		{
			_spifReceiveBuffer[_spifReceiveBufferTail] = SPIF_DATA;
			if((_spifReceiveBufferTail + 1) == _spifReceiveBufferHead)
			{
				_spifReceiveBufferFull = true;
			}
			else
			{
				//further data can be received.
				_spifReceiveBufferTail++;
				if(!_spifSendFinished)
				{
					_spifCurrentSendIndex++;
					if(_spifCurrentSendIndex < _spifSendBufferLength)
					{
						SPIF_DATA = _pSpifSendBuffer[_spifCurrentSendIndex];
						_spi_f_slave_data_available(true);
					}
					else
					{
						_spifSendFinished = true;
						SPIF_DATA = 0x00;
						_spi_f_slave_data_available(false);
					}
				}
				else
				{
					SPIF_DATA = 0x00;
					_spi_f_slave_data_available(false);
				}
				_spi_f_slave_singal_ready();
			}
		}
	}
}

#endif
