
/**
 * \file
 *
 * \brief XMEGA PDI NVM command driver
 *
 * Copyright (C) 2009 Atmel Corporation. All rights reserved.
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 * Atmel AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */
 
#include "ECD300_pdi.h"
#include "ECD300_pdi_physic.h"
#include "ECD300_delay.h"

extern void printString(char * pString);
extern void printHex(unsigned char hex);



#define XNVM_PDI_LDS_INSTR    0x00 //!< LDS instruction.
#define XNVM_PDI_STS_INSTR    0x40 //!< STS instruction.
#define XNVM_PDI_LD_INSTR     0x20 //!< LD instruction.
#define XNVM_PDI_ST_INSTR     0x60 //!< ST instruction.
#define XNVM_PDI_LDCS_INSTR   0x80 //!< LDCS instruction.
#define XNVM_PDI_STCS_INSTR   0xC0 //!< STCS instruction.
#define XNVM_PDI_REPEAT_INSTR 0xA0 //!< REPEAT instruction.
#define XNVM_PDI_KEY_INSTR    0xE0 //!< KEY instruction.

/** Byte size address mask for LDS and STS instruction */
#define XNVM_PDI_BYTE_ADDRESS_MASK 0x00
/** Word size address mask for LDS and STS instruction */
#define XNVM_PDI_WORD_ADDRESS_MASK 0x04
/** 3 bytes size address mask for LDS and STS instruction */
#define XNVM_PDI_3BYTES_ADDRESS_MASK 0x08
/** Long size address mask for LDS and STS instruction */
#define XNVM_PDI_LONG_ADDRESS_MASK 0x0C
/** Byte size data mask for LDS and STS instruction */
#define XNVM_PDI_BYTE_DATA_MASK 0x00
/** Word size data mask for LDS and STS instruction */
#define XNVM_PDI_WORD_DATA_MASK 0x01
/** 3 bytes size data mask for LDS and STS instruction */
#define XNVM_PDI_3BYTES_DATA_MASK 0x02
/** Long size data mask for LDS and STS instruction */
#define XNVM_PDI_LONG_DATA_MASK 0x03
/** Byte size address mask for LDS and STS instruction */
#define XNVM_PDI_LD_PTR_STAR_MASK 0x00
/** Word size address mask for LDS and STS instruction */
#define XNVM_PDI_LD_PTR_STAR_INC_MASK 0x04
/** 3 bytes size address mask for LDS and STS instruction */
#define XNVM_PDI_LD_PTR_ADDRESS_MASK 0x08

#define XNVM_CMD_NOP                         0x00 //!< No Operation.
#define XNVM_CMD_CHIP_ERASE                  0x40 //!< Chip Erase.
#define XNVM_CMD_READ_NVM_PDI                0x43 //!< Read NVM PDI.
#define XNVM_CMD_LOAD_FLASH_PAGE_BUFFER      0x23 //!< Load Flash Page Buffer.
#define XNVM_CMD_ERASE_FLASH_PAGE_BUFFER     0x26 //!< Erase Flash Page Buffer.
#define XNVM_CMD_ERASE_FLASH_PAGE            0x2B //!< Erase Flash Page.
#define XNVM_CMD_WRITE_FLASH_PAGE            0x2E //!< Flash Page Write.
#define XNVM_CMD_ERASE_AND_WRITE_FLASH_PAGE  0x2F //!< Erase & Write Flash Page.
#define XNVM_CMD_CALC_CRC_ON_FLASH           0x78 //!< Flash CRC.

#define XNVM_CMD_ERASE_APP_SECTION           0x20 //!< Erase Application Section.
#define XNVM_CMD_ERASE_APP_PAGE              0x22 //!< Erase Application Section.
#define XNVM_CMD_WRITE_APP_SECTION           0x24 //!< Write Application Section.
#define XNVM_CMD_ERASE_AND_WRITE_APP_SECTION 0x25 //!< Erase & Write Application Section Page.
#define XNVM_CMD_CALC_CRC_APP_SECTION        0x38 //!< Application Section CRC.

#define XNVM_CMD_ERASE_BOOT_SECTION          0x68 //!< Erase Boot Section.
#define XNVM_CMD_ERASE_BOOT_PAGE             0x2A //!< Erase Boot Loader Section Page.
#define XNVM_CMD_WRITE_BOOT_PAGE             0x2C //!< Write Boot Loader Section Page.
#define XNVM_CMD_ERASE_AND_WRITE_BOOT_PAGE   0x2D //!< Erase & Write Boot Loader Section Page.
#define XNVM_CMD_CALC_CRC_BOOT_SECTION       0x39 //!< Boot Loader Section CRC.

