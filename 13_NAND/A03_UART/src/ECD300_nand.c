#include "ECD300_nand_config.h"
#include "ECD300_nand.h"
#include "asf.h"

#ifdef ECD300_NAND_S34ML01G1

#define NAND_PAGE_SIZE (2048+64)

//controls
#define setCLE	(PORTE_OUTSET=0x10)
#define clrCLE	(PORTE_OUTCLR=0x10)
#define setCE	(PORTE_OUTSET=0x20)
#define clrCE		(PORTE_OUTCLR=0x20)
#define setWE	(PORTE_OUTSET=0x04)
#define clrWE	(PORTE_OUTCLR=0x04)
#define setALE	(PORTE_OUTSET=0x08)
#define clrALE	(PORTE_OUTCLR=0x08)
#define setRE	(PORTE_OUTSET=0x40)
#define clrRE		(PORTE_OUTCLR=0x40)
#define isReady	(PORTE_IN&0x80)
#define setWP 	(PORTE_OUTSET=0x02)
#define clrWP	(PORTE_OUTCLR=0x02)
#define initCommand (PORTE_OUT=0x40|0x20|0x04|0x02) //RE|CE|CLE#|ALE#|WE|WP
//data
#define enableWriteDataBus 	(PORTF_DIR=0xff)
#define writeDataBus(data)	(PORTF_OUT=(unsigned char)(data))
#define disableWriteDataBus	(PORTF_DIR=0x00)
#define getData				(PORTF_IN)

static enum
{
	COMMAND_IDLE=0,
	COMMAND_READ,
	COMMAND_PROGRAM,
	COMMAND_ERASE
} _currentCommand;

static unsigned char _planeAddress;
static unsigned short _rowAddress;
static unsigned short _colAddress;

static void _readPage(unsigned short rowAddress, unsigned short colAddress)
{
	initCommand;
	/* 1st cycle */
	//00h
	clrCE;
	clrWE;
	writeDataBus(0x00);
	enableWriteDataBus;
	setCLE;
	setWE;
	clrCLE;
	//ColAdd.1
	setALE;
	clrWE;
	writeDataBus(colAddress);
	setWE;
	//ColAdd.2
	clrWE;
	writeDataBus(colAddress>>8);
	setWE;
	//RowAdd.1
	clrWE;
	writeDataBus(rowAddress);
	setWE;
	//RowAdd.2
	clrWE;
	writeDataBus(rowAddress>>8);
	setWE;
	clrALE;
	//30h
	setCLE;
	clrWE;
	writeDataBus(0x30);
	setWE;
	clrCLE;
	disableWriteDataBus;
}

static void _setColAddress(unsigned short colAddress)
{
	//05h
	setCLE;
	clrWE;
	writeDataBus(0x05);
	enableWriteDataBus;
	setWE;
	clrCLE;
	//ColAdd.1
	setALE;
	clrWE;
	writeDataBus(colAddress);
	setWE;
	//colAdd.2
	clrWE;
	writeDataBus(colAddress>>8);
	setWE;
	clrALE;
	//E0h
	setCLE;
	clrWE;
	writeDataBus(0xE0);
	setWE;
	clrCLE;
	disableWriteDataBus;
}

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
							bool bWait)
{
	unsigned short i;
	
	if(0!=planeAddress)
	{
		return ECD300_NAND_INVALID_PLANE_ADDRESS;
	}
	if(colAddress>=NAND_PAGE_SIZE)
	{
		return ECD300_NAND_INVALID_COL_ADDRESS;
	}

	if(!isReady)
	{
		if(!bWait)
		{
			return ECD300_NAND_CHIP_BUSY;
		}
	}
	while(!isReady)
	{
		;
	}
	
	if(COMMAND_READ!=_currentCommand)
	{
		//switch to read command from other command.
		_currentCommand=COMMAND_READ;

		_readPage(rowAddress, colAddress);
		
		//remember the starting address.
		_rowAddress=rowAddress;
		_colAddress=colAddress;
	}

	if(_rowAddress!=rowAddress)
	{
		//page address was updated.
		_readPage(rowAddress, colAddress);
		
		//remember the new address.
		_rowAddress=rowAddress;
		_colAddress=colAddress;
	}

	if(!bWait)
	{
		//do not wait if busy.
		if(!isReady)
		{
			//busy.
			return 0;
		}
	}
	//wait until ready.
	while(!isReady)
	{
		;
	}

	//read collumns.
	if(_colAddress!=colAddress)
	{
		_setColAddress(colAddress);
		_colAddress=colAddress;
	}
	for(i=0;(i<amount)&&(_colAddress<NAND_PAGE_SIZE);i++, _colAddress++)
	{
		clrRE;
		clrRE;//repeat this command to allocate enough time for the output of data.
		*pBuffer=getData;
		setRE;
		pBuffer++;
	}

	return i;
}


/**
This function returns the page size in bytes.
*/
unsigned short ecd300GetNandPageSize(void)
{
	return NAND_PAGE_SIZE;
}

