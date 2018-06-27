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

unsigned char getChar(void)
{
	unsigned char c;

	for(;;)
	{
		c=pollKey();
		if(0!=c)
		{
			break;
		}
	}

	return c;
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


extern void app_process_transfer(void);

void ecd300TestBulk(void)
{
	usart_rs232_options_t uartOption;
	unsigned long i;
	
	sysclk_init();

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

	app_process_transfer();
	
}


void ecd300TestDac(void)
{
	struct dac_config conf;
	unsigned long       i = 0;
	unsigned long j;

	sysclk_init();

	// Initialize the DACB configuration.
	dac_read_configuration(&DACB, &conf);
	/* AVCC as reference, right adjusted channel value */
	dac_set_conversion_parameters(&conf, DAC_REF_AVCC, DAC_ADJ_RIGHT);
	/* one active DAC channel, no internal output */
	dac_set_active_channel(&conf, DAC_CH0, 0);
	/* conversions triggered by DATA */
	dac_set_conversion_trigger(&conf, 0, 0);
	dac_write_configuration(&DACB, &conf);
	dac_enable(&DACB);
	j=0x8ffff;
	do {
		//update CH0
		dac_wait_for_channel_ready(&DACB, DAC_CH0);
		dac_set_channel_value(&DACB, DAC_CH0, (i>>4)&0xfff);

		i++;
		i %= 0xffff;
	} while (j--);	
	
	dac_disable(&DACB);
	/* two active DAC channels, no internal output */
	dac_set_active_channel(&conf, DAC_CH0|DAC_CH1, 0);
	/* conversions triggered by DATA */
	dac_set_conversion_trigger(&conf, 0, 0);
	dac_write_configuration(&DACB, &conf);
	dac_enable(&DACB);
	j=0x8ffff;
	do {
		//update CH0
		dac_wait_for_channel_ready(&DACB, DAC_CH0);
		dac_set_channel_value(&DACB, DAC_CH0, (i>>4)&0xfff);
		//update CH1
		dac_wait_for_channel_ready(&DACB, DAC_CH1);
		dac_set_channel_value(&DACB, DAC_CH1, (~(i>>4))&0xfff);

		i++;
		i %= 0xffff;
	} while (j--);	

	//set up DACA as current DACB
	dac_read_configuration(&DACA, &conf);
	/* AVCC as reference, right adjusted channel value */
	dac_set_conversion_parameters(&conf, DAC_REF_AVCC, DAC_ADJ_RIGHT);
	/* one active DAC channel, no internal output */
	dac_set_active_channel(&conf, DAC_CH0|DAC_CH1, 0);
	/* conversions triggered by DATA */
	dac_set_conversion_trigger(&conf, 0, 0);
	dac_write_configuration(&DACA, &conf);
	dac_enable(&DACA);
	j=0x8ffff;
	do {
		//update CH0
		dac_wait_for_channel_ready(&DACA, DAC_CH0);
		dac_set_channel_value(&DACA, DAC_CH0, (i>>4)&0xfff);
		//update CH1
		dac_wait_for_channel_ready(&DACA, DAC_CH1);
		dac_set_channel_value(&DACA, DAC_CH1, (~(i>>4))&0xfff);
		//update CH0
		dac_wait_for_channel_ready(&DACB, DAC_CH0);
		dac_set_channel_value(&DACB, DAC_CH0, (i>>4)&0xfff);
		//update CH1
		dac_wait_for_channel_ready(&DACB, DAC_CH1);
		dac_set_channel_value(&DACB, DAC_CH1, (~(i>>4))&0xfff);

		i++;
		i %= 0xffff;
	} while (1);
	
}

static void _adcCallback(ADC_t *adc, unsigned char ch_mask, CONFIG_ADC_CALLBACK_TYPE res)
{
	if((&ADCA)==adc)
	{
		switch(ch_mask)
		{
			case ADC_CH0:
				printString("####ADCA, CH0: ");
				printHex(res>>8);
				printString(", ");
				printHex(res&0xff);
				printString("\r\n");
				break;

			case ADC_CH1:
				printString("####ADCA, CH1: ");
				printHex(res>>8);
				printString(", ");
				printHex(res&0xff);
				printString("\r\n");
				break;

			case ADC_CH2:
				printString("####ADCA, CH2: ");
				printHex(res>>8);
				printString(", ");
				printHex(res&0xff);
				printString("\r\n");
				break;

			case ADC_CH3:
				printString("####ADCA, CH3: ");
				printHex(res>>8);
				printString(", ");
				printHex(res&0xff);
				printString("\r\n");
				break;

			default:
				printString("####Unknown ACDA channel: ");
				printHex(ch_mask);
				printString("\r\n");
				break;
		}
	}
	else if((&ADCB)==adc)
	{
		switch(ch_mask)
		{
			case ADC_CH0:
				printString("####ADCB, CH0: ");
				printHex(res>>8);
				printString(", ");
				printHex(res&0xff);
				printString("\r\n");
				break;

			case ADC_CH1:
				printString("####ADCB, CH1: ");
				printHex(res>>8);
				printString(", ");
				printHex(res&0xff);
				printString("\r\n");
				break;

			case ADC_CH2:
				printString("####ADCB, CH2: ");
				printHex(res>>8);
				printString(", ");
				printHex(res&0xff);
				printString("\r\n");
				break;

			case ADC_CH3:
				printString("####ADCB, CH3: ");
				printHex(res>>8);
				printString(", ");
				printHex(res&0xff);
				printString("\r\n");
				break;

			default:
				printString("####Unknown ACDB channel: ");
				printHex(ch_mask);
				printString("\r\n");
				break;
		}
	}
}

void ecd300TestAdc(void)
{
	usart_rs232_options_t uartOption;

	struct dac_config dac_conf;
	
	struct adc_config         adc_conf;
	struct adc_channel_config adcch_conf;

	disableJtagPort();
	sysclk_init();
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

	//set up DACA
	dac_read_configuration(&DACA, &dac_conf);
	/* AVCC as reference, right adjusted channel value */
	dac_set_conversion_parameters(&dac_conf, DAC_REF_AVCC, DAC_ADJ_RIGHT);
	/* two active DAC channels, no internal output */
	dac_set_active_channel(&dac_conf, DAC_CH0|DAC_CH1, 0);
	/* conversions triggered by DATA */
	dac_set_conversion_trigger(&dac_conf, 0, 0);
	dac_write_configuration(&DACA, &dac_conf);
	dac_enable(&DACA);

	//set up DACB
	dac_read_configuration(&DACB, &dac_conf);
	/* AVCC as reference, right adjusted channel value */
	dac_set_conversion_parameters(&dac_conf, DAC_REF_AVCC, DAC_ADJ_RIGHT);
	/* two active DAC channels, no internal output */
	dac_set_active_channel(&dac_conf, DAC_CH0|DAC_CH1, 0);
	/* conversions triggered by DATA */
	dac_set_conversion_trigger(&dac_conf, 0, 0);
	dac_write_configuration(&DACB, &dac_conf);
	dac_enable(&DACB);

	// Initialize configuration structures of ADCA.
	adc_read_configuration(&ADCA, &adc_conf);
	/* Configure the ADC module:
	 * - signed, 12-bit results
	 * - AVCC/2  voltage reference
	 * - 200 kHz maximum clock rate
	 * - manual conversion triggering
	 * - set interrupt routine
	 */
  	adc_set_conversion_parameters(&adc_conf, ADC_SIGN_OFF, ADC_RES_12, ADC_REF_VCCDIV2);
	adc_set_clock_rate(&adc_conf, 200000UL);
	adc_set_conversion_trigger(&adc_conf, ADC_TRIG_MANUAL, 1, 0);
	adc_write_configuration(&ADCA, &adc_conf);
	adc_set_callback(&ADCA, _adcCallback);
	/* Configure ADC channel 0:
	 * - single-ended measurement from configured input pin
	 * - interrupt flag set on completed conversion
	 */
	adcch_read_configuration(&ADCA, ADC_CH0, &adcch_conf);
	adcch_set_input(&adcch_conf, ADCCH_POS_PIN2, ADCCH_NEG_NONE, 1);
	adcch_set_interrupt_mode(&adcch_conf, ADCCH_MODE_COMPLETE);
	adcch_disable_interrupt(&adcch_conf);
	adcch_write_configuration(&ADCA, ADC_CH0, &adcch_conf);
	/* Configure ADC channel 1:
	 * - single-ended measurement from configured input pin
	 * - interrupt flag set on completed conversion
	 */
	adcch_read_configuration(&ADCA, ADC_CH1, &adcch_conf);
	adcch_set_input(&adcch_conf, ADCCH_POS_PIN3, ADCCH_NEG_NONE, 1);
	adcch_set_interrupt_mode(&adcch_conf, ADCCH_MODE_COMPLETE);
	adcch_disable_interrupt(&adcch_conf);
	adcch_write_configuration(&ADCA, ADC_CH1, &adcch_conf);
	// Enable the ADC and do one dummy conversion with ADC channel interrupt disabled.
	adc_enable(&ADCA);
	adc_start_conversion(&ADCA, ADC_CH0);
	adc_wait_for_interrupt_flag(&ADCA, ADC_CH0);

	// Initialize configuration structures of ADCB.
	adc_read_configuration(&ADCB, &adc_conf);
	/* Configure the ADC module:
	 * - signed, 12-bit results
	 * - AVCC/2  voltage reference
	 * - 200 kHz maximum clock rate
	 * - manual conversion triggering
	 * - set interrupt routine
	 */
  	adc_set_conversion_parameters(&adc_conf, ADC_SIGN_OFF, ADC_RES_12, ADC_REF_VCCDIV2);
	adc_set_clock_rate(&adc_conf, 200000UL);
	adc_set_conversion_trigger(&adc_conf, ADC_TRIG_MANUAL, 1, 0);
	adc_write_configuration(&ADCB, &adc_conf);
	adc_set_callback(&ADCB, _adcCallback);
	/* Configure ADC channel 0:
	 * - single-ended measurement from configured input pin
	 * - interrupt flag set on completed conversion
	 */
	adcch_read_configuration(&ADCB, ADC_CH0, &adcch_conf);
	adcch_set_input(&adcch_conf, ADCCH_POS_PIN2, ADCCH_NEG_NONE, 1);
	adcch_set_interrupt_mode(&adcch_conf, ADCCH_MODE_COMPLETE);
	adcch_disable_interrupt(&adcch_conf);
	adcch_write_configuration(&ADCB, ADC_CH0, &adcch_conf);
	/* Configure ADC channel 1:
	 * - single-ended measurement from configured input pin
	 * - interrupt flag set on completed conversion
	 */
	adcch_read_configuration(&ADCB, ADC_CH1, &adcch_conf);
	adcch_set_input(&adcch_conf, ADCCH_POS_PIN3, ADCCH_NEG_NONE, 1);
	adcch_set_interrupt_mode(&adcch_conf, ADCCH_MODE_COMPLETE);
	adcch_disable_interrupt(&adcch_conf);
	adcch_write_configuration(&ADCB, ADC_CH1, &adcch_conf);
	// Enable the ADC and do one dummy conversion with ADC channel interrupt disabled.
	adc_enable(&ADCB);
	adc_start_conversion(&ADCB, ADC_CH0);
	adc_wait_for_interrupt_flag(&ADCB, ADC_CH0);

	/**
	* DAC outputs 1.6V, ADC samples this voltage
	*/
	printString("DACA, DAC_CH0: 0x800, DAC_CH1: 0x800\r\n");
	//update DACA CH0
	dac_wait_for_channel_ready(&DACA, DAC_CH0);
	dac_set_channel_value(&DACA, DAC_CH0, 0x800);
	//update DACA CH1
	dac_wait_for_channel_ready(&DACA, DAC_CH1);
	dac_set_channel_value(&DACA, DAC_CH1, 0x800);
	//wait for the readiness of DACA.
	dac_wait_for_channel_ready(&DACA, DAC_CH0);
	dac_wait_for_channel_ready(&DACA, DAC_CH1);
	printString("DACB, DAC_CH0: 0x800, DAC_CH1: 0x800\r\n");
	//update DACB CH0
	dac_wait_for_channel_ready(&DACB, DAC_CH0);
	dac_set_channel_value(&DACB, DAC_CH0, 0x800);
	//update DACB CH1
	dac_wait_for_channel_ready(&DACA, DAC_CH1);
	dac_set_channel_value(&DACB, DAC_CH1, 0x800);
	//wait for the readiness of DACB.
	dac_wait_for_channel_ready(&DACB, DAC_CH0);
	dac_wait_for_channel_ready(&DACB, DAC_CH1);
	// Sample the output of DAC
	// Enable interrupts on ADC channel
	//adcch_enable_interrupt(&adcch_conf);
	//adcch_write_configuration(&ADCA, ADC_CH0, &adcch_conf);
	adc_start_conversion(&ADCA, ADC_CH0);
	adc_wait_for_interrupt_flag(&ADCA, ADC_CH0);
	{
		unsigned short result;

		result=adc_get_result(&ADCA, ADC_CH0);
		printString("    ADCA ADC_CH0: ");
		printHex(result>>8);
		printString(", ");
		printHex(result&0xff);
		printString("\r\n");
	}
	adc_start_conversion(&ADCA, ADC_CH1);
	adc_wait_for_interrupt_flag(&ADCA, ADC_CH1);
	{
		unsigned short result;

		result=adc_get_result(&ADCA, ADC_CH1);
		printString("    ADCA ADC_CH1: ");
		printHex(result>>8);
		printString(", ");
		printHex(result&0xff);
		printString("\r\n");
	}
	adc_start_conversion(&ADCB, ADC_CH0);
	adc_wait_for_interrupt_flag(&ADCB, ADC_CH0);
	{
		unsigned short result;

		result=adc_get_result(&ADCB, ADC_CH0);
		printString("    ADCB ADC_CH0: ");
		printHex(result>>8);
		printString(", ");
		printHex(result&0xff);
		printString("\r\n");
	}
	adc_start_conversion(&ADCB, ADC_CH1);
	adc_wait_for_interrupt_flag(&ADCB, ADC_CH1);
	{
		unsigned short result;

		result=adc_get_result(&ADCB, ADC_CH1);
		printString("    ADCB ADC_CH1: ");
		printHex(result>>8);
		printString(", ");
		printHex(result&0xff);
		printString("\r\n");
	}

	/**
	* DAC outputs 0.8V, ADC samples this voltage
	*/
	printString("DACA, DAC_CH0: 0x400, DAC_CH1: 0x400\r\n");
	//update DACA CH0
	dac_wait_for_channel_ready(&DACA, DAC_CH0);
	dac_set_channel_value(&DACA, DAC_CH0, 0x400);
	//update DACA CH1
	dac_wait_for_channel_ready(&DACA, DAC_CH1);
	dac_set_channel_value(&DACA, DAC_CH1, 0x400);
	//wait for the readiness of DACA.
	dac_wait_for_channel_ready(&DACA, DAC_CH0);
	dac_wait_for_channel_ready(&DACA, DAC_CH1);
	printString("DACB, DAC_CH0: 0x400, DAC_CH1: 0x400\r\n");
	//update DACB CH0
	dac_wait_for_channel_ready(&DACB, DAC_CH0);
	dac_set_channel_value(&DACB, DAC_CH0, 0x400);
	//update DACB CH1
	dac_wait_for_channel_ready(&DACA, DAC_CH1);
	dac_set_channel_value(&DACB, DAC_CH1, 0x400);
	//wait for the readiness of DACB.
	dac_wait_for_channel_ready(&DACB, DAC_CH0);
	dac_wait_for_channel_ready(&DACB, DAC_CH1);
	// Sample the output of DAC.
	adc_start_conversion(&ADCA, ADC_CH0);
	adc_wait_for_interrupt_flag(&ADCA, ADC_CH0);
	{
		unsigned short result;

		result=adc_get_result(&ADCA, ADC_CH0);
		printString("    ADCA ADC_CH0: ");
		printHex(result>>8);
		printString(", ");
		printHex(result&0xff);
		printString("\r\n");
	}
	adc_start_conversion(&ADCA, ADC_CH1);
	adc_wait_for_interrupt_flag(&ADCA, ADC_CH1);
	{
		unsigned short result;

		result=adc_get_result(&ADCA, ADC_CH1);
		printString("    ADCA ADC_CH1: ");
		printHex(result>>8);
		printString(", ");
		printHex(result&0xff);
		printString("\r\n");
	}
	adc_start_conversion(&ADCB, ADC_CH0);
	adc_wait_for_interrupt_flag(&ADCB, ADC_CH0);
	{
		unsigned short result;

		result=adc_get_result(&ADCB, ADC_CH0);
		printString("    ADCB ADC_CH0: ");
		printHex(result>>8);
		printString(", ");
		printHex(result&0xff);
		printString("\r\n");
	}
	adc_start_conversion(&ADCB, ADC_CH1);
	adc_wait_for_interrupt_flag(&ADCB, ADC_CH1);
	{
		unsigned short result;

		result=adc_get_result(&ADCB, ADC_CH1);
		printString("    ADCB ADC_CH1: ");
		printHex(result>>8);
		printString(", ");
		printHex(result&0xff);
		printString("\r\n");
	}

	/**
	* DAC outputs 0V, ADC samples this voltage
	*/
	printString("DACA, DAC_CH0: 0x000, DAC_CH1: 0x000\r\n");
	//update DACA CH0
	dac_wait_for_channel_ready(&DACA, DAC_CH0);
	dac_set_channel_value(&DACA, DAC_CH0, 0x000);
	//update DACA CH1
	dac_wait_for_channel_ready(&DACA, DAC_CH1);
	dac_set_channel_value(&DACA, DAC_CH1, 0x000);
	//wait for the readiness of DACA.
	dac_wait_for_channel_ready(&DACA, DAC_CH0);
	dac_wait_for_channel_ready(&DACA, DAC_CH1);
	printString("DACB, DAC_CH0: 0x000, DAC_CH1: 0x000\r\n");
	//update DACB CH0
	dac_wait_for_channel_ready(&DACB, DAC_CH0);
	dac_set_channel_value(&DACB, DAC_CH0, 0);
	//update DACB CH1
	dac_wait_for_channel_ready(&DACA, DAC_CH1);
	dac_set_channel_value(&DACB, DAC_CH1, 0);
	//wait for the readiness of DACB.
	dac_wait_for_channel_ready(&DACB, DAC_CH0);
	dac_wait_for_channel_ready(&DACB, DAC_CH1);
	// Sample the output of DAC.
	adc_start_conversion(&ADCA, ADC_CH0);
	adc_wait_for_interrupt_flag(&ADCA, ADC_CH0);
	{
		unsigned short result;

		result=adc_get_result(&ADCA, ADC_CH0);
		printString("    ADCA ADC_CH0: ");
		printHex(result>>8);
		printString(", ");
		printHex(result&0xff);
		printString("\r\n");
	}
	adc_start_conversion(&ADCA, ADC_CH1);
	adc_wait_for_interrupt_flag(&ADCA, ADC_CH1);
	{
		unsigned short result;

		result=adc_get_result(&ADCA, ADC_CH1);
		printString("    ADCA ADC_CH1: ");
		printHex(result>>8);
		printString(", ");
		printHex(result&0xff);
		printString("\r\n");
	}
	adc_start_conversion(&ADCB, ADC_CH0);
	adc_wait_for_interrupt_flag(&ADCB, ADC_CH0);
	{
		unsigned short result;

		result=adc_get_result(&ADCB, ADC_CH0);
		printString("    ADCB ADC_CH0: ");
		printHex(result>>8);
		printString(", ");
		printHex(result&0xff);
		printString("\r\n");
	}
	adc_start_conversion(&ADCB, ADC_CH1);
	adc_wait_for_interrupt_flag(&ADCB, ADC_CH1);
	{
		unsigned short result;

		result=adc_get_result(&ADCB, ADC_CH1);
		printString("    ADCB ADC_CH1: ");
		printHex(result>>8);
		printString(", ");
		printHex(result&0xff);
		printString("\r\n");
	}

	while(1)
	{
		sleepmgr_enter_sleep();
	}
}


void ecd300TestTwi(void)
{
	usart_rs232_options_t uartOption;
  	twi_master_options_t opt;
	twi_package_t packet;
	unsigned long i;
	unsigned short counter;
	unsigned short data;
	unsigned char buffer[CONF_TWI_SLAVE0_PAGE_SIZE];
	status_code_t status;

	disableJtagPort();
	sysclk_init();
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

	opt.speed=CONF_TWI_MASTER_D_FREQUENCY;
	twi_master_setup(CONF_TWI_MASTER_D, &opt);

	/**
	* write to EEPROM 1
	*/
	printString("Data is being written to EEPROM1 ...\r\n");
	//slave address.
	packet.chip=CONF_TWI_SLAVE0_ADDRESS;
	//size of data address
	packet.addr_length=2;
	//data to write
	packet.buffer=buffer;
	//length of data to write
	packet.length=2; //two bytes will be sent to slave.
	packet.no_wait=false;//wait for the driver if it is busy.
	for(i=0;i<0x10000;i+=2)
	{
		//data address
		packet.addr[0]=i>>8;//MSB of address
		packet.addr[1]=i;//LSB of address
		buffer[0]=i>>8;
		buffer[1]=i;
		//continuous writing could fail because EEPROM needs a maxium of 5ms to save data to internal memory, specified in EEPROM datasheet.
		while(STATUS_OK!=twi_master_write(CONF_TWI_MASTER_D, &packet))
		{
			;
		}
		if((i&0xff)==0)
		{
			printString("current index: ");
			printHex(i>>24);
			printString(", ");
			printHex(i>>16);
			printString(", ");
			printHex(i>>8);
			printString(", ");
			printHex(i);
			printString("\r\n");
		}
	}
	printString("Data was written to EEPROM, current index: ");
	printHex(i>>24);
	printString(", ");
	printHex(i>>16);
	printString(", ");
	printHex(i>>8);
	printString(", ");
	printHex(i);
	printString("\r\n");

	/**
	* read from EEPROM 1
	*/
	packet.chip=CONF_TWI_SLAVE0_ADDRESS;
	packet.addr_length=2;
	packet.buffer=buffer;
	packet.length=2;
	packet.no_wait=false;
	printString("Data is being read from EEPROM1 ...\r\n");
	for(i=0;i<0x10000;i+=2)
	{
		packet.addr[0]=i>>8;
		packet.addr[1]=i;
		counter=0;
		while(0!=twi_master_read(CONF_TWI_MASTER_D, &packet))
		{
			counter++;
		}
		if(0!=counter)
		{
			printString("repeat: ");
			printHex(counter>>8);
			printString(", ");
			printHex(counter);
			printString("\r\n");
		}
		data=buffer[0];
		data<<=8;
		data+=buffer[1];
		if(data!=i)
		{
			printString("Data read: ");
			printHex(buffer[0]);
			printString(", ");
			printHex(buffer[1]);
			printString("\r\n");
			break;
		}
	}
	if(0x10000==i)
	{
		printString("All data in EEPROM1 was verified successfully\r\n");
	}
	else
	{
		printString("Error occurred at ");
		printHex(i>>8);
		printString(", ");
		printHex(i);
		printString("\r\n");
	}

	/**
	* write to EEPROM 2
	*/
	printString("Data is being written to EEPROM2 ...\r\n");
	//slave address.
	packet.chip=CONF_TWI_SLAVE1_ADDRESS;
	//size of data address
	packet.addr_length=2;
	//data to write
	packet.buffer=buffer;
	//length of data to write
	packet.length=2;
	packet.no_wait=false;
	for(i=0;i<0x10000;i+=2)
	{
		//data address
		packet.addr[0]=i>>8;
		packet.addr[1]=i;
		buffer[0]=i>>8;
		buffer[1]=i;
		//continuous writing could fail because EEPROM needs a maxium 5ms to save data to internal memory, specified in EEPROM datasheet.
		while(STATUS_OK!=twi_master_write(CONF_TWI_MASTER_D, &packet))
		{
			;
		}
		if((i&0xff)==0)
		{
			printString("current index: ");
			printHex(i>>24);
			printString(", ");
			printHex(i>>16);
			printString(", ");
			printHex(i>>8);
			printString(", ");
			printHex(i);
			printString("\r\n");
		}
	}
	printString("Data was written to EEPROM, current index: ");
	printHex(i>>24);
	printString(", ");
	printHex(i>>16);
	printString(", ");
	printHex(i>>8);
	printString(", ");
	printHex(i);
	printString("\r\n");

	/**
	* read from EEPROM 2
	*/
	packet.chip=CONF_TWI_SLAVE1_ADDRESS;
	packet.addr_length=2;
	packet.buffer=buffer;
	packet.length=2;
	packet.no_wait=false;
	printString("Data is being read from EEPROM2 ...\r\n");
	for(i=0;i<0x10000;i+=2)
	{
		packet.addr[0]=i>>8;
		packet.addr[1]=i;
		counter=0;
		while(0!=twi_master_read(CONF_TWI_MASTER_D, &packet))
		{
			counter++;
		}
		if(0!=counter)
		{
			printString("repeat: ");
			printHex(counter>>8);
			printString(", ");
			printHex(counter);
			printString("\r\n");
		}
		data=buffer[0];
		data<<=8;
		data+=buffer[1];
		if(data!=i)
		{
			printString("Data read: ");
			printHex(buffer[0]);
			printString(", ");
			printHex(buffer[1]);
			printString("\r\n");
			break;
		}
	}
	if(0x10000==i)
	{
		printString("All data in EEPROM2 was verified successfully\r\n");
	}
	else
	{
		printString("Error occurred at ");
		printHex(i>>8);
		printString(", ");
		printHex(i);
		printString("\r\n");
	}
	while(1)
	{
		;
	}
}

void ecd300TestMax485(void)
{
	usart_rs232_options_t uartOption;
	unsigned long i;
	char rc;
	
	{
		unsigned long i;

		PORTA_DIR=0xff;
		for(i=0;i<0x100000; i++)
		{
			if(i&0x10000)
			{
				PORTA_OUT=0xff;
			}
			else
			{
				PORTA_OUT=0x00;
			}
		}
	}

	PORTA_DIR=0x00;
	PORTB_DIR=0x00;
	PORTD_DIRCLR=0x10;
	PORTF_DIR=0x00;
	PORTH_DIR=0x00;
	PORTJ_DIR=0x00;
	PORTK_DIR=0x00;

	disableJtagPort();
	sysclk_init();
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

	printString("Press any key to initialize MAX1 and MAX2\r\n");
	getChar();
	rc=ecd300InitMax485(ECD300_MAX485_1);
	if(0!=rc)
	{
		printString("Fail to initialize MAX1, return value: ");
		printHex(rc);
		printString("\r\n");
	}
	rc=ecd300InitMax485(ECD300_MAX485_2);
	if(0!=rc)
	{
		printString("Fail to initialize MAX2, return value: ");
		printHex(rc);
		printString("\r\n");
	}

	printString("Press any key to set MAX2 output, MAX1 input\r\n");
	getChar();
	rc=ecd300SetMax485Direction(ECD300_MAX485_2, ECD300_MAX485_OUTPUT);
	if(0!=rc)
	{
		printString("Fail to set MAX2 output, return value: ");
		printHex(rc);
		printString("\r\n");
	}
	rc=ecd300SetMax485Direction(ECD300_MAX485_1, ECD300_MAX485_INPUT);
	if(0!=rc)
	{
		printString("Fail to set MAX1 input, return value: ");
		printHex(rc);
		printString("\r\n");
	}
	
	//clear the possible data received in MAX1, caused by the transition of data direction.
	{
		unsigned long i;
		unsigned char buffer;
		unsigned short amount;

		for(i=0;i<0x10000;i++)
		{
			amount=1;
			ecd300ReadMax485(ECD300_MAX485_1, &buffer, &amount);
		}
	}
	
	printString("Press any key to make MAX2 send data, MAX1 receive data\r\n");
	getChar();
	{
		unsigned short amount;
		unsigned char buffer[50];
		unsigned char i;
		bool bFinished;

		amount=sizeof("hello world!");
		rc=ecd300WriteMax485(ECD300_MAX485_2, "hello world!", &amount);
		if(0!=rc)
		{
			printString("MAX2 failed to output data, return value: ");
			printHex(rc);
			printString("\r\n");
		}
		else
		{
			printString("MAX2 sends out data successfully, amount: ");
			printHex(amount);
			printString("\r\n");
		}

		for(i=0;i<50;i++)
		{
			buffer[i]=0;
		}
		
		for(;;)
		{
			rc=ecd300IsWrittingFinished(ECD300_MAX485_2, &bFinished);
			if(0!=rc)
			{
				printString("Fail to check if MAX2 finishes sending, return value: ");
				printHex(rc);
				printString("\r\n");
				break;
			}
			if(bFinished)
			{
				break;
			}
		}

		amount=50;
		rc=ecd300ReadMax485(ECD300_MAX485_1, buffer, &amount);
		if(0!=rc)
		{
			printString("MAX1 failed to receive data, return value: ");
			printHex(rc);
			printString("\r\n");
		}
		else
		{
			printString("Content received: ");
			printString(buffer);
			printString("\r\n");
			printString("amount: ");
			printHex(amount);
			printString("\r\n");
		}
	}

	printString("Press any key to revert the flow of data between MAX1 and MAX2\r\n");
	printString("!!!! Make sure that nothing is connected to PD1 !!!!\r\n");
	getChar();
	rc=ecd300SetMax485Direction(ECD300_MAX485_1, ECD300_MAX485_OUTPUT);
	if(0!=rc)
	{
		printString("Fail to set MAX1 output, return value: ");
		printHex(rc);
		printString("\r\n");
	}
	rc=ecd300SetMax485Direction(ECD300_MAX485_2, ECD300_MAX485_INPUT);
	if(0!=rc)
	{
		printString("Fail to set MAX2 input, return value: ");
		printHex(rc);
		printString("\r\n");
	}
	
	//clear the possible data received in MAX1, caused by the transition of data direction.
	{
		unsigned long i;
		unsigned char buffer;
		unsigned short amount;

		for(i=0;i<0x10000;i++)
		{
			amount=1;
			ecd300ReadMax485(ECD300_MAX485_2, &buffer, &amount);
		}
	}
	
	{
		unsigned short amount;
		unsigned char buffer[50];
		char i;
		bool bFinished;

		amount=sizeof("hello world!");
		rc=ecd300WriteMax485(ECD300_MAX485_1, "hello world!", &amount);
		if(0!=rc)
		{
			printString("MAX1 failed to output data, return value: ");
			printHex(rc);
			printString("\r\n");
		}
		else
		{
			printString("MAX1 sends out data successfully, amount: ");
			printHex(amount);
			printString("\r\n");
		}

		for(i=0;i<50;i++)
		{
			buffer[i]=0;
		}
		
		for(;;)
		{
			rc=ecd300IsWrittingFinished(ECD300_MAX485_1, &bFinished);
			if(0!=rc)
			{
				printString("Fail to check if MAX1 finishes sending, return value: ");
				printHex(rc);
				printString("\r\n");
				break;
			}
			if(bFinished)
			{
				break;
			}
		}

		amount=50;
		rc=ecd300ReadMax485(ECD300_MAX485_2, buffer, &amount);
		if(0!=rc)
		{
			printString("MAX2 failed to receive data, return value: ");
			printHex(rc);
			printString("\r\n");
		}
		else
		{
			printString("MAX2 receives: ");
			printString(buffer);
			printString("\r\n");
			printString("amount: ");
			printHex(amount);
			printString("\r\n");
		}
	}
	

	printString("\r\npress any key to execute continuous test\r\n");
	getChar();
	{
		unsigned long i;
		unsigned char buffer[64];
		unsigned short j;
		char k;
		unsigned short amount;

		unsigned char c;
		bool bFinished;

		for(i=0;;i+=0x400)
		{
			//change the data direction each 1024 bytes are transferred.
			if(i&0x400)
			{//from MAX1 to MAX2
				//switch data direction
				rc=ecd300SetMax485Direction(ECD300_MAX485_1, ECD300_MAX485_OUTPUT);
				if(0!=rc)
				{
					printString("Fail to set MAX1 output, return value: ");
					printHex(rc);
					printString("\r\n");
					goto error_handling;
				}
				rc=ecd300SetMax485Direction(ECD300_MAX485_2, ECD300_MAX485_INPUT);
				if(0!=rc)
				{
					printString("Fail to set MAX2 input, return value: ");
					printHex(rc);
					printString("\r\n");
					goto error_handling;
				}
		
				for(j=0;j<0x400;j+=64)
				{
					//initialize content to send out
					for(c=0;c<64;c++)
					{
						buffer[c]=c+1;
					}
					//send out data
					amount=c;
					rc=ecd300WriteMax485(ECD300_MAX485_1, buffer, &amount);
					if(0!=rc)
					{
						printString("MAX1 failed to output data, return value: ");
						printHex(rc);
						printString("\r\n");
						goto error_handling;
					}
					if(64!=amount)
					{
						printString("MAX1 cannot sent all data\r\n");
						goto error_handling;
					}

					for(c=0;c<64;c++)
					{
						buffer[c]=0;
					}

					//receive data.
					for(k=64;k>0;)
					{
						amount=k;
						rc=ecd300ReadMax485(ECD300_MAX485_2, buffer+64-k, &amount);
						if(0!=rc)
						{
							printString("MAX2 failed to receive data, return value: ");
							printHex(rc);
							printString("\r\n");
							goto error_handling;
						}
						k-=amount;
						if(k<0)
						{
							printString("amount of data is wrong when from MAX1 to MAX2\r\n");
							goto error_handling;
						}
					}

					//check data.
					{
						for(c=0;c<64;c++)
						{
							if(buffer[c]!=(c+1))
							{
								printString("Error in transfer from MAX1 to MAX2, index: ");
								printHex(c);
								printString(", content: ");
								printHex(buffer[c]);
								printString(", amount: ");
								printHex(amount);
								printString("\r\n");
								goto error_handling;
							}
						}
					}
				}
			}
			else
			{//from MAX2 to MAX1
				//switch data direction
				rc=ecd300SetMax485Direction(ECD300_MAX485_2, ECD300_MAX485_OUTPUT);
				if(0!=rc)
				{
					printString("Fail to set MAX2 output, return value: ");
					printHex(rc);
					printString("\r\n");
					goto error_handling;
				}
				rc=ecd300SetMax485Direction(ECD300_MAX485_1, ECD300_MAX485_INPUT);
				if(0!=rc)
				{
					printString("Fail to set MAX1 input, return value: ");
					printHex(rc);
					printString("\r\n");
					goto error_handling;
				}
		
				for(j=0;j<0x400;j+=64)
				{
					//initialize content to send out
					for(c=0;c<64;c++)
					{
						buffer[c]=c+1;
					}
					//send out data
					amount=c;
					rc=ecd300WriteMax485(ECD300_MAX485_2, buffer, &amount);
					if(0!=rc)
					{
						printString("MAX2 failed to output data, return value: ");
						printHex(rc);
						printString("\r\n");
						goto error_handling;
					}
					if(64!=amount)
					{
						printString("MAX2 cannot sent all data\r\n");
						goto error_handling;
					}

					for(c=0;c<64;c++)
					{
						buffer[c]=0;
					}

					//receive data.
					for(k=64;k>0;)
					{
						amount=k;
						rc=ecd300ReadMax485(ECD300_MAX485_1, buffer+64-k, &amount);
						if(0!=rc)
						{
							printString("MAX1 failed to receive data, return value: ");
							printHex(rc);
							printString("\r\n");
							goto error_handling;
						}
						k-=amount;
						if(k<0)
						{
							printString("amount of data is wrong when from MAX2 to MAX1\r\n");
							goto error_handling;
						}
					}

					//check data.
					{
						for(c=0;c<64;c++)
						{
							if(buffer[c]!=(c+1))
							{
								printString("Error in transfer from MAX2 to MAX1, index: ");
								printHex(c);
								printString(", content: ");
								printHex(buffer[c]);
								printString(", amount: ");
								printHex(amount);
								printString("\r\n");
								goto error_handling;
							}
						}
					}
				}
			}

			printString("i: ");
			printHex(i>>24);
			printString(", ");
			printHex(i>>16);
			printString(", ");
			printHex(i>>8);
			printString(", ");
			printHex(i>>0);
			printString("\r\n");
		}
	}
	
error_handling:
	
	printString("\r\n**** Test finished ****\r\n");
	
	while(1)
	{
		;
	}	
}
