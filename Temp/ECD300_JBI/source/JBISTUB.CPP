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

typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
#define TRUE 1
#define FALSE 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
	#include <malloc.h>
	#define POINTER_ALIGNMENT sizeof(BYTE)
#include <time.h>
#include <conio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "jbiexprt.h"

/*
*	For Borland C compiler (16-bit), set the stack size
*/

/************************************************************************
*
*	Global variables
*/

/* file buffer for Jam STAPL ByteCode input file */
unsigned char *file_buffer = NULL;
long file_pointer = 0L;
long file_length = 0L;

/* delay count for one millisecond delay */
long one_ms_delay = 0L;

/* serial port interface available on all platforms */
BOOL jtag_hardware_initialized = FALSE;
char *serial_port_name = NULL;
BOOL specified_com_port = FALSE;
int com_port = -1;
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

BOOL verbose = FALSE;

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
	return jtagIo(tms, tdi, read_tdo);
}

void jbi_message(char *message_text)
{
	STUB_PRINT_1(message_text);
}

void jbi_export_integer(char *key, long value)
{
	if (verbose)
	{
		STUB_PRINT_3("Export: key = \"%s\", value = %ld\n", key, value);
	}
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
	char string[HEX_LINE_CHARS + 1];
	long i, offset;
	unsigned long size, line, lines, linebits, value, j, k;

	if (verbose)
	{
		if (count > HEX_LINE_BITS)
		{
			STUB_PRINT_3("Export: key = \"%s\", %ld bits, value = HEX\n", key, count);
			lines = (count + (HEX_LINE_BITS - 1)) / HEX_LINE_BITS;

			for (line = 0; line < lines; ++line)
			{
				if (line < (lines - 1))
				{
					linebits = HEX_LINE_BITS;
					size = HEX_LINE_CHARS;
					offset = count - ((line + 1) * HEX_LINE_BITS);
				}
				else
				{
					linebits = count - ((lines - 1) * HEX_LINE_BITS);
					size = (linebits + 3) / 4;
					offset = 0L;
				}

				string[size] = '\0';
				j = size - 1;
				value = 0;

				for (k = 0; k < linebits; ++k)
				{
					i = k + offset;
					if (data[i >> 3] & (1 << (i & 7))) value |= (1 << (i & 3));
					if ((i & 3) == 3)
					{
						string[j] = conv_to_hex(value);
						value = 0;
						--j;
					}
				}
				if ((k & 3) > 0) string[j] = conv_to_hex(value);

				STUB_PRINT_2("%s\n", string);
			}

		}
		else
		{
			size = (count + 3) / 4;
			string[size] = '\0';
			j = size - 1;
			value = 0;

			for (i = 0; i < count; ++i)
			{
				if (data[i >> 3] & (1 << (i & 7))) value |= (1 << (i & 3));
				if ((i & 3) == 3)
				{
					string[j] = conv_to_hex(value);
					value = 0;
					--j;
				}
			}
			if ((i & 3) > 0) string[j] = conv_to_hex(value);

			STUB_PRINT_4("Export: key = \"%s\", %ld bits, value = HEX %s\n",
				key, count, string);
		}
	}
}