#define XNVM_CMD_READ_USER_SIGN              0x03 //!< Read User Signature Row.
#define XNVM_CMD_ERASE_USER_SIGN             0x18 //!< Erase User Signature Row.
#define XNVM_CMD_WRITE_USER_SIGN             0x1A //!< Write User Signature Row.
#define XNVM_CMD_READ_CALIB_ROW              0x02 //!< Read Calibration Row.

#define XNVM_CMD_READ_FUSE                   0x07 //!< Read Fuse.
#define XNVM_CMD_WRITE_FUSE                  0x4C //!< Write Fuse.
#define XNVM_CMD_WRITE_LOCK_BITS             0x08 //!< Write Lock Bits.

#define XNVM_CMD_LOAD_EEPROM_PAGE_BUFFER     0x33 //!< Load EEPROM Page Buffer.
#define XNVM_CMD_ERASE_EEPROM_PAGE_BUFFER    0x36 //!< Erase EEPROM Page Buffer.

#define XNVM_CMD_ERASE_EEPROM                0x30 //!< Erase EEPROM.
#define XNVM_CMD_ERASE_EEPROM_PAGE           0x32 //!< Erase EEPROM Page.
#define XNVM_CMD_WRITE_EEPROM_PAGE           0x34 //!< Write EEPROM Page.
#define XNVM_CMD_ERASE_AND_WRITE_EEPROM      0x35 //!< Erase & Write EEPROM Page.
#define XNVM_CMD_READ_EEPROM                 0x06 //!< Read EEPROM.

/**
 * \brief Key used to enable the NVM interface.
 */
#define NVM_KEY_BYTE0 0xFF
#define NVM_KEY_BYTE1 0x88
#define NVM_KEY_BYTE2 0xD8
#define NVM_KEY_BYTE3 0xCD
#define NVM_KEY_BYTE4 0x45
#define NVM_KEY_BYTE5 0xAB
#define NVM_KEY_BYTE6 0x89
#define NVM_KEY_BYTE7 0x12


/**
 * \brief Move bytes in memory from one location to another
 *
 * \param from_var The address of the data source
 * \param to_var The address of the data destination
 * \param bytes The number of bytes to move
 */
static inline void mem_move(unsigned char * from_var, unsigned char * to_var, unsigned char bytes)
{
	do{
		*to_var++ = *from_var++;
		bytes--;
	}while(bytes);
}


unsigned char cmd_buffer[20];
enum status_code retval;
unsigned char initialized = 0;

/* Private prototypes */
static enum status_code xnvm_read_pdi_status(unsigned char *status);
static enum status_code xnvm_wait_for_nvmen(unsigned long retries);
static enum status_code xnvm_ctrl_read_reg(unsigned short reg, unsigned char *value);
static enum status_code xnvm_ctrl_write_reg(unsigned short reg, unsigned char value);
static enum status_code xnvm_st_ptr(unsigned long address);
static enum status_code xnvm_ctrl_cmd_write(unsigned char cmd_id);
static enum status_code xnvm_ctrl_read_status(unsigned char *value);
static enum status_code xnvm_erase_application_flash_page(unsigned long address, unsigned char *dat_buf, unsigned short length);
static enum status_code xnvm_erase_flash_buffer(unsigned long retries);
static enum status_code xnvm_load_flash_page_buffer(unsigned long addr, unsigned char *buf, unsigned short len);
static enum status_code xnvm_st_ptr(unsigned long address);
static enum status_code xnvm_st_star_ptr_postinc(unsigned char value);
static enum status_code xnvm_write_repeat(unsigned long count);
static enum status_code xnvm_ctrl_wait_nvmbusy(unsigned long retries);
static enum status_code xnvm_ctrl_cmdex_write(void);
static enum status_code xnvm_erase_eeprom_buffer(unsigned long retries);
static enum status_code xnvm_load_eeprom_page_buffer(unsigned long addr, unsigned char *buf, unsigned short len);
/*********************/

/**
 * \brief Initiliazation function for the PDI interface
 *
 * This function initializes the PDI interface agains
 * the connected target device.
 *
 * \retval STATUS_OK init ok
 * \retval ERR_TIMEOUT the init timed out
 */
