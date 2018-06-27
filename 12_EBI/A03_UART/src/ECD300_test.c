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
	ecd300PutString(ECD300_UART_1, pString);
}

void printHex(unsigned char hex)
{
	ecd300PutHexChar(ECD300_UART_1, hex);
}

static bool _pollHexChar(unsigned char * p)
{
	char rc;

	if(NULL==p)
	{
		return false;
	}
	
	rc=ecd300PollChar(ECD300_UART_1, p);

	if(rc==1)
		return true;
	else
		return false;
}


/**
 * \brief EBI chip select configuration
 *
 * This struct holds the configuration for the chip select used to set up the
 * SRAM. The example code will use the EBI helper function to setup the
 * contents before writing the configuration using ebi_cs_write_config().
 */
static struct ebi_cs_config     cs_config;

/**
 * \brief Test the EBI data bus wired to the SRAM
 *
 * This function will perform a walking 1s to locate any shorts or open leads
 * to the SRAM device.
 *
 * \param base Base address of the external memory device
 *
 * \retval STATUS_OK on success, and \ref status_code_t error code on failure
 */
static status_code_t ebi_test_data_bus(hugemem_ptr_t base)
{
	hugemem_ptr_t   p;
	uint_fast8_t    i;

	/* Write walking 1s */
	for (p = base, i = 0; i < 32; i++) {
		hugemem_write32(p, 1UL << i);
		p = (hugemem_ptr_t)((uint32_t)p + sizeof(uint32_t));
	}

	/* Read walking 1s, write walking 0s */
	for (p = base, i = 0; i < 32; i++) {
		uint32_t        expected = 1UL << i;
		uint32_t        actual;

		actual = hugemem_read32(p);
		if (actual != expected) {
			return ERR_IO_ERROR;
		}

		hugemem_write32(p, ~expected);
		p = (hugemem_ptr_t)((uint32_t)p + sizeof(uint32_t));
	}

	/* Read walking 0s */
	for (p = base, i = 0; i < 32; i++) {
		uint32_t        actual;
		uint32_t        expected = ~(1UL << i);

		actual = hugemem_read32(p);
		if (actual != expected) {
			return ERR_IO_ERROR;
		}

		p = (hugemem_ptr_t)((uint32_t)p + sizeof(uint32_t));
	}

	return STATUS_OK;
}

/**
 * \brief Test the EBI address bus wired to the SRAM
 *
 * This function will perform an address bus test to locate any shorts or open
 * leads to the SRAM device.
 *
 * \param base Base address of the external memory device
 * \param size Size of the external memory device
 *
 * \retval STATUS_OK on success, and \ref status_code_t error code on failure
 */
static status_code_t ebi_test_addr_bus(hugemem_ptr_t base, uint32_t size)
{
	uint32_t        offset;
	uint_fast8_t    i;

	/* Initialize all power-of-two locations with 0x55 */
	hugemem_write8(base, 0x55);
	for (offset = 1; offset < size; offset <<= 1) {
		hugemem_ptr_t   p;

		p = (hugemem_ptr_t)((uint32_t)base + offset);
		hugemem_write8(p, 0x55);
	}

	/* Check for address lines stuck high */
	hugemem_write8(base, 0xaa);
	for (i = 0, offset = 1; offset < size; offset <<= 1, i++) {
		hugemem_ptr_t   p;
		uint8_t         actual;

		p = (hugemem_ptr_t)((uint32_t)base + offset);
		actual = hugemem_read8(p);
		if (actual != 0x55) {
			return ERR_IO_ERROR;
		}
	}

	/* Check for address lines stuck low or shorted */
	hugemem_write8(base, 0x55);
	for (i = 0, offset = 1; offset < size; offset <<= 1, i++) {
		hugemem_ptr_t   p;
		uint32_t        offset2;
		uint_fast8_t    j;
		uint8_t         actual;

		p = (hugemem_ptr_t)((uint32_t)base + offset);
		hugemem_write8(p, 0xaa);

		actual = hugemem_read8(base);
		if (actual != 0x55) {
			return ERR_IO_ERROR;
		}

		for (j = 0, offset2 = 1; offset2 < size; offset2 <<= 1, j++) {
			hugemem_ptr_t   q;

			if (offset2 == offset)
				continue;

			q = (hugemem_ptr_t)((uint32_t)base + offset2);
			actual = hugemem_read8(q);
			if (actual != 0x55) {
				return ERR_IO_ERROR;
			}
		}

		hugemem_write8(p, 0x55);
	}

	return STATUS_OK;
}

/**
 * \brief Perform a SRAM data integrity test
 *
 * This function will perform a SRAM data integrity test by writing 0s and 1s
 * to the entire external device.
 *
 * \param base Base address of the external memory device
 * \param size Size of the external memory device
 *
 * \retval STATUS_OK on success, and \ref status_code_t error code on failure
 */
