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

//IO port pin mask
#define ECD300_PORT_PIN0 	0x01
#define ECD300_PORT_PIN1	0x02
#define ECD300_PORT_PIN2	0x04
#define ECD300_PORT_PIN3	0x08
#define ECD300_PORT_PIN4	0x10
#define ECD300_PORT_PIN5	0x20
#define ECD300_PORT_PIN6	0x40
#define ECD300_PORT_PIN7	0x80

//prototype of interrupt callback function.
typedef void (*ecd300_ioport_callback_t) (void);

/**
This function provides ISR of INT0 a callback which will be called when ISR of INT0 is triggered.
Parameters:
	pPort:	address of a port's control structure, defined in iox128a1u.
	callback:	address of callback function.

return value:
	0: 	success
	-1:	the PIN ISR for this port is not enabled. It should be enabled in ECD300_ioport_config.h
*/
char ecd300IoPortRegisterInt0(volatile PORT_t * pPort, ecd300_ioport_callback_t pCallback);

/**
* This function clears the interrupt flag of INT0 of the designated port.
*/
static inline void ecd300IoPortClearInt0Flag(volatile PORT_t * pPort)
{
	pPort->INTFLAGS=PORT_INT0IF_bm;
}


/**
This function provides ISR of INT1 a callback which will be called when ISR of INT1 is triggered.
Parameters:
	pPort:	address of a port's control structure, defined in iox128a1u.
	callback:	address of callback function.

return value:
	0: 	success
	-1:	the PIN ISR for this port is not enabled. It should be enabled in ECD300_ioport_config.h
*/
char ecd300IoPortRegisterInt1(volatile PORT_t * pPort, ecd300_ioport_callback_t pCallback);

/**
* This function clears the interrupt flag of INT1 of the designated port.
*/
static inline void ecd300IoPortClearInt1Flag(volatile PORT_t * pPort)
{
	pPort->INTFLAGS=PORT_INT1IF_bm;
}


/**
This function sets the interrupt level of INT0 of the designated port.
Parameters:
	pPort:	address of a port's control structure, defined in iox128a1u.
	level:	interrupt level of INT0.
*/
static inline void ecd300IoPortSetInt0Level(volatile PORT_t * pPort, PORT_INT0LVL_t level)
{
	pPort->INTCTRL=(pPort->INTCTRL&(~PORT_INT0LVL_gm))|level;
}

/**
This function returns the interrupt level of INT0 of the designated port.
Parameters:
	pPort:	address of a port's control structure, defined in iox128a1u.
return value:
	An value of type PORT_INT0LVL_t.
*/
static inline PORT_INT0LVL_t ecd300IoPortGetInt0Level(volatile PORT_t * pPort)
{
	return pPort->INTCTRL&PORT_INT0LVL_gm;
}

/**
This function sets the interrupt level of INT1 of the designated port.
Parameters:
	pPort:	address of a port's control structure, defined in iox128a1u.
	level:	interrupt level of INT1.
*/
static inline void ecd300IoPortSetInt1Level(volatile PORT_t * pPort, PORT_INT1LVL_t level)
{
	pPort->INTCTRL=(pPort->INTCTRL&(~PORT_INT1LVL_gm))|level;
}

/**
This function returns the interrupt level of INT1 of the designated port.
Parameters:
	pPort:	address of a port's control structure, defined in iox128a1u.
return value:
	An value of type PORT_INT1LVL_t.
*/
static inline PORT_INT1LVL_t ecd300IoPortGetInt1Level(volatile PORT_t * pPort)
{
	return pPort->INTCTRL&PORT_INT1LVL_gm;
}


/**
This function configs the properties of pin/pins.
Parameters:
	pPort:		address of a port's control structure, defined in iox128a1u.
	pinMask:		a set of ECD300_PORT_PINX, such as ECD300_PORT_PIN0|ECD300_PORT_PIN3
	enableSlewRateLimit:
	enableInvertedIO:
	outputPullupConfiguration:	output type and pull-up/pull-down.
	inputSenseConfiguration:	edge type.
*/
void ecd300IoPortConfigPin(volatile PORT_t * pPort, unsigned char pinMask,  
								bool enableSlewRateLimit,
								bool enableInvertedIO,
								PORT_OPC_t outputPullupConfiguration, 
								PORT_ISC_t inputSenseConfiguration);

/**
This function selects pins which can trigger INT0.
Parameters:
	pPort:		address of a port's control structure, defined in iox128a1u.
	pinMask:		a set of ECD300_PORT_PINX, such as ECD300_PORT_PIN0|ECD300_PORT_PIN3
*/
static inline void ecd300IoPortInt0SelectPin(volatile PORT_t * pPort, unsigned char pinMask)
{
	pPort->INT0MASK=pinMask;
}

/**
This function returns the pin combination which can trigger INT0.
Parameters:
	pPort:		address of a port's control structure, defined in iox128a1u.
return value:
	pin combination, for example, ECD300_PORT_PIN0|ECD300_PORT_PIN3
*/
static inline unsigned char ecd300IoPortGetInt0SelectedPin(volatile PORT_t * pPort)
{
	return pPort->INT0MASK;
}

/**
This function selects pins which can trigger INT1.
Parameters:
	pPort:		address of a port's control structure, defined in iox128a1u.
	pinMask:		a set of ECD300_PORT_PINX, such as ECD300_PORT_PIN0|ECD300_PORT_PIN3
*/
static inline void ecd300IoPortInt1SelectPin(volatile PORT_t * pPort, unsigned char pinMask)
{
	pPort->INT1MASK=pinMask;
}

/**
This function returns the pin combination which can trigger INT1.
Parameters:
	pPort:		address of a port's control structure, defined in iox128a1u.
return value:
	pin combination, for example, ECD300_PORT_PIN0|ECD300_PORT_PIN3
*/
static inline unsigned char ecd300IoPortGetInt1SelectedPin(volatile PORT_t * pPort)
{
	return pPort->INT1MASK;
}


#endif /* ECD300_IOPORT_H_ */
