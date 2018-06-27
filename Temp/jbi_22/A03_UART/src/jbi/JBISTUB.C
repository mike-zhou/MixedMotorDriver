/****************************************************************************/
/*																			*/
/*	Module:			jbistub.c												*/
/*																			*/
/*					Copyright (C) Altera Corporation 1997-2001				*/
/*																			*/
/*	Description:	Jam STAPL ByteCode Player main source file				*/
/*																			*/
/*					Supports Altera ByteBlaster hardware download cable		*/
/*					on Windows 95 and Windows NT operating systems.			*/
/*					(A device driver is required for Windows NT.)			*/
/*																			*/
/*					Also supports BitBlaster hardware download cable on		*/
/*					Windows 95, Windows NT, and UNIX platforms.				*/
/*																			*/
/*	Revisions:		1.1 fixed control port initialization for ByteBlaster	*/
/*					2.0 added support for STAPL bytecode format, added code	*/
/*						to get printer port address from Windows registry	*/
/*					2.1 improved messages, fixed delay-calibration bug in	*/
/*						16-bit DOS port, added support for "alternative		*/
/*						cable X", added option to control whether to reset	*/
/*						the TAP after execution, moved porting macros into	*/
/*						jbiport.h											*/
/*					2.2 added support for static memory						*/
/*						fixed /W4 warnings									*/
/*																			*/
/****************************************************************************/

#ifndef NO_ALTERA_STDIO
#define NO_ALTERA_STDIO
#endif

#include "asf.h"

#include "jbiport.h"

typedef char BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
#define TRUE 1
#define FALSE 0

#include "jbiexprt.h"


extern void printString(char * pString);
extern void printHex(unsigned char hex);


unsigned char ecd300GetJbcChar(unsigned long index);

/************************************************************************
*
*	Global variables
*/
#define NULL ((void *)0)
/* file buffer for Jam STAPL ByteCode input file */
static unsigned char *file_buffer = NULL;
static long file_length = 0L;

/* serial port interface available on all platforms */
BOOL jtag_hardware_initialized = FALSE;

void initialize_jtag_hardware(void);
void close_jtag_hardware(void);

/*
*	This structure stores information about each available vector signal
*/
struct VECTOR_LIST_STRUCT
{
	char *signal_name;
	int  hardware_bit;
	int  vector_index;
};

struct VECTOR_LIST_STRUCT vector_list[] =
{
	/* add a record here for each vector signal */
	{ "", 0, -1 }
};

#define VECTOR_SIGNAL_COUNT ((int)(sizeof(vector_list)/sizeof(vector_list[0])))

/************************************************************************
*
*	Customized interface functions for Jam STAPL ByteCode Player I/O:
*
*	jbi_jtag_io()
*	jbi_message()
*	jbi_delay()
*/

int jbi_jtag_io(int tms, int tdi, int read_tdo)
{
	int tdo = 0;

	if (!jtag_hardware_initialized)
	{
		initialize_jtag_hardware();
		jtag_hardware_initialized = TRUE;
	}


	return (tdo);
}

void jbi_message(char *message_text)
{
	printString(message_text);
	printString("\r\n");
}

void jbi_export_integer(char *key, long value)
{
	printString("Export: key = \"");
	printString(key);
	printString("\", value = ");
	printHex(value>>24);
	printString(", ");
	printHex(value>>16);
	printString(", ");
	printHex(value>>8);
	printString(", ");
	printHex(value);
	printString("\r\n");
}

#define HEX_LINE_CHARS 72
#define HEX_LINE_BITS (HEX_LINE_CHARS * 4)

char conv_to_hex(unsigned long value)
{
	char c;

	if (value > 9)
	{
		c = (char) (value + ('A' - 10));
	}
	else
	{
		c = (char) (value + '0');
	}

	return (c);
}

void jbi_export_boolean_array(char *key, unsigned char *data, long count)
{
}

void jbi_delay(long microseconds)
{
	unsigned long preStamp, curStamp, duration;

	preStamp=tc_read_count(&TCC0);
	preStamp=preStamp<<16;
	preStamp+=tc_read_count(&TCC1);

	for(;;)
	{
		curStamp=tc_read_count(&TCC0);
		curStamp=preStamp<<16;
		curStamp+=tc_read_count(&TCC1);

		if(curStamp>preStamp)
		{
			duration=curStamp-preStamp;
		}
		else
		{
			duration=0xffffffff-preStamp;
			duration+=curStamp+1;
		}
		
		//timer at 4MHz.
		if((duration>>2)>=microseconds)
		{
			break;
		}
	}
}

