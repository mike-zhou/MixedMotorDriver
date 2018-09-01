#include "Invenco_lib.h"

//enable pull down resistors in all ports.
static void shortcircute_pull_down_ports()
{
	PORTA.PIN0CTRL=(PORTA.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTA.PIN1CTRL=(PORTA.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTA.PIN2CTRL=(PORTA.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTA.PIN3CTRL=(PORTA.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTA.PIN4CTRL=(PORTA.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTA.PIN5CTRL=(PORTA.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTA.PIN6CTRL=(PORTA.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTA.PIN7CTRL=(PORTA.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	
	PORTB.PIN0CTRL=(PORTB.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTB.PIN1CTRL=(PORTB.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTB.PIN2CTRL=(PORTB.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTB.PIN3CTRL=(PORTB.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTB.PIN4CTRL=(PORTB.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTB.PIN5CTRL=(PORTB.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTB.PIN6CTRL=(PORTB.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTB.PIN7CTRL=(PORTB.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTC.PIN0CTRL=(PORTC.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTC.PIN1CTRL=(PORTC.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTC.PIN2CTRL=(PORTC.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTC.PIN3CTRL=(PORTC.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTC.PIN4CTRL=(PORTC.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTC.PIN5CTRL=(PORTC.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTC.PIN6CTRL=(PORTC.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTC.PIN7CTRL=(PORTC.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTD.PIN0CTRL=(PORTD.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTD.PIN1CTRL=(PORTD.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTD.PIN4CTRL=(PORTD.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTD.PIN5CTRL=(PORTD.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	
	PORTE.PIN0CTRL=(PORTE.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTE.PIN1CTRL=(PORTE.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTE.PIN2CTRL=(PORTE.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTE.PIN3CTRL=(PORTE.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTE.PIN4CTRL=(PORTE.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTE.PIN5CTRL=(PORTE.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTE.PIN6CTRL=(PORTE.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTE.PIN7CTRL=(PORTE.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTF.PIN0CTRL=(PORTF.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTF.PIN1CTRL=(PORTF.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTF.PIN2CTRL=(PORTF.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTF.PIN3CTRL=(PORTF.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTF.PIN4CTRL=(PORTF.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTF.PIN5CTRL=(PORTF.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTF.PIN6CTRL=(PORTF.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTF.PIN7CTRL=(PORTF.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTH.PIN0CTRL=(PORTH.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTH.PIN1CTRL=(PORTH.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTH.PIN2CTRL=(PORTH.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTH.PIN3CTRL=(PORTH.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTH.PIN4CTRL=(PORTH.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTH.PIN5CTRL=(PORTH.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTH.PIN6CTRL=(PORTH.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTH.PIN7CTRL=(PORTH.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTJ.PIN0CTRL=(PORTJ.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTJ.PIN1CTRL=(PORTJ.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTJ.PIN2CTRL=(PORTJ.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTJ.PIN3CTRL=(PORTJ.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTJ.PIN4CTRL=(PORTJ.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTJ.PIN5CTRL=(PORTJ.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTJ.PIN6CTRL=(PORTJ.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTJ.PIN7CTRL=(PORTJ.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTK.PIN0CTRL=(PORTK.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTK.PIN1CTRL=(PORTK.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTK.PIN2CTRL=(PORTK.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTK.PIN3CTRL=(PORTK.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTK.PIN4CTRL=(PORTK.PIN4CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTK.PIN5CTRL=(PORTK.PIN5CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTK.PIN6CTRL=(PORTK.PIN6CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTK.PIN7CTRL=(PORTK.PIN7CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTQ.PIN0CTRL=(PORTQ.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTQ.PIN1CTRL=(PORTQ.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTQ.PIN2CTRL=(PORTQ.PIN2CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTQ.PIN3CTRL=(PORTQ.PIN3CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;

	PORTR.PIN0CTRL=(PORTR.PIN0CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
	PORTR.PIN1CTRL=(PORTR.PIN1CTRL&0xC7)|PORT_OPC_PULLDOWN_gc;
}

// return true if short circute
// return false if no short circute
static bool shortcircute_check_pins()
{
	if((PORTA_DIR & PORTA_OUT) != (PORTA_IN & PORTA_OUT)) {
		printString("PORTA: ");printHex(PORTA_DIR & PORTA_OUT); printString(":");printHex(PORTA_IN & PORTA_OUT);printString("\r\n");
		return true;
	}
	if((PORTB_DIR & PORTB_OUT) != (PORTB_IN & PORTB_OUT)) {
		printString("PORTB: ");printHex(PORTB_DIR & PORTB_OUT); printString(":");printHex(PORTB_IN & PORTB_OUT);printString("\r\n");
		return true;
	}
	if((PORTC_DIR & PORTC_OUT) != (PORTC_IN & PORTC_OUT)) {
		printString("PORTC: ");printHex(PORTC_DIR & PORTC_OUT); printString(":");printHex(PORTC_IN & PORTC_OUT);printString("\r\n");
		return true;
	}
	if((PORTD_DIR & 0x33) != (PORTD_IN & 0x33)) {
		printString("PORTD: ");printHex(PORTD_DIR & PORTD_OUT); printString(":");printHex(PORTD_IN & PORTD_OUT);printString("\r\n");
		return true;
	}
	if((PORTE_DIR & PORTE_OUT) != (PORTE_IN & PORTE_OUT)) {
		printString("PORTE: ");printHex(PORTE_DIR & PORTE_OUT); printString(":");printHex(PORTE_IN & PORTE_OUT);printString("\r\n");
		return true;
	}
	if((PORTF_DIR & PORTF_OUT) != (PORTF_IN & PORTF_OUT)) {
		printString("PORTF: ");printHex(PORTF_DIR & PORTF_OUT); printString(":");printHex(PORTF_IN & PORTF_OUT);printString("\r\n");
		return true;
	}
	if((PORTH_DIR & PORTH_OUT) != (PORTH_IN & PORTH_OUT)) {
		printString("PORTH: ");printHex(PORTH_DIR & PORTH_OUT); printString(":");printHex(PORTH_IN & PORTH_OUT);printString("\r\n");
		return true;
	}
	if((PORTJ_DIR & PORTJ_OUT) != (PORTJ_IN & PORTJ_OUT)) {
		printString("PORTJ: ");printHex(PORTJ_DIR & PORTJ_OUT); printString(":");printHex(PORTJ_IN & PORTJ_OUT);printString("\r\n");
		return true;
	}
	if((PORTK_DIR & PORTK_OUT) != (PORTK_IN & PORTK_OUT)) {
		printString("PORTK: ");printHex(PORTK_DIR & PORTK_OUT); printString(":");printHex(PORTK_IN & PORTK_OUT);printString("\r\n");
		return true;
	}
	if((PORTQ_DIR & PORTQ_OUT) != (PORTQ_IN & PORTQ_OUT)) {
		printString("PORTQ: ");printHex(PORTQ_DIR & PORTQ_OUT); printString(":");printHex(PORTQ_IN & PORTQ_OUT);printString("\r\n");
		return true;
	}
	if((PORTR_DIR & PORTR_OUT) != (PORTR_IN & PORTR_OUT)) {
		printString("PORTR: ");printHex(PORTR_DIR & PORTR_OUT); printString(":");printHex(PORTR_IN & PORTR_OUT);printString("\r\n");
		return true;
	}
	
	return false;
}

// check if there short circute between pins.
// return true if all ports are ok
// return false if short circute in any ports.
bool shortcircute_test()
{
	shortcircute_pull_down_ports();
	
	PORTA_DIR = 0x00;
	PORTA_OUT = 0xff;
	PORTB_DIR = 0x00;
	PORTB_OUT = 0xff;
	PORTC_DIR = 0x00;
	PORTC_OUT = 0xff;
	PORTD_DIRCLR = 0x33;
	PORTD_OUTSET = 0x33;
	PORTE_DIR = 0x00;
	PORTE_OUT = 0xff;
	PORTF_DIR = 0x00;
	PORTF_OUT = 0xff;
	PORTH_DIR = 0x00;
	PORTH_OUT = 0xff;
	PORTJ_DIR = 0x00;
	PORTJ_OUT = 0xff;
	PORTK_DIR = 0x00;
	PORTK_OUT = 0xff;
	PORTQ_DIR = 0x00;
	PORTQ_OUT = 0xff;
	PORTR_DIR = 0x00;
	PORTR_OUT = 0xff;

	PORTA_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTA_DIR = 0x00;

	PORTB_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTB_DIR = 0x00;

	PORTC_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTC_DIR = 0x00;
	

	PORTD_DIRSET = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false; PORTD_DIRCLR = 0x01;
	PORTD_DIRSET = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false; PORTD_DIRCLR = 0x02;
	PORTD_DIRSET = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false; PORTD_DIRCLR = 0x10;
	PORTD_DIRSET = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false; PORTD_DIRCLR = 0x20;

	PORTE_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTE_DIR = 0x00;

	PORTF_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTF_DIR = 0x00;

	PORTH_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTH_DIR = 0x00;

	PORTJ_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTJ_DIR = 0x00;

	PORTK_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x10; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x20; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x40; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x80; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTK_DIR = 0x00;

	PORTQ_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTQ_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTQ_DIR = 0x04; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTQ_DIR = 0x08; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTQ_DIR = 0x00;

	PORTR_DIR = 0x01; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTR_DIR = 0x02; counter_wait(0xff); if(shortcircute_check_pins()) return false;
	PORTR_DIR = 0x00;
	
	return true;
}

void ecd300CheckShortCircute()
{
	usart_rs232_options_t uartOption;
	unsigned char c;
	
	PORTA_DIR=0x00;
	PORTB_DIR=0x00;
	PORTC_DIR=0x00;
	PORTD_DIR=0x00;
	PORTE_DIR=0x00;
	PORTF_DIR=0x00;
	PORTH_DIR=0x00;
	PORTJ_DIR=0x00;
	PORTK_DIR=0x00;

	disableJtagPort();
	sysclk_init();
	sleepmgr_init();
	irq_initialize_vectors(); //enable LOW, MED and HIGH level interrupt in PMIC.
	cpu_irq_enable();
	
	counter_init();
	//wait for 5 seconds
	for(c=0; c<30; c++){
		counter_wait(1);
	}

	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;
	ecd300InitUart(ECD300_UART_2, &uartOption);
	printString("Serial Port in Power Allocator was initialized\r\n");

	if(shortcircute_test()) {
		printString("No short circute in ports\r\n");
	}
	else {
		printString("Short circute in a port\r\n");
	}
	
	while(1);
}