enum status_code xnvm_init (void)
{
	if(initialized == 0){

		pdi_init();
		/* Put the device in reset mode */
		xnvm_put_dev_in_reset();

		/* Create the key command */
		cmd_buffer[0] = XNVM_PDI_KEY_INSTR;
		cmd_buffer[1] = NVM_KEY_BYTE0;
		cmd_buffer[2] = NVM_KEY_BYTE1;
		cmd_buffer[3] = NVM_KEY_BYTE2;
		cmd_buffer[4] = NVM_KEY_BYTE3;
		cmd_buffer[5] = NVM_KEY_BYTE4;
		cmd_buffer[6] = NVM_KEY_BYTE5;
		cmd_buffer[7] = NVM_KEY_BYTE6;
		cmd_buffer[8] = NVM_KEY_BYTE7;

		pdi_write(cmd_buffer, 9);
		{
			//check the PDI status after KEY command.

			char rc;
			unsigned long i;
			unsigned char status;

			for(i=0;i<WAIT_RETRIES_NUM;i++)
			{
				status=0;
				rc=xnvm_read_pdi_status(&status);

				//check the NVMEN in PDI status.
				if(status&0x02)
				{
					break;
				}
				ecd300DelayMicrosecond(1);
			}
			if(i==WAIT_RETRIES_NUM)
			{
				return ERR_PROTOCOL;
			}
		}

		retval = xnvm_ctrl_wait_nvmbusy(WAIT_RETRIES_NUM);

		initialized = 1;
	}
	return retval;
}

/**
 * \brief Function for putting the device into reset
 *
 * \retval STATUS_OK if all went well
 * \retval ERR_IO_ERROR if the pdi write failed
 */
enum status_code xnvm_put_dev_in_reset (void)
{
	/* Reset the device */
	cmd_buffer[0] = XNVM_PDI_STCS_INSTR | XOCD_RESET_REGISTER_ADDRESS;
	cmd_buffer[1] = XOCD_RESET_SIGNATURE;
	if(pdi_write(cmd_buffer, 2)){
		return ERR_IO_ERROR;
	}
	return STATUS_OK;
}

/**
 * \brief Function for releasing the reset of the device
 *
 * \retval STATUS_OK if all went well
 * \retval ERR_IO_ERROR if the pdi write failed
 */
enum status_code xnvm_pull_dev_out_of_reset (void)
{
	/* Pull device out of reset */
	cmd_buffer[0] = XNVM_PDI_STCS_INSTR | XOCD_RESET_REGISTER_ADDRESS;
	cmd_buffer[1] = 0;
	if(pdi_write(cmd_buffer, 2)){
		return ERR_IO_ERROR;
	}
	return STATUS_OK;
}

/**
 *  \internal
 *  \brief Wait until the NVM module has completed initialization
 *
 *  \param  retries the retry count.
 *  \retval STATUS_OK the NVMEN was set successfully.
 *  \retval ERR_BAD_DATA One of the bytes sent was corrupted during transmission.
 *  \retval ERR_TIMEOUT Time out.
 */
static enum status_code xnvm_wait_for_nvmen(unsigned long retries)
{
	unsigned char pdi_status=0;

	while (retries != 0) {
		if (xnvm_read_pdi_status(&pdi_status) != STATUS_OK) {
				return ERR_BAD_DATA;
		}
		if ((pdi_status & XNVM_NVMEN) != 0) {
				return STATUS_OK;
		}
		--retries;
	}
	return ERR_TIMEOUT;

}

/**
 *  \internal
 *  \brief Read the PDI Controller's STATUS register
 *
 *  \param  status the status buffer pointer.
 *  \retval STATUS_OK read successfully.
 *  \retval ERR_BAD_DATA One of the bytes sent was corrupted during transmission.
 *  \retval ERR_TIMEOUT Time out.
 */
static enum status_code xnvm_read_pdi_status(unsigned char *status)
{
	enum status_code ret = STATUS_OK;

	cmd_buffer[0] = XNVM_PDI_LDCS_INSTR;
	if (STATUS_OK != pdi_write(cmd_buffer, 1)) {
			ret = ERR_BAD_DATA;
	}
	if (pdi_get_byte(status, WAIT_RETRIES_NUM) != STATUS_OK) {
			ret = ERR_TIMEOUT;
	}

	return ret;
}

enum status_code xnvm_read_pdi_reset(unsigned char *pReset)
{
	enum status_code ret = STATUS_OK;
	unsigned char cmd_buffer[2];

