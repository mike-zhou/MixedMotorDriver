/*
 * ECD300_test.c
 *
 * Created: 2014/12/20 1:42:02
 *  Author: user
 */ 

#include <asf.h>
#include "ECD300.h"
#include "ECD300_test.h"

void printString(char * pString)
{
	ecd300PutString(ECD300_UART_0, pString);
}

void printHex(unsigned char hex)
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

void ecd300TestDFU(void)
{
	usart_rs232_options_t uartOption;
	unsigned long i;
	
	sysclk_init();

	// Map interrupt vectors table in bootloader section
	//ccp_write_io((uint8_t*)&PMIC.CTRL, PMIC_IVSEL_bm | PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm);
	
	irq_initialize_vectors();
	cpu_irq_enable();

	// Initialize the sleep manager
	sleepmgr_init();

	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;
	ecd300InitUart(ECD300_UART_0, &uartOption);

	printString("UART0 was initialized 9\r\n");

	// Start USB stack to authorize VBus monitoring
	udc_start();

	//udc_attach();

	i=0;
	PORTF.DIR=0xff;
	while (true) {
		i++;
		if(i&0x80000)
			PORTF.OUT=0xff;
		else
			PORTF.OUT=0;
	}
	
}

/**
	UART 1 exchange data with HC-06 blue tooth device.
	Before any data is exchanged between the mobile phone and ECD300,
	the connection between these two device should be successful.
	
	The mobile phone can retrieve LED state with command:
		GXXX
		For example, sending GDA2 to ECD300 to retrieve state of LED DA2.
		If DA2 is on, ECD300 will reply DA2-ON, else DA2-OFF.
	The mobile phone can set LED state with command:
		SXXX1 or SXXX0
		For example, sending SDA21 will turn on LED DA2 and ECD300 will reply DA2-ON;
		sending SDA20 will turn off LED DA2 and ECD300 will reply DA2-OFF.
*/
static void _blueToothInit(void)
{
	usart_rs232_options_t uartOption;
	
	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;
	ecd300InitUart(ECD300_UART_1, &uartOption);
}

static unsigned char _blueToothGetChar(void)
{
	unsigned char c;
	char rc=-1;
	
	//get a character from UART 1.
	while(-1==rc)
	{
		rc=ecd300GetChar(ECD300_UART_1, &c);
	}
	
	return c;
}
/************************************************************************/
/* Send a string terminated with '\0' to  the HC-06 device, but the '\0' 
   is not sent.                                                         */
/************************************************************************/
static void _blueToothSendString(unsigned char * pString)
{
	ecd300PutString(ECD300_UART_1, pString);
	ecd300PutString(ECD300_UART_1, "\n");
}

