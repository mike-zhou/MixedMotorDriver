/*
 * crc.h
 *
 *  Created on: 25/02/2021
 *      Author: user1
 */

#ifndef CRC_H_
#define CRC_H_


#define CRC_16BIT 16

void crc_set_initial_value(unsigned long v);
unsigned long crc_io_checksum(void * pBuf, unsigned short len, unsigned char type);

#endif /* CRC_H_ */
