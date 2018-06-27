/*
 * ECD300_ioport.c
 *
 * Created: 2015/2/5 0:02:07
 *  Author: user
 */ 

#include "asf.h"

#include "ECD300_ioport_config.h"
#include "ECD300_ioport.h"

#ifdef ECD300_ENABLE_PORTA_PIN_INTERRUPT
static ecd300_ioport_callback_t pPortAInt0Func=NULL;
static ecd300_ioport_callback_t pPortAInt1Func=NULL;

ISR(PORTA_INT0_vect)
{
	//clear the flag.
	PORTA_INTFLAGS=0x01;
	//call function.
	if(pPortAInt0Func)
	{
		pPortAInt0Func();
	}
}

ISR(PORTA_INT1_vect)
{
	//clear the flag.
	PORTA_INTFLAGS=0x02;
	if(pPortAInt1Func)
	{
		pPortAInt1Func();
	}
}
#endif

#ifdef ECD300_ENABLE_PORTB_PIN_INTERRUPT
static ecd300_ioport_callback_t pPortBInt0Func=NULL;
static ecd300_ioport_callback_t pPortBInt1Func=NULL;

ISR(PORTB_INT0_vect)
{
	//clear the flag.
	PORTB_INTFLAGS=0x01;
	//call function.
	if(pPortBInt0Func)
	{
		pPortBInt0Func();
	}
}

ISR(PORTB_INT1_vect)
{
	//clear the flag.
	PORTB_INTFLAGS=0x02;
	if(pPortBInt1Func)
	{
		pPortBInt1Func();
	}
}
#endif

#ifdef ECD300_ENABLE_PORTC_PIN_INTERRUPT
static ecd300_ioport_callback_t pPortCInt0Func=NULL;
static ecd300_ioport_callback_t pPortCInt1Func=NULL;

ISR(PORTC_INT0_vect)
{
	//clear the flag.
	PORTC_INTFLAGS=0x01;
	//call function.
	if(pPortCInt0Func)
	{
		pPortCInt0Func();
	}
}

ISR(PORTC_INT1_vect)
{
	//clear the flag.
	PORTC_INTFLAGS=0x02;
	if(pPortCInt1Func)
	{
		pPortCInt1Func();
	}
}
#endif

#ifdef ECD300_ENABLE_PORTD_PIN_INTERRUPT
static ecd300_ioport_callback_t pPortDInt0Func=NULL;
static ecd300_ioport_callback_t pPortDInt1Func=NULL;

ISR(PORTD_INT0_vect)
{
	//clear the flag.
	PORTD_INTFLAGS=0x01;
	//call function.
	if(pPortDInt0Func)
	{
		pPortDInt0Func();
	}
}

ISR(PORTD_INT1_vect)
{
	//clear the flag.
	PORTD_INTFLAGS=0x02;
	if(pPortDInt1Func)
	{
		pPortDInt1Func();
	}
}
#endif

#ifdef ECD300_ENABLE_PORTE_PIN_INTERRUPT
static ecd300_ioport_callback_t pPortEInt0Func=NULL;
static ecd300_ioport_callback_t pPortEInt1Func=NULL;

ISR(PORTE_INT0_vect)
{
	//clear the flag.
	PORTE_INTFLAGS=0x01;
	//call function.
	if(pPortEInt0Func)
	{
		pPortEInt0Func();
	}
}

ISR(PORTE_INT1_vect)
{
	//clear the flag.
	PORTE_INTFLAGS=0x02;
	if(pPortEInt1Func)
	{
		pPortEInt1Func();
	}
}
#endif

#ifdef ECD300_ENABLE_PORTF_PIN_INTERRUPT
static ecd300_ioport_callback_t pPortFInt0Func=NULL;
static ecd300_ioport_callback_t pPortFInt1Func=NULL;

ISR(PORTF_INT0_vect)
{
	//clear the flag.
	PORTF_INTFLAGS=0x01;
	//call function.
	if(pPortFInt0Func)
	{
		pPortFInt0Func();
	}
}

ISR(PORTF_INT1_vect)
{
	//clear the flag.
	PORTF_INTFLAGS=0x02;
	if(pPortFInt1Func)
	{
		pPortFInt1Func();
	}
}
#endif

#ifdef ECD300_ENABLE_PORTH_PIN_INTERRUPT
static ecd300_ioport_callback_t pPortHInt0Func=NULL;
static ecd300_ioport_callback_t pPortHInt1Func=NULL;

ISR(PORTH_INT0_vect)
{
	//clear the flag.
	PORTH_INTFLAGS=0x01;
	//call function.
	if(pPortHInt0Func)
	{
		pPortHInt0Func();
	}
}

ISR(PORTH_INT1_vect)
{
	//clear the flag.
	PORTH_INTFLAGS=0x02;
	if(pPortHInt1Func)
	{
		pPortHInt1Func();
	}
}
#endif

#ifdef ECD300_ENABLE_PORTJ_PIN_INTERRUPT
static ecd300_ioport_callback_t pPortJInt0Func=NULL;
static ecd300_ioport_callback_t pPortJInt1Func=NULL;

ISR(PORTJ_INT0_vect)
{
	//clear the flag.
	PORTJ_INTFLAGS=0x01;
	//call function.
	if(pPortJInt0Func)
	{
		pPortJInt0Func();
	}
}

