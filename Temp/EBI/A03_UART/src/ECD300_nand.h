#ifndef _ECD300_NAND_H__
#define _ECD300_NAND_H__


/**
This function initializes the nand flash driver.
*/
void ecd300InitNand(void);

/**
This function tries to read the designated amount of bytes with the designated address.
Parameter:
	planeAddress:		the plane to read data from.
	rowAddress:		the row (page of a block) to read data from.
	colAddress:		the beginning to read data from.
	pBuffer:			the address where the data should be saved to.
	amount:			the amount of data to read. The caller should make sure that the buffer is enough.
	bWait:			true: this function will wait if the flash chip is busy.
					false: this function won't wait if the flash chip is busy.
Return value:
	positive:	the actual amount of data which is put to pBuffer.
	0:	no data is put to pBuffer.
	-1:	planeAddress is out of range.
	-2: 	rowAddress is out of range.
	-3:	colAddress is out of range.
	-4: 	other command is being executed.
*/
short ecd300ReadNand(unsigned char planeAddress, 
						unsigned short rowAddress, 
						unsigned short colAddress, 
						unsigned char * pBuffer, 
						unsigned short amount,
						bool bWait);


#endif

