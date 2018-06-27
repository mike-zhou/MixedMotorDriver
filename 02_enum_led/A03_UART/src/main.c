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
	gpio_set_pin_high(PORTE0);
	gpio_set_pin_high(PORTE1);
	gpio_set_pin_high(PORTE2);
	gpio_set_pin_high(PORTE3);
	gpio_set_pin_high(PORTE4);
	gpio_set_pin_high(PORTE5);
	gpio_set_pin_high(PORTE6);
	gpio_set_pin_high(PORTE7);
	//port F
	gpio_set_pin_high(PORTF0);
	gpio_set_pin_high(PORTF1);
	gpio_set_pin_high(PORTF2);
	gpio_set_pin_high(PORTF3);
	gpio_set_pin_high(PORTF4);
	gpio_set_pin_high(PORTF5);
	gpio_set_pin_high(PORTF6);
	gpio_set_pin_high(PORTF7);
	//port H
	gpio_set_pin_high(PORTH0);
	gpio_set_pin_high(PORTH1);
	gpio_set_pin_high(PORTH2);
	gpio_set_pin_high(PORTH3);
	gpio_set_pin_high(PORTH4);
	gpio_set_pin_high(PORTH5);
	gpio_set_pin_high(PORTH6);
	gpio_set_pin_high(PORTH7);
	//port J
	gpio_set_pin_high(PORTJ0);
	gpio_set_pin_high(PORTJ1);
	gpio_set_pin_high(PORTJ2);
	gpio_set_pin_high(PORTJ3);
	gpio_set_pin_high(PORTJ4);
	gpio_set_pin_high(PORTJ5);
	gpio_set_pin_high(PORTJ6);
	gpio_set_pin_high(PORTJ7);
	//port K
	gpio_set_pin_high(PORTK0);
	gpio_set_pin_high(PORTK1);
	gpio_set_pin_high(PORTK2);
	gpio_set_pin_high(PORTK3);
	gpio_set_pin_high(PORTK4);
	gpio_set_pin_high(PORTK5);
	gpio_set_pin_high(PORTK6);
	gpio_set_pin_high(PORTK7);
	//port A
	gpio_set_pin_high(PORTA0);
	gpio_set_pin_high(PORTA1);
	gpio_set_pin_high(PORTA2);
	gpio_set_pin_high(PORTA3);
	gpio_set_pin_high(PORTA4);
	gpio_set_pin_high(PORTA5);
	gpio_set_pin_high(PORTA6);
	gpio_set_pin_high(PORTA7);
	//port B
	gpio_set_pin_high(PORTB0);
	gpio_set_pin_high(PORTB1);
	gpio_set_pin_high(PORTB2);
	gpio_set_pin_high(PORTB3);
	gpio_set_pin_high(PORTB4);
	gpio_set_pin_high(PORTB5);
	gpio_set_pin_high(PORTB6);
	gpio_set_pin_high(PORTB7);
	//port C
	gpio_set_pin_high(PORTC0);
	gpio_set_pin_high(PORTC1);
	gpio_set_pin_high(PORTC2);
	gpio_set_pin_high(PORTC3);
	gpio_set_pin_high(PORTC4);
	gpio_set_pin_high(PORTC5);
	gpio_set_pin_high(PORTC6);
	gpio_set_pin_high(PORTC7);
	//port D
	gpio_set_pin_high(PORTD0);
	gpio_set_pin_high(PORTD1);
	gpio_set_pin_high(PORTD2);
	gpio_set_pin_high(PORTD3);
}

