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
#include "ECD300_test.h"

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
	ecd300TestMax485();
}

