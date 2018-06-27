/*
 * ECD300_test.c
 *
 * Created: 2014/12/20 1:42:02
 *  Author: user
 */ 

#include <asf.h>
#include "ECD300.h"
#include "ECD300_test.h"

static void printString(char * pString)
{
	ecd300PutString(ECD300_UART_0, pString);
}

static void printHex(unsigned char hex)
{
	ecd300PutHexChar(ECD300_UART_0, hex);
}

static void printBin(unsigned char bin)
{
	ecd300PutBinChar(ECD300_UART_0, bin);
}

static unsigned char pollKey(void)
{
	unsigned char c;
	char rc;

	rc=ecd300PollChar(ECD300_UART_0, &c);

	if(rc==1)
		return c;
	else
		return 0;
}

void ecd300TestUart(void)
 {
	usart_rs232_options_t uartOption;
	
	board_init();
	disableJtagPort();
	offAllLeds();
	
	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;

	if(0!=ecd300InitUart(ECD300_UART_0, &uartOption))
		onLed(0);
	else
		onLed(1);

	if(0!=ecd300InitUart(ECD300_UART_1, &uartOption))
		onLed(0);
	else
		onLed(1);

	//make CPU respond to interrupts.
	cpu_irq_enable();

	while(1)
	{
		onLed(3);

		if(0==ecd300PutString(ECD300_UART_0, "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"))
		{
			onLed(4);
		}
		else
		{
			onLed(5);
		}
		offLed(3);
		offLed(4);
		offLed(5);

		//ecd300PollChar(ECD300_UART_0, &character);
	}
}

void ecd300TestUartEcho(void)
{
	usart_rs232_options_t uartOption;
	unsigned char character;
	
	board_init();
	disableJtagPort();
	offAllLeds();
	
	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;

	if(0!=ecd300InitUart(ECD300_UART_6, &uartOption))
		onLed(0);
	else
		onLed(1);

	//make CPU respond to interrupts.
	cpu_irq_enable();

	character='c';
	while(1)
	{
		onLed(3);

		if(0==ecd300GetChar(ECD300_UART_6, &character))
		{
			onLed(4);
		}
		else
		{
			onLed(5);
		}
		offLed(3);
		offLed(4);
		offLed(5);

		ecd300PutChar(ECD300_UART_6, character);
	}
}

static void _printSysClkSettings(void)
{
	printString("\r\n");
	printString("---------------------------------\r\n");
	printString("default system clock settings:\r\n");
	printString("    SCLKSEL:	");printBin(CLK.CTRL);printString("\r\n");
	printString("    PSCTRL:	");printBin(CLK.PSCTRL);printString("\r\n");
	printString("    LOCK:		");printBin(CLK.LOCK);printString("\r\n");
	printString("    RTCCTRL:	");printBin(CLK.RTCCTRL);printString("\r\n");
	printString("    USBCTRL:	");printBin(CLK.USBCTRL);printString("\r\n");
	printString("---------------------------------\r\n");
	printString("Oscillator settings:\r\n");
	printString("    CTRL:		");printBin(OSC.CTRL);printString("\r\n");
	printString("    STATUS:	");printBin(OSC.STATUS);printString("\r\n");
	printString("    XOSCCTRL:	");printBin(OSC.XOSCCTRL);printString("\r\n");
	printString("    XOSCFAIL:	");printBin(OSC.XOSCFAIL);printString("\r\n");
	printString("    RC32KCAL:	");printBin(OSC.RC32KCAL);printString("\r\n");
	printString("    PLLCTRL:	");printBin(OSC.PLLCTRL);printString("\r\n");
	printString("    DFLLCTRL:	");printBin(OSC.DFLLCTRL);printString("\r\n");
	printString("---------------------------------\r\n");
	printString("DFLL32M settings:\r\n");
	printString("    CTRL:			");printBin(DFLLRC32M.CTRL);printString("\r\n");
	printString("    reserved_0x01:	");printBin(DFLLRC32M.reserved_0x01);printString("\r\n");
	printString("    CALA:			");printBin(DFLLRC32M.CALA);printString("\r\n");
	printString("    CALB:			");printBin(DFLLRC32M.CALB);printString("\r\n");
	printString("    COMP0:		");printBin(DFLLRC32M.COMP0);printString("\r\n");
	printString("    COMP1:		");printBin(DFLLRC32M.COMP1);printString("\r\n");
	printString("    COMP2:		");printBin(DFLLRC32M.COMP2);printString("\r\n");
	printString("    reserved_0x07:	");printBin(DFLLRC32M.reserved_0x07);printString("\r\n");
	printString("---------------------------------\r\n");
	printString("DFLL2M settings:\r\n");
	printString("    CTRL:			");printBin(DFLLRC2M.CTRL);printString("\r\n");
	printString("    reserved_0x01:	");printBin(DFLLRC2M.reserved_0x01);printString("\r\n");
	printString("    CALA:			");printBin(DFLLRC2M.CALA);printString("\r\n");
	printString("    CALB:			");printBin(DFLLRC2M.CALB);printString("\r\n");
	printString("    COMP0:		");printBin(DFLLRC2M.COMP0);printString("\r\n");
	printString("    COMP1:		");printBin(DFLLRC2M.COMP1);printString("\r\n");
	printString("    COMP2:		");printBin(DFLLRC2M.COMP2);printString("\r\n");
	printString("    reserved_0x07:	");printBin(DFLLRC2M.reserved_0x07);printString("\r\n");
}

