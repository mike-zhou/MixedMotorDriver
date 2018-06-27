/*
 * ECD300_sys_clock.h
 *
 * Created: 2014/12/23 10:05:31
 *  Author: user
 */ 


#ifndef ECD300_SYS_CLOCK_H_
#define ECD300_SYS_CLOCK_H_

#include <asf.h>

/**
	Functions relating to system clock selection
*/
static inline void sysClkSelect2Mhz(void)
{
	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;
	
	CLK_CTRL=0;
}
static inline void sysClkSelect32Mhz(void)
{
	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;
	
	CLK_CTRL=1;
}
static inline void sysClkSelect32Khz(void)
{
	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;
	
	CLK_CTRL=2;
}
static inline void sysClkSelectExternalOsc(void)
{
	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;
	
	CLK_CTRL=3;
}
static inline void sysClkSelectPll(void)
{
	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;
	
	CLK_CTRL=4;
}




/**
	Function relating to Peripheral clocks and CPU clocks.
*/
static inline void sysClkEnablePer4Per2(void)
{
	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;

	CLK_PSCTRL=0x03;
}




/**
	Functions relating to PLL.
*/
static inline void sysClkEnablePll(void)
{
	OSC_CTRL|=OSC_PLLEN_bm;
}
static inline void sysClkDisablePll(void)
{
	OSC_CTRL&=~OSC_PLLEN_bm;
}
static inline unsigned char sysClkIsPllReady(void)
{
	if(OSC_STATUS&OSC_PLLRDY_bm)
		return 1;
	else
		return 0;
}
static inline void sysClkSelectPllSource2M(void)
{
	OSC_PLLCTRL=(OSC_PLLCTRL&OSC_PLLSRC_gm);
}
static inline void sysClkSelectPllSource32MDiv4(void)
{
	OSC_PLLCTRL=(OSC_PLLCTRL&OSC_PLLSRC_gm)+(0x2<<OSC_PLLSRC_gp);
}
static inline void sysClkSelectPllSourceExternalOscillator(void)
{
	OSC_PLLCTRL=(OSC_PLLCTRL&OSC_PLLSRC_gm)+(0x3<<OSC_PLLSRC_gp);
}
/**
	Factor is one of 1 to 32, result = source * factor .
*/
static inline void sysClkPllSetMulFactor(unsigned char factor)
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
static inline void sysClkDisable32Mhz(void)
{
	OSC_CTRL&=~OSC_RC32MEN_bm;
}
static inline void sysClkEnable32Mhz(void)
{
	OSC_CTRL|=OSC_RC32MEN_bm;
}
static inline void sysClkCalibrate32MhzTo48MHz(void)
{
	DFLLRC32M_COMP1=0x1B;
	DFLLRC32M_COMP2=0xB7;
}
static inline unsigned char sysClkIs32MhzReady(void)
{
	if(OSC_STATUS&OSC_RC32MRDY_bm)
		return 1;
	else
		return 0;
}



/**
	Functions relating USB.
*/
static inline void sysClkEnableUsb(void)
{
	CLK_USBCTRL|=CLK_USBSEN_bm;
}
static inline void sysClkDisableUsb(void)
{
	CLK_USBCTRL&=~CLK_USBSEN_bm;
}
static inline void sysClkUsbSelect32Mhz(void)
{
	CLK_USBCTRL=(CLK_USBCTRL&~CLK_USBSRC_gm)+(1<<CLK_USBSRC_gp);
}


#endif /* ECD300_SYS_CLOCK_H_ */
