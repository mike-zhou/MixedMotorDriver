/*
 * ECD300_usart.c
 *
 * Created: 2014/12/18 10:51:24
 *  Author: user
 */ 
#include "ECD300.h"
#include "ECD300_usart.h"
#include "ECD300_usart_config.h"

#ifdef ECD300_UART0_ENABLED_SYNC
#else
#ifdef ECD300_UART0_ENABLED_INT
static volatile unsigned char uart0RxBuffer[ECD300_UART0_INT_RECEIVE_BUFFER_LENGTH+1];
static volatile unsigned char uart0RxHead=0;
static volatile unsigned char uart0RxTail=0;
static volatile unsigned char uart0RxOverflow=0;
static volatile unsigned char uart0TxBuffer[ECD300_UART0_INT_TRANSMIT_BUFFER_LENGTH+1];
static volatile unsigned short uart0TxHead=0;
static volatile unsigned short uart0TxTail=0;
//interrupt handler, append the character received to the tail of receiving buffer.
ISR(USARTC0_RXC_vect)
{
	unsigned char c;
	unsigned char nextIndex;

	c=USARTC0.DATA; //read character to clear the interrupt flag.
	nextIndex=(uart0RxTail+1)&ECD300_UART0_INT_RECEIVE_BUFFER_LENGTH;
	
	//only tail index is moved in this function.
	if(nextIndex!=uart0RxHead)
	{//receiving buffer is not full.
		uart0RxBuffer[uart0RxTail]=c;
		uart0RxTail=nextIndex;
	}
	else
	{//receiving buffer is full.
		uart0RxOverflow=1;
	}
}
//interrupt handler, retrieve a character from the head of transmitting buffer.
ISR(USARTC0_DRE_vect)
{
	//only head index is moved in this function.
	if(uart0TxHead!=uart0TxTail)
	{//transmitting buffer is not empty.
		USARTC0.DATA=uart0TxBuffer[uart0TxHead];
		uart0TxHead+=1;
		uart0TxHead&=ECD300_UART0_INT_TRANSMIT_BUFFER_LENGTH;
	}
	else
	{//buffer is empty, disable this interrupt according to the description about DREIF in the xmega mannual
		USARTC0.CTRLA=(USARTC0.CTRLA&~USART_DREINTLVL_gm)|USART_DREINTLVL_OFF_gc;
	}
}
#endif
#endif

#ifdef ECD300_UART1_ENABLED_SYNC
#else
#ifdef ECD300_UART1_ENABLED_INT
static volatile unsigned char uart1RxBuffer[ECD300_UART1_INT_RECEIVE_BUFFER_LENGTH+1];
static volatile unsigned char uart1RxHead=0;
static volatile unsigned char uart1RxTail=0;
static volatile unsigned char uart1RxOverflow=0;
static volatile unsigned char uart1TxBuffer[ECD300_UART1_INT_TRANSMIT_BUFFER_LENGTH+1];
static volatile unsigned char uart1TxHead=0;
static volatile unsigned char uart1TxTail=0;
//interrupt handler, append the character received to the tail of receiving buffer.
ISR(USARTC1_RXC_vect)
{
	unsigned char c;
	unsigned char nextIndex;

	c=USARTC1.DATA; //read character to clear the interrupt flag.
	nextIndex=(uart1RxTail+1)&ECD300_UART1_INT_RECEIVE_BUFFER_LENGTH;
	
	//only tail index is moved in this function.
	if(nextIndex!=uart1RxHead)
	{//receiving buffer is not full.
		uart1RxBuffer[uart1RxTail]=c;
		uart1RxTail=nextIndex;
	}
	else
	{//receiving buffer is full.
		uart1RxOverflow=1;
	}
}
//interrupt handler, retrieve a character from the head of transmitting buffer.
ISR(USARTC1_DRE_vect)
{
	//only head index is moved in this function.
	if(uart1TxHead!=uart1TxTail)
	{//transmitting buffer is not empty.
		USARTC1.DATA=uart1TxBuffer[uart1TxHead];
		uart1TxHead+=1;
		uart1TxHead&=ECD300_UART1_INT_TRANSMIT_BUFFER_LENGTH;
	}
	else
	{//buffer is empty, disable this interrupt according to the description about DREIF in the xmega mannual
		USARTC1.CTRLA=(USARTC1.CTRLA&~USART_DREINTLVL_gm)|USART_DREINTLVL_OFF_gc;
	}
}
#endif
#endif

#ifdef ECD300_UART2_ENABLED_SYNC
#else
#ifdef ECD300_UART2_ENABLED_INT
static volatile unsigned char uart2RxBuffer[ECD300_UART2_INT_RECEIVE_BUFFER_LENGTH+1];
static volatile unsigned char uart2RxHead=0;
static volatile unsigned char uart2RxTail=0;
static volatile unsigned char uart2RxOverflow=0;
static volatile unsigned char uart2TxBuffer[ECD300_UART2_INT_TRANSMIT_BUFFER_LENGTH+1];
static volatile unsigned char uart2TxHead=0;
static volatile unsigned char uart2TxTail=0;
//interrupt handler, append the character received to the tail of receiving buffer.
ISR(USARTD0_RXC_vect)
{
	unsigned char c;
	unsigned char nextIndex;

	c=USARTD0.DATA; //read character to clear the interrupt flag.
	nextIndex=(uart2RxTail+1)&ECD300_UART2_INT_RECEIVE_BUFFER_LENGTH;
	
	//only tail index is moved in this function.
	if(nextIndex!=uart2RxHead)
	{//receiving buffer is not full.
		uart2RxBuffer[uart2RxTail]=c;
		uart2RxTail=nextIndex;
	}
	else
	{//receiving buffer is full.
		uart2RxOverflow=1;
	}
}
//interrupt handler, retrieve a character from the head of transmitting buffer.
ISR(USARTD0_DRE_vect)
{
	//only head index is moved in this function.
	if(uart2TxHead!=uart2TxTail)
	{//transmitting buffer is not empty.
		USARTD0.DATA=uart2TxBuffer[uart2TxHead];
		uart2TxHead+=1;
		uart2TxHead&=ECD300_UART2_INT_TRANSMIT_BUFFER_LENGTH;
	}
	else
	{//buffer is empty, disable this interrupt according to the description about DREIF in the xmega mannual
		USARTD0.CTRLA=(USARTD0.CTRLA&~USART_DREINTLVL_gm)|USART_DREINTLVL_OFF_gc;
	}
}
#endif
#endif

