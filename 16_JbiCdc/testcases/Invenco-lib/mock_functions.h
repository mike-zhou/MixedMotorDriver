/*
 * mock_functions.h
 *
 *  Created on: 26/02/2021
 *      Author: user1
 */

#ifndef MOCK_FUNCTIONS_H_
#define MOCK_FUNCTIONS_H_

////////////////////////////////
// timer counter
////////////////////////////////
void tcOneClock();
void tcClocks(unsigned int ticks);

////////////////////////////////
// usb
////////////////////////////////
#define MOCK_USB_INPUT_BUFFER_MASK 0xFF
#define MOCK_USB_OUTPUT_BUFFER_MASK 0xFF

// output
void usbClearOutputBuffer();
int usbOutputBufferUsed();
int usbConsumeData(unsigned char * pBuffer, int size);
int usbOutputBufferConsumerIndex();
bool usbOutputBufferSetConsumerIndex(unsigned short index);
int usbOutputBufferProducerIndex();
bool usbOutputBufferSetProducerIndex(unsigned short index);
// input
void usbClearInputBuffer();
int usbGetInputBufferUsed();
int usbProduceData(unsigned char * pBuffer, int size);
int usbInputBufferConsumerIndex();
int usbInputBufferProducerIndex();

//////////////////////////////
// uart
//////////////////////////////
#define MOCK_UART_INPUT_BUFFER_MASK 0xFF
#define MOCK_UART_OUTPUT_BUFFER_MASK 0xFF

void uartReset();
//input
int uartProduceData(unsigned char * pBuffer, int size);
int uartInputBufferConsumerIndex();
int uartInputBufferProducerIndex();
//output
int uartConsumeData(unsigned char * pBuffer, int size);
int uartOutputBufferConsumerIndex();
int uartOutputBufferProducerIndex();
bool uartOutputBufferSetConsumerIndex(int index);
bool uartOutputBufferSetProducerIndex(int index);

////////////////////////////////////
// crc
////////////////////////////////////
void crc_set_initial_value(unsigned long v);
unsigned long crc_io_checksum(void * pBuf, unsigned short len, unsigned char type);


//////////////////////////////
// assert
//////////////////////////////
static inline void breakpointFunc()
{
    return;
}
#define ASSERT(condition) { if((condition) != true) {printf("ERROR: assert failed at line %d in function %s in file %s\r\n", __LINE__, __FUNCTION__, __FILE__); breakpointFunc(); }}

#endif /* MOCK_FUNCTIONS_H_ */
