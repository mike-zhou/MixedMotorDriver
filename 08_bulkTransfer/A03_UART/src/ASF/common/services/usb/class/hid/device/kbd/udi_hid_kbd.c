/**
 * \file
 *
 * \brief USB Device Human Interface Device (HID) keyboard interface.
 *
 * Copyright (c) 2009-2012 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include "conf_usb.h"
#include "usb_protocol.h"
#include "udd.h"
#include "udc.h"
#include "udi_hid.h"
#include "udi_hid_kbd.h"
#include <string.h>

/**
 * \ingroup udi_hid_keyboard_group
 * \defgroup udi_hid_keyboard_group_udc Interface with USB Device Core (UDC)
 *
 * Structures and functions required by UDC.
 * 
 * @{
 */

extern void printString(char * pString);
extern void printHex(unsigned char hex);


bool udi_hid_kbd_enable(void);
void udi_hid_kbd_disable(void);
bool udi_hid_kbd_setup(void);
uint8_t udi_hid_kbd_getsetting(void);

//! Global structure which contains standard UDI interface for UDC
UDC_DESC_STORAGE udi_api_t udi_api_hid_kbd = {
	.enable = (bool(*)(void))udi_hid_kbd_enable,
	.disable = (void (*)(void))udi_hid_kbd_disable,
	.setup = (bool(*)(void))udi_hid_kbd_setup,
	.getsetting = (uint8_t(*)(void))udi_hid_kbd_getsetting,
	.sof_notify = NULL,
};
//@}


/**
 * \ingroup udi_hid_keyboard_group
 * \defgroup udi_hid_keyboard_group_internal Implementation of UDI HID keyboard
 *
 * Class internal implementation
 * @{
 */

/**
 * \name Internal defines and variables to manage HID keyboard
 */
//@{

static unsigned char ep1_buffer[4];
static unsigned char ep2_buffer[4];
static unsigned char ep3_buffer[4];
static unsigned char ep4_buffer[4];

static unsigned long ep1_bytes_received;
static unsigned long ep2_bytes_received;
static unsigned long ep3_bytes_sent;
static unsigned long ep4_bytes_sent;




//! Size of report for standard HID keyboard
#define UDI_HID_KBD_REPORT_SIZE  8


//! To store current rate of HID keyboard
static uint8_t udi_hid_kbd_rate;
//! To store current protocol of HID keyboard
static uint8_t udi_hid_kbd_protocol;
//! To store report feedback from USB host
static uint8_t udi_hid_kbd_report_set;
//! To signal if a valid report is ready to send
//static bool udi_hid_kbd_b_report_valid;
//! Report ready to send
//static uint8_t udi_hid_kbd_report[UDI_HID_KBD_REPORT_SIZE];
//! Signal if a report transfer is on going
//static bool udi_hid_kbd_b_report_trans_ongoing;
//! Buffer used to send report
COMPILER_WORD_ALIGNED
		static uint8_t
		udi_hid_kbd_report_trans[UDI_HID_KBD_REPORT_SIZE];

//@}

//! HID report descriptor for standard HID keyboard
UDC_DESC_STORAGE udi_hid_kbd_report_desc_t udi_hid_kbd_report_desc = {
	{
				0x05, 0x01,	/* Usage Page (Generic Desktop)      */
				0x09, 0x06,	/* Usage (Keyboard)                  */
				0xA1, 0x01,	/* Collection (Application)          */
				0x05, 0x07,	/* Usage Page (Keyboard)             */
				0x19, 224,	/* Usage Minimum (224)               */
				0x29, 231,	/* Usage Maximum (231)               */
				0x15, 0x00,	/* Logical Minimum (0)               */
				0x25, 0x01,	/* Logical Maximum (1)               */
				0x75, 0x01,	/* Report Size (1)                   */
				0x95, 0x08,	/* Report Count (8)                  */
				0x81, 0x02,	/* Input (Data, Variable, Absolute)  */
				0x81, 0x01,	/* Input (Constant)                  */
				0x19, 0x00,	/* Usage Minimum (0)                 */
				0x29, 101,	/* Usage Maximum (101)               */
				0x15, 0x00,	/* Logical Minimum (0)               */
				0x25, 101,	/* Logical Maximum (101)             */
				0x75, 0x08,	/* Report Size (8)                   */
				0x95, 0x06,	/* Report Count (6)                  */
				0x81, 0x00,	/* Input (Data, Array)               */
				0x05, 0x08,	/* Usage Page (LED)                  */
				0x19, 0x01,	/* Usage Minimum (1)                 */
				0x29, 0x05,	/* Usage Maximum (5)                 */
				0x15, 0x00,	/* Logical Minimum (0)               */
				0x25, 0x01,	/* Logical Maximum (1)               */
				0x75, 0x01,	/* Report Size (1)                   */
				0x95, 0x05,	/* Report Count (5)                  */
				0x91, 0x02,	/* Output (Data, Variable, Absolute) */
				0x95, 0x03,	/* Report Count (3)                  */
				0x91, 0x01,	/* Output (Constant)                 */
				0xC0	/* End Collection                    */
			}
};

/**
 * \name Internal routines
 */
//@{

/**
 * \brief Changes keyboard report states (like LEDs)
 *
 * \param rate       New rate value
 *
 */
static bool udi_hid_kbd_setreport(void);

/**
 * \brief Send the report
 *
 * \return \c 1 if send on going, \c 0 if delay.
 */
static bool udi_hid_kbd_send_report(void);

