/*
 * ECD300_board.c
 *
 * Created: 2014/12/17 9:59:09
 *  Author: user
 */ 
//#include "iox128a1u.h"
#include "compiler.h" //compiler.h can find the iox128a1u.h
#include "ECD300_board.h"

void onAllLeds(void)
{
	//port E
	PORTE.OUTSET=0xff;
	//port F
	PORTF.OUTSET=0xff;
	//port H
	PORTH.OUTSET=0xff;
	//port J
	PORTJ.OUTSET=0xff;
	//port K
	PORTK.OUTSET=0xff;
	//port A
	PORTA.OUTSET=0xff;
	//port B
	PORTB.OUTSET=0xff;
	//port C
	PORTC.OUTSET=0xff;
	//port D
	PORTD.OUTSET=0xff;
}

void offAllLeds(void)
{
	//port E
	PORTE.OUTCLR=0xff;
	//port F
	PORTF.OUTCLR=0xff;
	//port H
	PORTH.OUTCLR=0xff;
	//port J
	PORTJ.OUTCLR=0xff;
	//port K
	PORTK.OUTCLR=0xff;
	//port A
	PORTA.OUTCLR=0xff;
	//port B
	PORTB.OUTCLR=0xff;
	//port C
	PORTC.OUTCLR=0xff;
	//port D
	PORTD.OUTCLR=0xff;
}

void disableJtagPort(void)
{
	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;
	//disable JTAG port.
	MCU_MCUCR = MCU_JTAGD_bm;
}

void onLed(unsigned char ledIndex)
{
	switch(ledIndex)
	{
		case 0:
		PORTE.OUTSET=PIN0_bm;
		break;
		
		case 1:
		PORTE.OUTSET=PIN1_bm;
		break;
		
		case 2:
		PORTE.OUTSET=PIN2_bm;
		break;
		
		case 3:
		PORTE.OUTSET=PIN3_bm;
		break;
		
		case 4:
		PORTE.OUTSET=PIN4_bm;
		break;
		
		case 5:
		PORTE.OUTSET=PIN5_bm;
		break;
		
		case 6:
		PORTE.OUTSET=PIN6_bm;
		break;
		
		case 7:
		PORTE.OUTSET=PIN7_bm;
		break;

		case 8:
		PORTF.OUTSET=PIN0_bm;
		break;
		
		case 9:
		PORTF.OUTSET=PIN1_bm;
		break;
		
		case 10:
		PORTF.OUTSET=PIN2_bm;
		break;
		
		case 11:
		PORTF.OUTSET=PIN3_bm;
		break;
		
		case 12:
		PORTF.OUTSET=PIN4_bm;
		break;
		
		case 13:
		PORTF.OUTSET=PIN5_bm;
		break;
		
		case 14:
		PORTF.OUTSET=PIN6_bm;
		break;

		case 15:
		PORTF.OUTSET=PIN7_bm;
		break;

		case 16:
		PORTH.OUTSET=PIN0_bm;
		break;
		
		case 17:
		PORTH.OUTSET=PIN1_bm;
		break;
		
		case 18:
		PORTH.OUTSET=PIN2_bm;
		break;
		
		case 19:
		PORTH.OUTSET=PIN3_bm;
		break;
		
		case 20:
		PORTH.OUTSET=PIN4_bm;
		break;
		
		case 21:
		PORTH.OUTSET=PIN5_bm;
		break;
		
		case 22:
		PORTH.OUTSET=PIN6_bm;
		break;
		
		case 23:
		PORTH.OUTSET=PIN7_bm;
		break;

		case 24:
		PORTJ.OUTSET=PIN0_bm;
		break;
		
		case 25:
		PORTJ.OUTSET=PIN1_bm;
		break;
		
		case 26:
		PORTJ.OUTSET=PIN2_bm;
		break;
		
		case 27:
		PORTJ.OUTSET=PIN3_bm;
		break;
		
		case 28:
		PORTJ.OUTSET=PIN4_bm;
		break;
		
		case 29:
		PORTJ.OUTSET=PIN5_bm;
		break;
		
		case 30:
		PORTJ.OUTSET=PIN6_bm;
		break;
		
		case 31:
		PORTJ.OUTSET=PIN7_bm;
		break;

		case 32:
		PORTK.OUTSET=PIN0_bm;
		break;
		
		case 33:
		PORTK.OUTSET=PIN1_bm;
		break;
		
		case 34:
		PORTK.OUTSET=PIN2_bm;
		break;
		
		case 35:
		PORTK.OUTSET=PIN3_bm;
		break;
		
		case 36:
		PORTK.OUTSET=PIN4_bm;
		break;
		
		case 37:
		PORTK.OUTSET=PIN5_bm;
		break;
		
		case 38:
		PORTK.OUTSET=PIN6_bm;
		break;
		
		case 39:
		PORTK.OUTSET=PIN7_bm;
		break;

		case 40:
		PORTA.OUTSET=PIN0_bm;
		break;
		
		case 41:
		PORTA.OUTSET=PIN1_bm;
		break;
		
		case 42:
		PORTA.OUTSET=PIN2_bm;
		break;
		
		case 43:
		PORTA.OUTSET=PIN3_bm;
		break;
		
		case 44:
		PORTA.OUTSET=PIN4_bm;
		break;
		
		case 45:
		PORTA.OUTSET=PIN5_bm;
		break;
		
		case 46:
		PORTA.OUTSET=PIN6_bm;
		break;
		
		case 47:
		PORTA.OUTSET=PIN7_bm;
		break;

		case 48:
		PORTB.OUTSET=PIN0_bm;
		break;
		
		case 49:
		PORTB.OUTSET=PIN1_bm;
		break;
		
		case 50:
		PORTB.OUTSET=PIN2_bm;
		break;
		
		case 51:
		PORTB.OUTSET=PIN3_bm;
		break;
		
		case 52:
		PORTB.OUTSET=PIN4_bm;
		break;
		
		case 53:
		PORTB.OUTSET=PIN5_bm;
		break;
		
		case 54:
		PORTB.OUTSET=PIN6_bm;
		break;
		
		case 55:
		PORTB.OUTSET=PIN7_bm;
		break;

		case 56:
		PORTC.OUTSET=PIN0_bm;
		break;
		
		case 57:
		PORTC.OUTSET=PIN1_bm;
		break;
		
		case 58:
		PORTC.OUTSET=PIN2_bm;
		break;
		
		case 59:
		PORTC.OUTSET=PIN3_bm;
		break;
		
		case 60:
		PORTC.OUTSET=PIN4_bm;
		break;
		
		case 61:
		PORTC.OUTSET=PIN5_bm;
		break;
		
		case 62:
		PORTC.OUTSET=PIN6_bm;
		break;
		
		case 63:
		PORTC.OUTSET=PIN7_bm;
		break;
		
		case 64:
		PORTD.OUTSET=PIN0_bm;
		break;
		
		case 65:
		PORTD.OUTSET=PIN1_bm;
		break;
		
		case 66:
		PORTD.OUTSET=PIN2_bm;
		break;
		
		case 67:
		PORTD.OUTSET=PIN3_bm;
		break;
		
		default:
		break;
	}
}