#ifdef ECD300_UART3_ENABLED_SYNC
#else
#ifdef ECD300_UART3_ENABLED_INT
static volatile unsigned char uart3RxBuffer[ECD300_UART3_INT_RECEIVE_BUFFER_LENGTH+1];
static volatile unsigned char uart3RxHead=0;
static volatile unsigned char uart3RxTail=0;
static volatile unsigned char uart3RxOverflow=0;
static volatile unsigned char uart3TxBuffer[ECD300_UART3_INT_TRANSMIT_BUFFER_LENGTH+1];
static volatile unsigned char uart3TxHead=0;
static volatile unsigned char uart3TxTail=0;
//interrupt handler, append the character received to the tail of receiving buffer.
ISR(USARTE0_RXC_vect)
{
	unsigned char c;
	unsigned char nextIndex;

	c=USARTE0.DATA; //read character to clear the interrupt flag.
	nextIndex=(uart3RxTail+1)&ECD300_UART3_INT_RECEIVE_BUFFER_LENGTH;
	
	//only tail index is moved in this function.
	if(nextIndex!=uart3RxHead)
	{//receiving buffer is not full.
		uart3RxBuffer[uart3RxTail]=c;
		uart3RxTail=nextIndex;
	}
	else
	{//receiving buffer is full.
		uart3RxOverflow=1;
	}
}
//interrupt handler, retrieve a character from the head of transmitting buffer.
ISR(USARTE0_DRE_vect)
{
	//only head index is moved in this function.
	if(uart3TxHead!=uart3TxTail)
	{//transmitting buffer is not empty.
		USARTE0.DATA=uart3TxBuffer[uart3TxHead];
		uart3TxHead+=1;
		uart3TxHead&=ECD300_UART3_INT_TRANSMIT_BUFFER_LENGTH;
	}
	else
	{//buffer is empty, disable this interrupt according to the description about DREIF in the xmega mannual
		USARTE0.CTRLA=(USARTE0.CTRLA&~USART_DREINTLVL_gm)|USART_DREINTLVL_OFF_gc;
	}
}
#endif
#endif

#ifdef ECD300_UART4_ENABLED_SYNC
#else
#ifdef ECD300_UART4_ENABLED_INT
static volatile unsigned char uart4RxBuffer[ECD300_UART4_INT_RECEIVE_BUFFER_LENGTH+1];
static volatile unsigned char uart4RxHead=0;
static volatile unsigned char uart4RxTail=0;
static volatile unsigned char uart4RxOverflow=0;
static volatile unsigned char uart4TxBuffer[ECD300_UART4_INT_TRANSMIT_BUFFER_LENGTH+1];
static volatile unsigned char uart4TxHead=0;
static volatile unsigned char uart4TxTail=0;
//interrupt handler, append the character received to the tail of receiving buffer.
ISR(USARTE1_RXC_vect)
{
	unsigned char c;
	unsigned char nextIndex;

	c=USARTE1.DATA; //read character to clear the interrupt flag.
	nextIndex=(uart4RxTail+1)&ECD300_UART4_INT_RECEIVE_BUFFER_LENGTH;
	
	//only tail index is moved in this function.
	if(nextIndex!=uart4RxHead)
	{//receiving buffer is not full.
		uart4RxBuffer[uart4RxTail]=c;
		uart4RxTail=nextIndex;
	}
	else
	{//receiving buffer is full.
		uart4RxOverflow=1;
	}
}
//interrupt handler, retrieve a character from the head of transmitting buffer.
ISR(USARTE1_DRE_vect)
{
	//only head index is moved in this function.
	if(uart4TxHead!=uart4TxTail)
	{//transmitting buffer is not empty.
		USARTE1.DATA=uart4TxBuffer[uart4TxHead];
		uart4TxHead+=1;
		uart4TxHead&=ECD300_UART4_INT_TRANSMIT_BUFFER_LENGTH;
	}
	else
	{//buffer is empty, disable this interrupt according to the description about DREIF in the xmega mannual
		USARTE1.CTRLA=(USARTE1.CTRLA&~USART_DREINTLVL_gm)|USART_DREINTLVL_OFF_gc;
	}
}
#endif
#endif