static status_code_t ebi_test_data_integrity(hugemem_ptr_t base, uint32_t size)
{
	uint32_t        offset;
	uint32_t        pattern;

	/* Fill memory with a known pattern. */
	for (pattern = 1, offset = 0; offset < size; pattern++,
			offset += sizeof(uint32_t)) {
		hugemem_ptr_t   p;

		p = (hugemem_ptr_t)((uint32_t)base + offset);
		hugemem_write32(p, pattern);
	}

	/* Check each location and invert it for the second pass. */
	for (pattern = 1, offset = 0; offset < size; pattern++,
			offset += sizeof(uint32_t)) {
		hugemem_ptr_t   p;
		uint32_t        actual;
		uint32_t        expected;

		p = (hugemem_ptr_t)((uint32_t)base + offset);

		actual = hugemem_read32(p);
		if (actual != pattern) {
			return ERR_IO_ERROR;
		}

		expected = ~pattern;
		hugemem_write32(p, expected);
	}

	/* Check each location for the inverted pattern and zero it. */
	for (pattern = 1, offset = 0; offset < size; pattern++,
			offset += sizeof(uint32_t)) {
		hugemem_ptr_t   p;
		uint32_t        actual;
		uint32_t        expected;

		p = (hugemem_ptr_t)((uint32_t)base + offset);

		expected = ~pattern;
		actual = hugemem_read32(p);
		if (actual != expected) {
			return ERR_IO_ERROR;
		}
	}

	return STATUS_OK;
}

/**
 * \brief Main function.
 */
void ecd300TestEbi(void)
{
	status_code_t   retval;
	usart_rs232_options_t uartOption;
	unsigned long i;
	unsigned short j;
	
	for(i=0;i<0x40000000;i++)
	{
		;
	}

	disableJtagPort();
	sysclk_init();
	irq_initialize_vectors();
	sleepmgr_init();
	cpu_irq_enable();

	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;
	ecd300InitUart(ECD300_UART_1, &uartOption);
	printString("UART1 was initialized 9\r\n");

	/*
	 * Configure the EBI port with 17 address lines, enable both address
	 * latches, no low pin count mode, and set it in SRAM mode with 3-port
	 * EBI port.
	 */
	ebi_setup_port(17, 2, 0, EBI_PORT_SRAM | EBI_PORT_3PORT
			| EBI_PORT_CS0);

	/*
	 * Configure the EBI chip select for an 128 kB SRAM located at
	 * \ref BOARD_EBI_SDRAM_BASE.
	 */
	ebi_cs_set_mode(&cs_config, EBI_CS_MODE_SRAM_gc);
	ebi_cs_set_address_size(&cs_config, EBI_CS_ASPACE_128KB_gc);
	ebi_cs_set_base_address(&cs_config, BOARD_EBI_SRAM_BASE);

	/*
	 * Refer to 36.2 in xmega manual and datasheet of cy62128e:
	 * In reading, the maximum time from OE# to data valid is 25ns,
	 * the maxium time from address valid to data valid is 55ns.
	 * So OE# should keep 55ns at least.
	 * In writing, the minimum delay from CE# to sampling is 40ns,
	 * the minimum delay from WE# to sampling is 40ns,
	 * So WE# should keep 40ns at least.
	 
	 * The EBI is configured to run at maximum
	 * speed, 64 MHz, which gives a minimum wait state of 16 ns per clock
	 * cycle. 3 additional clock cycles as wait state give enough headroom. (64ns (4 clks) > 55ns > 40ns)
	 */
	ebi_cs_set_sram_wait_states(&cs_config, EBI_CS_SRWS_3CLK_gc);

	/* Write the chip select configuration into the EBI registers. */
	ebi_cs_write_config(0, &cs_config);

	ebi_enable_cs(0, &cs_config);

	/* Enable LED0: EBI is configured and enabled. */
	printString("EBI is configured and enabled\r\n");

	retval = ebi_test_data_bus((hugemem_ptr_t)BOARD_EBI_SRAM_BASE);
	if (retval) {
		printString("ebi_test_data_bus failed\r\n");
		goto out;
	}

	/* Enable LED1: data bus test completed successfully. */
	printString("data bus test completed successfully\r\n");

	retval = ebi_test_addr_bus((hugemem_ptr_t)BOARD_EBI_SRAM_BASE,
			BOARD_EBI_SRAM_SIZE);
	if (retval) {
		printString("ebi_test_addr_bus failed\r\n");
		goto out;
	}

	/* Enable LED2: address bus test completed successfully. */
	printString("address bus test completed successfully\r\n");

	retval = ebi_test_data_integrity((hugemem_ptr_t)BOARD_EBI_SRAM_BASE,
			BOARD_EBI_SRAM_SIZE);
	if (retval) {
		printString("ebi_test_data_integrity failed\r\n");
		goto out;
	}

	//SRAM scan.
	for(i=0;i<0x20000;i+=2)
	{
		hugemem_write16(BOARD_EBI_SRAM_BASE+i, i>>1);
	}
	for(i=0;i<0x20000;i+=2)
	{
		j=hugemem_read16(BOARD_EBI_SRAM_BASE+i);
		if(j!=(i>>1))
			break;
	}
	if(i!=0x20000)
	{
		printString("SRAM scan failed at: ");
		printHex(i>>8);
		printString(", ");
		printHex(i);
		printString("\r\n");
		goto out;
	}
	else
	{
		printString("SRAM scan succeed\r\n");
	}
	/* Enable LED3: data integrity test completed successfully. */
	printString("data integrity test completed successfully\r\n");

out:
	/* Enable LED7: example application has completed. */
	printString("example application has completed\r\n");

	for (;;) {
		sleepmgr_enter_sleep();
	}
}


