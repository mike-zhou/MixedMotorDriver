/*
 * ECD300_ioport.h
 *
 * Created: 2014/12/18 19:18:19
 *  Author: user
 */ 


#ifndef ECD300_IOPORT_H_
#define ECD300_IOPORT_H_

#include <asf.h>

#define ecd300SetPortData(port, data) (port.OUT)=(data)
#define ecd300GetPortData(port) (port.OUT)

static inline void disableJtagPort(void)
{
	//enable the change of MCUCR.
	CPU_CCP = CCP_IOREG_gc;
	//disable JTAG port.
	MCU_MCUCR = MCU_JTAGD_bm;
}


#endif /* ECD300_IOPORT_H_ */