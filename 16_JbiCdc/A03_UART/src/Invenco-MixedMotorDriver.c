#include "Invenco_lib.h"

enum MMD_command_e
{
	OPT_POWER_ON = 10,
	OPT_POWER_OFF,
	OPT_POWER_QUERY,
	STEPPERS_POWER_ON = 20,
	STEPPERS_POWER_OFF,
	STEPPERS_POWER_QUERY,
	DCM_POWER_ON = 30,
	DCM_POWER_OFF,
	DCM_POWER_QUERY,
	BDC_COAST = 40,
	BDC_REVERSE,
	BDC_FORWARD,
	BDC_BREAK,
	BDC_QUERY,
	STEPPER_STEP_RESOLUTION_QUERY = 50,
	STEPPER_STEP_DURATION,
	STEPPER_ACCELERATION_BUFFER,
	STEPPER_ACCELERATION_BUFFER_DECREMENT,
	STEPPER_DECELERATION_BUFFER,
	STEPPER_DECELERATION_BUFFER_INCREMENT,
	STEPPER_ENABLE,
	STEPPER_DIR,
	STEPPER_STEPS,
	STEPPER_HOME,
	STEPPER_QUREY,
	LOCATOR_QUERY = 100
};

struct MMD_command_state
{
	enum CommandState state;
	enum MMD_command_e command;
} mmd_command_state;

static void MMD_init()
{
	
}

static bool MMD_power_on_opt()
{
	
}

static bool MMD_power_off_opt()
{
	
}

static bool MMD_power_on_steppers()
{
	
}

static bool MMD_power_off_steppers()
{
	
}

static bool MMD_power_on_dcm(unsigned char dcmIndex)
{
	
}

static bool MMD_power_off_dcm(unsigned char dcmIndex)
{
	
}

// return the resolution of step length in microseconds.
static unsigned short MMD_stepper_resolution()
{
	
}


static bool MMD_stepper_set_step_duration(unsigned short duration)
{
	
}

static bool MMD_coast_stepper(unsigned char stepperIndex)
{
	
}

static bool MMD_reverse_stepper(unsigned char stepperIndex)
{
	
}

static bool MMD_forward_stepper(unsigned char stepperIndex)
{
	
}

static bool MMD_break_stepper(unsigned char stepperIndex)
{
	
}

static bool MMD_clock_stepper(unsigned char stepperIndex, unsigned short steps)
{
	
}

// check which input line is triggered
// return value:
//	0: no input line is triggered
//  1 - 8: an input line is triggered
//	others: error occurs.
static unsigned char MMD_locator_get(unsigned char locatorIndex)
{
	
}

static void MMD_check_status()
{
	
}

static void MMD_run_command()
{
	
	
}

void ecd300MixedMotorDrivers()
{
	usart_rs232_options_t uartOption;
	unsigned char c;
	
	PORTA_DIR=0x00;
	PORTB_DIR=0x00;
	PORTC_DIR=0x00;
	PORTD_DIR=0x00;
	PORTE_DIR=0x00;
	PORTF_DIR=0x00;
	PORTH_DIR=0x00;
	PORTJ_DIR=0x00;
	PORTK_DIR=0x00;

	disableJtagPort();
	sysclk_init();
	sleepmgr_init();
	irq_initialize_vectors(); //enable LOW, MED and HIGH level interrupt in PMIC.
	cpu_irq_enable();
	
	counter_init();

	uartOption.baudrate=115200;
	uartOption.charlength=USART_CHSIZE_8BIT_gc;
	uartOption.paritytype=USART_PMODE_DISABLED_gc;
	uartOption.stopbits=false;
	ecd300InitUart(ECD300_UART_2, &uartOption);
	printString("Serial Port in Power Allocator was initialized\r\n");

	//PD0 works as indicator of host output
	PORTD_DIRSET = 0x01;
	
	udc_start();
	
	while(1)
	{
		unsigned char key, tdo;
		
		if (udi_cdc_is_rx_ready())
		{
			//read a command from USB buffer.
			key = (unsigned char)udi_cdc_getc();
			
			writeInputBuffer(key);
			writeOutputBufferChar(key);
			if(key == 0x0D) {
				writeOutputBufferChar(0x0A); //append a line feed.
			}
			
			printHex(key);
			printString("\r\n");
			
			//toggle PD0 to indicate character reception.
			if(PORTD_IN&0x01) {
				PORTD_OUTCLR = 0x01;
			}
			else {
				PORTD_OUTSET = 0x01;
			}
			
			// 0x0D is command terminator
			if((key == 0x0D) && (mmd_command_state.state == AWAITING_COMMAND))
			{
				unsigned char cmd;
				unsigned char param = 0;
				bool validCmd = true;
				
				cmd = readInputBuffer();
				printString("CMD:");printHex(cmd);printString("\r\n");
				if(0 == cmd) {
					validCmd = false;
				}
				else if(0x0D == cmd) {
					validCmd = false;
				}
				else {
					//read parameter of the command
					for(;;) {
						unsigned char c;
						c = readInputBuffer();
						
						if((c >= '0') && (c <= '9')) {
							param =  param * 10 + c - '0';
						}
						else if(0x0D == c) {
							break; //end of a command
						}
						else {
							//illegal character in parameter
							writeOutputBufferString("Illegal parameters\r\n");
							clearInputBuffer();
							validCmd = false;
							break;
						}
					}
				}
				
				printString("param:");printHex(param);printString("\r\n");
				if(!validCmd) {
					;
				}
				else
				{
					switch(cmd)
					{
						case 'I':
						case 'i':
						// Insert smart card.
						break;
						
						case 'P':
						case 'p':
						// Pullout smart card.
						break;
						
						case 'C':
						case 'c':
						//connect smart card
						break;
						
						case 'D':
						case 'd':
						//solenoid duration division
						break;
						
						case 'Q':
						case 'q':
						// Query
						break;
						default:
						writeOutputBufferString("Invalid command\r\n");
						break;
					}
				}
			}
		}

		MMD_run_command();
		
		MMD_check_status();

		sendOutputBufferToHost();
	}

	while(1)
	{
		;
	}
}