static void offAllLeds(void)
{
	//port E
	gpio_set_pin_low(PORTE0);
	gpio_set_pin_low(PORTE1);
	gpio_set_pin_low(PORTE2);
	gpio_set_pin_low(PORTE3);
	gpio_set_pin_low(PORTE4);
	gpio_set_pin_low(PORTE5);
	gpio_set_pin_low(PORTE6);
	gpio_set_pin_low(PORTE7);
	//port F
	gpio_set_pin_low(PORTF0);
	gpio_set_pin_low(PORTF1);
	gpio_set_pin_low(PORTF2);
	gpio_set_pin_low(PORTF3);
	gpio_set_pin_low(PORTF4);
	gpio_set_pin_low(PORTF5);
	gpio_set_pin_low(PORTF6);
	gpio_set_pin_low(PORTF7);
	//port H
	gpio_set_pin_low(PORTH0);
	gpio_set_pin_low(PORTH1);
	gpio_set_pin_low(PORTH2);
	gpio_set_pin_low(PORTH3);
	gpio_set_pin_low(PORTH4);
	gpio_set_pin_low(PORTH5);
	gpio_set_pin_low(PORTH6);
	gpio_set_pin_low(PORTH7);
	//port J
	gpio_set_pin_low(PORTJ0);
	gpio_set_pin_low(PORTJ1);
	gpio_set_pin_low(PORTJ2);
	gpio_set_pin_low(PORTJ3);
	gpio_set_pin_low(PORTJ4);
	gpio_set_pin_low(PORTJ5);
	gpio_set_pin_low(PORTJ6);
	gpio_set_pin_low(PORTJ7);
	//port K
	gpio_set_pin_low(PORTK0);
	gpio_set_pin_low(PORTK1);
	gpio_set_pin_low(PORTK2);
	gpio_set_pin_low(PORTK3);
	gpio_set_pin_low(PORTK4);
	gpio_set_pin_low(PORTK5);
	gpio_set_pin_low(PORTK6);
	gpio_set_pin_low(PORTK7);
	//port A
	gpio_set_pin_low(PORTA0);
	gpio_set_pin_low(PORTA1);
	gpio_set_pin_low(PORTA2);
	gpio_set_pin_low(PORTA3);
	gpio_set_pin_low(PORTA4);
	gpio_set_pin_low(PORTA5);
	gpio_set_pin_low(PORTA6);
	gpio_set_pin_low(PORTA7);
	//port B
	gpio_set_pin_low(PORTB0);
	gpio_set_pin_low(PORTB1);
	gpio_set_pin_low(PORTB2);
	gpio_set_pin_low(PORTB3);
	gpio_set_pin_low(PORTB4);
	gpio_set_pin_low(PORTB5);
	gpio_set_pin_low(PORTB6);
	gpio_set_pin_low(PORTB7);
	//port C
	gpio_set_pin_low(PORTC0);
	gpio_set_pin_low(PORTC1);
	gpio_set_pin_low(PORTC2);
	gpio_set_pin_low(PORTC3);
	gpio_set_pin_low(PORTC4);
	gpio_set_pin_low(PORTC5);
	gpio_set_pin_low(PORTC6);
	gpio_set_pin_low(PORTC7);
	//port D
	gpio_set_pin_low(PORTD0);
	gpio_set_pin_low(PORTD1);
	gpio_set_pin_low(PORTD2);
	gpio_set_pin_low(PORTD3);
}

static void disableJtagPort(void)
{
	volatile unsigned char * pMCUCR = (volatile unsigned char *)0x0096;

	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;	
	//disable JTAG port.
	*pMCUCR = MCU_JTAGD_bm;
}

