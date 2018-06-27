A03_UART==>==153====>=====155====>====160====>=====165===>====168====>====186====>==191====>=====207====>====213===>===217==>==222===
              |            |           |            |          |           |         |            |           |         |       |
			  V            |           |            |          |           |         |            |           |         |       +--Temp/PDI==>==226
          01_blink_led	   V           |            |          |           |         |            |           |         |       |                |
                      02_enum_led      V            |          |           |         |            |           |         |       |                V
                                   03_uart_0        V          |           |         |            |           |         |       |             Temp/NAND==>==233============>====244
								               04_uart_int     V           |         |            |           |         |       |                            |                   |
											               05_sys_clk      V         |            |           |         |       |                            V                   +--Temp/InfraRed==>==261
														                 06_DFU      V            |           |         |       |                         Temp/EBI==>==237       |                     |
																		        07_BlueTooth      V           |         |       |                                       |        |                     |
																				           08_bulkTransfer    V         |       |                                       |        |                     |
																						                    09_DAC      V       |                                       |        |                     |
																											          10_ADC    V                                       |        |                     |
																													          11_TWI                                    V        |                     |
											                                                                                                                          12_EBI     V                     |
																																									          13_NAND                  V
																																											                     14_InfraRed
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
	