int jbi_vector_map
(
	int signal_count,
	char **signals
)
{
	return 0;
}

int jbi_vector_io
(
	int signal_count,
	long *dir_vect,
	long *data_vect,
	long *capture_vect
)
{
	return 0;
}


#define ECD300_MAX_MALLOC_AMONT 128
#define ECD300_MALLOC_POOL_SIZE	2048

//_mallocOffset records the offset of a malloc[X].
unsigned short _mallocOffset[ECD300_MAX_MALLOC_AMONT];
//_mallocLength records the byte amount of a malloc[X].
unsigned short _mallocLength[ECD300_MAX_MALLOC_AMONT];
unsigned long _mallocPool[ECD300_MALLOC_POOL_SIZE>>2];

void *jbi_malloc(unsigned short size)
{
	unsigned short slotIndex;
	void * rc=NULL;
	unsigned short startingPoint;
	unsigned short i;

	unsigned short tempOffset, tempLength;

	// 4 bytes alliggment.
	size=(size+3)&0xfffc;

	for(slotIndex=0;slotIndex<ECD300_MAX_MALLOC_AMONT;slotIndex++)
	{
		if(_mallocLength[slotIndex]==0)
		{
			break;// find the first empty slot.
		}
	}
	if(ECD300_MAX_MALLOC_AMONT!=slotIndex)
	{//a slot is found.
		startingPoint=0;
		for(i=0;i<slotIndex;i++)
		{//there is malloc records.
			if((_mallocOffset[i]-startingPoint)>=size)
			{// memory can be allocated between the startingPoint and this record.
				_mallocOffset[slotIndex]=startingPoint;
				_mallocLength[slotIndex]=size;

				rc=(void *)((unsigned long)_mallocPool+startingPoint);
				break;
			}
			else
			{
				//update startingPoint to the end of this record.
				startingPoint=_mallocOffset[i]+_mallocLength[i];
			}
		}
		if(0==slotIndex)
		{//no malloc occurred before.
			if(size<=ECD300_MALLOC_POOL_SIZE)
			{
				_mallocOffset[slotIndex]=startingPoint;
				_mallocLength[slotIndex]=size;

				rc=(void *)((unsigned long)_mallocPool+startingPoint);
			}
		}
		else if(i==slotIndex)
		{
			if(size<=(ECD300_MALLOC_POOL_SIZE-startingPoint))
			{
				_mallocOffset[slotIndex]=startingPoint;
				_mallocLength[slotIndex]=size;

				rc=(void *)((unsigned long)_mallocPool+startingPoint);
			}
		}

		if(NULL!=rc)
		{
			//sort the records.
			for(;slotIndex>=1;slotIndex--)
			{
				for(i=0;i<slotIndex;i++)
				{
					if(_mallocOffset[i]>_mallocOffset[i+1])
					{//bubble up.
						tempOffset=_mallocOffset[i+1];
						tempLength=_mallocLength[i+1];

						_mallocOffset[i+1]=_mallocOffset[i];
						_mallocLength[i+1]=_mallocLength[i];

						_mallocOffset[i]=tempOffset;
						_mallocLength[i]=tempLength;
					}
				}
			}
		}
	}
	else
	{//no slot available
		printString("!!! No slot available\r\n");
	}
	
	return rc;
}

void jbi_free(void *ptr)
{
	unsigned short i;
	unsigned short offset;

	offset=(unsigned short)ptr;

	for(i=0;i<ECD300_MAX_MALLOC_AMONT;i++)
	{
		if((_mallocOffset[i]<=offset)&&(offset<=(_mallocOffset[i]+_mallocLength[i])))
		{
			break;//this address is found within an allocation.
		}
	}
	if(ECD300_MAX_MALLOC_AMONT!=i)
	{//a slot is found to release.
		
		for(;i<(ECD300_MAX_MALLOC_AMONT-1);i++)
		{
			_mallocOffset[i]=_mallocOffset[i+1];
			_mallocLength[i]=_mallocLength[i+1];
		}
		_mallocLength[i]=0;
	}
	else
	{
		printString("!!! Wrong address is passed to jbi_free\r\n");
	}
}

