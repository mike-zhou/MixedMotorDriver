/*
 * ECD300_usart_config.h
 *
 * Created: 2014/12/18 10:50:33
 *  Author: user
 */ 


#ifndef ECD300_USART_CONFIG_H_
#define ECD300_USART_CONFIG_H_

//select one of the following 2 items to config UART0.
//#define ECD300_UART0_ENABLED_SYNC
//#define ECD300_UART0_ENABLED_INT
#ifdef ECD300_UART0_ENABLED_INT
//select the length of buffers, should be one of 0xff, 0x7f, 0x3f, 0x1f, 0xf, 0x7, 0x3.
#define ECD300_UART0_INT_RECEIVE_BUFFER_LENGTH 		0x3ff
#define ECD300_UART0_INT_TRANSMIT_BUFFER_LENGTH 	0x3ff
//the level can be 0x1(low), 0x2(medium), 0x3(high)
#define ECD300_UART0_INT_LEVEL_RECEIVE 	0x1
#define ECD300_UART0_INT_LEVEL_TRANSMIT 	0x1
#endif

//select one of the following 2 items to config UART1.
//#define ECD300_UART1_ENABLED_SYNC
//#define ECD300_UART1_ENABLED_INT
#ifdef ECD300_UART1_ENABLED_INT
//select the length of buffers, should be one of 0xff, 0x7f, 0x3f, 0x1f, 0xf, 0x7, 0x3.
#define ECD300_UART1_INT_RECEIVE_BUFFER_LENGTH 		0xf
#define ECD300_UART1_INT_TRANSMIT_BUFFER_LENGTH 	0x7f
//the level can be 0x1(low), 0x2(medium), 0x3(high)
#define ECD300_UART1_INT_LEVEL_RECEIVE 	0x1
#define ECD300_UART1_INT_LEVEL_TRANSMIT 	0x1
#endif

//select one of the following 2 items to config UART2.
//#define ECD300_UART2_ENABLED_SYNC
#define ECD300_UART2_ENABLED_INT
#ifdef ECD300_UART2_ENABLED_INT
//select the length of buffers, should be one of 0xff, 0x7f, 0x3f, 0x1f, 0xf, 0x7, 0x3.
#define ECD300_UART2_INT_RECEIVE_BUFFER_LENGTH 		0xff
#define ECD300_UART2_INT_TRANSMIT_BUFFER_LENGTH 	0xff
//the level can be 0x1(low), 0x2(medium), 0x3(high)
#define ECD300_UART2_INT_LEVEL_RECEIVE 	0x1
#define ECD300_UART2_INT_LEVEL_TRANSMIT 	0x1
#endif

//select one of the following 2 items to config UART3.
//#define ECD300_UART3_ENABLED_SYNC
//#define ECD300_UART3_ENABLED_INT
#ifdef ECD300_UART3_ENABLED_INT
//select the length of buffers, should be one of 0xff, 0x7f, 0x3f, 0x1f, 0xf, 0x7, 0x3.
#define ECD300_UART3_INT_RECEIVE_BUFFER_LENGTH 		0x7f
#define ECD300_UART3_INT_TRANSMIT_BUFFER_LENGTH 	0x7f
//the level can be 0x1(low), 0x2(medium), 0x3(high)
#define ECD300_UART3_INT_LEVEL_RECEIVE 	0x1
#define ECD300_UART3_INT_LEVEL_TRANSMIT 	0x1
#endif

//select one of the following 2 items to config UART4.
//#define ECD300_UART4_ENABLED_SYNC
//#define ECD300_UART4_ENABLED_INT
#ifdef ECD300_UART4_ENABLED_INT
//select the length of buffers, should be one of 0xff, 0x7f, 0x3f, 0x1f, 0xf, 0x7, 0x3.
#define ECD300_UART4_INT_RECEIVE_BUFFER_LENGTH 		0x7f
#define ECD300_UART4_INT_TRANSMIT_BUFFER_LENGTH 	0x7f
//the level can be 0x1(low), 0x2(medium), 0x3(high)
#define ECD300_UART4_INT_LEVEL_RECEIVE 	0x1
#define ECD300_UART4_INT_LEVEL_TRANSMIT 	0x1
#endif

//select one of the following 2 items to config UART5.
//#define ECD300_UART5_ENABLED_SYNC
//#define ECD300_UART5_ENABLED_INT
#ifdef ECD300_UART5_ENABLED_INT
//select the length of buffers, should be one of 0xff, 0x7f, 0x3f, 0x1f, 0xf, 0x7, 0x3.
#define ECD300_UART5_INT_RECEIVE_BUFFER_LENGTH 		0x7f
#define ECD300_UART5_INT_TRANSMIT_BUFFER_LENGTH 	0x7f
//the level can be 0x1(low), 0x2(medium), 0x3(high)
#define ECD300_UART5_INT_LEVEL_RECEIVE 	0x1
#define ECD300_UART5_INT_LEVEL_TRANSMIT 	0x1
#endif

//select one of the following 2 items to config UART6.
//#define ECD300_UART6_ENABLED_SYNC
//#define ECD300_UART6_ENABLED_INT
#ifdef ECD300_UART6_ENABLED_INT
//select the length of buffers, should be one of 0xff, 0x7f, 0x3f, 0x1f, 0xf, 0x7, 0x3.
#define ECD300_UART6_INT_RECEIVE_BUFFER_LENGTH 		0x7f
#define ECD300_UART6_INT_TRANSMIT_BUFFER_LENGTH 	0x7f
//the level can be 0x1(low), 0x2(medium), 0x3(high)
#define ECD300_UART6_INT_LEVEL_RECEIVE 	0x1
#define ECD300_UART6_INT_LEVEL_TRANSMIT 	0x1
#endif

#endif /* ECD300_USART_CONFIG_H_ */
