/****************************************************************************/
/*																			*/
/*	Module:			jbimain.c												*/
/*																			*/
/*					Copyright (C) Altera Corporation 1998-2001				*/
/*																			*/
/*	Description:	Jam STAPL ByteCode Player (Interpreter)					*/
/*																			*/
/*	Revisions:		2.2  fixed /W4 warnings									*/
/*					2.0  added support for STAPL ByteCode format			*/
/*																			*/
/****************************************************************************/
#include "jbi_player.h"

#include "jbiport.h"
#include "jbiexprt.h"
#include "jbijtag.h"
#include "jbicomp.h"

/****************************************************************************/
/*																			*/
/*	MACROS																	*/
/*																			*/
/****************************************************************************/

#define NULL 0

#define JBI_STACK_SIZE 128

#define JBIC_MESSAGE_LENGTH 1024

/*
*	This macro checks if enough parameters are available on the stack. The
*	argument is the number of parameters needed.
*/
#define IF_CHECK_STACK(x) \
	if (stack_ptr < (long) (x)) \
	{ \
		status = JBIC_STACK_OVERFLOW; \
	} \
	else

/*
*	This macro checks if a code address is inside the code section
*/
#define CHECK_PC \
	if ((pc < code_section) || (pc >= debug_section)) \
	{ \
		status = JBIC_BOUNDS_ERROR; \
	}

/****************************************************************************/
/*																			*/
/*	GLOBAL VARIABLES														*/
/*																			*/
/****************************************************************************/


/****************************************************************************/
/*																			*/
/*	UTILITY FUNCTIONS														*/
/*																			*/
/****************************************************************************/

long jbi_strlen(char *string)
{
	long len = 0;

	while (string[len] != '\0') ++len;

	return (len);
}

long jbi_strlen_ex(unsigned long string_int8Ptr)
{
	long len = 0;

	while (getInt8Array(string_int8Ptr, len) != '\0') ++len;

	return (len);
}

long jbi_atol(char *buffer)
{
	long result = 0L;
	long index = 0;

	while ((buffer[index] >= '0') && (buffer[index] <= '9'))
	{
		result = (result * 10) + (buffer[index] - '0');
		++index;
	}

	return (result);
}

void jbi_ltoa(char *buffer, long number)
{
	long index = 0;
	long rev_index = 0;
	char reverse[32];

	if (number < 0L)
	{
		buffer[index++] = '-';
		number = 0 - number;
	}
	else if (number == 0)
	{
		buffer[index++] = '0';
	}

	while (number != 0)
	{
		reverse[rev_index++] = (char) ((number % 10) + '0');
		number /= 10;
	}

	while (rev_index > 0)
	{
		buffer[index++] = reverse[--rev_index];
	}

	buffer[index] = '\0';
}

void jbi_ltoa_ex(unsigned long buffer_int8Ptr, long number)
{
	long index = 0;
	long rev_index = 0;
	char reverse[32];

	if (number < 0L)
	{
		setInt8Array(buffer_int8Ptr, index++, '-');
		number = 0 - number;
	}
	else if (number == 0)
	{
		setInt8Array(buffer_int8Ptr, index++, '0');
	}

	while (number != 0)
	{
		reverse[rev_index++] = (char) ((number % 10) + '0');
		number /= 10;
	}

	while (rev_index > 0)
	{
		setInt8Array(buffer_int8Ptr, index++, reverse[--rev_index]);
	}

	setInt8Array(buffer_int8Ptr, index, '\0');
}

char jbi_toupper(char ch)
{
	return ((char) (((ch >= 'a') && (ch <= 'z')) ? (ch + 'A' - 'a') : ch));
}

long jbi_stricmp(char *left, char *right)
{
	long result = 0;
	char l, r;

	do
	{
		l = jbi_toupper(*left);
		r = jbi_toupper(*right);
		result = l - r;
		++left;
		++right;
	}
	while ((result == 0) && (l != '\0') && (r != '\0'));

	return (result);
}

long jbi_stricmp_ex(char *left, unsigned long right_int8Ptr)
{
	long result = 0;
	char l, r;

	do
	{
		l = jbi_toupper(*left);
		r = jbi_toupper(getInt8(right_int8Ptr));
		result = l - r;
		++left;
		++right_int8Ptr;
	}
	while ((result == 0) && (l != '\0') && (r != '\0'));

	return (result);
}

void jbi_strncpy(char *left, char *right, long count)
{
	char ch;

	do
	{
		ch = *right;
		*left = ch;
		++left;
		++right;
		--count;
	}
	while ((ch != '\0') && (count != 0));
}

void jbi_strncpy_ex(unsigned long leftInt8Ptr, unsigned long rightInt8Ptr, long count)
{
	char ch;

	do
	{
		ch = getInt8(rightInt8Ptr);
		setInt8(leftInt8Ptr, ch);
		++leftInt8Ptr;
		++rightInt8Ptr;
		--count;
	}
	while ((ch != '\0') && (count != 0));
}

void jbi_make_dword(unsigned char *buf, unsigned long num)
{
	buf[0] = (unsigned char) num;
	buf[1] = (unsigned char) (num >> 8L);
	buf[2] = (unsigned char) (num >> 16L);
	buf[3] = (unsigned char) (num >> 24L);
}

unsigned long jbi_get_dword(unsigned char *buf)
{
	return
		(((unsigned long) buf[0]) |
		(((unsigned long) buf[1]) << 8L) |
		(((unsigned long) buf[2]) << 16L) |
		(((unsigned long) buf[3]) << 24L));
}

unsigned long jbi_get_dword_ex(unsigned long bufUint8Ptr)
{
	return
		(((unsigned long) getUint8(bufUint8Ptr+0)) |
		(((unsigned long) getUint8(bufUint8Ptr+1)) << 8L) |
		(((unsigned long) getUint8(bufUint8Ptr+2)) << 16L) |
		(((unsigned long) getUint8(bufUint8Ptr+3)) << 24L));
}

/****************************************************************************/
/*																			*/

static	long stack[JBI_STACK_SIZE] = {0};


JBI_RETURN_TYPE jbi_execute
(
	PROGRAM_PTR program,
	long program_size,
	char *workspace,
	long workspace_size,
	char *action,
	char **init_list,
	long reset_jtag,
	long *error_address,
	long *exit_code,
	long *format_version
)

