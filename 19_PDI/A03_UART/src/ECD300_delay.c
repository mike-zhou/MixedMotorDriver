#include "ECD300_delay_config.h"
#include "ECD300_delay.h"
#include "asf.h"

static bool bDriverInitialized=false;
/**
* 
* return value:
*	0: 	success.
*	-1:	delay is too long for this function.
*	-2:	this driver is not initialized.
*/
char ecd300DelayMicrosecond(unsigned short us)
{
	unsigned short curStamp, preStamp;

	preStamp=tc_read_count(&ECD300_TIMER);
	
	//check if the delay is too long for this function.
	if(us>1000)
	{
		return -1;
	}

	if(false==bDriverInitialized)
	{
		return -2;
	}
	
	for(;;)
	{
		curStamp=tc_read_count(&ECD300_TIMER);
		//calculate the period.
		if(curStamp>preStamp)
		{
			curStamp-=preStamp;
		}
		else
		{
			curStamp+=(0x10000-preStamp);
		}
		
		if((curStamp>>2)>=us)
		{
			break;
		}
	}
	
	return 0;
}

/**
* 
* return value:
*	0: 	success.
*	-2:	this driver is not initialized.
*/
char ecd300DelayMillisecond(unsigned short ms)
{
	if(false==bDriverInitialized)
	{
		return -2;
	}

	do
	{
		ecd300DelayMicrosecond(1000);
	}while(--ms);

	return 0;
}

char ecd300DelayInit(void)
{
	tc_enable(&ECD300_TIMER);
	tc_reset(&ECD300_TIMER);
	tc_set_wgm(&ECD300_TIMER, TC_WG_NORMAL);
	tc_write_period(&ECD300_TIMER, 0xffff);
	//the actual resolution is about 4MHz
	tc_write_clock_source(&ECD300_TIMER, TC_CLKSEL_DIV8_gc);
	tc_restart(&ECD300_TIMER);

	bDriverInitialized=true;

	return 0;
}