void offLed(unsigned char ledIndex)
{
	switch(ledIndex)
	{
		case 0:
		PORTE.OUTCLR=PIN0_bm;
		break;
		
		case 1:
		PORTE.OUTCLR=PIN1_bm;
		break;
		
		case 2:
		PORTE.OUTCLR=PIN2_bm;
		break;
		
		case 3:
		PORTE.OUTCLR=PIN3_bm;
		break;
		
		case 4:
		PORTE.OUTCLR=PIN4_bm;
		break;
		
		case 5:
		PORTE.OUTCLR=PIN5_bm;
		break;
		
		case 6:
		PORTE.OUTCLR=PIN6_bm;
		break;
		
		case 7:
		PORTE.OUTCLR=PIN7_bm;
		break;

		case 8:
		PORTF.OUTCLR=PIN0_bm;
		break;
		
		case 9:
		PORTF.OUTCLR=PIN1_bm;
		break;
		
		case 10:
		PORTF.OUTCLR=PIN2_bm;
		break;
		
		case 11:
		PORTF.OUTCLR=PIN3_bm;
		break;
		
		case 12:
		PORTF.OUTCLR=PIN4_bm;
		break;
		
		case 13:
		PORTF.OUTCLR=PIN5_bm;
		break;
		
		case 14:
		PORTF.OUTCLR=PIN6_bm;
		break;

		case 15:
		PORTF.OUTCLR=PIN7_bm;
		break;

		case 16:
		PORTH.OUTCLR=PIN0_bm;
		break;
		
		case 17:
		PORTH.OUTCLR=PIN1_bm;
		break;
		
		case 18:
		PORTH.OUTCLR=PIN2_bm;
		break;
		
		case 19:
		PORTH.OUTCLR=PIN3_bm;
		break;
		
		case 20:
		PORTH.OUTCLR=PIN4_bm;
		break;
		
		case 21:
		PORTH.OUTCLR=PIN5_bm;
		break;
		
		case 22:
		PORTH.OUTCLR=PIN6_bm;
		break;
		
		case 23:
		PORTH.OUTCLR=PIN7_bm;
		break;

		case 24:
		PORTJ.OUTCLR=PIN0_bm;
		break;
		
		case 25:
		PORTJ.OUTCLR=PIN1_bm;
		break;
		
		case 26:
		PORTJ.OUTCLR=PIN2_bm;
		break;
		
		case 27:
		PORTJ.OUTCLR=PIN3_bm;
		break;
		
		case 28:
		PORTJ.OUTCLR=PIN4_bm;
		break;
		
		case 29:
		PORTJ.OUTCLR=PIN5_bm;
		break;
		
		case 30:
		PORTJ.OUTCLR=PIN6_bm;
		break;
		
		case 31:
		PORTJ.OUTCLR=PIN7_bm;
		break;

		case 32:
		PORTK.OUTCLR=PIN0_bm;
		break;
		
		case 33:
		PORTK.OUTCLR=PIN1_bm;
		break;
		
		case 34:
		PORTK.OUTCLR=PIN2_bm;
		break;
		
		case 35:
		PORTK.OUTCLR=PIN3_bm;
		break;
		
		case 36:
		PORTK.OUTCLR=PIN4_bm;
		break;
		
		case 37:
		PORTK.OUTCLR=PIN5_bm;
		break;
		
		case 38:
		PORTK.OUTCLR=PIN6_bm;
		break;
		
		case 39:
		PORTK.OUTCLR=PIN7_bm;
		break;

		case 40:
		PORTA.OUTCLR=PIN0_bm;
		break;
		
		case 41:
		PORTA.OUTCLR=PIN1_bm;
		break;
		
		case 42:
		PORTA.OUTCLR=PIN2_bm;
		break;
		
		case 43:
		PORTA.OUTCLR=PIN3_bm;
		break;
		
		case 44:
		PORTA.OUTCLR=PIN4_bm;
		break;
		
		case 45:
		PORTA.OUTCLR=PIN5_bm;
		break;
		
		case 46:
		PORTA.OUTCLR=PIN6_bm;
		break;
		
		case 47:
		PORTA.OUTCLR=PIN7_bm;
		break;

		case 48:
		PORTB.OUTCLR=PIN0_bm;
		break;
		
		case 49:
		PORTB.OUTCLR=PIN1_bm;
		break;
		
		case 50:
		PORTB.OUTCLR=PIN2_bm;
		break;
		
		case 51:
		PORTB.OUTCLR=PIN3_bm;
		break;
		
		case 52:
		PORTB.OUTCLR=PIN4_bm;
		break;
		
		case 53:
		PORTB.OUTCLR=PIN5_bm;
		break;
		
		case 54:
		PORTB.OUTCLR=PIN6_bm;
		break;
		
		case 55:
		PORTB.OUTCLR=PIN7_bm;
		break;

		case 56:
		PORTC.OUTCLR=PIN0_bm;
		break;
		
		case 57:
		PORTC.OUTCLR=PIN1_bm;
		break;
		
		case 58:
		PORTC.OUTCLR=PIN2_bm;
		break;
		
		case 59:
		PORTC.OUTCLR=PIN3_bm;
		break;
		
		case 60:
		PORTC.OUTCLR=PIN4_bm;
		break;
		
		case 61:
		PORTC.OUTCLR=PIN5_bm;
		break;
		
		case 62:
		PORTC.OUTCLR=PIN6_bm;
		break;
		
		case 63:
		PORTC.OUTCLR=PIN7_bm;
		break;
		
		case 64:
		PORTD.OUTCLR=PIN0_bm;
		break;
		
		case 65:
		PORTD.OUTCLR=PIN1_bm;
		break;
		
		case 66:
		PORTD.OUTCLR=PIN2_bm;
		break;
		
		case 67:
		PORTD.OUTCLR=PIN3_bm;
		break;
		
		default:
		break;
	}
}