void ecd300TestSysClk(void)
{
	usart_rs232_options_t uartOption;
	unsigned long i;
	
	board_init();
	disableJtagPort();
	offAllLeds();
	
	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;

	if(0!=ecd300InitUart(ECD300_UART_0, &uartOption))
		onLed(0);
	else
		onLed(1);

	//make CPU respond to interrupts.
	cpu_irq_enable();

	_printSysClkSettings();
	
	printString("\r\n");

	printString("\r\n");
	printString("LEDs blink at 2MHz frequency, press any key to switch to 32MHz\r\n");
	for(i=0;i<(0x40000<<2);i++)
	{
		if(i&0x40000)
			PORTJ.OUT=0xff;
		else
			PORTJ.OUT=0;
	}
	
	printString("Enable 32M internal:\r\n");
	sysClkEnable32Mhz();
	printString("Check whether 32M oscillator ready?\r\n");
	for(;;)
	{
		if(sysClkIs32MhzReady())
		{
			break;
		}
	}
	printString("OSC_STATUS:");printBin(OSC_STATUS);printString("\r\n");
	
	//switch to 32MHz.
	sysClkSelect32Mhz();	
	for(i=0;i<(0x40000<<4);i++)
	{
		if(i&0x40000)
		{
			PORTJ.OUT=0xff;
		}
		else
		{
			PORTJ.OUT=0;
		}
	}

	//switch back to 2MHz.
	sysClkSelect2Mhz();
	printString("CPU is switched back to 2Mhz\r\n");
	printString("disable 32M internal\r\n");
	sysClkDisable32Mhz();
	printString("calibrate 32MHz to 48MHz\r\n");
	sysClkCalibrate32MhzTo48MHz();
	sysClkEnable32Mhz();
	for(;;)
	{
		if(sysClkIs32MhzReady())
		{
			break;
		}
	}
	printString("32MHz is calibrated to 48MHz\r\n");
	//set CPU clock to 12MHz.
	//enable PerClk4 and PerClk2.
	sysClkEnablePer4Per2();
	sysClkSelect32Mhz();
	for(i=0;i<(0x40000<<4);i++)
	{
		if(i&0x40000)
		{
			PORTJ.OUT=0xff;
		}
		else
		{
			PORTJ.OUT=0;
		}
	}

	//enable USB clock
	sysClkDisableUsb();
	sysClkUsbSelect32Mhz();
	sysClkEnableUsb();

	//set CPU clock to 30MHz.
	sysClkDisablePll();
	sysClkSelectPllSource32MDiv4();
	sysClkPllSetMulFactor(10);
	sysClkEnablePll();//output of PLL is 120MHz.
	for(;;)
	{
		if(sysClkIsPllReady())
		{
			break;
		}
	}
	
	sysClkSelectPll();
	for(i=0;;i++)
	{
		if(i&0x40000)
		{
			PORTJ.OUT=0xff;
		}
		else
		{
			PORTJ.OUT=0;
		}
	}
	
	while(1)
	{
		;
	}
}

