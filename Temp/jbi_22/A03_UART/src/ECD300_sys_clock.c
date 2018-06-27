/*
 * ECD300_sys_clock.c
 *
 * Created: 2015/1/9 10:36:54
 *  Author: user
 */ 

#include <asf.h>

#include "ECD300_sys_clock.h"

/**
	Functions relating to system clock selection
*/
inline void sysClkSelect2Mhz(void)
{
	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;
	
	CLK_CTRL=0;
}

inline void sysClkSelect32Mhz(void)
{
	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;
	
	CLK_CTRL=1;
}

inline void sysClkSelect32Khz(void)
{
	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;
	
	CLK_CTRL=2;
}

inline void sysClkSelectExternalOsc(void)
{
	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;
	
	CLK_CTRL=3;
}

inline void sysClkSelectPll(void)
{
	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;
	
	CLK_CTRL=4;
}




/**
	Function relating to Peripheral clocks and CPU clocks.
*/
inline void sysClkEnablePer4Per2(void)
{
	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;

	CLK_PSCTRL=0x03;
}




/**
	Functions relating to PLL.
*/
inline void sysClkEnablePll(void)
{
	OSC_CTRL|=OSC_PLLEN_bm;
}

inline void sysClkDisablePll(void)
{
	OSC_CTRL&=~OSC_PLLEN_bm;
}

inline unsigned char sysClkIsPllReady(void)
{
	if(OSC_STATUS&OSC_PLLRDY_bm)
		return 1;
	else
		return 0;
}

inline void sysClkSelectPllSource2M(void)
{
	OSC_PLLCTRL=(OSC_PLLCTRL&OSC_PLLSRC_gm);
}

inline void sysClkSelectPllSource32MDiv4(void)
{
	OSC_PLLCTRL=(OSC_PLLCTRL&OSC_PLLSRC_gm)+(0x2<<OSC_PLLSRC_gp);
}

inline void sysClkSelectPllSourceExternalOscillator(void)
{
	OSC_PLLCTRL=(OSC_PLLCTRL&OSC_PLLSRC_gm)+(0x3<<OSC_PLLSRC_gp);
}
/**
	Factor is one of 1 to 32, result = source * factor .
*/

inline void sysClkPllSetMulFactor(unsigned char factor)
{
	if(factor==0)
	{
		factor=1;
	}
	if(factor>32)
	{
		factor=32;
	}
	OSC_PLLCTRL=(OSC_PLLCTRL&~OSC_PLLFAC_gm)+((factor-1)&OSC_PLLFAC_gm);
}



/**
	Functions relating to the internal 32MHz Oscillator.
*/
inline void sysClkDisable32Mhz(void)
{
	OSC_CTRL&=~OSC_RC32MEN_bm;
}

inline void sysClkEnable32Mhz(void)
{
	OSC_CTRL|=OSC_RC32MEN_bm;
}

inline void sysClkCalibrate32MhzTo48MHz(void)
{
	DFLLRC32M_COMP1=0x1B;
	DFLLRC32M_COMP2=0xB7;
}

inline unsigned char sysClkIs32MhzReady(void)
{
	if(OSC_STATUS&OSC_RC32MRDY_bm)
		return 1;
	else
		return 0;
}



/**
	Functions relating USB.
*/
inline void sysClkEnableUsb(void)
{
	CLK_USBCTRL|=CLK_USBSEN_bm;
}

inline void sysClkDisableUsb(void)
{
	CLK_USBCTRL&=~CLK_USBSEN_bm;
}

inline void sysClkUsbSelect32Mhz(void)
{
	CLK_USBCTRL=(CLK_USBCTRL&~CLK_USBSRC_gm)+(1<<CLK_USBSRC_gp);
}