ISR(PORTJ_INT1_vect)
{
	//clear the flag.
	PORTJ_INTFLAGS=0x02;
	if(pPortJInt1Func)
	{
		pPortJInt1Func();
	}
}
#endif

#ifdef ECD300_ENABLE_PORTK_PIN_INTERRUPT
static ecd300_ioport_callback_t pPortKInt0Func=NULL;
static ecd300_ioport_callback_t pPortKInt1Func=NULL;

ISR(PORTK_INT0_vect)
{
	//clear the flag.
	PORTK_INTFLAGS=0x01;
	//call function.
	if(pPortKInt0Func)
	{
		pPortKInt0Func();
	}
}

ISR(PORTK_INT1_vect)
{
	//clear the flag.
	PORTK_INTFLAGS=0x02;
	if(pPortKInt1Func)
	{
		pPortKInt1Func();
	}
}
#endif


/**
This function provides ISR of INT0 a callback which will be called when ISR of INT0 is triggered.
Parameters:
	pPort:	address of a port's control structure, defined in iox128a1u.
	callback:	address of callback function.

return value:
	0: 	success
	-1:	the PIN ISR for this port is not enabled. It should be enabled in ECD300_ioport_config.h
*/
char ecd300IoPortRegisterInt0(volatile PORT_t * pPort, ecd300_ioport_callback_t pCallback)
{
	char rc=-1;

#ifdef ECD300_ENABLE_PORTA_PIN_INTERRUPT
	if((pPort)==(&PORTA))
	{
		rc=0;
		pPortAInt0Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTB_PIN_INTERRUPT
	if((pPort)==(&PORTB))
	{
		rc=0;
		pPortBInt0Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTC_PIN_INTERRUPT
	if((pPort)==(&PORTC))
	{
		rc=0;
		pPortCInt0Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTD_PIN_INTERRUPT
	if((pPort)==(&PORTD))
	{
		rc=0;
		pPortDInt0Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTE_PIN_INTERRUPT
	if((pPort)==(&PORTE))
	{
		rc=0;
		pPortEInt0Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTF_PIN_INTERRUPT
	if((pPort)==(&PORTF))
	{
		rc=0;
		pPortFInt0Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTH_PIN_INTERRUPT
	if((pPort)==(&PORTH))
	{
		rc=0;
		pPortHInt0Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTJ_PIN_INTERRUPT
	if((pPort)==(&PORTJ))
	{
		rc=0;
		pPortJInt0Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTK_PIN_INTERRUPT
	if((pPort)==(&PORTK))
	{
		rc=0;
		pPortKInt0Func=pCallback;
	}
#endif

	return rc;
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
char ecd300IoPortRegisterInt1(volatile PORT_t * pPort, ecd300_ioport_callback_t pCallback)
{
	char rc=-1;

#ifdef ECD300_ENABLE_PORTA_PIN_INTERRUPT
	if((pPort)==(&PORTA))
	{
		rc=0;
		pPortAInt1Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTB_PIN_INTERRUPT
	if((pPort)==(&PORTB))
	{
		rc=0;
		pPortBInt1Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTC_PIN_INTERRUPT
	if((pPort)==(&PORTC))
	{
		rc=0;
		pPortCInt1Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTD_PIN_INTERRUPT
	if((pPort)==(&PORTD))
	{
		rc=0;
		pPortDInt1Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTE_PIN_INTERRUPT
	if((pPort)==(&PORTE))
	{
		rc=0;
		pPortEInt1Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTF_PIN_INTERRUPT
	if((pPort)==(&PORTF))
	{
		rc=0;
		pPortFInt1Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTH_PIN_INTERRUPT
	if((pPort)==(&PORTH))
	{
		rc=0;
		pPortHInt1Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTJ_PIN_INTERRUPT
	if((pPort)==(&PORTJ))
	{
		rc=0;
		pPortJInt1Func=pCallback;
	}
#endif
#ifdef ECD300_ENABLE_PORTK_PIN_INTERRUPT
	if((pPort)==(&PORTK))
	{
		rc=0;
		pPortKInt1Func=pCallback;
	}
#endif

	return rc;
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
								PORT_ISC_t inputSenseConfiguration)
{
	unsigned char pinProperty=0;

	if(enableSlewRateLimit)
	{
		pinProperty|=PORT_SRLEN_bm;
	}
	if(enableInvertedIO)
	{
		pinProperty|=PORT_INVEN_bm;
	}
	pinProperty|=outputPullupConfiguration|inputSenseConfiguration;

	if(pinMask&ECD300_PORT_PIN0)
	{
		pPort->PIN0CTRL=pinProperty;
	}
	if(pinMask&ECD300_PORT_PIN1)
	{
		pPort->PIN1CTRL=pinProperty;
	}
	if(pinMask&ECD300_PORT_PIN2)
	{
		pPort->PIN2CTRL=pinProperty;
	}
	if(pinMask&ECD300_PORT_PIN3)
	{
		pPort->PIN3CTRL=pinProperty;
	}
	if(pinMask&ECD300_PORT_PIN4)
	{
		pPort->PIN4CTRL=pinProperty;
	}
	if(pinMask&ECD300_PORT_PIN5)
	{
		pPort->PIN5CTRL=pinProperty;
	}
	if(pinMask&ECD300_PORT_PIN6)
	{
		pPort->PIN6CTRL=pinProperty;
	}
	if(pinMask&ECD300_PORT_PIN7)
	{
		pPort->PIN7CTRL=pinProperty;
	}
}


