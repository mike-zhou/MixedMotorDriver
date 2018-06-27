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

static bool _pollHexChar(unsigned char * p)
{
	char rc;

	if(NULL==p)
	{
		return false;
	}
	
	rc=ecd300PollChar(ECD300_UART_0, p);

	if(rc==1)
		return true;
	else
		return false;
}

static unsigned char _nandPageBuffer[2112];

static void _testReadNand(void)
{
	short rc;
	unsigned long i;
	
	for(i=0;i<2112;i++)
	{
		_nandPageBuffer[i]=0;
	}

	rc=ecd300ReadNandPage(0, 0xffff, 0, _nandPageBuffer, 2112, true);
	printString("return value: ");
	printHex(rc>>8);
	printString(", ");
	printHex(rc);
	printString("\r\n");
	for(i=0;i<2112;i++)
	{
		if(0==(i&0xf))
		{
			printString("\r\n");
		}
		printHex(_nandPageBuffer[i]);
		printString(", ");
	}
}

static void _testEraseNand(void)
{
	short rc;
	unsigned long i;

	rc=ecd300EraseNandBlock(0, 0xffff, true);
	printString("return value: ");
	printHex(rc>>8);
	printString(", ");
	printHex(rc);
	printString("\r\n");
}

static void _testNandProgram(void)
{
	short rc;
	unsigned long i;

	for(i=0;i<4;i++)
	{
		_nandPageBuffer[i]=i;
	}

	rc=ecd300ProgramNandPage(0, 0xffff, 2048, _nandPageBuffer, 4, true);
	printString("return value: ");
	printHex(rc>>8);
	printString(", ");
	printHex(rc);
	printString("\r\n");
}