#ifdef ECD300_UART5_ENABLED_SYNC
#else
#ifdef ECD300_UART5_ENABLED_INT
static volatile unsigned char uart5RxBuffer[ECD300_UART5_INT_RECEIVE_BUFFER_LENGTH+1];
static volatile unsigned char uart5RxHead=0;
static volatile unsigned char uart5RxTail=0;
static volatile unsigned char uart5RxOverflow=0;
static volatile unsigned char uart5TxBuffer[ECD300_UART5_INT_TRANSMIT_BUFFER_LENGTH+1];
static volatile unsigned char uart5TxHead=0;
static volatile unsigned char uart5TxTail=0;
//interrupt handler, append the character received to the tail of receiving buffer.
ISR(USARTF0_RXC_vect)
{
	unsigned char c;
	unsigned char nextIndex;

	c=USARTF0.DATA; //read character to clear the interrupt flag.
	nextIndex=(uart5RxTail+1)&ECD300_UART5_INT_RECEIVE_BUFFER_LENGTH;
	
	//only tail index is moved in this function.
	if(nextIndex!=uart5RxHead)
	{//receiving buffer is not full.
		uart5RxBuffer[uart5RxTail]=c;
		uart5RxTail=nextIndex;
	}
	else
	{//receiving buffer is full.
		uart5RxOverflow=1;
	}
}
//interrupt handler, retrieve a character from the head of transmitting buffer.
ISR(USARTF0_DRE_vect)
{
	//only head index is moved in this function.
	if(uart5TxHead!=uart5TxTail)
	{//transmitting buffer is not empty.
		USARTF0.DATA=uart5TxBuffer[uart5TxHead];
		uart5TxHead+=1;
		uart5TxHead&=ECD300_UART5_INT_TRANSMIT_BUFFER_LENGTH;
	}
	else
	{//buffer is empty, disable this interrupt according to the description about DREIF in the xmega mannual
		USARTF0.CTRLA=(USARTF0.CTRLA&~USART_DREINTLVL_gm)|USART_DREINTLVL_OFF_gc;
	}
}
#endif
#endif

