#ifndef _ECD300_DELAY_H__
#define _ECD300_DELAY_H__

#warning Driver ECD300 delay should be adjusted according to the peripheral frequency.

/**
* 
*/
char ecd300DelayMicrosecond(unsigned short us);

char ecd300DelayMillisecond(unsigned short ms);

char ecd300DelayInit(void);

#endif