	cmd_buffer[0] = XNVM_PDI_LDCS_INSTR|0x1;
	if (STATUS_OK != pdi_write(cmd_buffer, 1)) {
			ret = ERR_BAD_DATA;
	}
	if (pdi_get_byte(pReset, WAIT_RETRIES_NUM) != STATUS_OK) {
			ret = ERR_TIMEOUT;
	}

	return ret;
}

enum status_code xnvm_read_pdi_control(unsigned char *pControl)
{
	enum status_code ret = STATUS_OK;
	unsigned char cmd_buffer[2];

	cmd_buffer[0] = XNVM_PDI_LDCS_INSTR|0x2;
	if (STATUS_OK != pdi_write(cmd_buffer, 1)) {
			ret = ERR_BAD_DATA;
	}
	if (pdi_get_byte(pControl, WAIT_RETRIES_NUM) != STATUS_OK) {
			ret = ERR_TIMEOUT;
	}

	return ret;
}

enum status_code xnvm_set_pdi_control(unsigned char value)
{
	unsigned char cmd_buffer[2];

	cmd_buffer[0] = XNVM_PDI_STCS_INSTR|0x2;
	cmd_buffer[1]=value;

	return pdi_write(cmd_buffer, 2);
}


/**
 *  \brief Read the IO space register with NVM controller
 *
 *  \param  address the register address in the IO space.
 *  \param  value the value buffer pointer.
 *  \retval STATUS_OK read successfully.
 *  \retval ERR_BAD_DATA One of the bytes sent was corrupted during transmission.
 *  \retval ERR_TIMEOUT Time out.
 */
enum status_code xnvm_ioread_byte(unsigned short address, unsigned char *value)
{
	enum status_code ret = STATUS_OK;
	unsigned long register_address;

	cmd_buffer[0] = XNVM_PDI_LDS_INSTR | XNVM_PDI_LONG_ADDRESS_MASK |
			XNVM_PDI_BYTE_DATA_MASK;

	register_address = XNVM_DATA_BASE + address;

	mem_move((unsigned char*)&register_address, (cmd_buffer + 1), 4);

	ret = pdi_write(cmd_buffer, 5);
	ret = pdi_get_byte(value, WAIT_RETRIES_NUM);

	return ret;
}

/**
 *  \brief Write the IO space register with NVM controller
 *
 *  \param  address the register address in the IO space.
 *  \param  value the value which should be write into the address.
 *  \retval STATUS_OK write successfully.
 *  \retval ERR_BAD_DATA One of the bytes sent was corrupted during transmission.
 *  \retval ERR_TIMEOUT Time out.
 */
enum status_code xnvm_iowrite_byte(unsigned short address, unsigned char value)
{

	unsigned long register_address = XNVM_DATA_BASE + address;

	cmd_buffer[0] = XNVM_PDI_STS_INSTR | XNVM_PDI_LONG_ADDRESS_MASK |
			XNVM_PDI_BYTE_DATA_MASK;

	mem_move((unsigned char*)&register_address, (cmd_buffer + 1), 4);
	cmd_buffer[5] = value;

	return (pdi_write(cmd_buffer, 6));
}

/**
 *  \internal
 *  \brief Read the NVM Controller's status register
 *
 *  \param  value the NVM Controller's status buffer pointer.
 *  \retval STATUS_OK read successfully.
 *  \retval ERR_TIMEOUT Time out.
 */
static enum status_code xnvm_ctrl_read_status(unsigned char *value)
{
	return xnvm_ctrl_read_reg(XNVM_CONTROLLER_STATUS_REG_OFFSET, value);
}

/**
 *  \internal
 *  \brief Read the NVM Controller's register
 *
 *  \param  reg the offset of the NVM Controller register.
 *  \param  value the pointer of the value buffer.
 *  \retval STATUS_OK read succussfully.
 *  \retval ERR_TIMEOUT Time out.
 */
static enum status_code xnvm_ctrl_read_reg(unsigned short reg, unsigned char *value)
{
	unsigned short address;

	address = XNVM_CONTROLLER_BASE + reg;
	return xnvm_ioread_byte(address, value);
}

/**
 *  \internal
 *  \brief Write the NVM Controller's register
 *
 *  \param  reg the offset of the NVM Controller register.
 *  \param  value the value which should be write into the register.
 *  \retval STATUS_OK write succussfully.
 *  \retval ERR_BAD_DATA One of the bytes sent was corrupted during transmission.
 *  \retval ERR_TIMEOUT Time out.
 */
