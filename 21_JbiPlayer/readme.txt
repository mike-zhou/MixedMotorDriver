A03_UART==>==153====>=====155====>====160====>=====165===>====168====>====186====>==191====>=====207====>====213===>===217==>==222=============================================>=========================================================================================350
              |            |           |            |          |           |         |            |           |         |       |                                                                                                                                         |
			  V            |           |            |          |           |         |            |           |         |       +--Temp/PDI==>==226                                                                                                                       |
          01_blink_led	   V           |            |          |           |         |            |           |         |       |                |                                                                                                                        |
                      02_enum_led      V            |          |           |         |            |           |         |       |                V                                                                                                                        |
                                   03_uart_0        V          |           |         |            |           |         |       |             Temp/NAND==>==233============>====244                                                                                       |
								               04_uart_int     V           |         |            |           |         |       |                            |                   |                                                                                        |
											               05_sys_clk      V         |            |           |         |       |                            V                   +--Temp/InfraRed==>==261                                                                 |
														                 06_DFU      V            |           |         |       |                         Temp/EBI==>==237       |                     |                                                                  |
																		        07_BlueTooth      V           |         |       |                                       |        |                     +--Temp/jbi_22 ==>==289                                            |
																				           08_bulkTransfer    V         |       |                                       |        |                     |                    |                                             |
																						                    09_DAC      V       |                                       |        |                     |                    V                                             |
																											          10_ADC    V                                       |        |                     |              Temp/jbi_cdc==>==338==>==348                        |
																													          11_TWI                                    V        |                     |                                |       |                         |
											                                                                                                                          12_EBI     V                     |                                |       V                         |
																																									          13_NAND                  V                                V   Temp/jbi_player==>==413       |
																																											                     14_InfraRed                       16_JbiCdc                     |        V
																																																                                        |                        |    18_Max485
																																																										V                        |
																																																									  19_PDI                     V
																																																									                       21_JbiPlayer
02 Enumerate LED one by one, then turn on the odd and even LEDs alternatively.

03 Output a character continuously through UART0. If a new character is input, then output this new character.

04 Add ISR for UART0 to UART6, configurations for these UARTs are in ECD300_usart_config.h. If UARTs are not necessary, do not define them so that flash space and SRAM space can be saved.

05 Adjust the frequency of CPU and peripheral devices.