void ecd300TestNand(void)
{
	usart_rs232_options_t uartOption;
	unsigned long i;
	short rc;

	unsigned short colAddress;
	unsigned char pageNumber;
	unsigned short blockNumber;

	PORTA_DIR=0xff;
	PORTA_OUT=0x55;
	for(i=0;i<0x40000;i++)
	{
		if(i&0x10000)
			PORTA_OUT=0xff;
		else
			PORTA_OUT=0x55;
	}

	disableJtagPort();
	sysclk_init();
	irq_initialize_vectors();
	cpu_irq_enable();

	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;
	ecd300InitUart(ECD300_UART_0, &uartOption);
	printString("UART0 was initialized 9\r\n");

	ecd300InitNand();

	printString("\r\nErase blocks ...\r\n");
	for(blockNumber=0;blockNumber<1024;blockNumber++)
	{
		printString("    Erase block: ");
		printHex(blockNumber>>8);
		printString(", ");
		printHex(blockNumber);
		rc=ecd300EraseNandBlock(0, blockNumber<<6, true);
		if(0==rc)
		{
			printString(": success\r\n");
		}
		else
		{
			printString(": return value: ");
			printHex(rc>>8);
			printString(", ");
			printHex(rc);
			printString("\r\n");
		}
	}

	printString("\r\nCheck result of erasing ...\r\n");
	for(blockNumber=0;blockNumber<1024;blockNumber++)
	{
		printString("    Check block ");
		printHex(blockNumber>>8);
		printString(", ");
		printHex(blockNumber);
		printString("\r\n");
		
		for(pageNumber=0;pageNumber<64;pageNumber++)
		{
			for(colAddress=0;colAddress<2112;colAddress++)
			{
				_nandPageBuffer[colAddress]=0x00;
			}
			
			rc=ecd300ReadNandPage(0, (blockNumber<<6)+pageNumber, 0, _nandPageBuffer, 2112, true);
			if(2112==rc)
			{
				for(colAddress=0;colAddress<2112;colAddress++)
				{
					if(_nandPageBuffer[colAddress]!=0xff)
					{
						printString("        Content error: pageNo.: ");
						printHex(pageNumber);
						printString(", colAddress: ");
						printHex(colAddress>>8);
						printString(", ");
						printHex(colAddress);
						printString(", content: ");
						printHex(_nandPageBuffer[colAddress]);
						printString("\r\n");
						break;
					}
				}
			}
			else
			{
				printString("        Reading page failed: page No.: ");
				printHex(pageNumber);
				printString(": return value: ");
				printHex(rc>>8);
				printString(", ");
				printHex(rc);
				printString("\r\n");
			}
		}
	}


	printString("\r\nProgram all data to ZERO ...\r\n");
	for(colAddress=0;colAddress<2112;colAddress++)
	{
		_nandPageBuffer[colAddress]=0x00;
	}
	for(blockNumber=0;blockNumber<1024;blockNumber++)
	{
		printString("    Program block ");
		printHex(blockNumber>>8);
		printString(", ");
		printHex(blockNumber);
		printString("\r\n");
		
		for(pageNumber=0;pageNumber<64;pageNumber++)
		{
			rc=ecd300ProgramNandPage(0, (blockNumber<<6)+pageNumber, 0, _nandPageBuffer, 2112, true);
			if(2112!=rc)
			{
				printString("        Programming page failed: page No.: ");
				printHex(pageNumber);
				printString(": return value: ");
				printHex(rc>>8);
				printString(", ");
				printHex(rc);
				printString("\r\n");
			}
		}
	}
	
	printString("\r\nCheck result of All Zero Programming ...\r\n");
	for(blockNumber=0;blockNumber<1024;blockNumber++)
	{
		printString("    Check block ");
		printHex(blockNumber>>8);
		printString(", ");
		printHex(blockNumber);
		printString("\r\n");
		
		for(pageNumber=0;pageNumber<64;pageNumber++)
		{
			for(colAddress=0;colAddress<2112;colAddress++)
			{
				_nandPageBuffer[colAddress]=0xff;
			}
			
			rc=ecd300ReadNandPage(0, (blockNumber<<6)+pageNumber, 0, _nandPageBuffer, 2112, true);
			if(2112==rc)
			{
				for(colAddress=0;colAddress<2112;colAddress++)
				{
					if(_nandPageBuffer[colAddress]!=0x00)
					{
						printString("        Content error: pageNo.: ");
						printHex(pageNumber);
						printString(", colAddress: ");
						printHex(colAddress>>8);
						printString(", ");
						printHex(colAddress);
						printString(", content: ");
						printHex(_nandPageBuffer[colAddress]);
						printString("\r\n");
						break;
					}
				}
			}
			else
			{
				printString("        Reading page failed: page No.: ");
				printHex(pageNumber);
				printString(": return value: ");
				printHex(rc>>8);
				printString(", ");
				printHex(rc);
				printString("\r\n");
			}
		}
	}

	printString("\r\nErase blocks ...\r\n");
	for(blockNumber=0;blockNumber<1024;blockNumber++)
	{
		printString("    Erase block: ");
		printHex(blockNumber>>8);
		printString(", ");
		printHex(blockNumber);
		rc=ecd300EraseNandBlock(0, blockNumber<<6, true);
		if(0==rc)
		{
			printString(": success\r\n");
		}
		else
		{
			printString(": return value: ");
			printHex(rc>>8);
			printString(", ");
			printHex(rc);
			printString("\r\n");
		}
	}

	printString("\r\nProgram address to the data ...\r\n");
	for(blockNumber=0;blockNumber<1024;blockNumber++)
	{
		printString("    Program block ");
		printHex(blockNumber>>8);
		printString(", ");
		printHex(blockNumber);
		printString("\r\n");
		
		for(pageNumber=0;pageNumber<64;pageNumber++)
		{
			for(colAddress=0;colAddress<2112;colAddress++)
			{
				if(0==(colAddress&0x3))
				{
					_nandPageBuffer[colAddress]=colAddress;
					_nandPageBuffer[colAddress+1]=colAddress>>8;
					_nandPageBuffer[colAddress+2]=((blockNumber<<6)+pageNumber);
					_nandPageBuffer[colAddress+3]=((blockNumber<<6)+pageNumber)>>8;
				}
			}
			rc=ecd300ProgramNandPage(0, (blockNumber<<6)+pageNumber, 0, _nandPageBuffer, 2112, true);
			if(2112!=rc)
			{
				printString("        Programming page failed: page No.: ");
				printHex(pageNumber);
				printString(": return value: ");
				printHex(rc>>8);
				printString(", ");
				printHex(rc);
				printString("\r\n");
			}
		}
	}


	printString("\r\nCheck result of data Programming ...\r\n");
	i=0;
	for(blockNumber=0;blockNumber<1024;blockNumber++)
	{
		printString("    Check block ");
		printHex(blockNumber>>8);
		printString(", ");
		printHex(blockNumber);
		printString("\r\n");
		
		for(pageNumber=0;pageNumber<64;pageNumber++)
		{
			for(colAddress=0;colAddress<2112;colAddress++)
			{
				_nandPageBuffer[colAddress]=0xff;
			}
			
			rc=ecd300ReadNandPage(0, (blockNumber<<6)+pageNumber, 0, _nandPageBuffer, 2112, true);
			if(2112==rc)
			{
				for(colAddress=0;colAddress<2112;colAddress++)
				{
					if(0==(colAddress&0x3))
					{
						if(
							((colAddress&0xff)!=_nandPageBuffer[colAddress])||
							(((colAddress>>8)&0xff)!=_nandPageBuffer[colAddress+1])||
							((((blockNumber<<6)+pageNumber)&0xff)!=_nandPageBuffer[colAddress+2])||
							(((((blockNumber<<6)+pageNumber)>>8)&0xff)!=_nandPageBuffer[colAddress+3])
							)
						{
							printString("        Content error: pageNo.: ");
							printHex(pageNumber);
							printString(", colAddress: ");
							printHex(colAddress>>8);
							printString(", ");
							printHex(colAddress);
							printString(", content: ");
							printHex(_nandPageBuffer[colAddress]);
							printString(", ");
							printHex(_nandPageBuffer[colAddress+1]);
							printString(", ");
							printHex(_nandPageBuffer[colAddress+2]);
							printString(", ");
							printHex(_nandPageBuffer[colAddress+3]);
							printString(", ");
							printString("\r\n");
							break;
						}
					}
				}
			}
			else
			{
				printString("        Reading page failed: page No.: ");
				printHex(pageNumber);
				printString(": return value: ");
				printHex(rc>>8);
				printString(", ");
				printHex(rc);
				printString("\r\n");
			}
		}
	}

	printString("\r\nNand tests finished.\r\n");
	
	while(1)
	{
		;
	}

}