static enum status_code xnvm_ctrl_write_reg(unsigned short reg, unsigned char value)
{
	unsigned short address;

	address = XNVM_CONTROLLER_BASE + reg;
	return xnvm_iowrite_byte(address, value);
}

/**
 * \internal
 * \brief Write the NVM CTRLA register CMDEX
 *
 * \retval STATUS_OK write successful.
 * \retval STATUS_OK write successfully.
 * \retval ERR_BAD_DATA One of the bytes sent was corrupted during transmission.
 * \retval ERR_TIMEOUT Time out.
 */
static enum status_code xnvm_ctrl_cmdex_write(void)
{
	return xnvm_ctrl_write_reg(XNVM_CONTROLLER_CTRLA_REG_OFFSET, XNVM_CTRLA_CMDEX);
}

/**
 *  \internal
 *  \brief Write NVM command register
 *
 *  \param  cmd_id the command code which should be write into the NVM command register.
 *  \retval STATUS_OK write successfully.
 *  \retval ERR_BAD_DATA One of the bytes sent was corrupted during transmission.
 *  \retval ERR_TIMEOUT Time out.
 */
static enum status_code xnvm_ctrl_cmd_write(unsigned char cmd_id)
{
	return xnvm_ctrl_write_reg(XNVM_CONTROLLER_CMD_REG_OFFSET, cmd_id);
}

/**
 *  \brief Erase the chip
 *
 *  \retval STATUS_OK erase chip succussfully.
 *  \retval ERR_TIMEOUT Time out.
 */
enum status_code xnvm_chip_erase(void)
{
	status_code_t rc;
	
	/* Write the chip erase command to the NVM command reg */
	rc=xnvm_ctrl_cmd_write(XNVM_CMD_CHIP_ERASE);
	/* Write the CMDEX to execute command */
	if(STATUS_OK==rc)
	{
		xnvm_ctrl_cmdex_write();
	}
	if(STATUS_OK==rc)
	{
		//check whether PDI controller is ready.
		rc=xnvm_wait_for_nvmen(WAIT_RETRIES_NUM);
	}
	if(STATUS_OK==rc)
	{
		//check whether NVM is ready.
		rc=xnvm_ctrl_wait_nvmbusy(WAIT_RETRIES_NUM);
	}

	return rc;
}

/**
 *  \internal
 *  \brief Load the flash page buffer
 *
 *  \param  addr the flash address.
 *  \param  buf the pointer which points to the data buffer.
 *  \param  len the length of data.
 *  \retval STATUS_OK write succussfully.
 *  \retval ERR_BAD_DATA One of the bytes sent was corrupted during transmission.
 *  \retval ERR_INVALID_ARG Invalid argument.
 */
static enum status_code xnvm_load_flash_page_buffer(unsigned long addr, unsigned char *buf, unsigned short len)
{
	if (buf == NULL || len == 0) {
			return ERR_INVALID_ARG;
	}

	xnvm_ctrl_cmd_write(XNVM_CMD_LOAD_FLASH_PAGE_BUFFER);
	xnvm_st_ptr(addr);

	if (len > 1) {
			xnvm_write_repeat(len);
	} else {
			return xnvm_st_star_ptr_postinc(*buf);
	}

	cmd_buffer[0] = XNVM_PDI_ST_INSTR | XNVM_PDI_LD_PTR_STAR_INC_MASK |
			XNVM_PDI_BYTE_DATA_MASK;
	pdi_write(cmd_buffer, 1);

	return pdi_write(buf, len);
}

/**
 *  \internal
 *  \brief Erase the flash buffer with NVM controller.
 *
 *  \param  retries the time out delay number.
 *  \retval STATUS_OK erase successfully.
 *  \retval ERR_TIMEOUT Time out.
 */
static enum status_code xnvm_erase_flash_buffer(unsigned long retries)
{
	xnvm_st_ptr(0);
	xnvm_ctrl_cmd_write(XNVM_CMD_ERASE_FLASH_PAGE_BUFFER);
	xnvm_ctrl_cmdex_write();

	return xnvm_ctrl_wait_nvmbusy(retries);
}

