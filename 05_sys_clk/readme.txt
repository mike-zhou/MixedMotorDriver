A03_UART==>==153====>=====155====>====160====>=====165===>====168==
              |            |           |            |          |
			  V            |           |            |          |
          01_blink_led	   V           |            |          |
                      02_enum_led      V            |          |
                                   03_uart_0        V          |
								               04_uart_int     V
											               05_sys_clk
											   
02 Enumerate LED one by one, then turn on the odd and even LEDs alternatively.

03 Output a character continuously through UART0. If a new character is input, then output this new character.

04 Add ISR for UART0 to UART6, configurations for these UARTs are in ECD300_usart_config.h. If UARTs are not necessary, do not define them so that flash space and SRAM space can be saved.

05 Adjust the frequency of CPU and peripheral devices.