void calibrate_delay(void)
{
	/**
	* configure 32-bit timer.
	*/
	//Use TCC0 as the lower 16-bit of a 32-bit timer.
	tc_enable(&TCC0);
	tc_reset(&TCC0);
	tc_set_wgm(&TCC0, TC_WG_NORMAL);
	tc_write_period(&TCC0, 0xffff);
	//frequency of clock is a bit slower than 4MHz.
	tc_write_clock_source(&TCC0, TC_CLKSEL_DIV8_gc);
	
	//Use TCC1 as the higher 16-bit of a 32-bit timer.
	tc_enable(&TCC1);
	tc_reset(&TCC1);
	tc_set_wgm(&TCC1, TC_WG_NORMAL);
	tc_write_period(&TCC1, 0xffff);
	sysclk_enable_module(SYSCLK_PORT_GEN, SYSCLK_EVSYS);
	EVSYS_CH0MUX=EVSYS_CHMUX_TCC0_OVF_gc;
	tc_write_clock_source(&TCC1, TC_CLKSEL_EVCH0_gc);
	tc_set_input_capture(&TCC1, TC_EVSEL_CH0_gc, TC_EVACT_OFF_gc);

	tc_restart(&TCC0);
	tc_restart(&TCC1);
}

char *error_text[] =
{
/* JBIC_SUCCESS            0 */ "success",
/* JBIC_OUT_OF_MEMORY      1 */ "out of memory",
/* JBIC_IO_ERROR           2 */ "file access error",
/* JAMC_SYNTAX_ERROR       3 */ "syntax error",
/* JBIC_UNEXPECTED_END     4 */ "unexpected end of file",
/* JBIC_UNDEFINED_SYMBOL   5 */ "undefined symbol",
/* JAMC_REDEFINED_SYMBOL   6 */ "redefined symbol",
/* JBIC_INTEGER_OVERFLOW   7 */ "integer overflow",
/* JBIC_DIVIDE_BY_ZERO     8 */ "divide by zero",
/* JBIC_CRC_ERROR          9 */ "CRC mismatch",
/* JBIC_INTERNAL_ERROR    10 */ "internal error",
/* JBIC_BOUNDS_ERROR      11 */ "bounds error",
/* JAMC_TYPE_MISMATCH     12 */ "type mismatch",
/* JAMC_ASSIGN_TO_CONST   13 */ "assignment to constant",
/* JAMC_NEXT_UNEXPECTED   14 */ "NEXT unexpected",
/* JAMC_POP_UNEXPECTED    15 */ "POP unexpected",
/* JAMC_RETURN_UNEXPECTED 16 */ "RETURN unexpected",
/* JAMC_ILLEGAL_SYMBOL    17 */ "illegal symbol name",
/* JBIC_VECTOR_MAP_FAILED 18 */ "vector signal name not found",
/* JBIC_USER_ABORT        19 */ "execution cancelled",
/* JBIC_STACK_OVERFLOW    20 */ "stack overflow",
/* JBIC_ILLEGAL_OPCODE    21 */ "illegal instruction code",
/* JAMC_PHASE_ERROR       22 */ "phase error",
/* JAMC_SCOPE_ERROR       23 */ "scope error",
/* JBIC_ACTION_NOT_FOUND  24 */ "action not found",
};

#define MAX_ERROR_CODE (int)((sizeof(error_text)/sizeof(error_text[0]))+1)

/************************************************************************/

static unsigned long _workSpacePool[160];

