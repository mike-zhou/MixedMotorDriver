#ifndef _ECD300_NAND_H__
#define _ECD300_NAND_H__

#include "asf.h"

#define ECD300_NAND_INVALID_PLANE_ADDRESS 	-1
#define ECD300_NAND_INVALID_ROW_ADDRESS		-2
#define ECD300_NAND_INVALID_COL_ADDRESS		-3
#define ECD300_NAND_CHIP_BUSY					-4
#define ECD300_NAND_PROGRAM_ERROR			-5
#define ECD300_NAND_PARAMETER_CHANGE		-6
#define ECD300_NAND_ERASE_ERROR				-7


/**
This function initializes the nand flash driver.
*/
void ecd300InitNand(void);

/**
This function returns the page size in bytes.
*/
unsigned short ecd300GetNandPageSize(void);

unsigned short ecd300GetPagesPerBlock(void);

unsigned short ecd300GetBlocksPerPlane(void);

unsigned short ecd300GetPlanesPerChip(void);

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
	ECD300_NAND_INVALID_PLANE_ADDRESS:	planeAddress is out of range.
	ECD300_NAND_INVALID_ROW_ADDRESS: 		rowAddress is out of range.
	ECD300_NAND_INVALID_COL_ADDRESS:		colAddress is out of range.
	ECD300_NAND_CHIP_BUSY: 				busy.
*/
short ecd300ReadNandPage(unsigned char planeAddress, 
							unsigned short rowAddress, 
							unsigned short colAddress, 
							unsigned char * pBuffer, 
							unsigned short amount,
							bool bWait);


short ecd300ReadNandPageEx(unsigned char planeAddress, 
								unsigned short blockAddress, 
								unsigned short pageAddress,
								unsigned short colAddress, 
								unsigned char * pBuffer, 
								unsigned short amount,
								bool bWait);


/**
This function tries to write the amount of data pointed by pBuffer to the page selected by planeAddress:rowAddress, starting from colAddress.
Parameter:
	planeAddress:		the plane to be written.
	rowAddress:		the row (page of a block) to be written.
	colAddress:		the beginning address to write to.
	pBuffer:			the address of the data which is to be saved.
	amount:			the amount of data to write. 
	bWait:			true: this function will wait if the flash chip is busy.
					false: this function won't wait if the flash chip is busy.
Return value:
	positive:	the actual amount of data which is written to the flash.
	0:	no data is written.
	ECD300_NAND_INVALID_PLANE_ADDRESS:	planeAddress is out of range.
	ECD300_NAND_INVALID_ROW_ADDRESS: 		rowAddress is out of range.
	ECD300_NAND_INVALID_COL_ADDRESS:		colAddress is out of range.
	ECD300_NAND_CHIP_BUSY: 				busy.
	ECD300_NAND_PROGRAM_ERROR: 			the flash reports data error.
	ECD300_NAND_PARAMETER_CHANGE:			parameters change before programming finishes.
*/
short ecd300ProgramNandPage(unsigned char planeAddress, 
							unsigned short rowAddress, 
							unsigned short colAddress, 
							unsigned char * pBuffer, 
							unsigned short amount,
							bool bWait);

short ecd300ProgramNandPageEx(unsigned char planeAddress, 
								unsigned short blockAddress, 
								unsigned short pageAddress,
								unsigned short colAddress, 
								unsigned char * pBuffer, 
								unsigned short amount,
								bool bWait);

/**
This function erases the whole block which contains the row planeAddress:rowAddress.
Parameter:
	planeAddress:		the plane which contains the block to be erased.
	rowAddress:		the row address which is contained in the block which is to be erased.
	bWait:			true: this function don't return until the whole block is erased.
					false: this function don't wait for the completion of erasing.
Return value:
	0:	the block is erased successfully.
	ECD300_NAND_INVALID_PLANE_ADDRESS:	planeAddress is out of range.
	ECD300_NAND_INVALID_ROW_ADDRESS: 		rowAddress is out of range.
	ECD300_NAND_CHIP_BUSY:					busy.
	ECD300_NAND_ERASE_ERROR:				the flash reports erasing error.
	ECD300_NAND_PARAMETER_CHANGE:			parameters change before erasing finishes.
*/
short ecd300EraseNandBlock(unsigned char planeAddress,
						unsigned short rowAddress,
						bool bWait);

short ecd300EraseNandBlockEx(unsigned char planeAddress,
						unsigned short blockAddress,
						bool bWait);


/**
This function returns the current status of the NAND chip.
Return value:
	TRUE:	The chip is ready for the next access.
	FALSE:	The chip is busy.
*/
bool ecd300IsNandReady(void);

#endif