void jbi_delay(long microseconds)
{
	stubDelay(microseconds);
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

void *jbi_malloc(unsigned int size)
{
       unsigned int n_bytes_to_allocate =size;
	unsigned char *ptr = 0;

	ptr = (unsigned char *) stubMalloc(n_bytes_to_allocate);

	return ptr;
}

void jbi_free(void *ptr)
{
	if(ptr != 0)
	{
		stubFree(ptr);
	}
}

void calibrate_delay(void)
{
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

static unsigned long _workSpacePool[160];


/************************************************************************/

int jp_main(unsigned char * fileBuffer, unsigned long fileLength, unsigned char * actionCommand, int bOutputInfo)
{
	BOOL help = FALSE;
	BOOL error = FALSE;
	char *filename = NULL;
	long offset = 0L;
	long error_address = 0L;
	JBI_RETURN_TYPE crc_result = JBIC_SUCCESS;
	JBI_RETURN_TYPE exec_result = JBIC_SUCCESS;
	unsigned short expected_crc = 0;
	unsigned short actual_crc = 0;
	char key[33] = {0};
	char value[257] = {0};
	int exit_status = 0;
	int arg = 0;
	int exit_code = 0;
	int format_version = 0;
	int time_delta = 0;
	char *workspace = NULL;
	char *action = NULL;
	char *init_list[10];
	int init_count = 0;
	long workspace_size = 0;
	char *exit_string = NULL;
	int reset_jtag = 1;
	int execute_program = 1;
	int action_count = 0;
	int procedure_count = 0;
	int index = 0;
	char *action_name = NULL;
	char *description = NULL;
	JBI_PROCINFO *procedure_list = NULL;
	JBI_PROCINFO *procptr = NULL;

	verbose = FALSE;

	init_list[0] = "DO_PROGRAM=1";
	init_list[0] = "DO_VERIFY=0";
	init_list[0] = "DO_SECURE=0";
	init_list[0] = 0;
	init_count=3;

	//set workspace
	workspace=(char *)_workSpacePool;
	workspace_size=160*4;

      	file_buffer=fileBuffer;
      	file_length=fileLength;
      	action=(char *)actionCommand;
	verbose = bOutputInfo?1:0;

	{
		if (exit_status == 0)
		{
			/*
			*	Check CRC
			*/
			crc_result = jbi_check_crc(file_buffer, file_length,
				&expected_crc, &actual_crc);

			if (verbose || (crc_result == JBIC_CRC_ERROR))
			{
				switch (crc_result)
				{
				case JBIC_SUCCESS:
					STUB_PRINT_2("CRC matched: CRC value = %04X\n", actual_crc);
					break;

				case JBIC_CRC_ERROR:
					STUB_PRINT_3("CRC mismatch: expected %04X, actual %04X\n",
						expected_crc, actual_crc);
					break;

				case JBIC_UNEXPECTED_END:
					STUB_PRINT_2("Expected CRC not found, actual CRC value = %04X\n",
						actual_crc);
					break;

				case JBIC_IO_ERROR:
					STUB_PRINT_1("Error: File format is not recognized.\n");
					exit(1);
					break;

				default:
					STUB_PRINT_2("CRC function returned error code %d\n", crc_result);
					break;
				}
			}

			if (verbose)
			{
				/*
				*	Display file format version
				*/
				jbi_get_file_info(file_buffer, file_length,
					&format_version, &action_count, &procedure_count);

				STUB_PRINT_2("File format is %s ByteCode format\n",
					(format_version == 2) ? "Jam STAPL" : "pre-standardized Jam 1.1");

				/*
				*	Dump out NOTE fields
				*/
				while (jbi_get_note(file_buffer, file_length,
					&offset, key, value, 256) == 0)
				{
					STUB_PRINT_3("NOTE \"%s\" = \"%s\"\n", key, value);
				}

				/*
				*	Dump the action table
				*/
				if ((format_version == 2) && (action_count > 0))
				{
					STUB_PRINT_1("\nActions available in this file:\n");

					for (index = 0; index < action_count; ++index)
					{
						jbi_get_action_info(file_buffer, file_length,
							index, &action_name, &description, &procedure_list);

						if (description == NULL)
						{
							STUB_PRINT_2("%s\n", action_name);
						}
						else
						{
							STUB_PRINT_3("%s \"%s\"\n", action_name, description);
						}

						procptr = procedure_list;
						while (procptr != NULL)
						{
							if (procptr->attributes != 0)
							{
								STUB_PRINT_3("    %s (%s)\n", procptr->name,
									(procptr->attributes == 1) ?
									"optional" : "recommended");
							}


							procedure_list = procptr->next;
							jbi_free(procptr);
							procptr = procedure_list;
						}
					}

					/* add a blank line before execution messages */
					if (execute_program) STUB_PRINT_1("\n");
				}
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

					STUB_PRINT_3("Exit code = %d... %s\n", exit_code, exit_string);
				}
				else if ((format_version == 2) &&
					(exec_result == JBIC_ACTION_NOT_FOUND))
				{
					if ((action == NULL) || (*action == '\0'))
					{
						STUB_PRINT_1("Error: no action specified for Jam STAPL file.\nProgram terminated.\n");
					}
					else
					{
						STUB_PRINT_2("Error: action \"%s\" is not supported for this Jam STAPL file.\nProgram terminated.\n", action);
					}
				}
				else if (exec_result < MAX_ERROR_CODE)
				{
					STUB_PRINT_3("Error at address %ld: %s.\nProgram terminated.\n",
						error_address, error_text[exec_result]);
				}
				else
				{
					STUB_PRINT_2("Unknown error code %ld\n", exec_result);
				}
			}
		}
	}

	if (jtag_hardware_initialized) close_jtag_hardware();

	if (workspace != NULL) jbi_free(workspace);
	if (file_buffer != NULL) jbi_free(file_buffer);

	return (exit_status);
}


void initialize_jtag_hardware()
{
    jtagInit();
}


void close_jtag_hardware()
{
	jtagRelease();
}


