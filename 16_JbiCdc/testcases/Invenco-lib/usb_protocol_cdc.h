/*
 * usb_protocol_cdc.h
 *
 *  Created on: 25/02/2021
 *      Author: user1
 */

#ifndef USB_PROTOCOL_CDC_H_
#define USB_PROTOCOL_CDC_H_


typedef struct usb_cdc_line_coding
{

} usb_cdc_line_coding_t;

iram_size_t udi_cdc_get_free_tx_buffer(void);
iram_size_t udi_cdc_write_buf(const void * buf, iram_size_t size);
iram_size_t udi_cdc_get_nb_received_data();
iram_size_t udi_cdc_read_buf(void * buf, iram_size_t size);
bool udi_cdc_is_tx_ready(void);
int udi_cdc_putc(int value);

#endif /* USB_PROTOCOL_CDC_H_ */
