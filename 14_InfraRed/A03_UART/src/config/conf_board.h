/**
 * \file
 *
 * \brief ATxmega128A1U on STK600 board configuration template
 *
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

//! State that huge memory access (beyond 64k RAM) should be enabled
#define CONFIG_HAVE_HUGEMEM

//! Base address of SRAM on board
#define BOARD_EBI_SRAM_BASE    0x800000UL

//! Size of SRAM on board, given in bytes: 1 Mb / 128 kB
#define BOARD_EBI_SRAM_SIZE    0x20000UL

#if 0
// Enable on-board AT45DBX interface (SPI)
#define CONF_BOARD_AT45DBX

// Enable UART Communication Port interface (UART)
#define CONF_BOARD_ENABLE_USARTC0
#define CONF_BOARD_ENABLE_USARTC1
#define CONF_BOARD_ENABLE_USARTD0
#define CONF_BOARD_ENABLE_USARTD1
#define CONF_BOARD_ENABLE_USARTE0
#define CONF_BOARD_ENABLE_USARTE1
#define CONF_BOARD_ENABLE_USARTF0
#define CONF_BOARD_ENABLE_USARTF1
#endif


#endif // CONF_BOARD_H