#ifdef ECD300_UART6_ENABLED_SYNC
#else
#ifdef ECD300_UART5_ENABLED_INT
static volatile unsigned char uart6RxBuffer[ECD300_UART6_INT_RECEIVE_BUFFER_LENGTH+1];
static volatile unsigned char uart6RxHead=0;
static volatile unsigned char uart6RxTail=0;
static volatile unsigned char uart6RxOverflow=0;
static volatile unsigned char uart6TxBuffer[ECD300_UART6_INT_TRANSMIT_BUFFER_LENGTH+1];
static volatile unsigned char uart6TxHead=0;
static volatile unsigned char uart6TxTail=0;
//interrupt handler, append the character received to the tail of receiving buffer.
ISR(USARTF1_RXC_vect)
{
	unsigned char c;
	unsigned char nextIndex;

	c=USARTF1.DATA; //read character to clear the interrupt flag.
	nextIndex=(uart6RxTail+1)&ECD300_UART6_INT_RECEIVE_BUFFER_LENGTH;
	
	//only tail index is moved in this function.
	if(nextIndex!=uart6RxHead)
	{//receiving buffer is not full.
		uart6RxBuffer[uart6RxTail]=c;
		uart6RxTail=nextIndex;
	}
	else
	{//receiving buffer is full.
		uart6RxOverflow=1;
	}
}
//interrupt handler, retrieve a character from the head of transmitting buffer.
ISR(USARTF1_DRE_vect)
{
	//only head index is moved in this function.
	if(uart6TxHead!=uart6TxTail)
	{//transmitting buffer is not empty.
		USARTF1.DATA=uart6TxBuffer[uart6TxHead];
		uart6TxHead+=1;
		uart6TxHead&=ECD300_UART6_INT_TRANSMIT_BUFFER_LENGTH;
	}
	else
	{//buffer is empty, disable this interrupt according to the description about DREIF in the xmega mannual
		USARTF1.CTRLA=(USARTF1.CTRLA&~USART_DREINTLVL_gm)|USART_DREINTLVL_OFF_gc;
	}
}
#endif
#endif


 char ecd300InitUart(ECD300_UART_e uart, usart_rs232_options_t * pOptions)
{
	char rc;
	
	switch(uart)
	{
		case ECD300_UART_0:
			PORTC.DIRCLR=PIN2_bm;//set this pin as input.
			PORTC.DIRSET=PIN3_bm;//set this pin as output.
			if(usart_init_rs232(&USARTC0, pOptions))
				rc=0;
			else
				rc=-1;
#ifdef ECD300_UART0_ENABLED_SYNC
#else
#ifdef ECD300_UART0_ENABLED_INT
			uart0TxHead=0;
			uart0TxTail=0;
			uart0RxHead=0;
			uart0RxTail=0;
			//set the receiving interrupt level.
			USARTC0.CTRLA=(USARTC0.CTRLA&~USART_RXCINTLVL_gm)|(ECD300_UART0_INT_LEVEL_RECEIVE<<USART_RXCINTLVL_gp);
			//enable the interrupt of designated level.
			switch(ECD300_UART0_INT_LEVEL_RECEIVE)
			{
				case 0x1:
					PMIC.CTRL |= PMIC_LOLVLEX_bm;
					break;
				case 0x2:
					PMIC.CTRL |= PMIC_MEDLVLEX_bm;
					break;
				case 0x3:
					PMIC.CTRL |= PMIC_HILVLEX_bm;
					break;
				default:
					break;
			}
			//disable the transmition interrupt at present, it will be enabled when data is sent to transfer buffer.
			USARTC0.CTRLA=USARTC0.CTRLA&~USART_DREINTLVL_gm;
			//enable the interrupt of designated level.
			switch(ECD300_UART0_INT_LEVEL_TRANSMIT)
			{
				case 0x1:
					PMIC.CTRL |= PMIC_LOLVLEX_bm;
					break;
				case 0x2:
					PMIC.CTRL |= PMIC_MEDLVLEX_bm;
					break;
				case 0x3:
					PMIC.CTRL |= PMIC_HILVLEX_bm;
					break;
				default:
					break;
			}
#endif
#endif
			break;

		case ECD300_UART_1:
			PORTC.DIRCLR=PIN6_bm;//set this pin as input.
			PORTC.DIRSET=PIN7_bm;//set this pin as output.
			if(usart_init_rs232(&USARTC1, pOptions))
				rc=0;
			else
				rc=-1;
#ifdef ECD300_UART1_ENABLED_SYNC
#else
#ifdef ECD300_UART1_ENABLED_INT
			uart1TxHead=0;
			uart1TxTail=0;
			uart1RxHead=0;
			uart1RxTail=0;
			//set the receiving interrupt level.
			USARTC1.CTRLA=(USARTC1.CTRLA&~USART_RXCINTLVL_gm)|(ECD300_UART1_INT_LEVEL_RECEIVE<<USART_RXCINTLVL_gp);
			//enable the interrupt of designated level.
			switch(ECD300_UART1_INT_LEVEL_RECEIVE)
			{
				case 0x1:
					PMIC.CTRL |= PMIC_LOLVLEX_bm;
					break;
				case 0x2:
					PMIC.CTRL |= PMIC_MEDLVLEX_bm;
					break;
				case 0x3:
					PMIC.CTRL |= PMIC_HILVLEX_bm;
					break;
				default:
					break;
			}
			//disable the transmition interrupt at present, it will be enabled when data is sent to transfer buffer.
			USARTC1.CTRLA=USARTC1.CTRLA&~USART_DREINTLVL_gm;
			//enable the interrupt of designated level.
			switch(ECD300_UART1_INT_LEVEL_TRANSMIT)
			{
				case 0x1:
					PMIC.CTRL |= PMIC_LOLVLEX_bm;
					break;
				case 0x2:
					PMIC.CTRL |= PMIC_MEDLVLEX_bm;
					break;
				case 0x3:
					PMIC.CTRL |= PMIC_HILVLEX_bm;
					break;
				default:
					break;
			}
#endif
#endif
			break;

		case ECD300_UART_2:
			PORTD.DIRCLR=PIN2_bm;//set this pin as input.
			PORTD.DIRSET=PIN3_bm;//set this pin as output.
			if(usart_init_rs232(&USARTD0, pOptions))
				rc=0;
			else
				rc=-1;
#ifdef ECD300_UART2_ENABLED_SYNC
#else
#ifdef ECD300_UART2_ENABLED_INT
			uart2TxHead=0;
			uart2TxTail=0;
			uart2RxHead=0;
			uart2RxTail=0;
			//set the receiving interrupt level.
			USARTD0.CTRLA=(USARTD0.CTRLA&~USART_RXCINTLVL_gm)|(ECD300_UART2_INT_LEVEL_RECEIVE<<USART_RXCINTLVL_gp);
			//enable the interrupt of designated level.
			switch(ECD300_UART2_INT_LEVEL_RECEIVE)
			{
				case 0x1:
					PMIC.CTRL |= PMIC_LOLVLEX_bm;
					break;
				case 0x2:
					PMIC.CTRL |= PMIC_MEDLVLEX_bm;
					break;
				case 0x3:
					PMIC.CTRL |= PMIC_HILVLEX_bm;
					break;
				default:
					break;
			}
			//disable the transmition interrupt at present, it will be enabled when data is sent to transfer buffer.
			USARTD0.CTRLA=USARTD0.CTRLA&~USART_DREINTLVL_gm;
			//enable the interrupt of designated level.
			switch(ECD300_UART2_INT_LEVEL_TRANSMIT)
			{
				case 0x1:
					PMIC.CTRL |= PMIC_LOLVLEX_bm;
					break;
				case 0x2:
					PMIC.CTRL |= PMIC_MEDLVLEX_bm;
					break;
				case 0x3:
					PMIC.CTRL |= PMIC_HILVLEX_bm;
					break;
				default:
					break;
			}
#endif
#endif
			break;

		case ECD300_UART_3:
			PORTE.DIRCLR=PIN2_bm;//set this pin as input.
			PORTE.DIRSET=PIN3_bm;//set this pin as output.
			if(usart_init_rs232(&USARTE0, pOptions))
				rc=0;
			else
				rc=-1;
#ifdef ECD300_UART3_ENABLED_SYNC
#else
#ifdef ECD300_UART3_ENABLED_INT
			uart3TxHead=0;
			uart3TxTail=0;
			uart3RxHead=0;
			uart3RxTail=0;
			//set the receiving interrupt level.
			USARTE0.CTRLA=(USARTE0.CTRLA&~USART_RXCINTLVL_gm)|(ECD300_UART3_INT_LEVEL_RECEIVE<<USART_RXCINTLVL_gp);
			//enable the interrupt of designated level.
			switch(ECD300_UART3_INT_LEVEL_RECEIVE)
			{
				case 0x1:
					PMIC.CTRL |= PMIC_LOLVLEX_bm;
					break;
				case 0x2:
					PMIC.CTRL |= PMIC_MEDLVLEX_bm;
					break;
				case 0x3:
					PMIC.CTRL |= PMIC_HILVLEX_bm;
					break;
				default:
					break;
			}
			//disable the transmition interrupt at present, it will be enabled when data is sent to transfer buffer.
			USARTE0.CTRLA=USARTE0.CTRLA&~USART_DREINTLVL_gm;
			//enable the interrupt of designated level.
			switch(ECD300_UART3_INT_LEVEL_TRANSMIT)
			{
				case 0x1:
					PMIC.CTRL |= PMIC_LOLVLEX_bm;
					break;
				case 0x2:
					PMIC.CTRL |= PMIC_MEDLVLEX_bm;
					break;
				case 0x3:
					PMIC.CTRL |= PMIC_HILVLEX_bm;
					break;
				default:
					break;
			}
#endif
#endif
			break;

		case ECD300_UART_4:
			PORTE.DIRCLR=PIN6_bm;//set this pin as input.
			PORTE.DIRSET=PIN7_bm;//set this pin as output.
			if(usart_init_rs232(&USARTE1, pOptions))
				rc=0;
			else
				rc=-1;
#ifdef ECD300_UART4_ENABLED_SYNC
#else
#ifdef ECD300_UART4_ENABLED_INT
			uart4TxHead=0;
			uart4TxTail=0;
			uart4RxHead=0;
			uart4RxTail=0;
			//set the receiving interrupt level.
			USARTE1.CTRLA=(USARTE1.CTRLA&~USART_RXCINTLVL_gm)|(ECD300_UART4_INT_LEVEL_RECEIVE<<USART_RXCINTLVL_gp);
			//enable the interrupt of designated level.
			switch(ECD300_UART4_INT_LEVEL_RECEIVE)
			{
				case 0x1:
					PMIC.CTRL |= PMIC_LOLVLEX_bm;
					break;
				case 0x2:
					PMIC.CTRL |= PMIC_MEDLVLEX_bm;
					break;
				case 0x3:
					PMIC.CTRL |= PMIC_HILVLEX_bm;
					break;
				default:
					break;
			}
			//disable the transmition interrupt at present, it will be enabled when data is sent to transfer buffer.
			USARTE1.CTRLA=USARTE1.CTRLA&~USART_DREINTLVL_gm;
			//enable the interrupt of designated level.
			switch(ECD300_UART4_INT_LEVEL_TRANSMIT)
			{
				case 0x1:
					PMIC.CTRL |= PMIC_LOLVLEX_bm;
					break;
				case 0x2:
					PMIC.CTRL |= PMIC_MEDLVLEX_bm;
					break;
				case 0x3:
					PMIC.CTRL |= PMIC_HILVLEX_bm;
					break;
				default:
					break;
			}
#endif
#endif
			break;

		case ECD300_UART_5:
			PORTF.DIRCLR=PIN2_bm;//set this pin as input.
			PORTF.DIRSET=PIN3_bm;//set this pin as output.
			if(usart_init_rs232(&USARTF0, pOptions))
				rc=0;
			else
				rc=-1;
#ifdef ECD300_UART5_ENABLED_SYNC
#else
#ifdef ECD300_UART5_ENABLED_INT
			uart5TxHead=0;
			uart5TxTail=0;
			uart5RxHead=0;
			uart5RxTail=0;
			//set the receiving interrupt level.
			USARTF0.CTRLA=(USARTF0.CTRLA&~USART_RXCINTLVL_gm)|(ECD300_UART5_INT_LEVEL_RECEIVE<<USART_RXCINTLVL_gp);
			//enable the interrupt of designated level.
			switch(ECD300_UART5_INT_LEVEL_RECEIVE)
			{
				case 0x1:
					PMIC.CTRL |= PMIC_LOLVLEX_bm;
					break;
				case 0x2:
					PMIC.CTRL |= PMIC_MEDLVLEX_bm;
					break;
				case 0x3:
					PMIC.CTRL |= PMIC_HILVLEX_bm;
					break;
				default:
					break;
			}
			//disable the transmition interrupt at present, it will be enabled when data is sent to transfer buffer.
			USARTF0.CTRLA=USARTF0.CTRLA&~USART_DREINTLVL_gm;
			//enable the interrupt of designated level.
			switch(ECD300_UART5_INT_LEVEL_TRANSMIT)
			{
				case 0x1:
					PMIC.CTRL |= PMIC_LOLVLEX_bm;
					break;
				case 0x2:
					PMIC.CTRL |= PMIC_MEDLVLEX_bm;
					break;
				case 0x3:
					PMIC.CTRL |= PMIC_HILVLEX_bm;
					break;
				default:
					break;
			}
#endif
#endif
			break;

		case ECD300_UART_6:
			PORTF.DIRCLR=PIN6_bm;//set this pin as input.
			PORTF.DIRSET=PIN7_bm;//set this pin as output.
			if(usart_init_rs232(&USARTF1, pOptions))
				rc=0;
			else
				rc=-1;
#ifdef ECD300_UART6_ENABLED_SYNC
#else
#ifdef ECD300_UART6_ENABLED_INT
			uart6TxHead=0;
			uart6TxTail=0;
			uart6RxHead=0;
			uart6RxTail=0;
			//set the receiving interrupt level.
			USARTF1.CTRLA=(USARTF1.CTRLA&~USART_RXCINTLVL_gm)|(ECD300_UART6_INT_LEVEL_RECEIVE<<USART_RXCINTLVL_gp);
			//enable the interrupt of designated level.
			switch(ECD300_UART6_INT_LEVEL_RECEIVE)
			{
				case 0x1:
					PMIC.CTRL |= PMIC_LOLVLEX_bm;
					break;
				case 0x2:
					PMIC.CTRL |= PMIC_MEDLVLEX_bm;
					break;
				case 0x3:
					PMIC.CTRL |= PMIC_HILVLEX_bm;
					break;
				default:
					break;
			}
			//disable the transmition interrupt at present, it will be enabled when data is sent to transfer buffer.
			USARTF1.CTRLA=USARTF1.CTRLA&~USART_DREINTLVL_gm;
			//enable the interrupt of designated level.
			switch(ECD300_UART6_INT_LEVEL_TRANSMIT)
			{
				case 0x1:
					PMIC.CTRL |= PMIC_LOLVLEX_bm;
					break;
				case 0x2:
					PMIC.CTRL |= PMIC_MEDLVLEX_bm;
					break;
				case 0x3:
					PMIC.CTRL |= PMIC_HILVLEX_bm;
					break;
				default:
					break;
			}
#endif
#endif
			break;

		default:
			rc=-1;
	}

	return rc;
}