/**
 *  \internal
 *  \brief Erase and program the flash page buffer with NVM controller.
 *
 *  \param  address the address of the flash.
 *  \param  dat_buf the pointer which points to the data buffer.
 *  \param  length the data length.
 *  \retval STATUS_OK program succussfully.
 *  \retval ERR_TIMEOUT Time out.
 */
enum status_code xnvm_erase_program_flash_page(unsigned long address, unsigned char *dat_buf, unsigned short length)
{
	address = address + XNVM_FLASH_BASE;

	xnvm_erase_flash_buffer(WAIT_RETRIES_NUM);
	xnvm_load_flash_page_buffer(address, dat_buf, length);
	xnvm_ctrl_cmd_write(XNVM_CMD_ERASE_AND_WRITE_FLASH_PAGE);

	/* Dummy write for starting the erase and write command */
	xnvm_st_ptr(address);
	xnvm_st_star_ptr_postinc(DUMMY_BYTE);

	return xnvm_ctrl_wait_nvmbusy(WAIT_RETRIES_NUM);
}

/**
 *  \internal
 *  \brief Erase and program the flash page buffer with NVM controller.
 *
 *  \param  address the address of the flash.
 *  \param  dat_buf the pointer which points to the data buffer.
 *  \param  length the data length.
 *  \retval STATUS_OK program succussfully.
 *  \retval ERR_TIMEOUT Time out.
 */
static enum status_code xnvm_erase_application_flash_page(unsigned long address, unsigned char *dat_buf, unsigned short length)
{
	address = address + XNVM_FLASH_BASE;

	xnvm_erase_flash_buffer(WAIT_RETRIES_NUM);
	xnvm_load_flash_page_buffer(address, dat_buf, length);
	xnvm_ctrl_cmd_write(XNVM_CMD_ERASE_AND_WRITE_APP_SECTION);

	/* Dummy write for starting the erase and write command */
	xnvm_st_ptr(address);
	xnvm_st_star_ptr_postinc(DUMMY_BYTE);

	return xnvm_ctrl_wait_nvmbusy(WAIT_RETRIES_NUM);
}

/**
 *  \internal
 *  \brief Write the repeating number with PDI port
 *
 *  \param  count the repeating number.
 *  \retval STATUS_OK write succussfully.
 *  \retval ERR_BAD_DATA One of the bytes sent was corrupted during transmission.
 */
static enum status_code xnvm_write_repeat(unsigned long count)
{
	unsigned char cmd_len;

	--count;

	if (count < (1 << 8)) {
		cmd_buffer[0] = XNVM_PDI_REPEAT_INSTR | XNVM_PDI_BYTE_DATA_MASK;
		cmd_buffer[1] = count;
		cmd_len = 2;
	} else if (count < ((unsigned long)(1) << 16)) {
		cmd_buffer[0] = XNVM_PDI_REPEAT_INSTR | XNVM_PDI_WORD_DATA_MASK;
		mem_move((unsigned char*)&count, (cmd_buffer + 1), 2);
		cmd_len = 3;
	} else if (count < ((unsigned long)(1) << 24)) {
		cmd_buffer[0] = XNVM_PDI_REPEAT_INSTR | XNVM_PDI_3BYTES_DATA_MASK;
		mem_move((unsigned char*)&count, (cmd_buffer + 1), 3);
		cmd_len = 4;
	} else {
		cmd_buffer[0] = XNVM_PDI_REPEAT_INSTR | XNVM_PDI_LONG_DATA_MASK;
		mem_move((unsigned char*)&count, (cmd_buffer + 1), 4);
		cmd_len = 5;
	}

	return pdi_write(cmd_buffer, cmd_len);
}

/**
 *  \internal
 *  \brief Write a value to a address with *(ptr++) instruction through the PDI Controller.
 *
 *  \param  value the value should be write into the *ptr.
 *  \retval STATUS_OK write succussfully.
 *  \retval ERR_BAD_DATA One of the bytes sent was corrupted during transmission.
 */
static enum status_code xnvm_st_star_ptr_postinc(unsigned char value)
{
	cmd_buffer[0] = XNVM_PDI_ST_INSTR | XNVM_PDI_LD_PTR_STAR_INC_MASK |
			XNVM_PDI_BYTE_DATA_MASK;
	cmd_buffer[1] = value;

	return pdi_write(cmd_buffer, 2);
}

/**
 *  \internal
 *  \brief Write a address in PDI Controller's pointer.
 *
 *  \param  address the address which should be written into the ptr.
 *  \retval STATUS_OK write successfully.
 *  \retval ERR_BAD_DATA One of the bytes sent was corrupted during transmission.
 */