/*																			*/
/*	Description:															*/
/*																			*/
/*	Returns:																*/
/*																			*/
/****************************************************************************/
{
	JBI_RETURN_TYPE status = JBIC_SUCCESS;
	unsigned long first_word = 0L;
	unsigned long action_table = 0L;
	unsigned long proc_table = 0L;
	unsigned long string_table = 0L;
	unsigned long symbol_table = 0L;
	unsigned long data_section = 0L;
	unsigned long code_section = 0L;
	unsigned long debug_section = 0L;
	unsigned long action_count = 0L;
	unsigned long proc_count = 0L;
	unsigned long symbol_count = 0L;
	unsigned long variables = NULL;
	unsigned long variable_size = NULL;
	unsigned long attributes = NULL;
	unsigned long proc_attributes = NULL;
	unsigned long pc;
	unsigned long opcode_address;
	unsigned long args[3];
	unsigned long opcode;
	unsigned long name_id;
	unsigned char charbuf[4];
	long long_temp;
	unsigned long variable_id;
	unsigned long charptr_temp;
	unsigned long charptr_temp2;
	unsigned long longptr_temp;
	long version = 0;
	long delta = 0;
	long stack_ptr = 0;
	unsigned long arg_count;
	long done = 0;
	long bad_opcode = 0;
	unsigned long count;
	unsigned long index;
	unsigned long index2;
	long long_count;
	long long_index;
	long long_index2;
	unsigned long i;
	unsigned long j;
	unsigned long uncompressed_size;
	unsigned long offset;
	unsigned long value;
	long current_proc = 0;
	long reverse;
	unsigned long message_buffer;

	char *name;

	jbi_workspace = workspace;
	jbi_workspace_size = workspace_size;

	message_buffer=jbi_malloc_ex(JBIC_MESSAGE_LENGTH + 1);
	if (message_buffer == NULL) status = JBIC_OUT_OF_MEMORY;
	
	/*
	*	Read header information
	*/
	if (program_size > 52L)
	{
		first_word    = GET_DWORD(0);
		version = (long) (first_word & 1L);
		*format_version = version + 1;
		delta = version * 8;

		action_table  = GET_DWORD(4);
		proc_table    = GET_DWORD(8);
		string_table  = GET_DWORD(4 + delta);
		symbol_table  = GET_DWORD(16 + delta);
		data_section  = GET_DWORD(20 + delta);
		code_section  = GET_DWORD(24 + delta);
		debug_section = GET_DWORD(28 + delta);
		action_count  = GET_DWORD(40 + delta);
		proc_count    = GET_DWORD(44 + delta);
		symbol_count  = GET_DWORD(48 + (2 * delta));
	}

	if ((first_word != 0x4A414D00L) && (first_word != 0x4A414D01L))
	{
		done = 1;
		status = JBIC_IO_ERROR;
	}

	if ((status == JBIC_SUCCESS) && (symbol_count > 0))
	{
		variables = jbi_malloc_ex(
			(unsigned long) symbol_count * sizeof(long));

		if (variables == NULL) status = JBIC_OUT_OF_MEMORY;

		if (status == JBIC_SUCCESS)
		{
			variable_size = jbi_malloc_ex(
				(unsigned long) symbol_count * sizeof(long));

			if (variable_size == NULL) status = JBIC_OUT_OF_MEMORY;
		}

		if (status == JBIC_SUCCESS)
		{
			attributes = jbi_malloc_ex((unsigned long) symbol_count);

			if (attributes == NULL) status = JBIC_OUT_OF_MEMORY;
		}

		if ((status == JBIC_SUCCESS) && (version > 0))
		{
			proc_attributes = jbi_malloc_ex((unsigned long) proc_count);

			if (proc_attributes == NULL) status = JBIC_OUT_OF_MEMORY;
		}

		if (status == JBIC_SUCCESS)
		{
			delta = version * 2;

			for (i = 0; i < (unsigned long) symbol_count; ++i)
			{
				offset = (unsigned long) (symbol_table + ((11 + delta) * i));

				value = GET_DWORD(offset + 3 + delta);

				setInt8Array(attributes, i, GET_BYTE(offset));

				/* use bit 7 of attribute byte to indicate that this buffer */
				/* was dynamically allocated and should be freed later */
				setInt8Array(attributes, i, getInt8Array(attributes, i)&0x7f);

				setInt32Array(variable_size, i, GET_DWORD(offset + 7 + delta));

				/*
				*	Attribute bits:
				*	bit 0:	0 = read-only, 1 = read-write
				*	bit 1:	0 = not compressed, 1 = compressed
				*	bit 2:	0 = not initialized, 1 = initialized
				*	bit 3:	0 = scalar, 1 = array
				*	bit 4:	0 = Boolean, 1 = integer
				*	bit 5:	0 = declared variable,
				*			1 = compiler created temporary variable
				*/

				if ((getInt8Array(attributes, i) & 0x0c) == 0x04)
				{
					/* initialized scalar variable */
					setInt32Array(variables, i, value);
				}
				else if ((getInt8Array(attributes, i) & 0x1e) == 0x0e)
				{
					/* initialized compressed Boolean array */
					uncompressed_size = jbi_get_dword_ex(
						program + data_section + value);

					/* allocate a buffer for the uncompressed data */
					setInt32Array(variables, i, jbi_malloc_ex(uncompressed_size));

					if (getInt32Array(variables, i) == 0L)
					{
						status = JBIC_OUT_OF_MEMORY;
					}
					else
					{
						/* set flag so buffer will be freed later */
						setInt8Array(attributes, i, getInt8Array(attributes, i) |0x80);

						/* uncompress the data */
						if (jbi_uncompress_ex(
							program+data_section + value,
							getInt32Array(variable_size, i),
							getInt32Array(variables, i),
							uncompressed_size,
							version)
							!= uncompressed_size)
						{
							/* decompression failed */
							status = JBIC_IO_ERROR;
						}
						else
						{
							setInt32Array(variable_size, i, uncompressed_size * 8L);
						}
					}
				}
				else if ((getInt8Array(attributes, i) & 0x1e) == 0x0c)
				{
					/* initialized Boolean array */
					setInt32Array(variables, i, value + data_section + (long) program);
				}
				else if ((getInt8Array(attributes, i) & 0x1c) == 0x1c)
				{
					/* initialized integer array */
					setInt32Array(variables, i, value + data_section);
				}
				else if ((getInt8Array(attributes, i) & 0x0c) == 0x08)
				{
					/* uninitialized array */

					/* flag attributes so that memory is freed */
					setInt8Array(attributes, i, getInt8Array(attributes, i) |0x80);

					if (getInt32Array(variable_size, i) > 0)
					{
						unsigned long size;

						if (getInt8Array(attributes, i) & 0x10)
						{
							/* integer array */
							size = (unsigned long)
								(getInt32Array(variable_size, i) * sizeof(long));
						}
						else
						{
							/* Boolean array */
							size = (unsigned long)
								((getInt32Array(variable_size, i) + 7L) / 8L);
						}

						setInt32Array(variables, i, jbi_malloc_ex(size));

						if (getInt32Array(variables, i) == NULL)
						{
							status = JBIC_OUT_OF_MEMORY;
						}
						else
						{
							/* zero out memory */
							for (j = 0; j < size; ++j)
							{
								setUint8(getInt32Array(variables, i)+j, 0);
							}
						}
					}
					else
					{
						setInt32Array(variables, i, 0);
					}
				}
				else
				{
					setInt32Array(variables, i, 0);
				}
			}
		}
	}


	if (status != JBIC_SUCCESS) done = 1;

	jbi_init_jtag();

	pc = code_section;
	setInt8Array(message_buffer, 0, '\0');

	/*
	*	For JBC version 2, we will execute the procedures corresponding to
	*	the selected ACTION
	*/
	if (version > 0)
	{
		if (action == NULL)
		{
			status = JBIC_ACTION_NOT_FOUND;
			done = 1;
		}
		else
		{
			unsigned long actionNamePtr;
			long action_found = 0;

			for (i = 0; (i < action_count) && !action_found; ++i)
			{
				name_id = GET_DWORD(action_table + (12 * i));

				actionNamePtr = program + string_table + name_id;

				if (jbi_stricmp_ex(action, actionNamePtr) == 0)
				{
					action_found = 1;
					current_proc = (long) GET_DWORD(action_table + (12 * i) + 8);
				}
			}

			if (!action_found)
			{
				status = JBIC_ACTION_NOT_FOUND;
				done = 1;
			}
		}

		if (status == JBIC_SUCCESS)
		{
			long first_time = 1;
			i = current_proc;
			while ((i != 0) || first_time)
			{
				first_time = 0;
				/* check procedure attribute byte */
				setUint8Array(proc_attributes, i, (unsigned char)
					(GET_BYTE(proc_table + (13 * i) + 8) & 0x03));

				i = (unsigned long) GET_DWORD(proc_table + (13 * i) + 4);
			}

			/*
			*	Set current_proc to the first procedure to be executed
			*/
			i = current_proc;
			while ((i != 0) &&
				((getUint8Array(proc_attributes, i) == 1) ||
				((getUint8Array(proc_attributes, i) & 0xc0) == 0x40)))
			{
				i = (unsigned long) GET_DWORD(proc_table + (13 * i) + 4);
			}

			if ((i != 0) || ((i == 0) && (current_proc == 0) &&
				((getUint8Array(proc_attributes, 0) != 1) &&
				((getUint8Array(proc_attributes, 0) & 0xc0) != 0x40))))
			{
				current_proc = i;
				pc = code_section + GET_DWORD(proc_table + (13 * i) + 9);
				CHECK_PC;
			}
			else
			{
				/* there are no procedures to execute! */
				done = 1;
			}
		}
	}

	setInt8Array(message_buffer, 0, '\0');

	while (!done)
	{
		opcode = (unsigned long) (GET_BYTE(pc) & 0xff);
		opcode_address = pc;
		++pc;

		arg_count = (opcode >> 6) & 3;
		for (i = 0; i < arg_count; ++i)
		{
			args[i] = GET_DWORD(pc);
			pc += 4;
		}

		switch (opcode)
		{
		case 0x00: /* NOP  */
			/* do nothing */
			break;

		case 0x01: /* DUP  */
			IF_CHECK_STACK(1)
			{
				stack[stack_ptr] = stack[stack_ptr - 1];
				++stack_ptr;
			}
			break;

		case 0x02: /* SWP  */
			IF_CHECK_STACK(2)
			{
				long_temp = stack[stack_ptr - 2];
				stack[stack_ptr - 2] = stack[stack_ptr - 1];
				stack[stack_ptr - 1] = long_temp;
			}
			break;

		case 0x03: /* ADD  */
			IF_CHECK_STACK(2)
			{
				--stack_ptr;
				stack[stack_ptr - 1] += stack[stack_ptr];
			}
			break;

		case 0x04: /* SUB  */
			IF_CHECK_STACK(2)
			{
				--stack_ptr;
				stack[stack_ptr - 1] -= stack[stack_ptr];
			}
			break;

		case 0x05: /* MULT */
			IF_CHECK_STACK(2)
			{
				--stack_ptr;
				stack[stack_ptr - 1] *= stack[stack_ptr];
			}
			break;

		case 0x06: /* DIV  */
			IF_CHECK_STACK(2)
			{
				--stack_ptr;
				stack[stack_ptr - 1] /= stack[stack_ptr];
			}
			break;

		case 0x07: /* MOD  */
			IF_CHECK_STACK(2)
			{
				--stack_ptr;
				stack[stack_ptr - 1] %= stack[stack_ptr];
			}
			break;

		case 0x08: /* SHL  */
			IF_CHECK_STACK(2)
			{
				--stack_ptr;
				stack[stack_ptr - 1] <<= stack[stack_ptr];
			}
			break;

		case 0x09: /* SHR  */
			IF_CHECK_STACK(2)
			{
				--stack_ptr;
				stack[stack_ptr - 1] >>= stack[stack_ptr];
			}
			break;

		case 0x0A: /* NOT  */
			IF_CHECK_STACK(1)
			{
				stack[stack_ptr - 1] ^= (-1L);
			}
			break;

		case 0x0B: /* AND  */
			IF_CHECK_STACK(2)
			{
				--stack_ptr;
				stack[stack_ptr - 1] &= stack[stack_ptr];
			}
			break;

		case 0x0C: /* OR   */
			IF_CHECK_STACK(2)
			{
				--stack_ptr;
				stack[stack_ptr - 1] |= stack[stack_ptr];
			}
			break;

		case 0x0D: /* XOR  */
			IF_CHECK_STACK(2)
			{
				--stack_ptr;
				stack[stack_ptr - 1] ^= stack[stack_ptr];
			}
			break;

		case 0x0E: /* INV */
			IF_CHECK_STACK(1)
			{
				stack[stack_ptr - 1] = stack[stack_ptr - 1] ? 0L : 1L;
			}
			break;

		case 0x0F: /* GT   */
			IF_CHECK_STACK(2)
			{
				--stack_ptr;
				stack[stack_ptr - 1] =
					(stack[stack_ptr - 1] > stack[stack_ptr]) ? 1L : 0L;
			}
			break;

		case 0x10: /* LT   */
			IF_CHECK_STACK(2)
			{
				--stack_ptr;
				stack[stack_ptr - 1] =
					(stack[stack_ptr - 1] < stack[stack_ptr]) ? 1L : 0L;
			}
			break;

		case 0x11: /* RET  */
			if ((version > 0) && (stack_ptr == 0))
			{
				/*
				*	We completed one of the main procedures of an ACTION.
				*	Find the next procedure to be executed and jump to it.
				*	If there are no more procedures, then EXIT.
				*/
				i = (unsigned long) GET_DWORD(proc_table + (13 * current_proc) + 4);
				while ((i != 0) &&
					((getUint8Array(proc_attributes, i) == 1) ||
					((getUint8Array(proc_attributes, i) & 0xc0) == 0x40)))
				{
					i = (unsigned long) GET_DWORD(proc_table + (13 * i) + 4);
				}

				if (i == 0)
				{
					/* there are no procedures to execute! */
					done = 1;
					*exit_code = 0;	/* success */
				}
				else
				{
					current_proc = i;
					pc = code_section + GET_DWORD(proc_table + (13 * i) + 9);
					CHECK_PC;
				}
			}
			else IF_CHECK_STACK(1)
			{
				pc = stack[--stack_ptr] + code_section;
				CHECK_PC;
				if (pc == code_section)
				{
					status = JBIC_BOUNDS_ERROR;
				}
			}
			break;

		case 0x12: /* CMPS */
			/*
			*	Array short compare
			*	...stack 0 is source 1 value
			*	...stack 1 is source 2 value
			*	...stack 2 is mask value
			*	...stack 3 is count
			*/
			IF_CHECK_STACK(4)
			{
				long a = stack[--stack_ptr];
				long b = stack[--stack_ptr];
				long_temp = stack[--stack_ptr];
				count = (unsigned long) stack[stack_ptr - 1];

				if ((count < 1) || (count > 32))
				{
					status = JBIC_BOUNDS_ERROR;
				}
				else
				{
					long_temp &= ((-1L) >> (32 - count));

					stack[stack_ptr - 1] =
						((a & long_temp) == (b & long_temp)) ? 1L : 0L;
				}
			}
			break;

		case 0x13: /* PINT */
			/*
			*	PRINT add integer
			*	...stack 0 is integer value
			*/
			IF_CHECK_STACK(1)
			{
				jbi_ltoa_ex(message_buffer+jbi_strlen_ex(message_buffer),
					stack[--stack_ptr]);
			}
			break;

		case 0x14: /* PRNT */
			/*
			*	PRINT finish
			*/
			jbi_message_ex(message_buffer);
			setInt8Array(message_buffer, 0, '\0');
			break;

		case 0x15: /* DSS  */
			/*
			*	DRSCAN short
			*	...stack 0 is scan data
			*	...stack 1 is count
			*/
			IF_CHECK_STACK(2)
			{
				long_temp = stack[--stack_ptr];
				count = (unsigned long) stack[--stack_ptr];
				jbi_make_dword(charbuf, long_temp);
				status = jbi_do_drscan(count, charbuf, 0);
			}
			break;

		case 0x16: /* DSSC */
			/*
			*	DRSCAN short with capture
			*	...stack 0 is scan data
			*	...stack 1 is count
			*/
			IF_CHECK_STACK(2)
			{
				long_temp = stack[--stack_ptr];
				count = (unsigned long) stack[stack_ptr - 1];
				jbi_make_dword(charbuf, long_temp);
				status = jbi_swap_dr(count, charbuf, 0, charbuf, 0);
				stack[stack_ptr - 1] = jbi_get_dword(charbuf);
			}
			break;

		case 0x17: /* ISS  */
			/*
			*	IRSCAN short
			*	...stack 0 is scan data
			*	...stack 1 is count
			*/
			IF_CHECK_STACK(2)
			{
				long_temp = stack[--stack_ptr];
				count = (unsigned long) stack[--stack_ptr];
				jbi_make_dword(charbuf, long_temp);
				status = jbi_do_irscan(count, charbuf, 0);
			}
			break;

		case 0x18: /* ISSC */
			/*
			*	IRSCAN short with capture
			*	...stack 0 is scan data
			*	...stack 1 is count
			*/
			IF_CHECK_STACK(2)
			{
				long_temp = stack[--stack_ptr];
				count = (unsigned long) stack[stack_ptr - 1];
				jbi_make_dword(charbuf, long_temp);
				status = jbi_swap_ir(count, charbuf, 0, charbuf, 0);
				stack[stack_ptr - 1] = jbi_get_dword(charbuf);
			}
			break;

		case 0x19: /* VSS  */
			/*
			*	VECTOR short
			*	...stack 0 is scan data
			*	...stack 1 is count
			*/
			bad_opcode = 1;
			break;

		case 0x1A: /* VSSC */
			/*
			*	VECTOR short with capture
			*	...stack 0 is scan data
			*	...stack 1 is count
			*/
			bad_opcode = 1;
			break;

		case 0x1B: /* VMPF */
			/*
			*	VMAP finish
			*/
			bad_opcode = 1;
			break;

		case 0x1C: /* DPR  */
			IF_CHECK_STACK(1)
			{
				count = (unsigned long) stack[--stack_ptr];
				status = jbi_set_dr_preamble(count, 0, NULL);
			}
			break;

		case 0x1D: /* DPRL */
			/*
			*	DRPRE with literal data
			*	...stack 0 is count
			*	...stack 1 is literal data
			*/
			IF_CHECK_STACK(2)
			{
				count = (unsigned long) stack[--stack_ptr];
				long_temp = stack[--stack_ptr];				
				jbi_make_dword(charbuf, long_temp);
				status = jbi_set_dr_preamble(count, 0, charbuf);
			}
			break;

		case 0x1E: /* DPO  */
			/*
			*	DRPOST
			*	...stack 0 is count
			*/
			IF_CHECK_STACK(1)
			{
				count = (unsigned long) stack[--stack_ptr];
				status = jbi_set_dr_postamble(count, 0, NULL);
			}
			break;

		case 0x1F: /* DPOL */
			/*
			*	DRPOST with literal data
			*	...stack 0 is count
			*	...stack 1 is literal data
			*/
			IF_CHECK_STACK(2)
			{
				count = (unsigned long) stack[--stack_ptr];
				long_temp = stack[--stack_ptr];				
				jbi_make_dword(charbuf, long_temp);
				status = jbi_set_dr_postamble(count, 0, charbuf);
			}
			break;

		case 0x20: /* IPR  */
			IF_CHECK_STACK(1)
			{
				count = (unsigned long) stack[--stack_ptr];
				status = jbi_set_ir_preamble(count, 0, NULL);
			}
			break;

		case 0x21: /* IPRL */
			/*
			*	IRPRE with literal data
			*	...stack 0 is count
			*	...stack 1 is literal data
			*/
			IF_CHECK_STACK(2)
			{
				count = (unsigned long) stack[--stack_ptr];
				long_temp = stack[--stack_ptr];				
				jbi_make_dword(charbuf, long_temp);
				status = jbi_set_ir_preamble(count, 0, charbuf);
			}
			break;

		case 0x22: /* IPO  */
			/*
			*	IRPOST
			*	...stack 0 is count
			*/
			IF_CHECK_STACK(1)
			{
				count = (unsigned long) stack[--stack_ptr];
				status = jbi_set_ir_postamble(count, 0, NULL);
			}
			break;

		case 0x23: /* IPOL */
			/*
			*	IRPOST with literal data
			*	...stack 0 is count
			*	...stack 1 is literal data
			*/
			IF_CHECK_STACK(2)
			{
				count = (unsigned long) stack[--stack_ptr];
				long_temp = stack[--stack_ptr];				
				jbi_make_dword(charbuf, long_temp);
				status = jbi_set_ir_postamble(count, 0, charbuf);
			}
			break;

		case 0x24: /* PCHR */
			IF_CHECK_STACK(1)
			{
				unsigned char ch;
				count = jbi_strlen_ex(message_buffer);
				ch = (char) stack[--stack_ptr];
				if ((ch < 1) || (ch > 127))
				{
					/* character code out of range */
					/* instead of flagging an error, force the value to 127 */
					ch = 127;
				}
				setInt8Array(message_buffer, count, ch);
				setInt8Array(message_buffer, count + 1, '\0');
			}
			break;

		case 0x25: /* EXIT */
			IF_CHECK_STACK(1)
			{
				*exit_code = (long) stack[--stack_ptr];
			}
			done = 1;
			break;

		case 0x26: /* EQU  */
			IF_CHECK_STACK(2)
			{
				--stack_ptr;
				stack[stack_ptr - 1] =
					(stack[stack_ptr - 1] == stack[stack_ptr]) ? 1L : 0L;
			}
			break;

		case 0x27: /* POPT  */
			IF_CHECK_STACK(1)
			{
				--stack_ptr;
			}
			break;

		case 0x28: /* TRST  */
			bad_opcode = 1;
			break;

		case 0x29: /* FRQ   */
			bad_opcode = 1;
			break;

		case 0x2A: /* FRQU  */
			bad_opcode = 1;
			break;

		case 0x2B: /* PD32  */
			bad_opcode = 1;
			break;

		case 0x2C: /* ABS   */
			IF_CHECK_STACK(1)
			{
				if (stack[stack_ptr - 1] < 0)
				{
					stack[stack_ptr - 1] = 0 - stack[stack_ptr - 1];
				}
			}
			break;

		case 0x2D: /* BCH0  */
			/*
			*	Batch operation 0
			*	SWP
			*	SWPN 7
			*	SWP
			*	SWPN 6
			*	DUPN 8
			*	SWPN 2
			*	SWP
			*	DUPN 6
			*	DUPN 6
			*/

			/* SWP  */
			IF_CHECK_STACK(2)
			{
				long_temp = stack[stack_ptr - 2];
				stack[stack_ptr - 2] = stack[stack_ptr - 1];
				stack[stack_ptr - 1] = long_temp;
			}

			/* SWPN 7 */
			index = 7 + 1;
			IF_CHECK_STACK(index)
			{
				long_temp = stack[stack_ptr - index];
				stack[stack_ptr - index] = stack[stack_ptr - 1];
				stack[stack_ptr - 1] = long_temp;
			}

			/* SWP  */
			IF_CHECK_STACK(2)
			{
				long_temp = stack[stack_ptr - 2];
				stack[stack_ptr - 2] = stack[stack_ptr - 1];
				stack[stack_ptr - 1] = long_temp;
			}

			/* SWPN 6 */
			index = 6 + 1;
			IF_CHECK_STACK(index)
			{
				long_temp = stack[stack_ptr - index];
				stack[stack_ptr - index] = stack[stack_ptr - 1];
				stack[stack_ptr - 1] = long_temp;
			}

			/* DUPN 8 */
			index = 8 + 1;
			IF_CHECK_STACK(index)
			{
				stack[stack_ptr] = stack[stack_ptr - index];
				++stack_ptr;
			}

			/* SWPN 2 */
			index = 2 + 1;
			IF_CHECK_STACK(index)
			{
				long_temp = stack[stack_ptr - index];
				stack[stack_ptr - index] = stack[stack_ptr - 1];
				stack[stack_ptr - 1] = long_temp;
			}

			/* SWP  */
			IF_CHECK_STACK(2)
			{
				long_temp = stack[stack_ptr - 2];
				stack[stack_ptr - 2] = stack[stack_ptr - 1];
				stack[stack_ptr - 1] = long_temp;
			}

			/* DUPN 6 */
			index = 6 + 1;
			IF_CHECK_STACK(index)
			{
				stack[stack_ptr] = stack[stack_ptr - index];
				++stack_ptr;
			}

			/* DUPN 6 */
			index = 6 + 1;
			IF_CHECK_STACK(index)
			{
				stack[stack_ptr] = stack[stack_ptr - index];
				++stack_ptr;
			}
			break;

		case 0x2E: /* BCH1  */
			/*
			*	Batch operation 1
			*	SWPN 8
			*	SWP
			*	SWPN 9
			*	SWPN 3
			*	SWP
			*	SWPN 2
			*	SWP
			*	SWPN 7
			*	SWP
			*	SWPN 6
			*	DUPN 5
			*	DUPN 5
			*/
			bad_opcode = 1;
			break;

		case 0x2F: /* PSH0  */
			stack[stack_ptr++] = 0;
			break;

		case 0x40: /* PSHL */
			stack[stack_ptr++] = (long) args[0];
			break;

		case 0x41: /* PSHV */
			stack[stack_ptr++] = getInt32Array(variables, args[0]);
			break;

		case 0x42: /* JMP  */
			pc = args[0] + code_section;
			CHECK_PC;
			break;

		case 0x43: /* CALL */
			stack[stack_ptr++] = pc;
			pc = args[0] + code_section;
			CHECK_PC;
			break;

		case 0x44: /* NEXT */
			/*
			*	Process FOR / NEXT loop
			*	...argument 0 is variable ID
			*	...stack 0 is step value
			*	...stack 1 is end value
			*	...stack 2 is top address
			*/
			IF_CHECK_STACK(3)
			{
				long step = stack[stack_ptr - 1];
				long end = stack[stack_ptr - 2];
				long top = stack[stack_ptr - 3];
				long iterator = getInt32Array(variables, args[0]);
				long break_out = 0;

				if (step < 0)
				{
					if (iterator <= end) break_out = 1;
				}
				else
				{
					if (iterator >= end) break_out = 1;
				}

				if (break_out)
				{
					stack_ptr -= 3;
				}
				else
				{
					setInt32Array(variables, args[0], iterator + step);
					pc = top + code_section;
					CHECK_PC;
				}
			}
			break;

		case 0x45: /* PSTR */
			/*
			*	PRINT add string
			*	...argument 0 is string ID
			*/
			count = jbi_strlen_ex(message_buffer);
			jbi_strncpy_ex(message_buffer+count,
				program + string_table + args[0],
				JBIC_MESSAGE_LENGTH - count);
			setInt8Array(message_buffer, JBIC_MESSAGE_LENGTH, '\0');
			break;

		case 0x46: /* VMAP */
			/*
			*	VMAP add signal name
			*	...argument 0 is string ID
			*/
			bad_opcode = 1;
			break;

		case 0x47: /* SINT */
			/*
			*	STATE intermediate state
			*	...argument 0 is state code
			*/
			status = jbi_goto_jtag_state((JBIE_JTAG_STATE) args[0]);
			break;

		case 0x48: /* ST   */
			/*
			*	STATE final state
			*	...argument 0 is state code
			*/
			status = jbi_goto_jtag_state((JBIE_JTAG_STATE) args[0]);
			break;

		case 0x49: /* ISTP */
			/*
			*	IRSTOP state
			*	...argument 0 is state code
			*/
			status = jbi_set_irstop_state((JBIE_JTAG_STATE) args[0]);
			break;

		case 0x4A: /* DSTP */
			/*
			*	DRSTOP state
			*	...argument 0 is state code
			*/
			status = jbi_set_drstop_state((JBIE_JTAG_STATE) args[0]);
			break;

		case 0x4B: /* SWPN */
			/*
			*	Exchange top with Nth stack value
			*	...argument 0 is 0-based stack entry to swap with top element
			*/
			index = ((long) args[0]) + 1;
			IF_CHECK_STACK(index)
			{
				long_temp = stack[stack_ptr - index];
				stack[stack_ptr - index] = stack[stack_ptr - 1];
				stack[stack_ptr - 1] = long_temp;
			}
			break;

		case 0x4C: /* DUPN */
			/*
			*	Duplicate Nth stack value
			*	...argument 0 is 0-based stack entry to duplicate
			*/
			index = ((long) args[0]) + 1;
			IF_CHECK_STACK(index)
			{
				stack[stack_ptr] = stack[stack_ptr - index];
				++stack_ptr;
			}
			break;

		case 0x4D: /* POPV */
			/*
			*	Pop stack into scalar variable
			*	...argument 0 is variable ID
			*	...stack 0 is value
			*/
			IF_CHECK_STACK(1)
			{
				setInt32Array(variables, args[0], stack[--stack_ptr]);
			}
			break;

		case 0x4E: /* POPE */
			/*
			*	Pop stack into integer array element
			*	...argument 0 is variable ID
			*	...stack 0 is array index
			*	...stack 1 is value
			*/
			IF_CHECK_STACK(2)
			{
				variable_id = (unsigned long) args[0];

				/*
				*	If variable is read-only, convert to writable array
				*/
				if ((version > 0) &&
					((getInt8Array(attributes, variable_id) & 0x9c) == 0x1c))
				{
					/*
					*	Allocate a writable buffer for this array
					*/
					count = (unsigned long) getInt32Array(variable_size, variable_id);
					long_temp = getInt32Array(variables, variable_id);
					longptr_temp = jbi_malloc_ex(count * sizeof(long));
					setInt32Array(variables, variable_id, longptr_temp);

					if (getInt32Array(variables, variable_id) == NULL)
					{
						status = JBIC_OUT_OF_MEMORY;
						break;
					}
					else
					{
						/* copy previous contents into buffer */
						for (i = 0; i < count; ++i)
						{
							setInt32Array(longptr_temp, i, GET_DWORD(long_temp));
							long_temp += 4L;
						}

						/* set bit 7 - buffer was dynamically allocated */
						setInt8Array(attributes, variable_id, getInt8Array(attributes, variable_id) |0x80);

						/* clear bit 2 - variable is writable */
						setInt8Array(attributes, variable_id, getInt8Array(attributes, variable_id) & (~0x04));
						setInt8Array(attributes, variable_id, getInt8Array(attributes, variable_id) |0x01);
					}
				}


				/* check that variable is a writable integer array */
				if ((getInt8Array(attributes, variable_id) & 0x1c) != 0x18)
				{
					status = JBIC_BOUNDS_ERROR;
				}
				else
				{
					longptr_temp = getInt32Array(variables, variable_id);

					/* pop the array index */
					index = (unsigned long) stack[--stack_ptr];

					/* pop the value and store it into the array */
					setInt32Array(longptr_temp, index, stack[--stack_ptr]);
				}
			}
			break;

		case 0x4F: /* POPA */
			/*
			*	Pop stack into Boolean array
			*	...argument 0 is variable ID
			*	...stack 0 is count
			*	...stack 1 is array index
			*	...stack 2 is value
			*/
			IF_CHECK_STACK(3)
			{
				variable_id = (unsigned long) args[0];

				/*
				*	If variable is read-only, convert to writable array
				*/
				if ((version > 0) &&
					((getInt8Array(attributes, variable_id) & 0x9c) == 0x0c))
				{
					/*
					*	Allocate a writable buffer for this array
					*/
					long_temp = (getInt32Array(variable_size, variable_id) + 7L) >> 3L;
					charptr_temp2 =  getInt32Array(variables, variable_id);
					charptr_temp =  jbi_malloc_ex((unsigned long) long_temp);
					setInt32Array(variables, variable_id, charptr_temp);

					if (getInt32Array(variables, variable_id) == NULL)
					{
						status = JBIC_OUT_OF_MEMORY;
					}
					else
					{
						/* zero the buffer */
						for (long_index = 0L;
							long_index < long_temp;
							++long_index)
						{
							setUint8Array(charptr_temp, long_index, 0);
						}

						/* copy previous contents into buffer */
						for (long_index = 0L;
							long_index < getInt32Array(variable_size, variable_id);
							++long_index)
						{
							long_index2 = long_index;

							if (getUint8Array(charptr_temp2, long_index2 >> 3) &
								(1 << (long_index2 & 7)))
							{
								setUint8Array(charptr_temp, long_index >> 3, getUint8Array(charptr_temp, long_index >> 3)|(1 << (long_index & 7)));
							}
						}

						/* set bit 7 - buffer was dynamically allocated */
						setInt8Array(attributes, variable_id, getInt8Array(attributes, variable_id) |0x80);

						/* clear bit 2 - variable is writable */
						setInt8Array(attributes, variable_id, getInt8Array(attributes, variable_id) & (~0x04));
						setInt8Array(attributes, variable_id, getInt8Array(attributes, variable_id) | 0x01);
					}
				}


				/* check that variable is a writable Boolean array */
				if ((getInt8Array(attributes, variable_id) & 0x1c) != 0x08)
				{
					status = JBIC_BOUNDS_ERROR;
				}
				else
				{
					charptr_temp = getInt32Array(variables, variable_id);

					/* pop the count (number of bits to copy) */
					long_count = stack[--stack_ptr];

					/* pop the array index */
					long_index = stack[--stack_ptr];

					reverse = 0;

					if (version > 0)
					{
						/* stack 0 = array right index */
						/* stack 1 = array left index */

						if (long_index > long_count)
						{
							reverse = 1;
							long_temp = long_count;
							long_count = 1 + long_index - long_count;
							long_index = long_temp;

							/* reverse POPA is not supported */
							status = JBIC_BOUNDS_ERROR;
							break;
						}
						else
						{
							long_count = 1 + long_count - long_index;
						}
					}

					/* pop the data */
					long_temp = stack[--stack_ptr];

					if (long_count < 1)
					{
						status = JBIC_BOUNDS_ERROR;
					}
					else
					{
						for (i = 0; i < (unsigned long) long_count; ++i)
						{
							if (long_temp & (1L << (long) i))
							{
								setUint8Array(charptr_temp, long_index >> 3L, getUint8Array(charptr_temp, long_index >> 3L)|(1L << (long_index & 7L)));
							}
							else
							{
								setUint8Array(charptr_temp, long_index >> 3L, getUint8Array(charptr_temp, long_index >> 3L)&(~ (unsigned long) (1L << (long_index & 7L))));
							}
							++long_index;
						}
					}
				}
			}
			break;

		case 0x50: /* JMPZ */
			/*
			*	Pop stack and branch if zero
			*	...argument 0 is address
			*	...stack 0 is condition value
			*/
			IF_CHECK_STACK(1)
			{
				if (stack[--stack_ptr] == 0)
				{
					pc = args[0] + code_section;
					CHECK_PC;
				}
			}
			break;

		case 0x51: /* DS   */
		case 0x52: /* IS   */
			/*
			*	DRSCAN
			*	IRSCAN
			*	...argument 0 is scan data variable ID
			*	...stack 0 is array index
			*	...stack 1 is count
			*/
			IF_CHECK_STACK(2)
			{
				long_index = stack[--stack_ptr];
				long_count = stack[--stack_ptr];

				reverse = 0;

				if (version > 0)
				{
					/* stack 0 = array right index */
					/* stack 1 = array left index */
					/* stack 2 = count */
					long_temp = long_count;
					long_count = stack[--stack_ptr];

					if (long_index > long_temp)
					{
						reverse = 1;
						long_index = long_temp;
					}
				}

				charptr_temp = getInt32Array(variables, args[0]);

				if (reverse)
				{
					/* allocate a buffer and reverse the data order */
					charptr_temp2 = charptr_temp;
					charptr_temp = jbi_malloc_ex((long_count >> 3) + 1);
					if (charptr_temp == NULL)
					{
						status = JBIC_OUT_OF_MEMORY;
						break;
					}
					else
					{
						long_temp = long_index + long_count - 1;
						long_index2 = 0;
						while (long_index2 < long_count)
						{
							if (getUint8Array(charptr_temp2, long_temp >> 3) &
								(1 << (long_temp & 7)))
							{
								setUint8Array(charptr_temp, long_index2 >> 3, getUint8Array(charptr_temp, long_index2 >> 3)|(1 << (long_index2 & 7)));
							}
							else
							{
								setUint8Array(charptr_temp, long_index2 >> 3, getUint8Array(charptr_temp, long_index2 >> 3)&(~(1 << (long_index2 & 7))));
							}

							--long_temp;
							++long_index2;
						}
					}
				}

				if (opcode == 0x51)	/* DS */
				{
					status = jbi_do_drscan_ex((unsigned long) long_count,
						charptr_temp, (unsigned long) long_index);
				}
				else	/* IS */
				{
					status = jbi_do_irscan_ex((unsigned long) long_count,
						charptr_temp, (unsigned long) long_index);
				}

				if (reverse && (charptr_temp != NULL))
				{
					jbi_free(charptr_temp);
				}
			}
			break;

		case 0x53: /* DPRA */
			/*
			*	DRPRE with array data
			*	...argument 0 is variable ID
			*	...stack 0 is array index
			*	...stack 1 is count
			*/
			IF_CHECK_STACK(2)
			{
				index = (unsigned long) stack[--stack_ptr];
				count = (unsigned long) stack[--stack_ptr];

				if (version > 0)
				{
					/* stack 0 = array right index */
					/* stack 1 = array left index */
					count = 1 + count - index;
				}

				charptr_temp = getInt32Array(variables, args[0]);
				status = jbi_set_dr_preamble_ex(count, index, charptr_temp);
			}
			break;

		case 0x54: /* DPOA */
			/*
			*	DRPOST with array data
			*	...argument 0 is variable ID
			*	...stack 0 is array index
			*	...stack 1 is count
			*/
			IF_CHECK_STACK(2)
			{
				index = (unsigned long) stack[--stack_ptr];
				count = (unsigned long) stack[--stack_ptr];

				if (version > 0)
				{
					/* stack 0 = array right index */
					/* stack 1 = array left index */
					count = 1 + count - index;
				}

				charptr_temp = getInt32Array(variables, args[0]);
				status = jbi_set_dr_postamble_ex(count, index, charptr_temp);
			}
			break;

		case 0x55: /* IPRA */
			/*
			*	IRPRE with array data
			*	...argument 0 is variable ID
			*	...stack 0 is array index
			*	...stack 1 is count
			*/
			IF_CHECK_STACK(2)
			{
				index = (unsigned long) stack[--stack_ptr];
				count = (unsigned long) stack[--stack_ptr];

				if (version > 0)
				{
					/* stack 0 = array right index */
					/* stack 1 = array left index */
					count = 1 + count - index;
				}

				charptr_temp = getInt32Array(variables, args[0]);
				status = jbi_set_ir_preamble_ex(count, index, charptr_temp);
			}
			break;

		case 0x56: /* IPOA */
			/*
			*	IRPOST with array data
			*	...argument 0 is variable ID
			*	...stack 0 is array index
			*	...stack 1 is count
			*/
			IF_CHECK_STACK(2)
			{
				index = (unsigned long) stack[--stack_ptr];
				count = (unsigned long) stack[--stack_ptr];

				if (version > 0)
				{
					/* stack 0 = array right index */
					/* stack 1 = array left index */
					count = 1 + count - index;
				}

				charptr_temp = getInt32Array(variables, args[0]);
				status = jbi_set_ir_postamble_ex(count, index, charptr_temp);
			}
			break;

		case 0x57: /* EXPT */
			/*
			*	EXPORT
			*	...argument 0 is string ID
			*	...stack 0 is integer expression
			*/
			IF_CHECK_STACK(1)
			{
				//name = (char *) &program[string_table + args[0]];
				long_temp = stack[--stack_ptr];
				//jbi_export_integer(name, long_temp);
			}
			break;

		case 0x58: /* PSHE */
			/*
			*	Push integer array element
			*	...argument 0 is variable ID
			*	...stack 0 is array index
			*/
			IF_CHECK_STACK(1)
			{
				variable_id = (unsigned long) args[0];
				index = (unsigned long) stack[stack_ptr - 1];

				/* check variable type */
				if ((getInt8Array(attributes, variable_id) & 0x1f) == 0x19)
				{
					/* writable integer array */
					longptr_temp = getInt32Array(variables, variable_id);
					stack[stack_ptr - 1] = getInt32Array(longptr_temp, index);
				}
				else if ((getInt8Array(attributes, variable_id) & 0x1f) == 0x1c)
				{
					/* read-only integer array */
					long_temp = getInt32Array(variables, variable_id) + (4L * index);
					stack[stack_ptr - 1] = GET_DWORD(long_temp);
				}
				else
				{
					status = JBIC_BOUNDS_ERROR;
				}
			}
			break;

		case 0x59: /* PSHA */
			/*
			*	Push Boolean array
			*	...argument 0 is variable ID
			*	...stack 0 is count
			*	...stack 1 is array index
			*/
			IF_CHECK_STACK(2)
			{
				variable_id = (unsigned long) args[0];

				/* check that variable is a Boolean array */
				if ((getInt8Array(attributes, variable_id) & 0x18) != 0x08)
				{
					status = JBIC_BOUNDS_ERROR;
				}
				else
				{
					charptr_temp = getInt32Array(variables, variable_id);

					/* pop the count (number of bits to copy) */
					count = (unsigned long) stack[--stack_ptr];

					/* pop the array index */
					index = (unsigned long) stack[stack_ptr - 1];

					if (version > 0)
					{
						/* stack 0 = array right index */
						/* stack 1 = array left index */
						count = 1 + count - index;
					}

					if ((count < 1) || (count > 32))
					{
						status = JBIC_BOUNDS_ERROR;
					}
					else
					{
						long_temp = 0L;

						for (i = 0; i < count; ++i)
						{
							if (getUint8Array(charptr_temp, (i + index) >> 3) &
								(1 << ((i + index) & 7)))
							{
								long_temp |= (1L << i);
							}
						}

						stack[stack_ptr - 1] = long_temp;
					}
				}
			}
			break;

		case 0x5A: /* DYNA */
			/*
			*	Dynamically change size of array
			*	...argument 0 is variable ID
			*	...stack 0 is new size
			*/
			IF_CHECK_STACK(1)
			{
				variable_id = (unsigned long) args[0];
				long_temp = stack[--stack_ptr];

				if (long_temp > getInt32Array(variable_size, variable_id))
				{
					setInt32Array(variable_size, variable_id, long_temp);

					if (getInt8Array(attributes, variable_id) & 0x10)
					{
						/* allocate integer array */
						long_temp *= 4;
					}
					else
					{
						/* allocate Boolean array */
						long_temp = (long_temp + 7) >> 3;
					}

					/*
					*	If the buffer was previously allocated, free it
					*/
					if ((getInt8Array(attributes, variable_id) & 0x80) &&
						(getInt32Array(variables, variable_id) != NULL))
					{
						jbi_free(getInt32Array(variables, variable_id));
						setInt32Array(variables, variable_id, NULL);
					}

					/*
					*	Allocate a new buffer of the requested size
					*/
					setInt32Array(variables, variable_id, jbi_malloc_ex((unsigned long) long_temp));

					if (getInt32Array(variables, variable_id) == NULL)
					{
						status = JBIC_OUT_OF_MEMORY;
					}
					else
					{
						/*
						*	Set the attribute bit to indicate that this buffer
						*	was dynamically allocated and should be freed later
						*/
						setInt8Array(attributes, variable_id, getInt8Array(attributes, variable_id) |0x80);

						/* zero out memory */
						count = (unsigned long)
							((getInt32Array(variable_size, variable_id) + 7L) / 8L);
						charptr_temp = getInt32Array(variables, variable_id);
						for (index = 0; index < count; ++index)
						{
							setUint8Array(charptr_temp, index, 0);
						}
					}
				}
			}
			break;

		case 0x5B: /* EXPR */
			bad_opcode = 1;
			break;

		case 0x5C: /* EXPV */
			/*
			*	Export Boolean array
			*	...argument 0 is string ID
			*	...stack 0 is variable ID
			*	...stack 1 is array right index
			*	...stack 2 is array left index
			*/
			IF_CHECK_STACK(3)
			{
				if (version == 0)
				{
					/* EXPV is not supported in JBC 1.0 */
					bad_opcode = 1;
					break;
				}
				//name = (char *) &program[string_table + args[0]];
				variable_id = (unsigned long) stack[--stack_ptr];
				long_index = stack[--stack_ptr];	/* right index */
				long_index2 = stack[--stack_ptr];	/* left index */

				if (long_index > long_index2)
				{
					/* reverse indices not supported */
					status = JBIC_BOUNDS_ERROR;
					break;
				}
			}
			break;

		case 0x80: /* COPY */
			/*
			*	Array copy
			*	...argument 0 is dest ID
			*	...argument 1 is source ID
			*	...stack 0 is count
			*	...stack 1 is dest index
			*	...stack 2 is source index
			*/
			IF_CHECK_STACK(3)
			{
				long copy_count = stack[--stack_ptr];
				long copy_index = stack[--stack_ptr];
				long copy_index2 = stack[--stack_ptr];
				long destleft;
				long src_count;
				long dest_count;
				long src_reverse = 0;
				long dest_reverse = 0;

				reverse = 0;

				if (version > 0)
				{
					/* stack 0 = source right index */
					/* stack 1 = source left index */
					/* stack 2 = destination right index */
					/* stack 3 = destination left index */
					destleft = stack[--stack_ptr];

					if (copy_count > copy_index)
					{
						src_reverse = 1;
						reverse = 1;
						src_count = 1 + copy_count - copy_index;
						/* copy_index = source start index */
					}
					else
					{
						src_count = 1 + copy_index - copy_count;
						copy_index = copy_count;	/* source start index */
					}

					if (copy_index2 > destleft)
					{
						dest_reverse = 1;
						reverse = !reverse;
						dest_count = 1 + copy_index2 - destleft;
						copy_index2 = destleft;	/* destination start index */
					}
					else
					{
						dest_count = 1 + destleft - copy_index2;
						/* copy_index2 = destination start index */
					}

					copy_count = (src_count < dest_count) ? src_count : dest_count;

					if ((src_reverse || dest_reverse) &&
						(src_count != dest_count))
					{
						/* If either the source or destination is reversed, */
						/* we can't tolerate a length mismatch, because we  */
						/* "left justify" the arrays when copying.  This    */
						/* won't work correctly with reversed arrays.       */
						status = JBIC_BOUNDS_ERROR;
					}
				}

				count = (unsigned long) copy_count;
				index = (unsigned long) copy_index;
				index2 = (unsigned long) copy_index2;

				/*
				*	If destination is a read-only array, allocate a buffer
				*	and convert it to a writable array
				*/
				variable_id = (unsigned long) args[1];
				if ((version > 0) && ((getInt8Array(attributes, variable_id) & 0x9c) == 0x0c))
				{
					/*
					*	Allocate a writable buffer for this array
					*/
					long_temp = (getInt32Array(variable_size, variable_id) + 7L) >> 3L;
					charptr_temp2 = getInt32Array(variables, variable_id);
					charptr_temp = jbi_malloc_ex((unsigned long) long_temp);
					setInt32Array(variables, variable_id, charptr_temp);

					if (getInt32Array(variables, variable_id) == NULL)
					{
						status = JBIC_OUT_OF_MEMORY;
						break;
					}
					else
					{
						/* zero the buffer */
						for (long_index = 0L;
							long_index < long_temp;
							++long_index)
						{
							setUint8Array(charptr_temp, long_index, 0);
						}

						/* copy previous contents into buffer */
						for (long_index = 0L;
							long_index < getInt32Array(variable_size, variable_id);
							++long_index)
						{
							long_index2 = long_index;

							if (getUint8Array(charptr_temp2, long_index2 >> 3) &
								(1 << (long_index2 & 7)))
							{
								setUint8Array(charptr_temp, long_index >> 3, getUint8Array(charptr_temp, long_index >> 3)|(1 << (long_index & 7)));
							}
						}

						/* set bit 7 - buffer was dynamically allocated */
						setInt8Array(attributes, variable_id, getInt8Array(attributes, variable_id) |0x80);

						/* clear bit 2 - variable is writable */
						setInt8Array(attributes, variable_id, getInt8Array(attributes, variable_id) & (~0x04));
						setInt8Array(attributes, variable_id, getInt8Array(attributes, variable_id) | 0x01);
					}
				}


				charptr_temp = getInt32Array(variables, args[1]);
				charptr_temp2 = getInt32Array(variables, args[0]);


				/* check that destination is a writable Boolean array */
				if ((getInt8Array(attributes, args[1]) & 0x1c) != 0x08)
				{
					status = JBIC_BOUNDS_ERROR;
					break;
				}

				if (count < 1)
				{
					status = JBIC_BOUNDS_ERROR;
				}
				else
				{
					if (reverse)
					{
						index2 += (count - 1);
					}

					for (i = 0; i < count; ++i)
					{
						if (getUint8Array(charptr_temp2, index >> 3) & (1 << (index & 7)))
						{
							setUint8Array(charptr_temp, index2 >> 3, getUint8Array(charptr_temp, index2 >> 3)|(1 << (index2 & 7)));
						}
						else
						{
							setUint8Array(charptr_temp, index2 >> 3, getUint8Array(charptr_temp, index2 >> 3)&(~(unsigned long) (1 << (index2 & 7))));
						}
						++index;
						if (reverse) --index2; else ++index2;
					}
				}
			}
			break;

		case 0x81: /* REVA */
			/*
			*	ARRAY COPY reversing bit order
			*	...argument 0 is dest ID
			*	...argument 1 is source ID
			*	...stack 0 is dest index
			*	...stack 1 is source index
			*	...stack 2 is count
			*/
			bad_opcode = 1;
			break;

		case 0x82: /* DSC  */
		case 0x83: /* ISC  */
			/*
			*	DRSCAN with capture
			*	IRSCAN with capture
			*	...argument 0 is scan data variable ID
			*	...argument 1 is capture variable ID
			*	...stack 0 is capture index
			*	...stack 1 is scan data index
			*	...stack 2 is count
			*/
			IF_CHECK_STACK(3)
			{
				long scan_right, scan_left;
				long capture_count = 0;
				long scan_count = 0;
				long capture_index = stack[--stack_ptr];
				long scan_index = stack[--stack_ptr];
				if (version > 0)
				{
					/* stack 0 = capture right index */
					/* stack 1 = capture left index */
					/* stack 2 = scan right index */
					/* stack 3 = scan left index */
					/* stack 4 = count */
					scan_right = stack[--stack_ptr];
					scan_left = stack[--stack_ptr];
					capture_count = 1 + scan_index - capture_index;
					scan_count = 1 + scan_left - scan_right;
					scan_index = scan_right;
				}
				long_count = stack[--stack_ptr];

				/*
				*	If capture array is read-only, allocate a buffer
				*	and convert it to a writable array
				*/
				variable_id = (unsigned long) args[1];
				if ((version > 0) && ((getInt8Array(attributes, variable_id) & 0x9c) == 0x0c))
				{
					/*
					*	Allocate a writable buffer for this array
					*/
					long_temp = (getInt32Array(variable_size, variable_id) + 7L) >> 3L;
					charptr_temp2 = getInt32Array(variables, variable_id);
					charptr_temp = jbi_malloc_ex((unsigned long) long_temp);
					setInt32Array(variables, variable_id, charptr_temp);

					if (getInt32Array(variables, variable_id) == NULL)
					{
						status = JBIC_OUT_OF_MEMORY;
						break;
					}
					else
					{
						/* zero the buffer */
						for (long_index = 0L;
							long_index < long_temp;
							++long_index)
						{
							setUint8Array(charptr_temp, long_index, 0);
						}

						/* copy previous contents into buffer */
						for (long_index = 0L;
							long_index < getInt32Array(variable_size, variable_id);
							++long_index)
						{

							long_index2 = long_index;

							if (getUint8Array(charptr_temp2, long_index2 >> 3) &
								(1 << (long_index2 & 7)))
							{
								setUint8Array(charptr_temp, long_index >> 3, getUint8Array(charptr_temp, long_index >> 3)|(1 << (long_index & 7)));
							}
						}

						/* set bit 7 - buffer was dynamically allocated */
						setInt8Array(attributes, variable_id, getInt8Array(attributes, variable_id) |0x80);

						/* clear bit 2 - variable is writable */
						setInt8Array(attributes, variable_id, getInt8Array(attributes, variable_id) &(~0x04));
						setInt8Array(attributes, variable_id, getInt8Array(attributes, variable_id) |0x01);
					}
				}


				charptr_temp = getInt32Array(variables, args[0]);
				charptr_temp2 = getInt32Array(variables, args[1]);


				if ((version > 0) &&
					((long_count > capture_count) || (long_count > scan_count)))
				{
					status = JBIC_BOUNDS_ERROR;
				}

				/* check that capture array is a writable Boolean array */
				if ((getInt8Array(attributes, args[1]) & 0x1c) != 0x08)
				{
					status = JBIC_BOUNDS_ERROR;
				}

				if (status == JBIC_SUCCESS)
				{
					if (opcode == 0x82) /* DSC */
					{
						status = jbi_swap_dr_ex((unsigned long) long_count,
							charptr_temp, (unsigned long) scan_index,
							charptr_temp2, (unsigned long) capture_index);
					}
					else /* ISC */
					{
						status = jbi_swap_ir_ex((unsigned long) long_count,
							charptr_temp, (unsigned long) scan_index,
							charptr_temp2, (unsigned long) capture_index);
					}
				}
			}
			break;

		case 0x84: /* WAIT */
			/*
			*	WAIT
			*	...argument 0 is wait state
			*	...argument 1 is end state
			*	...stack 0 is cycles
			*	...stack 1 is microseconds
			*/
			IF_CHECK_STACK(2)
			{
				long_temp = stack[--stack_ptr];

				if (long_temp != 0L)
				{
					status = jbi_do_wait_cycles(long_temp, (JBIE_JTAG_STATE) args[0]);
				}

				long_temp = stack[--stack_ptr];

				if ((status == JBIC_SUCCESS) && (long_temp != 0L))
				{
					status = jbi_do_wait_microseconds(long_temp, (JBIE_JTAG_STATE) args[0]);
				}

				if ((status == JBIC_SUCCESS) && (args[1] != args[0]))
				{
					status = jbi_goto_jtag_state((JBIE_JTAG_STATE) args[1]);
				}

				if (version > 0)
				{
					--stack_ptr;	/* throw away MAX cycles */
					--stack_ptr;	/* throw away MAX microseconds */
				}
			}
			break;

		case 0x85: /* VS   */
			/*
			*	VECTOR
			*	...argument 0 is dir data variable ID
			*	...argument 1 is scan data variable ID
			*	...stack 0 is dir array index
			*	...stack 1 is scan array index
			*	...stack 2 is count
			*/
			bad_opcode = 1;
			break;

		case 0xC0: /* CMPA */
			/*
			*	Array compare
			*	...argument 0 is source 1 ID
			*	...argument 1 is source 2 ID
			*	...argument 2 is mask ID
			*	...stack 0 is source 1 index
			*	...stack 1 is source 2 index
			*	...stack 2 is mask index
			*	...stack 3 is count
			*/
			IF_CHECK_STACK(4)
			{
				long a, b;
				unsigned long source1_uint8Ptr = getInt32Array(variables, args[0]);
				unsigned long source2_uint8Ptr = getInt32Array(variables, args[1]);
				unsigned long mask_uint8Ptr    = getInt32Array(variables, args[2]);
				unsigned long index1 = stack[--stack_ptr];
				unsigned long index2 = stack[--stack_ptr];
				unsigned long mask_index = stack[--stack_ptr];
				long_count = stack[--stack_ptr];

				if (version > 0)
				{
					/* stack 0 = source 1 right index */
					/* stack 1 = source 1 left index */
					/* stack 2 = source 2 right index */
					/* stack 3 = source 2 left index */
					/* stack 4 = mask right index */
					/* stack 5 = mask left index */
					long mask_right = stack[--stack_ptr];
					long mask_left = stack[--stack_ptr];
					a = 1 + index2 - index1; /* source 1 count */
					b = 1 + long_count - mask_index; /* source 2 count */
					a = (a < b) ? a : b;
					b = 1 + mask_left - mask_right; /* mask count */
					a = (a < b) ? a : b;
					index2 = mask_index;	/* source 2 start index */
					mask_index = mask_right;	/* mask start index */
					long_count = a;
				}

				long_temp = 1L;

				if (long_count < 1)
				{
					status = JBIC_BOUNDS_ERROR;
				}
				else
				{
					count = (unsigned long) long_count;

					for (i = 0; i < count; ++i)
					{
						if (getUint8Array(mask_uint8Ptr, mask_index >> 3) & (1 << (mask_index & 7)))
						{
							a = getUint8Array(source1_uint8Ptr, index1 >> 3) & (1 << (index1 & 7))
								? 1 : 0;
							b = getUint8Array(source2_uint8Ptr, index2 >> 3) & (1 << (index2 & 7))
								? 1 : 0;

							if (a != b) long_temp = 0L;	/* failure */
						}
						++index1;
						++index2;
						++mask_index;
					}
				}

				stack[stack_ptr++] = long_temp;
			}
			break;

		case 0xC1: /* VSC  */
			/*
			*	VECTOR with capture
			*	...argument 0 is dir data variable ID
			*	...argument 1 is scan data variable ID
			*	...argument 2 is capture variable ID
			*	...stack 0 is capture index
			*	...stack 1 is scan data index
			*	...stack 2 is dir data index
			*	...stack 3 is count
			*/
			bad_opcode = 1;
			break;

		default:
			/*
			*	Unrecognized opcode -- ERROR!
			*/
			bad_opcode = 1;
			break;
		}

		if (bad_opcode)
		{
			status = JBIC_ILLEGAL_OPCODE;
		}

		if ((stack_ptr < 0) || (stack_ptr >= JBI_STACK_SIZE))
		{
			status = JBIC_STACK_OVERFLOW;
		}

		if (status != JBIC_SUCCESS)
		{
			done = 1;
			*error_address = (long) (opcode_address - code_section);
		}
	}

	jbi_free_jtag_padding_buffers(reset_jtag);

	/*
	*	Free all dynamically allocated arrays
	*/
	if ((attributes != NULL) && (variables != NULL))
	{
		for (i = 0; i < (unsigned long) symbol_count; ++i)
		{
			if ((getInt8Array(attributes, i) & 0x80) && (getInt32Array(variables, i) != NULL))
			{
				jbi_free(getInt32Array(variables, i));
			}
		}
	}

	if (variables != NULL) jbi_free(variables);

	if (variable_size != NULL) jbi_free(variable_size);

	if (attributes != NULL) jbi_free(attributes);

	if (proc_attributes != NULL) jbi_free(proc_attributes);

	return (status);
}



