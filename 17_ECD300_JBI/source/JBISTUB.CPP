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

#if ( _MSC_VER >= 800 )
#pragma warning(disable:4115)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#pragma warning(disable:4514)
#endif

#include "jbiport.h"
#include "stub_implement.h"

typedef long BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
#define TRUE 1
#define FALSE 0

#include "jbiexprt.h"

/*
*	This structure stores information about each available vector signal
*/
struct VECTOR_LIST_STRUCT
{
	char *signal_name;
	long  hardware_bit;
	long  vector_index;
};

struct VECTOR_LIST_STRUCT vector_list[] =
{
	/* add a record here for each vector signal */
	{ "", 0, -1 }
};

#define VECTOR_SIGNAL_COUNT ((long)(sizeof(vector_list)/sizeof(vector_list[0])))

void initialize_jtag_hardware()
{
    stubJtagInit();
}


void close_jtag_hardware()
{
	stubJtagRelease();
}

/************************************************************************
*
*	Customized interface functions for Jam STAPL ByteCode Player I/O:
*
*	jbi_jtag_io()
*	jbi_message()
*	jbi_delay()
*/

long jbi_jtag_io(long tms, long tdi, long read_tdo)
{
	return stubJtagIo(tms, tdi, read_tdo);
}

void jbi_message(char *message_text)
{
	stubPrintString(message_text);
	stubPrintString("\r\n");
}

void jbi_export_integer(char *key, long value)
{
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
	stubDelay(microseconds);
}


void *jbi_malloc(unsigned long size)
{
       return stubMalloc(size);
}

unsigned long jbi_malloc_ex(unsigned long size)
{
	return stubMallocExternalRam(size);
}

void jbi_free(void *ptr)
{
	if(ptr != 0)
	{
		stubFree(ptr);
	}
}

#if 0
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
#endif

static unsigned long _workSpacePool[160];
static unsigned short _workSpacePoolSize=160*4; //in byte.


/************************************************************************/

long jp_main(unsigned long fileBufferAddress, unsigned long fileLength, unsigned char * actionCommand, long bOutputInfo)
{
	long error_address = 0L;
	JBI_RETURN_TYPE crc_result = JBIC_SUCCESS;
	JBI_RETURN_TYPE exec_result = JBIC_SUCCESS;
	unsigned short expected_crc = 0;
	unsigned short actual_crc = 0;
	long exit_code = 0;
	long format_version = 0;
	char *action = 0;
	char *exit_string = 0;
	long reset_jtag = 1;

	initialize_jtag_hardware();

	action=(char *)actionCommand;

 	crc_result = jbi_check_crc(fileBufferAddress, fileLength, &expected_crc, &actual_crc);

	if(crc_result == JBIC_CRC_ERROR)
	{
		switch (crc_result)
		{
			case JBIC_SUCCESS:
				stubPrintString("CRC matched: CRC value = ");
				stubPrintHex(actual_crc>>8);
				stubPrintString(", ");
				stubPrintHex(actual_crc);
				stubPrintString("\r\n");
				break;

			case JBIC_CRC_ERROR:
				stubPrintString("CRC mismatch: expected ");
				stubPrintHex(expected_crc>>8);
				stubPrintString(", ");
				stubPrintHex(expected_crc);
				stubPrintString(", actual ");
				stubPrintHex(actual_crc>>8);
				stubPrintString(", ");
				stubPrintHex(actual_crc);
				stubPrintString("\r\n");
				break;

			case JBIC_UNEXPECTED_END:
				stubPrintString("Expected CRC not found, actual CRC value = ");
				stubPrintHex(actual_crc>>8);
				stubPrintString(", ");
				stubPrintHex(actual_crc);
				stubPrintString("\r\n");
				break;

			case JBIC_IO_ERROR:
				stubPrintString("Error: File format is not recognized.\r\n");
				break;

			default:
				stubPrintString("CRC function returned error code  ");
				stubPrintHex(crc_result>>8);
				stubPrintString(", ");
				stubPrintHex(crc_result);
				stubPrintString("\r\n");
				break;
		}
	}

	exec_result = jbi_execute(fileBufferAddress, fileLength, (char *)_workSpacePool,
					_workSpacePoolSize, action, 0, reset_jtag,
					&error_address, &exit_code, &format_version);
	if (exec_result == JBIC_SUCCESS)
	{
					/*
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
					*/
					stubPrintString("Exit code = ");
					stubPrintHex(exit_code);
					stubPrintString("... ");
					//stubPrintString(exit_string);
					stubPrintString("\r\n");
	}
	else if ((format_version == 2) &&
			(exec_result == JBIC_ACTION_NOT_FOUND))
	{
					if ((action == 0) || (*action == '\0'))
					{
						stubPrintString("Error: no action specified for Jam STAPL file.\nProgram terminated.\r\n");
					}
					else
					{
						stubPrintString("Error: action \"");
						stubPrintString(action);
						stubPrintString("\" is not supported for this Jam STAPL file.\r\nProgram terminated.\r\n");
					}
	}
	else if (exec_result < 25)
	{
					stubPrintString("Error at address ");
					stubPrintHex(error_address>>24);
					stubPrintString(", ");
					stubPrintHex(error_address>>16);
					stubPrintString(", ");
					stubPrintHex(error_address>>8);
					stubPrintString(", ");
					stubPrintHex(error_address>>0);
					stubPrintString(": ");
					//stubPrintString(error_text[exec_result]);
					stubPrintString(".\r\nProgram terminated.\r\n");
	}
	else
	{
					stubPrintString("Unknown error code ");
					stubPrintHex(exec_result);
					stubPrintString("\r\n");
	}

	close_jtag_hardware();

	return (0);
}