int jbi_main(void)
{
	long error_address = 0L;
	JBI_RETURN_TYPE crc_result = JBIC_SUCCESS;
	JBI_RETURN_TYPE exec_result = JBIC_SUCCESS;
	unsigned short expected_crc = 0;
	unsigned short actual_crc = 0;
	int exit_code = 0;
	int format_version = 0;
	unsigned char *workspace = NULL;
	char *action = NULL;
	char *init_list[10];
	long workspace_size;
	char *exit_string = NULL;
	int reset_jtag = 1;
	int execute_program = 1;
	unsigned long i;

	init_list[0] = NULL;

	//init pool for malloc and free.
	for(i=0;i<ECD300_MAX_MALLOC_AMONT;i++)
	{
		_mallocOffset[i]=0;
		_mallocLength[i]=0;
	}
	//set workspace
	workspace=(unsigned char *)_workSpacePool;
	workspace_size=160*4;
	
	{
		{
			/*
			*	Calibrate the delay loop function
			*/
			calibrate_delay();

			/*
			*	Check CRC
			*/
  			crc_result = jbi_check_crc(file_buffer, file_length, &expected_crc, &actual_crc);
  			switch (crc_result)
			{
				case JBIC_SUCCESS:
					printString("CRC matched: CRC value = %04X\n");
					break;

				case JBIC_CRC_ERROR:
					printString("CRC mismatch: expected %04X, actual %04X\n");
					break;

				case JBIC_UNEXPECTED_END:
					printString("Expected CRC not found, actual CRC value = %04X\n");
					break;

				case JBIC_IO_ERROR:
					printString("Error: File format is not recognized.\n");
 					break;

				default:
					printString("CRC function returned error code %d\n" );
					break;
			}
 

			if (execute_program)
			{
				/*
				*	Execute the Jam STAPL ByteCode program
				*/
 				exec_result = jbi_execute(file_buffer, file_length, workspace,
					workspace_size, action, init_list, reset_jtag,
					&error_address, &exit_code, &format_version);
 
				if (exec_result == JBIC_SUCCESS)
				{
					if (format_version == 2)
					{
						switch (exit_code)
						{
						case  0: exit_string = "Success"; break;
						case  1: exit_string = "Checking chain failure"; break;
						case  2: exit_string = "Reading IDCODE failure"; break;
						case  3: exit_string = "Reading USERCODE failure"; break;
						case  4: exit_string = "Reading UESCODE failure"; break;
						case  5: exit_string = "Entering ISP failure"; break;
						case  6: exit_string = "Unrecognized device"; break;
						case  7: exit_string = "Device revision is not supported"; break;
						case  8: exit_string = "Erase failure"; break;
						case  9: exit_string = "Device is not blank"; break;
						case 10: exit_string = "Device programming failure"; break;
						case 11: exit_string = "Device verify failure"; break;
						case 12: exit_string = "Read failure"; break;
						case 13: exit_string = "Calculating checksum failure"; break;
						case 14: exit_string = "Setting security bit failure"; break;
						case 15: exit_string = "Querying security bit failure"; break;
						case 16: exit_string = "Exiting ISP failure"; break;
						case 17: exit_string = "Performing system test failure"; break;
						default: exit_string = "Unknown exit code"; break;
						}
					}
					else
					{
						switch (exit_code)
						{
						case 0: exit_string = "Success"; break;
						case 1: exit_string = "Illegal initialization values"; break;
						case 2: exit_string = "Unrecognized device"; break;
						case 3: exit_string = "Device revision is not supported"; break;
						case 4: exit_string = "Device programming failure"; break;
						case 5: exit_string = "Device is not blank"; break;
						case 6: exit_string = "Device verify failure"; break;
						case 7: exit_string = "SRAM configuration failure"; break;
						default: exit_string = "Unknown exit code"; break;
						}
					}

					printString("Exit code = ");
					printHex(exit_code);
					printString(", ");
					printString(exit_string);
					printString("\r\n");
				}
				else if ((format_version == 2) &&
					(exec_result == JBIC_ACTION_NOT_FOUND))
				{
					if ((action == NULL) || (*action == '\0'))
					{
						printString("Error: no action specified for Jam STAPL file.\nProgram terminated.\n");
					}
					else
					{
						printString("Error: action \"");
						printString(action);
						printString("\" is not supported for this Jam STAPL file.\nProgram terminated.\n");
					}
				}
				else if (exec_result < MAX_ERROR_CODE)
				{
					printString("Error at address %ld: %s.\nProgram terminated.\n");
				}
				else
				{
					printString("Unknown error code: ");
					printHex(exec_result);
					printString("\r\n");
				}
			}
		}
	}

	if (jtag_hardware_initialized) 
	{
		close_jtag_hardware();
	}

	while(1)
	{
		;
	}
	
	return 0;
}


void initialize_jtag_hardware()
{

}

void close_jtag_hardware()
{

}

unsigned char ecd300GetJbcChar(unsigned long index)
{
}

void ecd300SetLongArray(unsigned long * pAddress, unsigned short index, unsigned long value)
{
}

unsigned long ecd300GetLongArray(unsigned long * pAddress, unsigned short index)
{
}

void ecd300SetCharArray(unsigned char * pAddress, unsigned short index, unsigned char value)
{
}

unsigned char ecd300GetCharArray(unsigned char * pAddress, unsigned short index)
{
}

