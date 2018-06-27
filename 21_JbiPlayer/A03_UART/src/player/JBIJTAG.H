/****************************************************************************/
/*																			*/
/*	Module:			jbijtag.h												*/
/*																			*/
/*					Copyright (C) Altera Corporation 1998-2001				*/
/*																			*/
/*	Description:	Definitions of JTAG constants, types, and functions		*/
/*																			*/
/****************************************************************************/

#ifndef INC_JBIJTAG_H
#define INC_JBIJTAG_H

/****************************************************************************/
/*																			*/
/*	Function Prototypes														*/
/*																			*/
/****************************************************************************/
typedef enum
{
	JBI_ILLEGAL_JTAG_STATE = -1,
	RESET = 0,
	IDLE = 1,
	DRSELECT = 2,
	DRCAPTURE = 3,
	DRSHIFT = 4,
	DREXIT1 = 5,
	DRPAUSE = 6,
	DREXIT2 = 7,
	DRUPDATE = 8,
	IRSELECT = 9,
	IRCAPTURE = 10,
	IRSHIFT = 11,
	IREXIT1 = 12,
	IRPAUSE = 13,
	IREXIT2 = 14,
	IRUPDATE = 15

} JBIE_JTAG_STATE;


JBI_RETURN_TYPE jbi_init_jtag
(
	void
);

JBI_RETURN_TYPE jbi_set_drstop_state
(
    JBIE_JTAG_STATE state
);

JBI_RETURN_TYPE jbi_set_irstop_state
(
    JBIE_JTAG_STATE state
);

JBI_RETURN_TYPE jbi_set_dr_preamble
(
	unsigned long count,
	unsigned long start_index,
	unsigned char *preamble_data
);

JBI_RETURN_TYPE jbi_set_ir_preamble
(
	unsigned long count,
	unsigned long start_index,
	unsigned char *preamble_data
);

JBI_RETURN_TYPE jbi_set_dr_postamble
(
	unsigned long count,
	unsigned long start_index,
	unsigned char *postamble_data
);

JBI_RETURN_TYPE jbi_set_ir_postamble
(
	unsigned long count,
	unsigned long start_index,
	unsigned char *postamble_data
);

JBI_RETURN_TYPE jbi_goto_jtag_state
(
    JBIE_JTAG_STATE state
);

JBI_RETURN_TYPE jbi_do_wait_cycles
(
	long cycles,
	JBIE_JTAG_STATE wait_state
);

JBI_RETURN_TYPE jbi_do_wait_microseconds
(
	long microseconds,
	JBIE_JTAG_STATE wait_state
);

JBI_RETURN_TYPE jbi_do_irscan
(
	unsigned long count,
	unsigned char *tdi_data,
	unsigned long start_index
);

JBI_RETURN_TYPE jbi_swap_ir
(
	unsigned long count,
	unsigned char *in_data,
	unsigned long in_index,
	unsigned char *out_data,
	unsigned long out_index
);

JBI_RETURN_TYPE jbi_do_drscan
(
	unsigned long count,
	unsigned char *tdi_data,
	unsigned long start_index
);

JBI_RETURN_TYPE jbi_swap_dr
(
	unsigned long count,
	unsigned char *in_data,
	unsigned long in_index,
	unsigned char *out_data,
	unsigned long out_index
);

void jbi_free_jtag_padding_buffers
(
	long reset_jtag
);

JBI_RETURN_TYPE jbi_do_drscan_multi_page
(
	unsigned long variable_id,
	unsigned long long_count,
	unsigned long long_index,
	long version
);

JBI_RETURN_TYPE jbi_set_dr_preamble_ex
(
	unsigned long count,
	unsigned long start_index,
	unsigned long preamble_data_uint8Ptr
);

JBI_RETURN_TYPE jbi_set_ir_preamble_ex
(
	unsigned long count,
	unsigned long start_index,
	unsigned long preamble_data_uint8Ptr
);

JBI_RETURN_TYPE jbi_set_dr_postamble_ex
(
	unsigned long count,
	unsigned long start_index,
	unsigned long postamble_data_uint8Ptr
);

JBI_RETURN_TYPE jbi_set_ir_postamble_ex
(
	unsigned long count,
	unsigned long start_index,
	unsigned long postamble_data_uint8Ptr
);

void jbi_jtag_concatenate_data_ex
(
	unsigned char *buffer,
	unsigned char *preamble_data,
	unsigned long preamble_count,
	unsigned long target_data_uint8Ptr,
	unsigned long start_index,
	unsigned long target_count,
	unsigned char *postamble_data,
	unsigned long postamble_count
);

void jbi_jtag_extract_target_data_ex
(
	unsigned char *buffer,
	unsigned long target_data_uint8Ptr,
	unsigned long start_index,
	unsigned long preamble_count,
	unsigned long target_count
);

JBI_RETURN_TYPE jbi_do_irscan_ex
(
	unsigned long count,
	unsigned long tdi_data_uint8Ptr,
	unsigned long start_index
);

JBI_RETURN_TYPE jbi_swap_ir_ex
(
	unsigned long count,
	unsigned long in_data_uint8Ptr,
	unsigned long in_index,
	unsigned long out_data_uint8Ptr,
	unsigned long out_index
);

JBI_RETURN_TYPE jbi_do_drscan_ex
(
	unsigned long count,
	unsigned long tdi_data_uint8Ptr,
	unsigned long start_index
);

JBI_RETURN_TYPE jbi_swap_dr_ex
(
	unsigned long count,
	unsigned long in_data_uint8Ptr,
	unsigned long in_index,
	unsigned long out_data_uint8Ptr,
	unsigned long out_index
);

#endif /* INC_JBIJTAG_H */