static void onLed(unsigned char ledIndex)
{
	switch(ledIndex)
	{
		case 0:
			gpio_set_pin_high(PORTE0);
			break;
			
		case 1:
			gpio_set_pin_high(PORTE1);
			break;
			
		case 2:
			gpio_set_pin_high(PORTE2);
			break;
			
		case 3:
			gpio_set_pin_high(PORTE3);
			break;
			
		case 4:
			gpio_set_pin_high(PORTE4);
			break;
			
		case 5:
			gpio_set_pin_high(PORTE5);
			break;
			
		case 6:
			gpio_set_pin_high(PORTE6);
			break;
			
		case 7:
			gpio_set_pin_high(PORTE7);
			break;

		case 8:
		gpio_set_pin_high(PORTF0);
		break;
		
		case 9:
		gpio_set_pin_high(PORTF1);
		break;
		
		case 10:
		gpio_set_pin_high(PORTF2);
		break;
		
		case 11:
		gpio_set_pin_high(PORTF3);
		break;
		
		case 12:
		gpio_set_pin_high(PORTF4);
		break;
		
		case 13:
		gpio_set_pin_high(PORTF5);
		break;
		
		case 14:
		gpio_set_pin_high(PORTF6);
		break;

		case 15:
		gpio_set_pin_high(PORTF7);
		break;

		case 16:
		gpio_set_pin_high(PORTH0);
		break;
		
		case 17:
		gpio_set_pin_high(PORTH1);
		break;
		
		case 18:
		gpio_set_pin_high(PORTH2);
		break;
		
		case 19:
		gpio_set_pin_high(PORTH3);
		break;
		
		case 20:
		gpio_set_pin_high(PORTH4);
		break;
		
		case 21:
		gpio_set_pin_high(PORTH5);
		break;
		
		case 22:
		gpio_set_pin_high(PORTH6);
		break;
		
		case 23:
		gpio_set_pin_high(PORTH7);
		break;

		case 24:
		gpio_set_pin_high(PORTJ0);
		break;
		
		case 25:
		gpio_set_pin_high(PORTJ1);
		break;
		
		case 26:
		gpio_set_pin_high(PORTJ2);
		break;
		
		case 27:
		gpio_set_pin_high(PORTJ3);
		break;
		
		case 28:
		gpio_set_pin_high(PORTJ4);
		break;
		
		case 29:
		gpio_set_pin_high(PORTJ5);
		break;
		
		case 30:
		gpio_set_pin_high(PORTJ6);
		break;
		
		case 31:
		gpio_set_pin_high(PORTJ7);
		break;

		case 32:
		gpio_set_pin_high(PORTK0);
		break;
		
		case 33:
		gpio_set_pin_high(PORTK1);
		break;
		
		case 34:
		gpio_set_pin_high(PORTK2);
		break;
		
		case 35:
		gpio_set_pin_high(PORTK3);
		break;
		
		case 36:
		gpio_set_pin_high(PORTK4);
		break;
		
		case 37:
		gpio_set_pin_high(PORTK5);
		break;
		
		case 38:
		gpio_set_pin_high(PORTK6);
		break;
		
		case 39:
		gpio_set_pin_high(PORTK7);
		break;

		case 40:
		gpio_set_pin_high(PORTA0);
		break;
		
		case 41:
		gpio_set_pin_high(PORTA1);
		break;
		
		case 42:
		gpio_set_pin_high(PORTA2);
		break;
		
		case 43:
		gpio_set_pin_high(PORTA3);
		break;
		
		case 44:
		gpio_set_pin_high(PORTA4);
		break;
		
		case 45:
		gpio_set_pin_high(PORTA5);
		break;
		
		case 46:
		gpio_set_pin_high(PORTA6);
		break;
		
		case 47:
		gpio_set_pin_high(PORTA7);
		break;

		case 48:
		gpio_set_pin_high(PORTB0);
		break;
		
		case 49:
		gpio_set_pin_high(PORTB1);
		break;
		
		case 50:
		gpio_set_pin_high(PORTB2);
		break;
		
		case 51:
		gpio_set_pin_high(PORTB3);
		break;
		
		case 52:
		gpio_set_pin_high(PORTB4);
		break;
		
		case 53:
		gpio_set_pin_high(PORTB5);
		break;
		
		case 54:
		gpio_set_pin_high(PORTB6);
		break;
		
		case 55:
		gpio_set_pin_high(PORTB7);
		break;

		case 56:
		gpio_set_pin_high(PORTC0);
		break;
		
		case 57:
		gpio_set_pin_high(PORTC1);
		break;
		
		case 58:
		gpio_set_pin_high(PORTC2);
		break;
		
		case 59:
		gpio_set_pin_high(PORTC3);
		break;
		
		case 60:
		gpio_set_pin_high(PORTC4);
		break;
		
		case 61:
		gpio_set_pin_high(PORTC5);
		break;
		
		case 62:
		gpio_set_pin_high(PORTC6);
		break;
		
		case 63:
		gpio_set_pin_high(PORTC7);
		break;
		
		case 64:
		gpio_set_pin_high(PORTD0);
		break;
		
		case 65:
		gpio_set_pin_high(PORTD1);
		break;
		
		case 66:
		gpio_set_pin_high(PORTD2);
		break;
		
		case 67:
		gpio_set_pin_high(PORTD3);
		break;
		
		default:
			break;
	}
}

int main (void)
{
	unsigned long i;
	unsigned long j;
	unsigned long k;
	
	board_init();
	disableJtagPort();

	// Insert application code here, after the board has been initialized.
	for(;;)
	{
		//blink all LEDs
		for(i=0;i<0x1000;i++)
		{
			offAllLeds();
		}
		for(i=0;i<0x1000;i++)
		{
			onAllLeds();
		}
		
		//enumerate LED one by one.
		for(j=0;j<68;j++)
		{
			offAllLeds();
			for(i=0;i<0x4000;i++)
				onLed(j);
		}	
		
		//light on even and odd LEDs alternatively.
		for(i=0;;i++)
		{
			//turn off all LEDs
			offAllLeds();
			//light odd LEDs.
			for(j=0;j<68;j++)
			{
				if((j&1)!=0)
					onLed(j);
			}
			//delay
			for(k=0;k<0x20000;k++)
				;
			//turn off all LEDs.
			offAllLeds();
			//turn on even LEDs.
			for(j=0;j<68;j++)
			{
				if((j&1)==0)
					onLed(j);
			}
			//delay
			for(k=0;k<0x20000;k++)
				;
	
		}
	}
}
