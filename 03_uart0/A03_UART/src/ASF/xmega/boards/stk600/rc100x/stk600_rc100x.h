/**
 * \file
 *
 * \brief STK600 with the RC100X routing card board header file.
 *
 * This file contains definitions and services related to the features of the
 * STK600 board with the routing card for 100-pin TQFP AVR XMEGA devices.
 *
 * To use this board, define BOARD=STK600_RC100X.
 *
 * Copyright (c) 2010-2011 Atmel Corporation. All rights reserved.
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

#ifndef _STK600_RC100_H_
#define _STK600_RC100_H_

#include "compiler.h"

/*! \name Miscellaneous data
 */
//! @{
//! @}

//! Number of LEDs.
#define LED_COUNT   8

/*! \name GPIO Connections of LEDs. To use these defines, connect the STK600
 * PORTE connector to the LEDs connector.
 */
//! @{
#  define LED0_GPIO   IOPORT_CREATE_PIN(PORTE,0)
#  define LED1_GPIO   IOPORT_CREATE_PIN(PORTE,1)
#  define LED2_GPIO   IOPORT_CREATE_PIN(PORTE,2)
#  define LED3_GPIO   IOPORT_CREATE_PIN(PORTE,3)
#  define LED4_GPIO   IOPORT_CREATE_PIN(PORTE,4)
#  define LED5_GPIO   IOPORT_CREATE_PIN(PORTE,5)
#  define LED6_GPIO   IOPORT_CREATE_PIN(PORTE,6)
#  define LED7_GPIO   IOPORT_CREATE_PIN(PORTE,7)
//! @}


/*! \name GPIO Connections of Push Buttons. To use these defines, connect the
 * STK600 the SWITCHES connector to:
 * - PORTF connector for ATxmegaA1
 * - PORTB connector for ATxmegaB1
 */
//! @{
#if !XMEGA_B1
#  define GPIO_PUSH_BUTTON_0   IOPORT_CREATE_PIN(PORTF,0)
#  define GPIO_PUSH_BUTTON_1   IOPORT_CREATE_PIN(PORTF,1)
#  define GPIO_PUSH_BUTTON_2   IOPORT_CREATE_PIN(PORTF,2)
#  define GPIO_PUSH_BUTTON_3   IOPORT_CREATE_PIN(PORTF,3)
#  define GPIO_PUSH_BUTTON_4   IOPORT_CREATE_PIN(PORTF,4)
#  define GPIO_PUSH_BUTTON_5   IOPORT_CREATE_PIN(PORTF,5)
#  define GPIO_PUSH_BUTTON_6   IOPORT_CREATE_PIN(PORTF,6)
#  define GPIO_PUSH_BUTTON_7   IOPORT_CREATE_PIN(PORTF,7)
#else //XMEGA B1 does not have PORTF, using PORTB instead
#  define GPIO_PUSH_BUTTON_0   IOPORT_CREATE_PIN(PORTB,0)
#  define GPIO_PUSH_BUTTON_1   IOPORT_CREATE_PIN(PORTB,1)
#  define GPIO_PUSH_BUTTON_2   IOPORT_CREATE_PIN(PORTB,2)
#  define GPIO_PUSH_BUTTON_3   IOPORT_CREATE_PIN(PORTB,3)
// !!Warning PB4 to PB7 are available only if JTAG is disabled.
#  define GPIO_PUSH_BUTTON_4   IOPORT_CREATE_PIN(PORTB,4)
#  define GPIO_PUSH_BUTTON_5   IOPORT_CREATE_PIN(PORTB,5)
#  define GPIO_PUSH_BUTTON_6   IOPORT_CREATE_PIN(PORTB,6)
#  define GPIO_PUSH_BUTTON_7   IOPORT_CREATE_PIN(PORTB,7)
#endif
//! @}

/*! \name SPI Connections of the AT45DBX Data Flash Memory. To use these defines,
 * connect :
 * - PC4 to /CS pin on DataFlash connector
 * - PC7 to SCK pin on DataFlash connector
 * - PC6 to SO pin on DataFlash connector
 * - PC5 to SI pin on DataFlash connector
 */
//! @{
#define AT45DBX_SPI           &SPIC
#define AT45DBX_CS            IOPORT_CREATE_PIN(PORTC,4)  // CS as output
#define AT45DBX_MASTER_SS     IOPORT_CREATE_PIN(PORTC,4)  // SS as output and overlayed with CS
#define AT45DBX_MASTER_SCK    IOPORT_CREATE_PIN(PORTC,7)  // SCK as output
#define AT45DBX_MASTER_MOSI   IOPORT_CREATE_PIN(PORTC,5)  // MOSI as output
#define AT45DBX_MASTER_MISO   IOPORT_CREATE_PIN(PORTC,6)  // MISO as input
#define AT45DBX_SPI_CLK_MASK  SYSCLK_PORT_C

//! @}

/*! \name External oscillator
 */
//@{
#define BOARD_XOSC_HZ          8000000
#define BOARD_XOSC_TYPE        XOSC_TYPE_EXTERNAL
#define BOARD_XOSC_STARTUP_US  500000
//@}

/*!
 Definations of Slots pins.
 */
