/****************************************************************************/
/*																			*/
/*	Module:			jamstub.c												*/
/*																			*/
/*					Copyright (C) Altera Corporation 1997-2000				*/
/*																			*/
/*	Description:	Main source file for stand-alone JAM test utility.		*/
/*																			*/
/*					Supports Altera ByteBlaster hardware download cable		*/
/*					on Windows 95 and Windows NT operating systems.			*/
/*					(A device driver is required for Windows NT.)			*/
/*																			*/
/*					Also supports BitBlaster hardware download cable on		*/
/*					Windows 95, Windows NT, and UNIX platforms.				*/
/*																			*/
/*	Revisions:		1.1	added dynamic memory allocation						*/
/*					1.11 added multi-page memory allocation for file_buffer */
/*                    to permit DOS version to read files larger than 64K   */
/*					1.2 fixed control port initialization for ByteBlaster	*/
/*					2.2 updated usage message, added support for alternate	*/
/*					  cable types, moved porting macros in jamport.h,		*/
/*					  fixed bug in delay calibration code for 16-bit port	*/
/*					2.3 added support for static memory						*/
/*						fixed /W4 warnings									*/
/*																			*/
/****************************************************************************/

#include "jamport.h"
#include "jamexprt.h"

#include "stub_implement.h"

/************************************************************************
*
*	Global variables
*/

/* file buffer for JAM input file */
static char *file_buffer = 0;
static long file_pointer = 0L;
static long file_length = 0L;

/* serial port interface available on all platforms */
static int jtag_hardware_initialized = 0;
static void initialize_jtag_hardware(void);
static void close_jtag_hardware(void);

static int verbose = 0;

/************************************************************************
*
*	Customized interface functions for JAM interpreter I/O:
*
*	jam_getc()
*	jam_seek()
*	jam_jtag_io()
*	jam_message()
*	jam_delay()
*/

int jam_getc(void)
{
	int ch = -1;

	if (file_pointer < file_length)
	{
		ch = (int) file_buffer[file_pointer++];
	}

	return (ch);
}

int jam_seek(long offset)
{
	int return_code = -1;

	if ((offset >= 0L) && (offset < file_length))
	{
		file_pointer = offset;
		return_code = 0;
	}

	return (return_code);
}

int jam_jtag_io(int tms, int tdi, int read_tdo)
{
	int data = 0;
	int tdo = 0;
	int i = 0;
	int result = 0;
	char ch_data = 0;

	if (!jtag_hardware_initialized)
	{
		initialize_jtag_hardware();
		jtag_hardware_initialized = 1;
	}
/*
        setJtagInput(0, tms, tdi);
        if(read_tdo)
            tdo=getJtagOutput()?1:0;

        setJtagInput(1, tms, tdi);
        setJtagInput(0, tms, tdi);
*/
	tdo=jtagIo(tms, tdi, read_tdo);

	return (tdo);
}

void jam_message(char *message_text)
{
	STUB_PRINT_1(message_text);
}

void jam_export_integer(char *key, long value)
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