06 DFU with UART0, and runs in application space.
After reset, bootloader at address 0x20000 runs. It will check the level of DC3. If it is high, then the program jumps to address 0 to execute the application; if it is low, then bootloader goes on.
Function ecd300StartApp is implemented in ccp.s.
The device should be set to start with BootLoader.
The application to download should be linked with command "-Wl,--relax -Wl,--section-start=.text=0x00000". The default linking command created by Atmel studio will overwrite this boot loader. A mistake was that 'test' other than 'text' was input as the segment name.
If ASF UART is integrated, the calculation of baudrate depends on the CONFIG_SYSCLK_PSBCDIV defined in conf_clock.h.
Latest update: Connect PIN2 of PORTD to high voltage, the application will be launched; to low level, the DFU continues. If left un-connected, this pin might not be low level sometimes. In this case, the application will not be launched.
Sometimes, the serial cable connected to UART0 affects the DFU bootloader. This cable needs to be removed to restore the normal function if this happens. In this case, the power is provided weakly through the serial cable and the device is not powered down actually.

 ISR:BusEvent
      |
      |   SOF
      +--------> udi.sof_notify -------> UDC_SOF_EVENT() -----------------------------------
	  |                                                                                     |
	  | Error                                                                               |
	  +--------> udd.under_flow -------> udd.over_flow -------------------------------------+
	  |                                                                                     |
	  | Reset                                                                               |
	  +--------> udc_reset      -------> udd.set_device_address ---> udd_ep_init -----------+
	  |           |             |                                                           |
	  |           + udi.disable +                                                           |
	  |                                                                                     |
	  | Suspend                                                                             |
	  +--------> UDC_SUSPEND_EVENT() -------------------------------------------------------+
	  |                                                                                     |
	  | Resume                                                                              |
	  +--------> UDC_RESUME_EVENT() --------------------------------------------------------+
                                                                                            |
																						  Return
																						 
 ISR:TCEvent
      |
	  | !TransferComplete    isSetupEvent       !UDD_EPCTRL_SETUP
	  +-------------------+------------------+----------------------> udd_g_ctrlreq.callback() ----+
	  |                   |                  |                                                     |    8 bytes packet                                                  In                          
	  |                   |                  + ----------------------------------------------------+---+---------------> udd_g_ctrlreq.req ----> udc_process_setup() --+-----> udd_ctrl_in_sent ----|
	  |                   |                                                                            |                                                               | Out                        |
  	  |                   |                                                                            |                                                               +----->                  ----+----|
  	  |                   |                                                                            +-------------------------------------------------------------------------------------------------+--+          	  
	  |                   +----Assert(false) ---|                                                                                                                                                           |
	  |                                         |                                                                                                                                                           |
	  |<----------------------------------------|                                                                                                                                                           |
      |                                                                                                                                                                                                     |
	  | EP0Out      UDD_EPCTRL_HANDSHAKE_WAIT_OUT_ZLP                                                                                                                                                       |
	  +----------+------------------------------------> udd_g_ctrlreq.callback() -----------------+                                                                                                         |
	  |          |                                                                                |                                                                                                         |
      | 		 +------> udd_g_ctrlreq.payload -------------> udd_g_ctrlreq.over_under_run() ----+---------------+-----------------------------------------------------------------------------------------+
      | EP0In                                                                                                     |
	  +---------- udd_ctrl_in_sent -------------------------------------------------------------------------------+
	  |                                                                                                           |
	  +---------- udd_ep_trans_complet                                                                            |
	                   | USB_EP_DIR_IN                                                                            |
					   +----------------> ptr_job->nb_trans ------+----> ptr_job->call_trans() -------------------+
					   | USB_EP_DIR_OUT                           |                                               |
					   +----------------> ptr_job->buf -----------+                                               |
					                                                                                            Return
					   
07_BlueTooth
    ECD300 can exchange data through blue tooth devices with android mobile phone.	  
	Integrate modifications in 07_BluetoothChat at #198.  
	  

08_bulkTransfer
	ECD300 can exchange data with PC through bulk endpoints.
	Due to the structure structure of endpoint control in the driver, the addresses of end points have to be continuous, starting from 1.
	USB driver of ECD100/200 can be used with ECD300, but it needs to be installed again. Perhaps because ECD300 is a full speed device.
	In windows, only one configuration of USB device is supported. So the configuration 0 is automatically set when the usb device is inserted into the slot.
	
09_DAC
	There are two DAC in xmega128, and each has two channels which have independent outputs.
	This demo shows methods to operate the DAC. 
	LED DB2 gets bright gradually for 8 times, then DB3 goes with the opposite way as DB2 for 8 times. Last, DA2 and DA3 run the same as DB2 and DB3.

10_ADC
	DAC outputs a designated voltage to its output pins, and ADC samples this voltage and prints it through UART0.

11_TWI
	Two EEPROM chips, E1 and E2, can be written and read successfully. 
	According to the datasheet of EEPROM, 5ms at most is needed to save the data to internal memory after each writing. 
	So the operation right after a writing could fail because of this limit.
	All of PORTC, PORTD, PORTE, PORTF have a IIC master, but the driver can only access one of them at a time. The driver needs to be improved to enable parallel operations.

12_EBI
	SRAM Cy62128E controlled by CS0 can be accessed successfully.
	This application is linked at 0x0000 and can be downloaded by DFU.