char ecd300PutChar(ECD300_UART_e uart, unsigned char character)
{
	char rc=-1;
	
	switch(uart)
	{
		case ECD300_UART_0:
#ifdef ECD300_UART0_ENABLED_SYNC
			if(STATUS_OK==usart_putchar(&USARTC0, character))
				rc=0;
			else
				rc=-1;
#else
#ifdef ECD300_UART0_ENABLED_INT
			if(((uart0TxTail+1)&ECD300_UART0_INT_TRANSMIT_BUFFER_LENGTH)!=uart0TxHead)
			{//the buffer is not full.
				//put character to buffer.
				uart0TxBuffer[uart0TxTail]=character;
				uart0TxTail=(uart0TxTail+1)&ECD300_UART0_INT_TRANSMIT_BUFFER_LENGTH;
				//triger the interrupt to send data.
				USARTC0.CTRLA=(USARTC0.CTRLA&~USART_DREINTLVL_gm)|(ECD300_UART0_INT_LEVEL_TRANSMIT<<USART_DREINTLVL_gp);
				rc=0;
			}
			else
			{//the buffer is full
				rc=-2;
			}
#endif
#endif
			break;

		case ECD300_UART_1:
#ifdef ECD300_UART1_ENABLED_SYNC
			if(STATUS_OK==usart_putchar(&USARTC1, character))
				rc=0;
			else
				rc=-1;
#else
#ifdef ECD300_UART1_ENABLED_INT
			if(((uart1TxTail+1)&ECD300_UART1_INT_TRANSMIT_BUFFER_LENGTH)!=uart1TxHead)
			{//the buffer is not full.
				//put character to buffer.
				uart1TxBuffer[uart1TxTail]=character;
				uart1TxTail=(uart1TxTail+1)&ECD300_UART1_INT_TRANSMIT_BUFFER_LENGTH;
				//triger the interrupt to send data.
				USARTC1.CTRLA=(USARTC1.CTRLA&~USART_DREINTLVL_gm)|(ECD300_UART1_INT_LEVEL_TRANSMIT<<USART_DREINTLVL_gp);
				rc=0;
			}
			else
			{//the buffer is full
				rc=-2;
			}
#endif
#endif
			break;

		case ECD300_UART_2:
#ifdef ECD300_UART2_ENABLED_SYNC
			if(STATUS_OK==usart_putchar(&USARTD0, character))
				rc=0;
			else
				rc=-1;
#else
#ifdef ECD300_UART2_ENABLED_INT
			if(((uart2TxTail+1)&ECD300_UART2_INT_TRANSMIT_BUFFER_LENGTH)!=uart2TxHead)
			{//the buffer is not full.
				//put character to buffer.
				uart2TxBuffer[uart2TxTail]=character;
				uart2TxTail=(uart2TxTail+1)&ECD300_UART2_INT_TRANSMIT_BUFFER_LENGTH;
				//triger the interrupt to send data.
				USARTD0.CTRLA=(USARTD0.CTRLA&~USART_DREINTLVL_gm)|(ECD300_UART2_INT_LEVEL_TRANSMIT<<USART_DREINTLVL_gp);
				rc=0;
			}
			else
			{//the buffer is full
				rc=-2;
			}
#endif
#endif
			break;

		case ECD300_UART_3:
#ifdef ECD300_UART3_ENABLED_SYNC
			if(STATUS_OK==usart_putchar(&USARTE0, character))
				rc=0;
			else
				rc=-1;
#else
#ifdef ECD300_UART3_ENABLED_INT
			if(((uart3TxTail+1)&ECD300_UART3_INT_TRANSMIT_BUFFER_LENGTH)!=uart3TxHead)
			{//the buffer is not full.
				//put character to buffer.
				uart3TxBuffer[uart3TxTail]=character;
				uart3TxTail=(uart3TxTail+1)&ECD300_UART3_INT_TRANSMIT_BUFFER_LENGTH;
				//triger the interrupt to send data.
				USARTE0.CTRLA=(USARTE0.CTRLA&~USART_DREINTLVL_gm)|(ECD300_UART3_INT_LEVEL_TRANSMIT<<USART_DREINTLVL_gp);
				rc=0;
			}
			else
			{//the buffer is full
				rc=-2;
			}
#endif
#endif
			break;

		case ECD300_UART_4:
#ifdef ECD300_UART4_ENABLED_SYNC
			if(STATUS_OK==usart_putchar(&USARTE1, character))
				rc=0;
			else
				rc=-1;
#else
#ifdef ECD300_UART4_ENABLED_INT
			if(((uart4TxTail+1)&ECD300_UART4_INT_TRANSMIT_BUFFER_LENGTH)!=uart4TxHead)
			{//the buffer is not full.
				//put character to buffer.
				uart4TxBuffer[uart4TxTail]=character;
				uart4TxTail=(uart4TxTail+1)&ECD300_UART4_INT_TRANSMIT_BUFFER_LENGTH;
				//triger the interrupt to send data.
				USARTE1.CTRLA=(USARTE1.CTRLA&~USART_DREINTLVL_gm)|(ECD300_UART4_INT_LEVEL_TRANSMIT<<USART_DREINTLVL_gp);
				rc=0;
			}
			else
			{//the buffer is full
				rc=-2;
			}
#endif
#endif
			break;

		case ECD300_UART_5:
#ifdef ECD300_UART5_ENABLED_SYNC
			if(STATUS_OK==usart_putchar(&USARTF0, character))
				rc=0;
			else
				rc=-1;
#else
#ifdef ECD300_UART5_ENABLED_INT
			if(((uart5TxTail+1)&ECD300_UART5_INT_TRANSMIT_BUFFER_LENGTH)!=uart5TxHead)
			{//the buffer is not full.
				//put character to buffer.
				uart5TxBuffer[uart5TxTail]=character;
				uart5TxTail=(uart5TxTail+1)&ECD300_UART5_INT_TRANSMIT_BUFFER_LENGTH;
				//triger the interrupt to send data.
				USARTF0.CTRLA=(USARTF0.CTRLA&~USART_DREINTLVL_gm)|(ECD300_UART5_INT_LEVEL_TRANSMIT<<USART_DREINTLVL_gp);
				rc=0;
			}
			else
			{//the buffer is full
				rc=-2;
			}
#endif
#endif
			break;

		case ECD300_UART_6:
#ifdef ECD300_UART6_ENABLED_SYNC
			if(STATUS_OK==usart_putchar(&USARTF1, character))
				rc=0;
			else
				rc=-1;
#else
#ifdef ECD300_UART6_ENABLED_INT
			if(((uart6TxTail+1)&ECD300_UART6_INT_TRANSMIT_BUFFER_LENGTH)!=uart6TxHead)
			{//the buffer is not full.
				//put character to buffer.
				uart6TxBuffer[uart6TxTail]=character;
				uart6TxTail=(uart6TxTail+1)&ECD300_UART6_INT_TRANSMIT_BUFFER_LENGTH;
				//triger the interrupt to send data.
				USARTF1.CTRLA=(USARTF1.CTRLA&~USART_DREINTLVL_gm)|(ECD300_UART6_INT_LEVEL_TRANSMIT<<USART_DREINTLVL_gp);
				rc=0;
			}
			else
			{//the buffer is full
				rc=-2;
			}
#endif
#endif
			break;

		default:
			rc=-1;
	}

	return rc;
}

