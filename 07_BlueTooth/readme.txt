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
	BlueToothChat was added in this folder. This application was imported to Android Studio, but files created by Android Studio were not added to Perforce. So the BlueToothChat should be imported again if it's souce code are copied to other place.
	The HC-06 devices are BlueTooth v2.0, so only insecure communication is supported. Secure communication is supported in BlueTooth v2.1.
	To connect to HC-06 devices, the InsecureRfcommSocket should be created with UUID 00001101-0000-1000-8000-00805F9B34FB.
	ECD300 can exchange data with mobile phone at 115200,8N1. To do this, the HC-06 needs to be connected to UART1. Mobile phone can send GXXX to retrieve the state of a LED. G stands for 'get' and XXX stands for the name of the designated LED. For example, GDE0 retrieves the state of LED DE0. SXXX0 or SXXX1 can be sent from the mobile phone to turn on or off the designated LED. S stands for 'set', XXX stands for the name of LED, 0 indicates that the LED should be turned off and 1 turned on.
	The title of BlueToothChat was changed to ECD300Chat; the description about this application was updated; the reply from ECD300 was sent to UI when a '\n' was detected, so that the reply was not broken.
	  
	  