//Port E
#  define PORTE0   IOPORT_CREATE_PIN(PORTE,0)
#  define PORTE1   IOPORT_CREATE_PIN(PORTE,1)
#  define PORTE2   IOPORT_CREATE_PIN(PORTE,2)
#  define PORTE3   IOPORT_CREATE_PIN(PORTE,3)
#  define PORTE4   IOPORT_CREATE_PIN(PORTE,4)
#  define PORTE5   IOPORT_CREATE_PIN(PORTE,5)
#  define PORTE6   IOPORT_CREATE_PIN(PORTE,6)
#  define PORTE7   IOPORT_CREATE_PIN(PORTE,7)
//Port F
#  define PORTF0   IOPORT_CREATE_PIN(PORTF,0)
#  define PORTF1   IOPORT_CREATE_PIN(PORTF,1)
#  define PORTF2   IOPORT_CREATE_PIN(PORTF,2)
#  define PORTF3   IOPORT_CREATE_PIN(PORTF,3)
#  define PORTF4   IOPORT_CREATE_PIN(PORTF,4)
#  define PORTF5   IOPORT_CREATE_PIN(PORTF,5)
#  define PORTF6   IOPORT_CREATE_PIN(PORTF,6)
#  define PORTF7   IOPORT_CREATE_PIN(PORTF,7)
//Port H
#  define PORTH0   IOPORT_CREATE_PIN(PORTH,0)
#  define PORTH1   IOPORT_CREATE_PIN(PORTH,1)
#  define PORTH2   IOPORT_CREATE_PIN(PORTH,2)
#  define PORTH3   IOPORT_CREATE_PIN(PORTH,3)
#  define PORTH4   IOPORT_CREATE_PIN(PORTH,4)
#  define PORTH5   IOPORT_CREATE_PIN(PORTH,5)
#  define PORTH6   IOPORT_CREATE_PIN(PORTH,6)
#  define PORTH7   IOPORT_CREATE_PIN(PORTH,7)
//Port J
#  define PORTJ0   IOPORT_CREATE_PIN(PORTJ,0)
#  define PORTJ1   IOPORT_CREATE_PIN(PORTJ,1)
#  define PORTJ2   IOPORT_CREATE_PIN(PORTJ,2)
#  define PORTJ3   IOPORT_CREATE_PIN(PORTJ,3)
#  define PORTJ4   IOPORT_CREATE_PIN(PORTJ,4)
#  define PORTJ5   IOPORT_CREATE_PIN(PORTJ,5)
#  define PORTJ6   IOPORT_CREATE_PIN(PORTJ,6)
#  define PORTJ7   IOPORT_CREATE_PIN(PORTJ,7)
//Port K
#  define PORTK0   IOPORT_CREATE_PIN(PORTK,0)
#  define PORTK1   IOPORT_CREATE_PIN(PORTK,1)
#  define PORTK2   IOPORT_CREATE_PIN(PORTK,2)
#  define PORTK3   IOPORT_CREATE_PIN(PORTK,3)
#  define PORTK4   IOPORT_CREATE_PIN(PORTK,4)
#  define PORTK5   IOPORT_CREATE_PIN(PORTK,5)
#  define PORTK6   IOPORT_CREATE_PIN(PORTK,6)
#  define PORTK7   IOPORT_CREATE_PIN(PORTK,7)
//Port A
#  define PORTA0   IOPORT_CREATE_PIN(PORTA,0)
#  define PORTA1   IOPORT_CREATE_PIN(PORTA,1)
#  define PORTA2   IOPORT_CREATE_PIN(PORTA,2)
#  define PORTA3   IOPORT_CREATE_PIN(PORTA,3)
#  define PORTA4   IOPORT_CREATE_PIN(PORTA,4)
#  define PORTA5   IOPORT_CREATE_PIN(PORTA,5)
#  define PORTA6   IOPORT_CREATE_PIN(PORTA,6)
#  define PORTA7   IOPORT_CREATE_PIN(PORTA,7)
//Port B
#  define PORTB0   IOPORT_CREATE_PIN(PORTB,0)
#  define PORTB1   IOPORT_CREATE_PIN(PORTB,1)
#  define PORTB2   IOPORT_CREATE_PIN(PORTB,2)
#  define PORTB3   IOPORT_CREATE_PIN(PORTB,3)
#  define PORTB4   IOPORT_CREATE_PIN(PORTB,4)
#  define PORTB5   IOPORT_CREATE_PIN(PORTB,5)
#  define PORTB6   IOPORT_CREATE_PIN(PORTB,6)
#  define PORTB7   IOPORT_CREATE_PIN(PORTB,7)
//Port C
#  define PORTC0   IOPORT_CREATE_PIN(PORTC,0)
#  define PORTC1   IOPORT_CREATE_PIN(PORTC,1)
#  define PORTC2   IOPORT_CREATE_PIN(PORTC,2)
#  define PORTC3   IOPORT_CREATE_PIN(PORTC,3)
#  define PORTC4   IOPORT_CREATE_PIN(PORTC,4)
#  define PORTC5   IOPORT_CREATE_PIN(PORTC,5)
#  define PORTC6   IOPORT_CREATE_PIN(PORTC,6)
#  define PORTC7   IOPORT_CREATE_PIN(PORTC,7)
//Port D
#  define PORTD0   IOPORT_CREATE_PIN(PORTD,0)
#  define PORTD1   IOPORT_CREATE_PIN(PORTD,1)
#  define PORTD2   IOPORT_CREATE_PIN(PORTD,2)
#  define PORTD3   IOPORT_CREATE_PIN(PORTD,3)



#endif  // _STK600_RC100_H_