char ecd300PollChar(ECD300_UART_e uart, unsigned char * pCharacter)
{
	char rc=-1;
	
	switch(uart)
	{
		case ECD300_UART_0:
#ifdef ECD300_UART0_ENABLED_SYNC
			if(USARTC0.STATUS&USART_RXCIF_bm)
			{//data available
				*pCharacter=USARTC0.DATA;
				rc=1;
			}
			else
			{//no data.
				rc=0;
			}
#else
#ifdef ECD300_UART0_ENABLED_INT
			if(uart0RxHead!=uart0RxTail)
			{//data available.
				*pCharacter=uart0RxBuffer[uart0RxHead];
				uart0RxHead=(uart0RxHead+1)&ECD300_UART0_INT_RECEIVE_BUFFER_LENGTH;
				rc=1;
			}
			else
			{//no data.
				rc=0;
			}
#endif
#endif

			break;

		case ECD300_UART_1:
#ifdef ECD300_UART1_ENABLED_SYNC
			if(USARTC1.STATUS&USART_RXCIF_bm)
			{//data available
				*pCharacter=USARTC1.DATA;
				rc=1;
			}
			else
			{//no data.
				rc=0;
			}
#else
#ifdef ECD300_UART1_ENABLED_INT
			if(uart1RxHead!=uart1RxTail)
			{//data available.
				*pCharacter=uart1RxBuffer[uart1RxHead];
				uart1RxHead=(uart1RxHead+1)&ECD300_UART1_INT_RECEIVE_BUFFER_LENGTH;
				rc=1;
			}
			else
			{//no data.
				rc=0;
			}
#endif
#endif
			break;

		case ECD300_UART_2:
#ifdef ECD300_UART2_ENABLED_SYNC
			if(USARTD0.STATUS&USART_RXCIF_bm)
			{//data available
				*pCharacter=USARTD0.DATA;
				rc=1;
			}
			else
			{//no data.
				rc=0;
			}
#else
#ifdef ECD300_UART2_ENABLED_INT
			if(uart2RxHead!=uart2RxTail)
			{//data available.
				*pCharacter=uart2RxBuffer[uart2RxHead];
				uart2RxHead=(uart2RxHead+1)&ECD300_UART2_INT_RECEIVE_BUFFER_LENGTH;
				rc=1;
			}
			else
			{//no data.
				rc=0;
			}
#endif
#endif
			break;

		case ECD300_UART_3:
#ifdef ECD300_UART3_ENABLED_SYNC
			if(USARTE0.STATUS&USART_RXCIF_bm)
			{//data available
				*pCharacter=USARTE0.DATA;
				rc=1;
			}
			else
			{//no data.
				rc=0;
			}
#else
#ifdef ECD300_UART3_ENABLED_INT
			if(uart3RxHead!=uart3RxTail)
			{//data available.
				*pCharacter=uart3RxBuffer[uart3RxHead];
				uart3RxHead=(uart3RxHead+1)&ECD300_UART3_INT_RECEIVE_BUFFER_LENGTH;
				rc=1;
			}
			else
			{//no data.
				rc=0;
			}
#endif
#endif
			break;

		case ECD300_UART_4:
#ifdef ECD300_UART4_ENABLED_SYNC
			if(USARTE1.STATUS&USART_RXCIF_bm)
			{//data available
				*pCharacter=USARTE1.DATA;
				rc=1;
			}
			else
			{//no data.
				rc=0;
			}
#else
#ifdef ECD300_UART4_ENABLED_INT
			if(uart4RxHead!=uart4RxTail)
			{//data available.
				*pCharacter=uart4RxBuffer[uart4RxHead];
				uart4RxHead=(uart4RxHead+1)&ECD300_UART4_INT_RECEIVE_BUFFER_LENGTH;
				rc=1;
			}
			else
			{//no data.
				rc=0;
			}
#endif
#endif
			break;

		case ECD300_UART_5:
#ifdef ECD300_UART5_ENABLED_SYNC
			if(USARTF0.STATUS&USART_RXCIF_bm)
			{//data available
				*pCharacter=USARTF0.DATA;
				rc=1;
			}
			else
			{//no data.
				rc=0;
			}
#else
#ifdef ECD300_UART5_ENABLED_INT
			if(uart5RxHead!=uart5RxTail)
			{//data available.
				*pCharacter=uart5RxBuffer[uart5RxHead];
				uart5RxHead=(uart5RxHead+1)&ECD300_UART5_INT_RECEIVE_BUFFER_LENGTH;
				rc=1;
			}
			else
			{//no data.
				rc=0;
			}
#endif
#endif
			break;

		case ECD300_UART_6:
#ifdef ECD300_UART6_ENABLED_SYNC
			if(USARTF1.STATUS&USART_RXCIF_bm)
			{//data available
				*pCharacter=USARTF1.DATA;
				rc=1;
			}
			else
			{//no data.
				rc=0;
			}
#else
#ifdef ECD300_UART6_ENABLED_INT
			if(uart6RxHead!=uart6RxTail)
			{//data available.
				*pCharacter=uart6RxBuffer[uart6RxHead];
				uart6RxHead=(uart6RxHead+1)&ECD300_UART6_INT_RECEIVE_BUFFER_LENGTH;
				rc=1;
			}
			else
			{//no data.
				rc=0;
			}
#endif
#endif
			break;

		default:
			break;
	}

	return rc;
}

