A03_UART==>==153====>=====155====>====160====>=====165===>====168====>====186====>==191====>=====207====>====213===>===217==>==222===
              |            |           |            |          |           |         |            |           |         |       |
			  V            |           |            |          |           |         |            |           |         |       |
          01_blink_led	   V           |            |          |           |         |            |           |         |       |
                      02_enum_led      V            |          |           |         |            |           |         |       |
                                   03_uart_0        V          |           |         |            |           |         |       |
								               04_uart_int     V           |         |            |           |         |       |
											               05_sys_clk      V         |            |           |         |       |
														                 06_DFU      V            |           |         |       |
																		        07_BlueTooth      V           |         |       |
																				           08_bulkTransfer    V         |       |
																						                    09_DAC      V       |
																											          10_ADC    V
																													          11_TWI
											   
02 Enumerate LED one by one, then turn on the odd and even LEDs alternatively.

03 Output a character continuously through UART0. If a new character is input, then output this new character.

04 Add ISR for UART0 to UART6, configurations for these UARTs are in ECD300_usart_config.h. If UARTs are not necessary, do not define them so that flash space and SRAM space can be saved.

05 Adjust the frequency of CPU and peripheral devices.

06 DFU with UART0, and runs in application space.

If ASF UART is integrated, the calculation of baudrate depends on the CONFIG_SYSCLK_PSBCDIV defined in conf_clock.h.

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