static unsigned char * _pBuffer;
static unsigned short _amount;
static unsigned short _actualAmount;
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
							bool bWait)
{
	unsigned short i;
	unsigned char status;
	
	if(0!=planeAddress)
	{
		return ECD300_NAND_INVALID_PLANE_ADDRESS;
	}
	if(colAddress>=NAND_PAGE_SIZE)
	{
		return ECD300_NAND_INVALID_COL_ADDRESS;
	}

	if(!isReady)
	{
		if(!bWait)
		{
			return ECD300_NAND_CHIP_BUSY;
		}
	}
	while(!isReady)
	{
		;
	}

	if(COMMAND_PROGRAM!=_currentCommand)
	{
		//remember the initial parameters.
		_pBuffer=pBuffer;
		_amount=amount;
		_colAddress=colAddress;
		_rowAddress=rowAddress;
		_planeAddress=planeAddress;

		_currentCommand=COMMAND_PROGRAM;
		
		initCommand;
		//80h
		clrCE;
		setCLE;
		clrWE;
		writeDataBus(0x80);
		enableWriteDataBus;
		setWE;
		clrCLE;
		//ColAdd1
		setALE;
		clrWE;
		writeDataBus(colAddress);
		setWE;
		//ColAdd2
		clrWE;
		writeDataBus(colAddress>>8);
		setWE;
		//RowAdd1
		clrWE;
		writeDataBus(rowAddress);
		setWE;
		//RowAdd2
		clrWE;
		writeDataBus(rowAddress>>8);
		setWE;
		clrALE;

		//data
		for(i=0;(i<amount)&&((colAddress+i)<NAND_PAGE_SIZE);i++)
		{
			clrWE;
			writeDataBus(pBuffer[i]);
			setWE;
		}
		_actualAmount=i;

		//10h
		setCLE;
		clrWE;
		writeDataBus(0x10);
		setWE;
		disableWriteDataBus;
		//CLE keeps high.
	}

	if((_planeAddress!=planeAddress)||
		(_rowAddress!=rowAddress)||
		(_colAddress!=colAddress)||
		(_pBuffer!=pBuffer)||
		(_amount!=amount))
	{
		return ECD300_NAND_PARAMETER_CHANGE;
	}

	//check R/B#
	if(!isReady)
	{
		if(!bWait)
		{
			return ECD300_NAND_CHIP_BUSY;
		}
	}
	while(!isReady)
	{
		;
	}

	//70h
	//CLE keeps high.
	clrWE;
	writeDataBus(0x70);
	enableWriteDataBus;
	setWE;
	clrCLE;
	disableWriteDataBus;
	//read status
	clrRE;
	clrRE;
	status=getData;
	setRE;

	//programming finishes.
	initCommand;
	_currentCommand=COMMAND_IDLE;
	
	if(status&0x01)
	{
		//error in programming
		return ECD300_NAND_PROGRAM_ERROR;
	}
	else
	{
		return _actualAmount;
	}
}

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
						bool bWait)
{
	unsigned char status;
	
	if(0!=planeAddress)
	{
		return ECD300_NAND_INVALID_PLANE_ADDRESS;
	}

	if(!isReady)
	{
		if(!bWait)
		{
			return ECD300_NAND_CHIP_BUSY;
		}
	}
	while(!isReady)
	{
		;
	}

	if(COMMAND_ERASE!=_currentCommand)
	{
		//remember the initial parameters.
		_rowAddress=rowAddress;
		_planeAddress=planeAddress;

		_currentCommand=COMMAND_ERASE;
		
		initCommand;
		//60h
		clrCE;
		setCLE;
		clrWE;
		writeDataBus(0x60);
		enableWriteDataBus;
		setWE;
		clrCLE;
		//RowAdd1
		setALE;
		clrWE;
		writeDataBus(rowAddress);
		setWE;
		//RowAdd2
		clrWE;
		writeDataBus(rowAddress>>8);
		setWE;
		clrALE;

		//D0h
		setCLE;
		clrWE;
		writeDataBus(0xD0);
		setWE;
		clrCLE;
		disableWriteDataBus;
	}

	if((_planeAddress!=planeAddress)||
		(_rowAddress!=rowAddress))
	{
		return ECD300_NAND_PARAMETER_CHANGE;
	}

	//check R/B#
	if(!isReady)
	{
		if(!bWait)
		{
			return ECD300_NAND_CHIP_BUSY;
		}
	}
	while(!isReady)
	{
		;
	}

	//70h
	setCLE;
	clrWE;
	writeDataBus(0x70);
	enableWriteDataBus;
	setWE;
	clrCLE;
	disableWriteDataBus;
	//read status
	clrRE;
	clrRE;
	status=getData;
	setRE;

	//ersinging finishes.
	initCommand;
	_currentCommand=COMMAND_IDLE;
	
	if(status&0x01)
	{
		//error in erasing
		return ECD300_NAND_ERASE_ERROR;
	}
	else
	{
		return 0;
	}
}

/**
This function initializes the nand flash driver.
*/
void ecd300InitNand(void)
{
	_currentCommand=COMMAND_IDLE;
	_planeAddress=0;
	_rowAddress=0;
	_colAddress=0;

	disableWriteDataBus;
	initCommand;
	PORTE_DIR=0x7E;
}

#endif