static enum status_code xnvm_st_ptr(unsigned long address)
{
	cmd_buffer[0] = XNVM_PDI_ST_INSTR | XNVM_PDI_LD_PTR_ADDRESS_MASK |
			XNVM_PDI_LONG_DATA_MASK;

	mem_move((unsigned char*)&address, (cmd_buffer + 1), 4);

	return pdi_write(cmd_buffer, 5);
}

/**
 *  \brief Read the memory (include flash, eeprom, user signature, fuse bits)with NVM controller.
 *
 *  \param  address the address of the memory.
 *  \param  data the pointer which points to the data buffer.
 *  \param  length the data length.
 *  \retval non-zero the read byte length.
 *  \retval zero read fail.
 */
unsigned short xnvm_read_memory(unsigned long address, unsigned char *data, unsigned short length)
{
	xnvm_ctrl_cmd_write(XNVM_CMD_READ_NVM_PDI);
	xnvm_st_ptr(address);

	if (length > 1) {
			xnvm_write_repeat(length);
	}

	cmd_buffer[0] = XNVM_PDI_LD_INSTR | XNVM_PDI_LD_PTR_STAR_INC_MASK |
			XNVM_PDI_BYTE_DATA_MASK;
	pdi_write(cmd_buffer, 1);

	return pdi_read(data, length, WAIT_RETRIES_NUM);
}

/**
 *  \internal
 *  \brief Erase and program the eeprom page buffer with NVM controller.
 *
 *  \param  address the address of the eeprom.
 *  \param  dat_buf the pointer which points to the data buffer.
 *  \param  length the data length.
 *  \retval STATUS_OK program succussfully.
 *  \retval ERR_TIMEOUT Time out.
 */
enum status_code xnvm_erase_program_eeprom_page(unsigned long address, unsigned char *dat_buf, unsigned short length)
{
	address = address + XNVM_EEPROM_BASE;

	xnvm_erase_eeprom_buffer(WAIT_RETRIES_NUM);
	xnvm_load_eeprom_page_buffer(address, dat_buf, length);
	xnvm_ctrl_cmd_write(XNVM_CMD_ERASE_AND_WRITE_EEPROM);

	/* Dummy write for starting the erase and write command */
	xnvm_st_ptr(address);
	xnvm_st_star_ptr_postinc(DUMMY_BYTE);

	return xnvm_ctrl_wait_nvmbusy(WAIT_RETRIES_NUM);
}

/**
 *  \internal
 *  \brief Erase the eeprom buffer with NVM controller.
 *
 *  \param  retries the time out delay number.
 *  \retval STATUS_OK erase succussfully.
 *  \retval ERR_TIMEOUT Time out.
 */
static enum status_code xnvm_erase_eeprom_buffer(unsigned long retries)
{
	xnvm_st_ptr(0);
	xnvm_ctrl_cmd_write(XNVM_CMD_ERASE_EEPROM_PAGE_BUFFER);

	/* Execute command by setting CMDEX */
	xnvm_ctrl_cmdex_write();

	return xnvm_ctrl_wait_nvmbusy(retries);
}

/**
 *  \internal
 *  \brief Load the eeprom page buffer
 *
 *  \param  addr the eeprom address.
 *  \param  buf the pointer which points to the data buffer.
 *  \param  len the length of data.
 *  \retval STATUS_OK load succussfully.
 *  \retval ERR_BAD_DATA One of the bytes sent was corrupted during transmission.
 *  \retval ERR_INVALID_ARG Invalid argument.
 */
static enum status_code xnvm_load_eeprom_page_buffer(unsigned long addr, unsigned char *buf, unsigned short len)
{
	if (buf == NULL || len == 0) {
			return ERR_INVALID_ARG;
	}

	xnvm_ctrl_cmd_write(XNVM_CMD_LOAD_EEPROM_PAGE_BUFFER);
	xnvm_st_ptr(addr);

	if (len > 1) {
			xnvm_write_repeat(len);
	} else {
			xnvm_st_star_ptr_postinc(*buf);
			return STATUS_OK;
	}

	cmd_buffer[0] = XNVM_PDI_ST_INSTR | XNVM_PDI_LD_PTR_STAR_INC_MASK |
			XNVM_PDI_BYTE_DATA_MASK;
	pdi_write(cmd_buffer, 1);

	return pdi_write(buf, len);
}