char ecd300GetChar(ECD300_UART_e uart, unsigned char * pCharacter)
{
	unsigned char c;
	char rc=-1;
	
	while(1)
	{
		rc=ecd300PollChar(uart, &c);
		if(rc==0)
		{//no data.
			continue;
		}
		else if(rc==1)
		{//a character is received.
			*pCharacter=c;
			rc=1;
			break;
		}
		else
		{//error.
			rc=-1;
			break;
		}
	}
	
	return rc;
}

char ecd300PutString(ECD300_UART_e uart, char * pString)
{
	char rc=0;
	unsigned char c;

	for(;;)
	{
		c=*pString;
		if(c!='\0')
		{
			//wait if there is no room.
			while(0!=ecd300PutChar(uart, c))
			{
				;
			}
			if(0!=rc)
			{
				break;
			}
			pString++;
		}
		else
		{
			break;
		}
	}

	if(0==rc)
		return 0;
	else 
		return -1;
}

char ecd300PutHexChar(ECD300_UART_e uart, unsigned char character)
{
	char buffer[5];
	char rc;

	buffer[0]='0';
	buffer[1]='x';
	buffer[4]='\0';

	if((character&0xf)<=9)
	{
		buffer[3]=(character&0xf)+'0';
	}
	else
	{
		buffer[3]=(character&0xf)-10+'A';
	}

	character>>=4;
	if((character&0xf)<=9)
	{
		buffer[2]=(character&0xf)+'0';
	}
	else
	{
		buffer[2]=(character&0xf)-10+'A';
	}

	rc=ecd300PutString(uart, buffer);
	if(rc==0)
	{
		return 0;
	}
	else 
	{
		return -1;
	}
}