13_NAND
	The Nand flash chip in ECD301 can be read, programmed, erased successfully with the nand flash driver implemented in ECD300_nand_config.h, ECD300_nand.h and ECD300_nand.c.
	File ECD300_nand_config.h provides configuration to select the correct chip driver.
	File ECD300_nand.h provides the common driver APIs.
	File ECd300_nand.c provides the implementation of driver APIs.
	This application is linked at 0x0000 and can be downloaded by DFU.

14_InfraRed
	TSMP58000 can detect carrier at frequency ranging from 20KHz to 60KHz. If the input is a constant level, it cannot be detected by the TSMP58000.
	Most of time, the remote controller of STB sends 3 repeated key signals. Sometimes it sends two. The interval between signals is about 64ms.
	The possible interrupt flag needs to be cleared before the corresponding interrupt is enabled. Otherwise an interrupt will be triggered when it is enabled if the flag is set.
	My own IOPORT driver is implemented in ecd300_ioport.*
	
Temp/jbi_cdc
	If the transmit buffer of UART1 is decreased to a level which causes the pause of trace output of USB event, the ecd300 will not be recognized as a CDC device by the host because it cannot responds Host's request in time because of the delay in event processing.
	
16_JbiCdc
	The application works as a virtual COM seen from the host. jtag_io commands are transferred through RS232 basing on USB.
	This is an example of virtual COM, and JBC player basing on the virtual COM.
	
17_ECD300_JBI
	Application of VC++ 6.0
	Execute JBC player in a thread and route JTAG command to ECD300 through virtual COM. 
	The designated name of virtual COM is COM9, and this name is fixed in the application. In other words, ECD300 should be enumerated as COM9 so that this application can communicate successfully with ECD300.
	Two buffers are created to simulate the internal SRAM and external SRAM in ECD300. JBI player can work in the comuter, but cannot work in ECD300.
	
18_Max485
	This application access the MAX485 chips in the ECD301, which transmit data through the twisted cable.
	ECD300_max485_config.h, ECD300_max485.h and ECD300_max485.c are added as a driver to ease the operation. In the driver, MAX485 chips exchange data externally through twisted cable; internally, however, each chip exchanges data with a UART. MAX1 communicates with UART1 and MAX2 UART2.
	PIN5 of PORTC controls the direction of data of MAX1, and PIN1 of PORTD controls that of MAX2.
	
19_PDI
	PDI_DATA is internally pull-down, which affects the detection of starting bit when the output of USART is tri-state. If there is two continuous pdi_write, the second writing will be affected by the low level of PID_DATA.
	USART doesn't output clock signal if both transmit and reception are disabled.
	Two continuous BREAK characters should be separated by one or more high bits.
	Command REPEAT could be cancelled if it is followed by BREAK character.
	The first parameter of xnvm_erase_program_flash_page is an offset, but the 1st parameter of xnvm_read_memory is a PDI address.
	Fuse bit can be changed from 0 to 1, and needs a reset to make it read correctly if it is changed.
	ECD300 receives HEX lines through USB CDC, and programs the flash, and resets the PDI device at the end of HEX file. 20_PDI_APP reads a HEX file and sends the content to ECD300 line by line.
	
20_PDI_APP
	Open a HEX file and sends the content line by line to PDI programmer through the virtual serial port provided by the programmer.

21_JbiPlayer
	JbiPlayer can program the EPM1270 successfully and takes about 60seconds to finish programming and verifying. 
	Dynamic memory in player is allocated in external SRAM.
	The speed is decreased about 3 times if jam content is read from NAND rather than from external SRAM. The larger the buffer, the lower the speed. It seems that jam content is not accessed continuously when the player runs.
	The external SRAM is 128KB. The former 64KB saves JBC file data, and the other 64KB is for the JBI player's dynamic memory allocation.
	If PIN3 of PORTD is pulled down, the application will received JBC file data through a virtual ported emulated by ECD300, and save the data to the second block of NAND flash with the first page saving  the length and later pages saving the content.
	If PIN3 of PORTD is high level, the application will transfer the JBC file content from NAND flash to external SRAM and program this content to the CPLD.