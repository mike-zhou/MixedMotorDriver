#include "ECD300_nand_config.h"
#include "ECD300_nand.h"
#include "asf.h"

#ifdef ECD300_NAND_S34ML01G1

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
//data
#define enableWriteDataBus 	(PORTF_DIR=0xff)
#define writeDataBus(data)	(PORTF_OUT=data)
#define disableWriteDataBus	(PORTF_DIR=0x00)
#define getData				(PORTF_IN)

static enum
{
	COMMAND_IDLE=0,
	COMMAND_READ,
	COMMAND_READ_ROW_STAGE,
	COMMAND_READ_COL_STAGE,
	COMMAND_WRITE,
} _currentCommand;

static unsigned char _planeAddress;
static unsigned char _rowAddress;
static unsigned char _colAddress;


short ecd300ReadNand(unsigned char planeAddress, 
						unsigned short rowAddress, 
						unsigned short colAddress, 
						unsigned char * pBuffer, 
						unsigned short amount,
						bool bWait);
{
	if(COMMAND_IDLE==_currentCommand)
	{
		_currentCommand=COMMAND_READ;
	}
	
}

void ecd300InitNand(void)
{
	_currentCommand=COMMAND_IDLE;
	_planeAddress=0;
	_rowAddress=0;
	_colAddress=0;
}

#endif

