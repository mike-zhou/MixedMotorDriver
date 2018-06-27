A03_UART==>==153====>=====155====>====160====>=====165===>====168====>====186====
              |            |           |            |          |           |
			  V            |           |            |          |           |
          01_blink_led	   V           |            |          |           |
                      02_enum_led      V            |          |           |
                                   03_uart_0        V          |           |
								               04_uart_int     V           |
											               05_sys_clk      V
														                 06_DFU
											   
02 Enumerate LED one by one, then turn on the odd and even LEDs alternatively.

03 Output a character continuously through UART0. If a new character is input, then output this new character.

04 Add ISR for UART0 to UART6, configurations for these UARTs are in ECD300_usart_config.h. If UARTs are not necessary, do not define them so that flash space and SRAM space can be saved.

05 Adjust the frequency of CPU and peripheral devices.

06 DFU with UART0, and runs in application space.
After reset, bootloader at address 0x20000 runs. It will check the level of DC3. If it is high, then the program jumps to address 0 to execute the application; if it is low, then bootloader goes on.
Function ecd300StartApp is implemented in ccp.s.
The device should be set to start with BootLoader.
The application to download should be linked with command "-Wl,--relax -Wl,--section-start=.text=0x00000". The default linking command created by Atmel studio will overwrite this boot loader.
If ASF UART is integrated, the calculation of baudrate depends on the CONFIG_SYSCLK_PSBCDIV defined in conf_clock.h.
Connect PIN2 of PORTD to high voltage, the application will be launched; to low level, the DFU continues.
Enable the internal pull-down resistor, and disable the JTAG function in PORTB.
Change the output name to ECD300_DFU.
July 10, 2017: The above description about PIN2 of PORTD is wrong. PORTD.2's pull down resistor is enabled. If this pin is not connected to VCC, then application runs, otherwise DFU runs.

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
					   
	  
	  
	  