char ecd300PutBinChar(ECD300_UART_e uart, unsigned char character)
{
	char buffer[11];
	char rc;

	buffer[0]='b';
	buffer[5]=',';
	buffer[10]='\0';

	if(character&0x80)
	{
		buffer[1]='1';
	}
	else
	{
		buffer[1]='0';
	}

	character<<=1;
	if(character&0x80)
	{
		buffer[2]='1';
	}
	else
	{
		buffer[2]='0';
	}

	character<<=1;
	if(character&0x80)
	{
		buffer[3]='1';
	}
	else
	{
		buffer[3]='0';
	}

	character<<=1;
	if(character&0x80)
	{
		buffer[4]='1';
	}
	else
	{
		buffer[4]='0';
	}

	character<<=1;
	if(character&0x80)
	{
		buffer[6]='1';
	}
	else
	{
		buffer[6]='0';
	}

	character<<=1;
	if(character&0x80)
	{
		buffer[7]='1';
	}
	else
	{
		buffer[7]='0';
	}

	character<<=1;
	if(character&0x80)
	{
		buffer[8]='1';
	}
	else
	{
		buffer[8]='0';
	}

	character<<=1;
	if(character&0x80)
	{
		buffer[9]='1';
	}
	else
	{
		buffer[9]='0';
	}
	
	rc=ecd300PutString(uart, buffer);
	if(rc==0)
	{
		return 0;
	}
	else 
	{
		return -1;
	}
}