void jam_export_boolean_array(char *key, unsigned char *data, long count)
{
	unsigned long size, line, lines, linebits, value, j, k;
	char string[HEX_LINE_CHARS + 1];
	long i, offset;

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

void jam_delay(long microseconds)
{
	stubDelay(microseconds);
}

int jam_vector_map
(
	int signal_count,
	char **signals
)
{
	return -1;
}

int jam_vector_io
(
	int signal_count,
	long *dir_vect,
	long *data_vect,
	long *capture_vect
)
{
	
	return -1;
}

int jam_set_frequency(long hertz)
{
	if (verbose)
	{
		STUB_PRINT_2("Frequency: %ld Hz\n", hertz);
	}

    stubSetClockFrequency(hertz);
    
    return (0);
}

void *jam_malloc(unsigned int size)
{	
       unsigned int n_bytes_to_allocate =size;
	unsigned char *ptr = 0;

	ptr = (unsigned char *) stubMalloc(n_bytes_to_allocate);

	return ptr;
}

void jam_free(void *ptr)
{
	if(ptr != 0)
	{
		stubFree(ptr);
	}
}

char *error_text[] =
{
/* JAMC_SUCCESS            0 */ "success",
/* JAMC_OUT_OF_MEMORY      1 */ "out of memory",
/* JAMC_IO_ERROR           2 */ "file access error",
/* JAMC_SYNTAX_ERROR       3 */ "syntax error",
/* JAMC_UNEXPECTED_END     4 */ "unexpected end of file",
/* JAMC_UNDEFINED_SYMBOL   5 */ "undefined symbol",
/* JAMC_REDEFINED_SYMBOL   6 */ "redefined symbol",
/* JAMC_INTEGER_OVERFLOW   7 */ "integer overflow",
/* JAMC_DIVIDE_BY_ZERO     8 */ "divide by zero",
/* JAMC_CRC_ERROR          9 */ "CRC mismatch",
/* JAMC_INTERNAL_ERROR    10 */ "internal error",
/* JAMC_BOUNDS_ERROR      11 */ "bounds error",
/* JAMC_TYPE_MISMATCH     12 */ "type mismatch",
/* JAMC_ASSIGN_TO_CONST   13 */ "assignment to constant",
/* JAMC_NEXT_UNEXPECTED   14 */ "NEXT unexpected",
/* JAMC_POP_UNEXPECTED    15 */ "POP unexpected",
/* JAMC_RETURN_UNEXPECTED 16 */ "RETURN unexpected",
/* JAMC_ILLEGAL_SYMBOL    17 */ "illegal symbol name",
/* JAMC_VECTOR_MAP_FAILED 18 */ "vector signal name not found",
/* JAMC_USER_ABORT        19 */ "execution cancelled",
/* JAMC_STACK_OVERFLOW    20 */ "stack overflow",
/* JAMC_ILLEGAL_OPCODE    21 */ "illegal instruction code",
/* JAMC_PHASE_ERROR       22 */ "phase error",
/* JAMC_SCOPE_ERROR       23 */ "scope error",
/* JAMC_ACTION_NOT_FOUND  24 */ "action not found",
};

#define MAX_ERROR_CODE (int)((sizeof(error_text)/sizeof(error_text[0]))+1)

/************************************************************************/

int jp_main(unsigned char * fileBuffer, unsigned long fileLength, unsigned char * actionCommand, int bOutputInfo)
{
	int error = 0;
	char *filename = 0;
	long offset = 0L;
	long error_line = 0L;
	JAM_RETURN_TYPE crc_result = JAMC_SUCCESS;
	JAM_RETURN_TYPE exec_result = JAMC_SUCCESS;
	unsigned short expected_crc = 0;
	unsigned short actual_crc = 0;
	char key[33] = {0};
	char value[257] = {0};
	int exit_status = 0;
	int exit_code = 0;
	int format_version = 0;
	char *workspace = 0;
	char *action = 0;
	char *init_list[10];
	int init_count = 0;
	long workspace_size = 0;
	char *exit_string = 0;
	int reset_jtag = 1;


	init_list[0] = 0;

      file_buffer=(char *)fileBuffer;
      file_length=fileLength;
      action=(char *)actionCommand;
	verbose = bOutputInfo?1:0;

	{
		if (exit_status == 0)
		{
			/*
			*	Check CRC
			*/
			crc_result = jam_check_crc(
				file_buffer, file_length,
				&expected_crc, &actual_crc);

			if (verbose || (crc_result == JAMC_CRC_ERROR))
			{
				switch (crc_result)
				{
				case JAMC_SUCCESS:
					STUB_PRINT_2("CRC matched: CRC value = %04X\n", actual_crc);
					break;

				case JAMC_CRC_ERROR:
					STUB_PRINT_3("CRC mismatch: expected %04X, actual %04X\n",
						expected_crc, actual_crc);
					break;

				case JAMC_UNEXPECTED_END:
					STUB_PRINT_2("Expected CRC not found, actual CRC value = %04X\n",
						actual_crc);
					break;

				default:
					STUB_PRINT_2("CRC function returned error code %d\n", crc_result);
					break;
				}
			}

			/*
			*	Dump out NOTE fields
			*/
			if (verbose)
			{
				while (jam_get_note(
					file_buffer, file_length,
					&offset, key, value, 256) == 0)
				{
					STUB_PRINT_3("NOTE \"%s\" = \"%s\"\n", key, value);
				}
			}

			/*
			*	Execute the JAM program
			*/
			exec_result = jam_execute(
				file_buffer, file_length,
				workspace, workspace_size, action, init_list,
				reset_jtag, &error_line, &exit_code, &format_version);

			if (exec_result == JAMC_SUCCESS)
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
				(exec_result == JAMC_ACTION_NOT_FOUND))
			{
				if ((action == 0) || (*action == '\0'))
				{
					STUB_PRINT_1("Error: no action specified for Jam file.\nProgram terminated.\n");
				}
				else
				{
					STUB_PRINT_2("Error: action \"%s\" is not supported for this Jam file.\nProgram terminated.\n", action);
				}
			}
			else if (exec_result < MAX_ERROR_CODE)
			{
				STUB_PRINT_3("Error on line %ld: %s.\nProgram terminated.\n",
					error_line, error_text[exec_result]);
			}
			else
			{
				STUB_PRINT_2("Unknown error code %ld\n", exec_result);
			}
		}
	}

	if (jtag_hardware_initialized) 
        close_jtag_hardware();

	if (workspace != 0) 
        jam_free(workspace);

	return (exit_status);
}

static void initialize_jtag_hardware()
{
    jtagInit();
}

static void close_jtag_hardware()
{
	jtagRelease();
}


