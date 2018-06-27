/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
#include <asf.h>

static void onAllLeds(void)
{
	//port E
	LED_On(PORTE0);
	LED_On(PORTE1);
	LED_On(PORTE2);
	LED_On(PORTE3);
	LED_On(PORTE4);
	LED_On(PORTE5);
	LED_On(PORTE6);
	LED_On(PORTE7);
	//port F
	LED_On(PORTF0);
	LED_On(PORTF1);
	LED_On(PORTF2);
	LED_On(PORTF3);
	LED_On(PORTF4);
	LED_On(PORTF5);
	LED_On(PORTF6);
	LED_On(PORTF7);
	//port H
	LED_On(PORTH0);
	LED_On(PORTH1);
	LED_On(PORTH2);
	LED_On(PORTH3);
	LED_On(PORTH4);
	LED_On(PORTH5);
	LED_On(PORTH6);
	LED_On(PORTH7);
	//port J
	LED_On(PORTJ0);
	LED_On(PORTJ1);
	LED_On(PORTJ2);
	LED_On(PORTJ3);
	LED_On(PORTJ4);
	LED_On(PORTJ5);
	LED_On(PORTJ6);
	LED_On(PORTJ7);
	//port K
	LED_On(PORTK0);
	LED_On(PORTK1);
	LED_On(PORTK2);
	LED_On(PORTK3);
	LED_On(PORTK4);
	LED_On(PORTK5);
	LED_On(PORTK6);
	LED_On(PORTK7);
	//port A
	LED_On(PORTA0);
	LED_On(PORTA1);
	LED_On(PORTA2);
	LED_On(PORTA3);
	LED_On(PORTA4);
	LED_On(PORTA5);
	LED_On(PORTA6);
	LED_On(PORTA7);
	//port B
	LED_On(PORTB0);
	LED_On(PORTB1);
	LED_On(PORTB2);
	LED_On(PORTB3);
	LED_On(PORTB4);
	LED_On(PORTB5);
	LED_On(PORTB6);
	LED_On(PORTB7);
	//port C
	LED_On(PORTC0);
	LED_On(PORTC1);
	LED_On(PORTC2);
	LED_On(PORTC3);
	LED_On(PORTC4);
	LED_On(PORTC5);
	LED_On(PORTC6);
	LED_On(PORTC7);
	//port D
	LED_On(PORTD0);
	LED_On(PORTD1);
	LED_On(PORTD2);
	LED_On(PORTD3);
}

static void offAllLeds(void)
{
	//port E
	LED_Off(PORTE0);
	LED_Off(PORTE1);
	LED_Off(PORTE2);
	LED_Off(PORTE3);
	LED_Off(PORTE4);
	LED_Off(PORTE5);
	LED_Off(PORTE6);
	LED_Off(PORTE7);
	//port F
	LED_Off(PORTF0);
	LED_Off(PORTF1);
	LED_Off(PORTF2);
	LED_Off(PORTF3);
	LED_Off(PORTF4);
	LED_Off(PORTF5);
	LED_Off(PORTF6);
	LED_Off(PORTF7);
	//port H
	LED_Off(PORTH0);
	LED_Off(PORTH1);
	LED_Off(PORTH2);
	LED_Off(PORTH3);
	LED_Off(PORTH4);
	LED_Off(PORTH5);
	LED_Off(PORTH6);
	LED_Off(PORTH7);
	//port J
	LED_Off(PORTJ0);
	LED_Off(PORTJ1);
	LED_Off(PORTJ2);
	LED_Off(PORTJ3);
	LED_Off(PORTJ4);
	LED_Off(PORTJ5);
	LED_Off(PORTJ6);
	LED_Off(PORTJ7);
	//port K
	LED_Off(PORTK0);
	LED_Off(PORTK1);
	LED_Off(PORTK2);
	LED_Off(PORTK3);
	LED_Off(PORTK4);
	LED_Off(PORTK5);
	LED_Off(PORTK6);
	LED_Off(PORTK7);
	//port A
	LED_Off(PORTA0);
	LED_Off(PORTA1);
	LED_Off(PORTA2);
	LED_Off(PORTA3);
	LED_Off(PORTA4);
	LED_Off(PORTA5);
	LED_Off(PORTA6);
	LED_Off(PORTA7);
	//port B
	LED_Off(PORTB0);
	LED_Off(PORTB1);
	LED_Off(PORTB2);
	LED_Off(PORTB3);
	LED_Off(PORTB4);
	LED_Off(PORTB5);
	LED_Off(PORTB6);
	LED_Off(PORTB7);
	//port C
	LED_Off(PORTC0);
	LED_Off(PORTC1);
	LED_Off(PORTC2);
	LED_Off(PORTC3);
	LED_Off(PORTC4);
	LED_Off(PORTC5);
	LED_Off(PORTC6);
	LED_Off(PORTC7);
	//port D
	LED_Off(PORTD0);
	LED_Off(PORTD1);
	LED_Off(PORTD2);
	LED_Off(PORTD3);
}

static void disableJtagPort(void)
{
	volatile unsigned char * pMCUCR = (volatile unsigned char *)0x0096;

	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;	
	//disable JTAG port.
	*pMCUCR = MCU_JTAGD_bm;
}

int main (void)
{
	unsigned long i;
	
	board_init();
	disableJtagPort();

	// Insert application code here, after the board has been initialized.
	for(i=0;;i++)
	{
		if(i&0x1000)
		{
			onAllLeds();
		}
		else
		{
			offAllLeds();
		}
	}
}
