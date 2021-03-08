/*
 * mock_functions.h
 *
 *  Created on: 26/02/2021
 *      Author: user1
 */

#ifndef MOCK_FUNCTIONS_H_
#define MOCK_FUNCTIONS_H_


void tcOneClock();
void tcClocks(unsigned int ticks);

#define MOCK_USB_INPUT_BUFFER_MASK 0xFF
#define MOCK_USB_OUTPUT_BUFFER_MASK 0xFF

void usbClearOutputBuffer();
int usbOutputBufferUsed();
int usbConsumeData(unsigned char * pBuffer, int size);
void usbClearInputBuffer();
int usbGetInputBufferUsed();
int usbProduceData(unsigned char * pBuffer, int size);
int usbInputBufferConsumerIndex();
int usbInputBufferProducerIndex();
int usbOutputBufferConsumerIndex();
int usbOutputBufferProducerIndex();


#define MOCK_UART_INPUT_BUFFER_MASK 0xFF
#define MOCK_UART_OUTPUT_BUFFER_MASK 0xFF

void uartReset();
int uartProduceData(unsigned char * pBuffer, int size);
int uartConsumeData(unsigned char * pBuffer, int size);
int uartInputBufferConsumerIndex();
int uartInputBufferProducerIndex();
int uartOutputBufferConsumerIndex();
int uartOutputBufferProducerIndex();

void crc_set_initial_value(unsigned long v);
unsigned long crc_io_checksum(void * pBuf, unsigned short len, unsigned char type);

#define ASSERT(condition) { if((condition) != true) printf("ERROR: assert failed at line %d in function %s in file %s\r\n", __LINE__, __FUNCTION__, __FILE__);}

#endif /* MOCK_FUNCTIONS_H_ */