/****************************************************************************/
/*																			*/

JBI_RETURN_TYPE jbi_check_crc
(
	PROGRAM_PTR program,
	long program_size,
	unsigned short *expected_crc,
	unsigned short *actual_crc
)

/*																			*/
/*	Description:	This function reads the entire input file and computes	*/
/*					the CRC of everything up to the CRC field.				*/
/*																			*/
/*	Returns:		JBIC_SUCCESS for success, JBIC_CRC_ERROR for failure	*/
/*																			*/
/****************************************************************************/
{
	JBI_RETURN_TYPE status = JBIC_SUCCESS;
	unsigned short local_expected, local_actual, shift_reg = 0xffff;
	long bit, feedback;
	unsigned char databyte;
	unsigned long i;
	unsigned long crc_section = 0L;
	unsigned long first_word = 0L;
	long version = 0;
	long delta = 0;


	if (program_size > 52L)
	{
		first_word  = GET_DWORD(0);
		version = (long) (first_word & 1L);
		delta = version * 8;

		crc_section = GET_DWORD(32 + delta);
	}

	if ((first_word != 0x4A414D00L) && (first_word != 0x4A414D01L))
	{
		status = JBIC_IO_ERROR;
	}

	if (crc_section >= (unsigned long) program_size)
	{
		status = JBIC_IO_ERROR;
	}

	if (status == JBIC_SUCCESS)
	{
		local_expected = (unsigned short) GET_WORD(crc_section);
		if (expected_crc != NULL) *expected_crc = local_expected;

		for (i = 0; i < crc_section; ++i)
		{
			databyte = GET_BYTE(i);
			for (bit = 0; bit < 8; bit++)	/* compute for each bit */
			{
				feedback = (databyte ^ shift_reg) & 0x01;
				shift_reg >>= 1;	/* shift the shift register */
				if (feedback) shift_reg ^= 0x8408;	/* invert selected bits */
				databyte >>= 1;		/* get the next bit of input_byte */
			}
		}

		local_actual = (unsigned short) ~shift_reg;
		if (actual_crc != NULL) *actual_crc = local_actual;

		if (local_expected != local_actual)
		{
			status = JBIC_CRC_ERROR;
		}
	}

	return (status);
}





