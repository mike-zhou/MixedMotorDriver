/*
 * asf.h
 *
 *  Created on: 25/02/2021
 *      Author: user1
 */

#ifndef ASF_H_
#define ASF_H_

#include <stdbool.h>
#include <stdio.h>

typedef unsigned short iram_size_t;
typedef unsigned char uint8_t;
typedef unsigned long uint32_t;

#define EBI_PORT_SRAM 	0x1
#define EBI_PORT_3PORT 	0x1
#define EBI_PORT_CS0 	0x1

#define EBI_CS_MODE_SRAM_gc 	0x1
#define EBI_CS_ASPACE_128KB_gc 	0x1
#define BOARD_EBI_SRAM_BASE 	0x1
#define EBI_CS_SRWS_3CLK_gc 	0x1

#define USART_CHSIZE_8BIT_gc 0x1
#define USART_PMODE_DISABLED_gc 0x1

typedef struct Fake_usart_rs232_options
{
	int baudrate;
	int charlength;
	int paritytype;
	int stopbits;
} usart_rs232_options_t;

struct ebi_cs_config
{

};

void ebi_setup_port(unsigned char addr, unsigned char sram, unsigned char lpc, unsigned char flags);
void ebi_cs_set_mode(struct ebi_cs_config * p, unsigned char mode);
void ebi_cs_set_address_size(struct ebi_cs_config * p, unsigned char mode);
void ebi_cs_set_base_address(struct ebi_cs_config * p, unsigned char mode);
void ebi_cs_set_sram_wait_states(struct ebi_cs_config * p, unsigned char mode);
void ebi_cs_write_config(unsigned char mode, struct ebi_cs_config * p);
void ebi_enable_cs(unsigned char mode, struct ebi_cs_config * p);


#endif /* ASF_H_ */