void ecd300TestBlueTooth(void)
{
	usart_rs232_options_t uartOption;
	unsigned char c;
	unsigned char debugBuffer[10];
	enum
	{
		CMD_BYTE_CMD = 0,
		CMD_BYTE_NAME_0,
		CMD_BYTE_NAME_PORT,
		CMD_BYTE_NAME_PIN,
		CMD_BYTE_ON_OFF
	}cmdByteIndex;
	unsigned char cmdBuffer[5];
	
	PORTA.OUT=0;
	PORTA.DIR=0xFF;
	PORTB.OUT=0;
	PORTB.DIR=0xFF;
	//PORTC.OUT=0;
	//PORTC.DIR=0xFF;
	PORTD.OUT=0;
	PORTD.DIR=0xFF;
	PORTE.OUT=0;
	PORTE.DIR=0xFF;
	PORTF.OUT=0;
	PORTF.DIR=0xFF;
	PORTH.OUT=0;
	PORTH.DIR=0xFF;
	PORTJ.OUT=0;
	PORTJ.DIR=0xFF;
	PORTK.OUT=0;
	PORTK.DIR=0xFF;
	
	sysclk_init();
	disableJtagPort();
	irq_initialize_vectors();
	cpu_irq_enable();

	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;
	ecd300InitUart(ECD300_UART_0, &uartOption);

	printString("UART0 was initialized 9\r\n");

	_blueToothInit();
	
	cmdByteIndex=CMD_BYTE_CMD;
	debugBuffer[1]='\r';
	debugBuffer[2]='\n';
	debugBuffer[3]='\0';
	while (true) 
	{
		c=_blueToothGetChar();
		
		printString("key: ");
		debugBuffer[0]=c;
		printString(debugBuffer);
		
		switch(cmdByteIndex)
		{
			case CMD_BYTE_CMD:
				/* this byte should be 'S'(Set) or 'G'(Get) */
				if(('S'==c)||('s'==c))
				{
					cmdBuffer[CMD_BYTE_CMD]='S';
					cmdByteIndex=CMD_BYTE_NAME_0;
				}
				else if(('G'==c)||('g'==c))
				{
					cmdBuffer[CMD_BYTE_CMD]='G';
					cmdByteIndex=CMD_BYTE_NAME_0;
				}
				else
				{
					printString("Illegal command byte 0\r\n");
					_blueToothSendString("Illegal command");
				}
				break;
			case CMD_BYTE_NAME_0:
				/* this byte should be 'D'*/
				if(('D'==c)||('d'==c))
				{
					cmdBuffer[CMD_BYTE_NAME_0]='D';
					cmdByteIndex=CMD_BYTE_NAME_PORT;
				}
				else
				{
					cmdByteIndex=CMD_BYTE_CMD;
				}
				break;
			case CMD_BYTE_NAME_PORT:
				if(('A'==c)||('a'==c))
				{
					cmdBuffer[CMD_BYTE_NAME_PORT]='A';
					cmdByteIndex=CMD_BYTE_NAME_PIN;
				}
				else if(('B'==c)||('b'==c))
				{
					cmdBuffer[CMD_BYTE_NAME_PORT]='B';
					cmdByteIndex=CMD_BYTE_NAME_PIN;
				}
				else if(('C'==c)||('c'==c))
				{
					cmdBuffer[CMD_BYTE_NAME_PORT]='C';
					cmdByteIndex=CMD_BYTE_NAME_PIN;
				}
				else if(('D'==c)||('d'==c))
				{
					cmdBuffer[CMD_BYTE_NAME_PORT]='D';
					cmdByteIndex=CMD_BYTE_NAME_PIN;
				}
				else if(('E'==c)||('e'==c))
				{
					cmdBuffer[CMD_BYTE_NAME_PORT]='E';
					cmdByteIndex=CMD_BYTE_NAME_PIN;
				}
				else if(('F'==c)||('f'==c))
				{
					cmdBuffer[CMD_BYTE_NAME_PORT]='F';
					cmdByteIndex=CMD_BYTE_NAME_PIN;
				}
				else if(('H'==c)||('h'==c))
				{
					cmdBuffer[CMD_BYTE_NAME_PORT]='H';
					cmdByteIndex=CMD_BYTE_NAME_PIN;
				}
				else if(('J'==c)||('j'==c))
				{
					cmdBuffer[CMD_BYTE_NAME_PORT]='J';
					cmdByteIndex=CMD_BYTE_NAME_PIN;
				}
				else if(('K'==c)||('k'==c))
				{
					cmdBuffer[CMD_BYTE_NAME_PORT]='K';
					cmdByteIndex=CMD_BYTE_NAME_PIN;
				}
				else
				{
					cmdByteIndex=CMD_BYTE_CMD;
				}
				break;
			case CMD_BYTE_NAME_PIN:
				if(('0'<=c)&&(c<='7'))
				{
					cmdBuffer[CMD_BYTE_NAME_PIN]=c;
					if('G'==cmdBuffer[CMD_BYTE_CMD])
					{
						int index;
						unsigned char replyBuffer[8];
						
						for(index=0;index<8;index++)
							replyBuffer[index]=0;
							
						replyBuffer[0]='D';
						replyBuffer[2]=cmdBuffer[CMD_BYTE_NAME_PIN];//pin nummber
						replyBuffer[3]='-';
						replyBuffer[4]='O';
						
						//retrieve LED state.
						switch(cmdBuffer[CMD_BYTE_NAME_PORT])
						{
							case 'A':
								replyBuffer[1]='A';
								//DAx-ON or DAx-OFF.
								if(PORTA.IN&(1<<(c-'0')))
								{
									replyBuffer[5]='N';
								}
								else
								{
									replyBuffer[5]='F';
									replyBuffer[6]='F';
								}
								break;
							case 'B':
								replyBuffer[1]='B';
								//DBx-ON or DBx-OFF.
								if(PORTB.IN&(1<<(c-'0')))
								{
									replyBuffer[5]='N';
								}
								else
								{
									replyBuffer[5]='F';
									replyBuffer[6]='F';
								}
								break;
							case 'C':
								replyBuffer[1]='C';
								//DCx-ON or DCx-OFF.
								if(PORTC.IN&(1<<(c-'0')))
								{
									replyBuffer[5]='N';
								}
								else
								{
									replyBuffer[5]='F';
									replyBuffer[6]='F';
								}
								break;
							case 'D':
								replyBuffer[1]='D';
								//DDx-ON or DDx-OFF.
								if(PORTD.IN&(1<<(c-'0')))
								{
									replyBuffer[5]='N';
								}
								else
								{
									replyBuffer[5]='F';
									replyBuffer[6]='F';
								}
								break;
							case 'E':
								replyBuffer[1]='E';
								//DEx-ON or DEx-OFF.
								if(PORTE.IN&(1<<(c-'0')))
								{
									replyBuffer[5]='N';
								}
								else
								{
									replyBuffer[5]='F';
									replyBuffer[6]='F';
								}
								break;
							case 'F':
								replyBuffer[1]='F';
								//DFx-ON or DFx-OFF.
								if(PORTF.IN&(1<<(c-'0')))
								{
									replyBuffer[5]='N';
								}
								else
								{
									replyBuffer[5]='F';
									replyBuffer[6]='F';
								}
								break;
							case 'H':
								replyBuffer[1]='H';
								//DHx-ON or DHx-OFF.
								if(PORTH.IN&(1<<(c-'0')))
								{
									replyBuffer[5]='N';
								}
								else
								{
									replyBuffer[5]='F';
									replyBuffer[6]='F';
								}
								break;
							case 'J':
								replyBuffer[1]='J';
								//DJx-ON or DJx-OFF.
								if(PORTJ.IN&(1<<(c-'0')))
								{
									replyBuffer[5]='N';
								}
								else
								{
									replyBuffer[5]='F';
									replyBuffer[6]='F';
								}
								break;
							case 'K':
								replyBuffer[1]='K';
								//DKx-ON or DKx-OFF.
								if(PORTK.IN&(1<<(c-'0')))
								{
									replyBuffer[5]='N';
								}
								else
								{
									replyBuffer[5]='F';
									replyBuffer[6]='F';
								}
								break;
							default:
								printString("Illegal command CMD_BYTE_NAME_PIN\r\n");
								break;
						}
						cmdByteIndex=CMD_BYTE_CMD;
						_blueToothSendString(replyBuffer);
					}
					else
					{
						//Set LED state
						cmdByteIndex=CMD_BYTE_ON_OFF;
					}
				}
				else
				{
					cmdByteIndex=CMD_BYTE_CMD;
				}
				break;
			case CMD_BYTE_ON_OFF:
				if('0'==c)
				{
					unsigned char replyBuffer[8];
					unsigned char index;
					for(index=0; index<8; index++)
						replyBuffer[index]=0;
					
					replyBuffer[0]=cmdBuffer[CMD_BYTE_NAME_0];
					replyBuffer[1]=cmdBuffer[CMD_BYTE_NAME_PORT];
					replyBuffer[2]=cmdBuffer[CMD_BYTE_NAME_PIN];
					replyBuffer[3]='-';
					replyBuffer[4]='O';
					replyBuffer[5]='F';
					replyBuffer[6]='F';
					//turn off LED
					switch(cmdBuffer[CMD_BYTE_NAME_PORT])
					{
						case 'A':
							PORTA.OUTCLR=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'B':
							PORTB.OUTCLR=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'C':
							PORTC.OUTCLR=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'D':
							PORTD.OUTCLR=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'E':
							PORTE.OUTCLR=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'F':
							PORTF.OUTCLR=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'H':
							PORTH.OUTCLR=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'J':
							PORTJ.OUTCLR=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'K':
							PORTK.OUTCLR=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						default:
							break;	
					}
					_blueToothSendString(replyBuffer);
				}
				else if('1'==c)
				{
					unsigned char replyBuffer[8];
					unsigned char index;
					
					for(index=0; index<8; index++)
						replyBuffer[index]=0;
					
					replyBuffer[0]=cmdBuffer[CMD_BYTE_NAME_0];
					replyBuffer[1]=cmdBuffer[CMD_BYTE_NAME_PORT];
					replyBuffer[2]=cmdBuffer[CMD_BYTE_NAME_PIN];
					replyBuffer[3]='-';
					replyBuffer[4]='O';
					replyBuffer[5]='N';
					//turn on LED
					switch(cmdBuffer[CMD_BYTE_NAME_PORT])
					{
						case 'A':
							PORTA.OUTSET=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'B':
							PORTB.OUTSET=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'C':
							PORTC.OUTSET=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'D':
							PORTD.OUTSET=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'E':
							PORTE.OUTSET=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'F':
							PORTF.OUTSET=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'H':
							PORTH.OUTSET=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'J':
							PORTJ.OUTSET=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						case 'K':
							PORTK.OUTSET=1<<(cmdBuffer[CMD_BYTE_NAME_PIN]-'0');
							break;
						default:
							break;
					}
					_blueToothSendString(replyBuffer);
				}
				else
				{
					//illegal command.
					printString("Illegal command CMD_BYTE_ON_OFF\r\n");
				}
				cmdByteIndex=CMD_BYTE_CMD;
				break;
			default:
				cmdByteIndex=CMD_BYTE_CMD;
				break;
		}
	}
}
