#ifndef _ECD300_PDI_H__
#define _ECD300_PDI_H__

#include "asf.h"

//The following is from atxmega128a1_nvm_regs.h
#define XNVM_FLASH_BASE                 0x0800000 //!< Adress where the flash starts.
#define XNVM_EEPROM_BASE                0x08C0000 //!< Address where eeprom starts.
#define XNVM_FUSE_BASE                  0x08F0020 //!< Address where fuses start.
#define XNVM_DATA_BASE                  0x1000000 //!< Address where data region starts.
#define XNVM_APPL_BASE            XNVM_FLASH_BASE //!< Addres where application section starts.
#define XNVM_CALIBRATION_BASE          0x008E0200 //!< Address where calibration row starts.
#define XNVM_SIGNATURE_BASE            0x008E0400 //!< Address where signature bytes start.

#define XNVM_FLASH_PAGE_SIZE			512			//

#define XNVM_CONTROLLER_BASE 0x01C0               //!< NVM Controller register base address.
#define XNVM_CONTROLLER_CMD_REG_OFFSET 0x0A       //!< NVM Controller Command Register offset.
#define XNVM_CONTROLLER_STATUS_REG_OFFSET 0x0F    //!< NVM Controller Status Register offset.
#define XNVM_CONTROLLER_CTRLA_REG_OFFSET 0x0B     //!< NVM Controller Control Register A offset.

#define XNVM_CTRLA_CMDEX (1 << 0)                 //!< CMDEX bit offset.
#define XNVM_NVMEN (1 << 1)                       //!< NVMEN bit offset.
#define XNVM_NVM_BUSY (1 << 7)                    //!< NVMBUSY bit offset.

#define XOCD_STATUS_REGISTER_ADDRESS 0x00         //!< PDI status register address.
#define XOCD_RESET_REGISTER_ADDRESS  0x01         //!< PDI reset register address.
#define XOCD_RESET_SIGNATURE         0x59         //!< PDI reset Signature.
#define XOCD_FCMR_ADDRESS 0x05

#define NVM_PAGE_ORDER    9                       //!< NVM Page Order of 2.
#define NVM_PAGE_SIZE   (1 << NVM_PAGE_ORDER)     //!< NVM Page Size.
#define NVM_EEPROM_PAGE_SIZE 32                   //!< EEPROM Page Size.
#define NVM_LOCKBIT_ADDR  7                       //!< Lockbit address.
#define NVM_MCU_CONTROL   0x90                    //!< MCU Control base address.

#define NVM_COMMAND_BUFFER_SIZE 20                //!< NVM Command buffer size.
#define WAIT_RETRIES_NUM 1000                     //!< Retry Number.
#define DUMMY_BYTE 0x55                           //!< Dummy byte for Dummy writing.
//the above is from atxmega128a1_nvm_regs.h

/* Public prototypes */
enum status_code xnvm_init (void);
enum status_code xnvm_ioread_byte(unsigned short address, unsigned char *value);
enum status_code xnvm_iowrite_byte(unsigned short address, unsigned char value);
enum status_code xnvm_chip_erase(void);
unsigned short xnvm_read_memory(unsigned long address, unsigned char *data, unsigned short length);
enum status_code xnvm_erase_program_flash_page(unsigned long address, unsigned char *dat_buf, unsigned short length);
enum status_code xnvm_put_dev_in_reset (void);
enum status_code xnvm_pull_dev_out_of_reset (void);
enum status_code xnvm_erase_program_eeprom_page(unsigned long address, unsigned char *dat_buf, unsigned short length);
enum status_code xnvm_erase_user_sign(void);
enum status_code xnvm_erase_program_user_sign(unsigned long address, unsigned char *dat_buf, unsigned short length);
enum status_code xnvm_write_fuse_bit(unsigned long address, unsigned char value, unsigned long retries);
enum status_code xnvm_write_lock_bit(unsigned char value, unsigned long retries);
enum status_code xnvm_deinit(void);

enum status_code xnvm_read_pdi_reset(unsigned char *pReset);
enum status_code xnvm_read_pdi_control(unsigned char *pControl);
enum status_code xnvm_set_pdi_control(unsigned char value);
#endif