/**
 *  \internal
 *  \brief Erase the user signature with NVM controller.
 *
 *  \retval STATUS_OK erase succussfully.
 *  \retval ERR_TIMEOUT time out.
 */
enum status_code xnvm_erase_user_sign(void)
{
	xnvm_ctrl_cmd_write(XNVM_CMD_ERASE_USER_SIGN);

	/* Dummy write for starting the erase command */
	xnvm_st_ptr(XNVM_SIGNATURE_BASE);
	xnvm_st_star_ptr_postinc(DUMMY_BYTE);

	return xnvm_ctrl_wait_nvmbusy(WAIT_RETRIES_NUM);
}

/**
 *  \internal
 *  \brief Erase and program the user signature with NVM controller.
 *
 *  \param  address the address of the user signature.
 *  \param  dat_buf the pointer which points to the data buffer.
 *  \param  length the data length.
 *  \retval STATUS_OK program succussfully.
 *  \retval ERR_TIMEOUT time out.
 */
enum status_code xnvm_erase_program_user_sign(unsigned long address, unsigned char *dat_buf, unsigned short length)
{
	address = address + XNVM_SIGNATURE_BASE;

	xnvm_erase_flash_buffer(WAIT_RETRIES_NUM);
	xnvm_load_flash_page_buffer(address, dat_buf, length);
	xnvm_erase_user_sign();
	xnvm_ctrl_cmd_write(XNVM_CMD_WRITE_USER_SIGN);

	/* Dummy write for starting the write command. */
	xnvm_st_ptr(address);
	xnvm_st_star_ptr_postinc(DUMMY_BYTE);

	return xnvm_ctrl_wait_nvmbusy(WAIT_RETRIES_NUM);
}

/**
 *  \brief Write the fuse bit with NVM controller
 *
 *  \param  address the fuse bit address.
 *  \param  value which should be write into the fuse bit.
 *  \param  retries the time out delay number.
 *  \retval STATUS_OK write succussfully.
 *  \retval ERR_TIMEOUT time out.
 */
enum status_code xnvm_write_fuse_bit(unsigned long address, unsigned char value, unsigned long retries)
{
	unsigned long register_address;

	xnvm_ctrl_cmd_write(XNVM_CMD_WRITE_FUSE);

	cmd_buffer[0] = XNVM_PDI_STS_INSTR | XNVM_PDI_LONG_ADDRESS_MASK |
			XNVM_PDI_BYTE_DATA_MASK;

	register_address = XNVM_FUSE_BASE + address;

	mem_move((unsigned char*)&register_address, (cmd_buffer + 1), 4);
	cmd_buffer[5] = value;

	pdi_write(cmd_buffer, 6);

	return xnvm_ctrl_wait_nvmbusy(retries);
}

enum status_code xnvm_write_lock_bit(unsigned char value, unsigned long retries)
{
	unsigned long register_address;

	xnvm_ctrl_cmd_write(XNVM_CMD_WRITE_LOCK_BITS);

	cmd_buffer[0] = XNVM_PDI_STS_INSTR | XNVM_PDI_LONG_ADDRESS_MASK |
			XNVM_PDI_BYTE_DATA_MASK;

	register_address = XNVM_FUSE_BASE + 0x7;

	mem_move((unsigned char*)&register_address, (cmd_buffer + 1), 4);
	cmd_buffer[5] = value;

	pdi_write(cmd_buffer, 6);

	return xnvm_ctrl_wait_nvmbusy(retries);
}

/**
 *  \internal
 *  \brief Wait until the NVM Controller is ready.
 *
 *  \param  retries the retry count.
 *  \retval STATUS_OK BUSY bit was set.
 *  \retval ERR_TIMEOUT Time out.
 */
static enum status_code xnvm_ctrl_wait_nvmbusy(unsigned long retries)
{
	unsigned char status=0xff;

	while (retries != 0) {
			xnvm_ctrl_read_status(&status);

			/* Check if the NVMBUSY bit is clear in the NVM_STATUS register. */
			if ((status & XNVM_NVM_BUSY) == 0) {
					return STATUS_OK;
			}
			--retries;
	}
	return ERR_TIMEOUT;
}

/**
 * \brief Function for closing the PDI communication to the device.
 *
 * \retval always STATUS_OK;
 */
enum status_code xnvm_deinit(void)
{
	pdi_deinit();
	initialized = 0;
	return STATUS_OK;
}