/**
 * \brief Callback called when the report is sent
 *
 * \param status     UDD_EP_TRANSFER_OK, if transfer is completed
 * \param status     UDD_EP_TRANSFER_ABORT, if transfer is aborted
 * \param nb_sent    number of data transfered
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
static void udi_hid_kbd_report_sent(udd_ep_status_t status, iram_size_t nb_sent,
		udd_ep_id_t ep);

/**
 * \brief Callback called to update report from USB host
 * udi_hid_kbd_report_set is updated before callback execution
 */
static void udi_hid_kbd_setreport_valid(void);

static void ep_callback(udd_ep_status_t status, iram_size_t nb_transferred, udd_ep_id_t ep)
{
	switch(ep)
	{
		case UDI_HID_KBD_EP_OUT_1:
			if(UDD_EP_TRANSFER_OK==status)
			{
				printString("|EP_OUT_1 succeed");
				ep1_bytes_received+=nb_transferred;
			}
			else
			{
				printString("|EP_OUT_1 failed");
			}
			break;
			
		case UDI_HID_KBD_EP_OUT_2:
			if(UDD_EP_TRANSFER_OK==status)
			{
				printString("|EP_OUT_2 succeed");
				ep2_bytes_received+=nb_transferred;
			}
			else
			{
				printString("|EP_OUT_2 failed");
			}
			break;
			
		case UDI_HID_KBD_EP_IN_3:
			if(UDD_EP_TRANSFER_OK==status)
			{
				printString("|EP_IN_3 succeed");
				ep3_bytes_sent+=nb_transferred;
			}
			else
			{
				printString("|EP_IN_3 failed");
			}
			break;
			
		case UDI_HID_KBD_EP_IN_4:
			if(UDD_EP_TRANSFER_OK==status)
			{
				printString("|EP_IN_4 succeed");
				ep4_bytes_sent+=nb_transferred;
			}
			else
			{
				printString("|EP_IN_4 failed");
			}
			break;
			
		default:
			printString("|unknown ep");
			break;
	}
}


//@}


//--------------------------------------------
//------ Interface for UDI HID level

bool udi_hid_kbd_enable(void)
{
	bool rc;
		
	printString("|udi_hid_kbd_enable");
	ep1_bytes_received=0;
	ep2_bytes_received=0;
	ep3_bytes_sent=0;
	ep4_bytes_sent=0;

	rc=udd_ep_run(UDI_HID_KBD_EP_OUT_1, false, ep1_buffer, UDI_HID_KBD_EP_SIZE, ep_callback);
	if(false==rc)
	{
		printString("|ep1 failed");
	}
		
	rc=udd_ep_run(UDI_HID_KBD_EP_OUT_2, false, ep2_buffer, UDI_HID_KBD_EP_SIZE, ep_callback);
	if(false==rc)
	{
		printString("|ep2 failed");
	}
	
	return true;
}


void udi_hid_kbd_disable(void)
{
	//UDI_HID_KBD_DISABLE_EXT();
}


bool udi_hid_kbd_setup(void)
{
	return udi_hid_setup(&udi_hid_kbd_rate,
								&udi_hid_kbd_protocol,
								(uint8_t *) &udi_hid_kbd_report_desc,
								udi_hid_kbd_setreport);
}


uint8_t udi_hid_kbd_getsetting(void)
{
	return 0;
}


static bool udi_hid_kbd_setreport(void)
{
	if ((USB_HID_REPORT_TYPE_OUTPUT == (udd_g_ctrlreq.req.wValue >> 8))
			&& (0 == (0xFF & udd_g_ctrlreq.req.wValue))
			&& (1 == udd_g_ctrlreq.req.wLength)) {
		// Report OUT type on report ID 0 from USB Host
		udd_g_ctrlreq.payload = &udi_hid_kbd_report_set;
		udd_g_ctrlreq.callback = udi_hid_kbd_setreport_valid;
		udd_g_ctrlreq.payload_size = 1;
		return true;
	}
	return false;
}


static void udi_hid_kbd_setreport_valid(void)
{
	//UDI_HID_KBD_CHANGE_LED(udi_hid_kbd_report_set);
}



void app_process_transfer(void)
{
	irqflags_t flags;

	while(1)
	{
		if(0!=ep1_bytes_received)
		{
			//EP1 received some data.
			flags = cpu_irq_save();
			
			printString("EP1 received ");
			printHex((unsigned char)ep1_bytes_received);
			printString("\r\n");
			//send back data to host with EP3
			udd_ep_run(UDI_HID_KBD_EP_IN_3, true, ep1_buffer, 4, ep_callback);
			ep1_bytes_received=0;
			
			cpu_irq_restore(flags);
		}
		if(0!=ep2_bytes_received)
		{
			//EP2 received some data.
			flags = cpu_irq_save();
			
			printString("EP2 received ");
			printHex((unsigned char)ep2_bytes_received);
			printString("\r\n");
			//send back data to host with EP4
			udd_ep_run(UDI_HID_KBD_EP_IN_4, true, ep2_buffer, 4, ep_callback);
			ep2_bytes_received=0;
			
			cpu_irq_restore(flags);
		}
		if(0!=ep3_bytes_sent)
		{
			//EP3 sent some data.
			flags = cpu_irq_save();
			
			printString("EP3 sent ");
			printHex((unsigned char)ep3_bytes_sent);
			printString("\r\n");
			ep3_bytes_sent=0;
			
			cpu_irq_restore(flags);
		}
		if(0!=ep4_bytes_sent)
		{
			//EP4 sent some data.
			flags = cpu_irq_save();
			
			printString("EP4 sent ");
			printHex((unsigned char)ep4_bytes_sent);
			printString("\r\n");
			ep4_bytes_sent=0;
			
			cpu_irq_restore(flags);
		}
		
	}
}

//@}
