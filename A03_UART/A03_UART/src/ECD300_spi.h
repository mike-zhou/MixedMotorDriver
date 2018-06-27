#ifndef __ECD300_SPI_H__
#define __ECD300_SPI_H__

#include "asf.h"

#include "ECD300_spi_config.h"

#ifdef ECD300_SPIC_ENABLED_INT
char spi_c_init(bool is_master, unsigned long frequency, unsigned char priority, unsigned char mode);
char spi_c_send_data(unsigned char * pBuffer, unsigned char length);
inline bool spi_c_send_finished( );
unsigned char spi_c_slave_receive_data(unsigned char * pBuffer, unsigned char maxLength);
inline bool is_spi_c_slave_receive_buffer_overflow();
bool spi_c_slave_clear_receive_buffer_overflow();
#endif

#ifdef ECD300_SPID_ENABLED_INT
char spi_d_init(bool is_master, unsigned long frequency, unsigned char priority, unsigned char mode);
char spi_d_send_data(unsigned char * pBuffer, unsigned char length);
inline bool spi_d_send_finished( );
unsigned char spi_d_slave_receive_data(unsigned char * pBuffer, unsigned char maxLength);
inline bool is_spi_d_slave_receive_buffer_overflow();
bool spi_d_slave_clear_receive_buffer_overflow();
#endif

#ifdef ECD300_SPIE_ENABLED_INT

/************************************************************************
Initialize SPI module in PORTE, including PINs.
If master, PIN 4, 5, 6 and 7 are configured; if slave, PIN 2, 3, 4, 5, 6 and 7 are configured.

Parameters:
	is_master:	SPI module is initialized as master if true; as slave if false
	frequency:	frequency of SCK if SPI master
	mode:		0, 1, 2, 3, refer to chapter 22.5 in XMEGA manual.
Return value:
	0:	success
	-1:	mode is not supported
	-2:	frequency is out of scope
************************************************************************/
char spi_e_init(bool is_master, unsigned long frequency, unsigned char priority, unsigned char mode);

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
char spi_e_send_data(unsigned char * pBuffer, unsigned char length);

/************************************************************************
Check status of data sending.

Return value:
	true:	data specified in spi_e_send_data( ) is sent out.
	false:	data specified in spi_e_send_data( ) hasn't been sent out.
************************************************************************/
inline bool spi_e_send_finished( );

/************************************************************************
Get received data from the receiving buffer when SPI module works as slave.

Parameter:
	pBuffer:	buffer address where the received data should be written to
	maxLength:	length of buffer pointed by pBuffer
Return value:
	amount of bytes actually written to the buffer.
************************************************************************/
unsigned char spi_e_slave_receive_data(unsigned char * pBuffer, unsigned char maxLength);

/************************************************************************
Get status about overflow in slave receiving buffer.

Return value:
	true:	overflow happened in receiving buffer
	false:	overflow didn't happen in receiving buffer
************************************************************************/
inline bool is_spi_e_slave_receive_buffer_overflow();

/************************************************************************
Clear the overflow flag.
This flag can be cleared if there is free space in slave receiving buffer.

Return value:
	true:	overflow flag is cleared
	false:	overflow flag cannot be cleared
************************************************************************/
bool spi_e_slave_clear_receive_buffer_overflow();

#endif

#ifdef ECD300_SPIF_ENABLED_INT
char spi_f_init(bool is_master, unsigned long frequency, unsigned char priority, unsigned char mode);
char spi_f_send_data(unsigned char * pBuffer, unsigned char length);
inline bool spi_f_send_finished( );
unsigned char spi_f_slave_receive_data(unsigned char * pBuffer, unsigned char maxLength);
inline bool is_spi_f_slave_receive_buffer_overflow();
bool spi_f_slave_clear_receive_buffer_overflow();
#endif

#endif

