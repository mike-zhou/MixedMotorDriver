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

//define MAX_EEPROM_PAGE_SIZE as max(CONF_TWI_SLAVE0_PAGE_SIZE, CONF_TWI_SLAVE1_PAGE_SIZE)
#define MAX_EEPROM_PAGE_SIZE CONF_TWI_SLAVE0_PAGE_SIZE

void ecd300TestPdi(void)
{
	usart_rs232_options_t uartOption;
  	twi_master_options_t opt;
	twi_package_t packet;

	unsigned char _hexDataBuffer[2][MAX_EEPROM_PAGE_SIZE];
	unsigned long eepromAddress;
	unsigned char cacheIndex;
	bool isWritingSlave0;
	unsigned short index;
	unsigned char * pCache;
	bool rc;
	unsigned char c;
	unsigned char echo[2];

	unsigned char * pEndOfFile=":00000001FF";
	unsigned char endOfFile[11];
	unsigned char endOfFileIndex;

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

	opt.speed=CONF_TWI_MASTER_D_FREQUENCY;
	twi_master_setup(CONF_TWI_MASTER_D, &opt);

	isWritingSlave0=true;
	eepromAddress=0;
	cacheIndex=0;
	pCache=_hexDataBuffer[cacheIndex&1];
	for(index=0;index<MAX_EEPROM_PAGE_SIZE;index++)
	{
		pCache[index]=0;
	}
	index=0;
	echo[1]=0;
	while(1)
	{
		rc=_pollHexChar(&c);
		if(true==rc)
		{
			//get a HEX character.
			echo[0]=c;
			printString(echo);//send back the character.

			//push this character to endOfFile array.
			for(endOfFileIndex=0;endOfFileIndex<10;endOfFileIndex++)
			{
				endOfFile[endOfFileIndex]=endOfFile[endOfFileIndex+1];
			}
			endOfFile[10]=c;

			//save this character to cache.
			pCache[index]=c;
			index++;
			
			if(isWritingSlave0)
			{
				//HEX data should be saved to E1.
				
				if(CONF_TWI_SLAVE0_PAGE_SIZE==index)
				{
					//a page is full, save this page to EEPROM.
					//slave address.
					packet.chip=CONF_TWI_SLAVE0_ADDRESS;
					packet.addr[0]=eepromAddress>>8;//MSB of address
					packet.addr[1]=eepromAddress;//LSB of address
					//size of data address
					packet.addr_length=2;
					//data to write
					packet.buffer=pCache;
					//length of data to write
					packet.length=CONF_TWI_SLAVE0_PAGE_SIZE; 
					packet.no_wait=true;//do not wait for any pending access.
					if(STATUS_OK!=twi_master_write(CONF_TWI_MASTER_D, &packet))
					{
						printString("\r\n!!!!! Error: Input is too fast while E1 is being written !!!!!\r\n");
						break;
					}
					eepromAddress+=CONF_TWI_SLAVE0_PAGE_SIZE;

					//check whether further data should be save to E2.
					if(CONF_TWI_SLAVE0_SIZE==eepromAddress)
					{
						//further data should be save to E2.
						isWritingSlave0=false;
						eepromAddress=0;
					}

					//while E1 is saving this page to internal memory, transfer the coming HEX data to the other cache.
					cacheIndex+=1;
					pCache=_hexDataBuffer[cacheIndex&1];
					for(index=0;index<CONF_TWI_SLAVE0_PAGE_SIZE;index++)
					{
						pCache[index]=0;
					}
					index=0;
				}
			}
			else
			{
				//HEX data should be saved to E2.

				if(CONF_TWI_SLAVE1_PAGE_SIZE==index)
				{
					//a page is full, save this page to EEPROM.
					//slave address.
					packet.chip=CONF_TWI_SLAVE1_ADDRESS;
					packet.addr[0]=eepromAddress>>8;//MSB of address
					packet.addr[1]=eepromAddress;//LSB of address
					//size of data address
					packet.addr_length=2;
					//data to write
					packet.buffer=pCache;
					//length of data to write
					packet.length=CONF_TWI_SLAVE1_PAGE_SIZE; 
					packet.no_wait=true;//do not wait for any pending access.
					if(STATUS_OK!=twi_master_write(CONF_TWI_MASTER_D, &packet))
					{
						printString("\r\n!!!!! Error: Input is too fast while E2 is being written  !!!!!\r\n");
						break;
					}
					eepromAddress+=CONF_TWI_SLAVE1_PAGE_SIZE;

					//check whether E2 is full.
					if(CONF_TWI_SLAVE1_SIZE==eepromAddress)
					{
						//further data should be save to E2.
						printString("\r\n!!!! Error: Too much HEX content, cannot be saved !!!!\r\n");
						break;
					}

					//while E1 is saving this page to internal memory, transfer the coming HEX data to the other cache.
					cacheIndex+=1;
					pCache=_hexDataBuffer[cacheIndex&1];
					for(index=0;index<CONF_TWI_SLAVE1_PAGE_SIZE;index++)
					{
						pCache[index]=0;
					}
					index=0;
				}
			}

			//check whether EOF received.
			for(endOfFileIndex=0;endOfFileIndex<11;endOfFileIndex++)
			{
				if(endOfFile[endOfFileIndex]!=pEndOfFile[endOfFileIndex])
				{
					break;
				}
			}
			if(11==endOfFileIndex)
			{
				//EOF is received.
				if(isWritingSlave0)
				{
					if(0!=index)
					{
						//a whole page is not filled, write the content of this page to E1.
						packet.chip=CONF_TWI_SLAVE0_ADDRESS;
						packet.addr[0]=eepromAddress>>8;//MSB of address
						packet.addr[1]=eepromAddress;//LSB of address
						//size of data address
						packet.addr_length=2;
						//data to write
						packet.buffer=pCache;
						//length of data to write
						packet.length=CONF_TWI_SLAVE0_PAGE_SIZE; 
						packet.no_wait=false;//wait for any pending access.
						while(STATUS_OK!=twi_master_write(CONF_TWI_MASTER_D, &packet))
						{
							;
						}
						eepromAddress+=CONF_TWI_SLAVE0_PAGE_SIZE;

						//check whether further data should be save to E2.
						if(CONF_TWI_SLAVE0_SIZE==eepromAddress)
						{
							//further data should be save to E2.
							isWritingSlave0=false;
							eepromAddress=0;
						}

						//while E1 is saving this page to internal memory, transfer the coming HEX data to the other cache.
						cacheIndex+=1;
						pCache=_hexDataBuffer[cacheIndex&1];
						for(index=0;index<CONF_TWI_SLAVE0_PAGE_SIZE;index++)
						{
							pCache[index]=0;
						}
						index=0;
					}
					else
					{
						//a whole page is filled, so the EOF has already been written to E1.
					}
				}
				else
				{
					if(0!=index)
					{
						//a whole page is not filled, write the content of this page to E2.
						packet.chip=CONF_TWI_SLAVE1_ADDRESS;
						packet.addr[0]=eepromAddress>>8;//MSB of address
						packet.addr[1]=eepromAddress;//LSB of address
						//size of data address
						packet.addr_length=2;
						//data to write
						packet.buffer=pCache;
						//length of data to write
						packet.length=CONF_TWI_SLAVE1_PAGE_SIZE; 
						packet.no_wait=false;//wait for any pending access.
						while(STATUS_OK!=twi_master_write(CONF_TWI_MASTER_D, &packet))
						{
							;
						}
						eepromAddress+=CONF_TWI_SLAVE1_PAGE_SIZE;

						//check whether E2 is full.
						if(CONF_TWI_SLAVE1_SIZE==eepromAddress)
						{
							//further data should be save to E2.
							printString("\r\n!!!! Success: This HEX has been written to EEPROM, but no space for further HEX file !!!!\r\n");
							break;
						}

						//while E1 is saving this page to internal memory, transfer the coming HEX data to the other cache.
						cacheIndex+=1;
						pCache=_hexDataBuffer[cacheIndex&1];
						for(index=0;index<CONF_TWI_SLAVE1_PAGE_SIZE;index++)
						{
							pCache[index]=0;
						}
						index=0;
					}
					else
					{
						//a whole page is filled, so the EOF has already been written to E2.
					}
				}	

				printString("\r\n!!!! Success: This HEX has been written to EEPROM, waiting for new HEX file !!!!\r\n");
			}
		}
	}
	
	while(1)
	{
		;
	}

}
