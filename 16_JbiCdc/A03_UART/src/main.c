/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */

/**
	Output a string repeatedly through UART0 at 115200, 8N1.a
*/
extern void ecd300TestUart(void);

/**
	Receive a character from UARTC0, and echo it back.
*/
extern void ecd300TestUartEcho(void);

extern void ecd300TestSysClk(void);

extern void ecd300TestDFU(void);

extern void ecd300TestBlueTooth(void);

extern void ecd300TestBulk(void);

extern void ecd300TestDac(void);

extern void ecd300TestAdc(void);

extern void ecd300TestTwi(void);

extern void ecd300TestPdi(void);

extern void ecd300TestNand(void);

extern void ecd300TestInfraRed(void);

extern void ecd300SmartCardSwitch(void);

extern void ecd300CheckShortCircute(void);

extern void ecd300MixedMotorDrivers(void);

extern void ecd300SolenoidDriver(void);

int main (void)
{
	//ecd300TestUart();
	//ecd300TestUartEcho();
	//ecd300TestSysClk();
	//ecd300TestDFU();
	//ecd300TestBlueTooth();
	//ecd300TestBulk();
	//ecd300TestDac();
	//ecd300TestAdc();
	//ecd300TestTwi();
	//ecd300TestPdi();
	//ecd300TestNand();
	//ecd300TestInfraRed();
	//ecd300TestJbi();
	//ecd300CheckShortCircute();
	//ecd300SmartCardSwitch();
	//ecd300MixedMotorDrivers();
	ecd300SolenoidDriver();	
}

