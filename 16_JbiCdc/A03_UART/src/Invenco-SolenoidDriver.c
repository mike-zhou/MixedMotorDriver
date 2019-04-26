#include "Invenco_lib.h"

#define SD_PRODUCT_NAME "Solenoid_Driver_HV1.0_SV1.0"

#define SD_SOLENOID_AMOUNT 32
#define SD_MAX_COMMAND_PARAMETERS 6

enum SD_command_e
{
	COMMAND_INVALID = 0,
	COMMAND_QUREY_NAME = 1,				// C 1 cmdId
	COMMAND_DEVICE_POWER_QUERY,			// C 2 cmdId
	COMMAND_DEVICE_FUSE_QUERY,			// C 3 cmdId
	COMMAND_ECHO_COMMAND = 6,			// C 6 1/0 cmdId
	//solenoid command starts from 200
	COMMAND_SOLENOID_ACTIVATE = 200		// C 200 index activeClks deactiveClks cmdId
};

static const char * STR_CARRIAGE_RETURN = "\r\n";
static const char * STR_INVALID_COMMAND = "\"error\":\"invalid command\"";
static const char * STR_TOO_MANY_PARAMETERS = "\"error\":\"too many parameters\"";
static const char * STR_UNKNOWN_COMMAND = "\"error\":\"unknown command\"";
static const char * STR_WRONG_COMMAND_FORMAT = "\"error\":\"wrong command format\"";
static const char * STR_INVALID_PARAMETER = "\"error\":\"invalid parameter\"";
static const char * STR_WRONG_PARAMETER_AMOUNT = "\"error\":\"wrong parameter amount\"";
static const char * STR_EXCESSIVE_DELAY = "\"error\":\"delay value exceeds limit\"";
static const char * STR_SOLENOID_WRONG_PHASE_STATE = "\"error\":\"wrong phase state\"";
static const char * STR_SOLENOID_INDEX_OUT_OF_SCOPE = "\"error\":\"solenoid index is out of scope\"";
static const char * STR_POWER_UNAVAILABLE = "\"error\":\"power supply is unavailable\"";
static const char * STR_MAIN_FUSE_OFF = "\"error\":\"main fuse is off\"";

static unsigned short sdCurrentClock;
static bool echoCommand;

enum SD_solenoid_phase
{
	SD_PAHSE_START,
	SD_PHASE_ACTIVATED,
	SD_PHASE_DEACTIVATED,
	SD_PHASE_FINISH
};

struct SD_solenoid_data
{
	unsigned char solenoidIndex;
	unsigned short activeClks;
	unsigned short deactiveClks;
	
	enum SD_solenoid_phase phase;
	unsigned short phaseStartClk;
};

struct SD_command
{
	enum CommandState state;
	enum SD_command_e command;
	unsigned short parameterAmount;
	unsigned short parameters[SD_MAX_COMMAND_PARAMETERS]; //command parameters
	
	struct SD_solenoid_data solenoidData;
} sdCommand;

enum SD_SOLENOID_STATE
{
	SD_STATE_ERROR = 0,
	SD_STATE_ACTIVATED,
	SD_STATE_DEACTIVATED
};

static void SD_deactivate_all_solenoids()
{
	PORTC_DIRCLR = 0xff;
	PORTB_DIRCLR = 0xff;
	PORTA_DIRCLR = 0xff;
	PORTK_DIRCLR = 0xff;
	PORTJ_DIRCLR = 0xff;
	PORTH_DIRCLR = 0xff;
	PORTF_DIRCLR = 0xff;
	PORTE_DIRCLR = 0xff;
}

static void SD_init(void)
{
	PORTD_DIRCLR = 0x30; //power & fuse monitor
	SD_deactivate_all_solenoids();
	
	//command data
	sdCommand.state = AWAITING_COMMAND;
	sdCommand.parameterAmount = 0;
}

// return (mmdCurrentClk - clkStamp)
static unsigned short MMD_elapsed_clocks(unsigned short clkStamp)
{
	if(sdCurrentClock >= clkStamp) {
		return (sdCurrentClock - clkStamp);
	}
	else {
		return (0xFFFF - clkStamp + sdCurrentClock);
	}
}

static inline unsigned short MMD_current_clock(void)
{
	return sdCurrentClock;
}

// check main power status
// returned value:
//		true: main power is applied
//		false: no main power
static bool SD_is_main_power_ok(void)
{
	//Pd5
	if(PORTD_IN & 0x20) {
		return false;
	}
	else {
		return true;
	}
}

// check main fuse status
// returned value:
//		true: main fuse is ok
//		false:	main fuse is broken
static bool SD_is_main_fuse_ok(void)
{
	//PD4
	if(PORTD_IN & 0x10) {
		return true;
	}
	else {
		return false;
	}
}

static void SD_solenoid_set_state(unsigned char index, enum SD_SOLENOID_STATE state)
{
	switch(index) 
	{
		case 0: {
			//pc7
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTC_OUTSET = 0x80;
					PORTC_DIRSET = 0x80;
				}
				break;
			
				case SD_STATE_DEACTIVATED:
				{
					PORTC_OUTCLR = 0x80;
					PORTC_DIRCLR = 0x80;
				}
				break;
			
				default:
					break;
			}
		}
		break;

		case 1: {
			//pc5
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTC_OUTSET = 0x20;
					PORTC_DIRSET = 0x20;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTC_OUTCLR = 0x20;
					PORTC_DIRCLR = 0x20;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 2: {
			//pc3
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTC_OUTSET = 0x08;
					PORTC_DIRSET = 0x08;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTC_OUTCLR = 0x08;
					PORTC_DIRCLR = 0x08;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 3: {
			//pc1
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTC_OUTSET = 0x02;
					PORTC_DIRSET = 0x02;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTC_OUTCLR = 0x02;
					PORTC_DIRCLR = 0x02;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 4: {
			//pb7
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTB_OUTSET = 0x80;
					PORTB_DIRSET = 0x80;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTB_OUTCLR = 0x80;
					PORTB_DIRCLR = 0x80;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 5: {
			//pb5
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTB_OUTSET = 0x20;
					PORTB_DIRSET = 0x20;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTB_OUTCLR = 0x20;
					PORTB_DIRCLR = 0x20;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 6: {
			//pb3
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTB_OUTSET = 0x08;
					PORTB_DIRSET = 0x08;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTB_OUTCLR = 0x08;
					PORTB_DIRCLR = 0x08;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 7: {
			//pb1
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTB_OUTSET = 0x02;
					PORTB_DIRSET = 0x02;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTB_OUTCLR = 0x02;
					PORTB_DIRCLR = 0x02;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 8: {
			//pa7
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTA_OUTSET = 0x80;
					PORTA_DIRSET = 0x80;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTA_OUTCLR = 0x80;
					PORTA_DIRCLR = 0x80;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 9: {
			//pa5
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTA_OUTSET = 0x20;
					PORTA_DIRSET = 0x20;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTA_OUTCLR = 0x20;
					PORTA_DIRCLR = 0x20;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 10: {
			//pa3
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTA_OUTSET = 0x08;
					PORTA_DIRSET = 0x08;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTA_OUTCLR = 0x08;
					PORTA_DIRCLR = 0x08;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 11: {
			//pa1
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTA_OUTSET = 0x02;
					PORTA_DIRSET = 0x02;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTA_OUTCLR = 0x02;
					PORTA_DIRCLR = 0x02;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 12: {
			//pk7
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTK_OUTSET = 0x80;
					PORTK_DIRSET = 0x80;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTK_OUTCLR = 0x80;
					PORTK_DIRCLR = 0x80;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 13: {
			//pk5
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTK_OUTSET = 0x20;
					PORTK_DIRSET = 0x20;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTK_OUTCLR = 0x20;
					PORTK_DIRCLR = 0x20;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 14: {
			//pK3
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTK_OUTSET = 0x08;
					PORTK_DIRSET = 0x08;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTK_OUTCLR = 0x08;
					PORTK_DIRCLR = 0x08;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 15: {
			//pK1
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTK_OUTSET = 0x02;
					PORTK_DIRSET = 0x02;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTK_OUTCLR = 0x02;
					PORTK_DIRCLR = 0x02;
				}
				break;
				
				default:
				break;
			}
		}
		break;
		
		case 16: {
			//pj7
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTJ_OUTSET = 0x80;
					PORTJ_DIRSET = 0x80;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTJ_OUTCLR = 0x80;
					PORTJ_DIRCLR = 0x80;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 17: {
			//pj5
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTJ_OUTSET = 0x20;
					PORTJ_DIRSET = 0x20;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTJ_OUTCLR = 0x20;
					PORTJ_DIRCLR = 0x20;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 18: {
			//pj3
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTJ_OUTSET = 0x08;
					PORTJ_DIRSET = 0x08;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTJ_OUTCLR = 0x08;
					PORTJ_DIRCLR = 0x08;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 19: {
			//pj1
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTJ_OUTSET = 0x02;
					PORTJ_DIRSET = 0x02;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTJ_OUTCLR = 0x02;
					PORTJ_DIRCLR = 0x02;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 20: {
			//ph7
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTH_OUTSET = 0x80;
					PORTH_DIRSET = 0x80;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTH_OUTCLR = 0x80;
					PORTH_DIRCLR = 0x80;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 21: {
			//ph5
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTH_OUTSET = 0x20;
					PORTH_DIRSET = 0x20;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTH_OUTCLR = 0x20;
					PORTH_DIRCLR = 0x20;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 22: {
			//ph3
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTH_OUTSET = 0x08;
					PORTH_DIRSET = 0x08;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTH_OUTCLR = 0x08;
					PORTH_DIRCLR = 0x08;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 23: {
			//ph1
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTH_OUTSET = 0x02;
					PORTH_DIRSET = 0x02;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTH_OUTCLR = 0x02;
					PORTH_DIRCLR = 0x02;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 24: {
			//pf7
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTF_OUTSET = 0x80;
					PORTF_DIRSET = 0x80;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTF_OUTCLR = 0x80;
					PORTF_DIRCLR = 0x80;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 25: {
			//pf5
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTF_OUTSET = 0x20;
					PORTF_DIRSET = 0x20;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTF_OUTCLR = 0x20;
					PORTF_DIRCLR = 0x20;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 26: {
			//pf3
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTF_OUTSET = 0x08;
					PORTF_DIRSET = 0x08;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTF_OUTCLR = 0x08;
					PORTF_DIRCLR = 0x08;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 27: {
			//pf1
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTF_OUTSET = 0x02;
					PORTF_DIRSET = 0x02;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTF_OUTCLR = 0x02;
					PORTF_DIRCLR = 0x02;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 28: {
			//pe7
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTE_OUTSET = 0x80;
					PORTE_DIRSET = 0x80;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTE_OUTCLR = 0x80;
					PORTE_DIRCLR = 0x80;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 29: {
			//pe5
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTE_OUTSET = 0x20;
					PORTE_DIRSET = 0x20;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTE_OUTCLR = 0x20;
					PORTE_DIRCLR = 0x20;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 30: {
			//pe3
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTE_OUTSET = 0x08;
					PORTE_DIRSET = 0x08;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTE_OUTCLR = 0x08;
					PORTE_DIRCLR = 0x08;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 31: {
			//pe1
			switch(state)
			{
				case SD_STATE_ACTIVATED:
				{
					PORTE_OUTSET = 0x02;
					PORTE_DIRSET = 0x02;
				}
				break;
				
				case SD_STATE_DEACTIVATED:
				{
					PORTE_OUTCLR = 0x02;
					PORTE_DIRCLR = 0x02;
				}
				break;
				
				default:
				break;
			}
		}
		break;	
									
		default:
			break;
	}
}

//write command and parameters to output buffer.
static void mmd_write_succeess_reply() 
{
	writeOutputBufferString("\"command\":\"");
	writeOutputBufferHex(sdCommand.command);
	writeOutputBufferString("\",\"params\":[");
	//the first parameter
	writeOutputBufferChar('"');
	writeOutputBufferHex(sdCommand.parameters[0] >> 8);
	writeOutputBufferHex(sdCommand.parameters[0] & 0xff);
	writeOutputBufferChar('"');
	//other parameters
	for(int i=1; i<sdCommand.parameterAmount; i++) {
		writeOutputBufferChar(',');
		writeOutputBufferChar('"');
		writeOutputBufferHex(sdCommand.parameters[i] >> 8);
		writeOutputBufferHex(sdCommand.parameters[i] & 0xff);
		writeOutputBufferChar('"');
	}
	writeOutputBufferChar(']');
	writeOutputBufferString(STR_CARRIAGE_RETURN);
}

static void mmd_write_reply_header()
{
	writeOutputBufferString("\"command\":\"");
	writeOutputBufferHex(sdCommand.command);
	writeOutputBufferString("\",\"params\":[");
	//the first parameter
	writeOutputBufferChar('"');
	writeOutputBufferHex(sdCommand.parameters[0] >> 8);
	writeOutputBufferHex(sdCommand.parameters[0] & 0xff);
	writeOutputBufferChar('"');
	//other parameters
	for(int i=1; i<sdCommand.parameterAmount; i++) {
		writeOutputBufferChar(',');
		writeOutputBufferChar('"');
		writeOutputBufferHex(sdCommand.parameters[i] >> 8);
		writeOutputBufferHex(sdCommand.parameters[i] & 0xff);
		writeOutputBufferChar('"');
	}
	writeOutputBufferChar(']');
	writeOutputBufferChar(',');
}

static void sd_run_solenoid()
{
	switch(sdCommand.solenoidData.phase)
	{
		case SD_PAHSE_START:
		{
			sdCommand.solenoidData.phaseStartClk = MMD_current_clock();
			SD_solenoid_set_state(sdCommand.solenoidData.solenoidIndex, SD_STATE_ACTIVATED); //activate solenoid
			sdCommand.solenoidData.phase = SD_PHASE_ACTIVATED;
		}
		break;
		
		case SD_PHASE_ACTIVATED:
		{
			if(MMD_elapsed_clocks(sdCommand.solenoidData.phaseStartClk) >= sdCommand.solenoidData.activeClks) {
				sdCommand.solenoidData.phase = SD_PHASE_DEACTIVATED;
				SD_deactivate_all_solenoids(); //deactivate solenoid
				sdCommand.solenoidData.phaseStartClk = MMD_current_clock();
			}
		}
		break;
		
		case SD_PHASE_DEACTIVATED:
		{
			if(MMD_elapsed_clocks(sdCommand.solenoidData.phaseStartClk) >= sdCommand.solenoidData.deactiveClks) {
				sdCommand.solenoidData.phase = SD_PHASE_FINISH;
			}
		}
		break;
		
		case SD_PHASE_FINISH:
		{
			mmd_write_succeess_reply();
			sdCommand.state = AWAITING_COMMAND;
		}
		break;
		
		default:
		{
			//wrong state
			mmd_write_reply_header();
			writeOutputBufferString(STR_SOLENOID_WRONG_PHASE_STATE);
			writeOutputBufferString(STR_CARRIAGE_RETURN);
			sdCommand.state = AWAITING_COMMAND;
		}
		break;
	}
}

static void mmd_parse_command(void)
{
	unsigned char tag;
	unsigned char cmd = 0;
	unsigned char data;
	bool validCmd = false;

	//command format:
	// C cmdNumber param0 param1 param2 param3 param4 param5

	//1st char
	tag = readInputBuffer();

	if((tag == 'C') || (tag == 'c'))
	{
		validCmd = true;
		
		//2nd char
		data = readInputBuffer();
		if(data != ' ') {
			validCmd = false;
		}
		else
		{
			unsigned char c;
			
			//reset command and parameters
			sdCommand.command = COMMAND_INVALID;
			sdCommand.parameterAmount = 0;
			for(unsigned char i = 0; i<SD_MAX_COMMAND_PARAMETERS; i++) {
				sdCommand.parameters[i] = 0;
			}

			//read command number
			for(;;)
			{
				c = readInputBuffer();
				if((c >= '0') && (c <= '9')) {
					cmd =  cmd * 10 + c - '0';
				}
				else if(0x0D == c) {
					break; //end of a command
				}
				else if(' ' == c) {
					break; //command number ends.
				}
				else {
					//illegal character
					validCmd = false;
					break;
				}
			}

			if(validCmd && (c != 0x0D))
			{
				unsigned char index;
				//read parameters
				for(index=0; index<SD_MAX_COMMAND_PARAMETERS; index++)
				{
					unsigned short p = 0;
					bool digitMet = false;
					
					//read a parameter
					for(;;)
					{
						c = readInputBuffer();
						if((c >= '0') && (c <= '9')) {
							p =  p * 10 + c - '0';
							digitMet = true;
						}
						else if(0x0D == c) {
							if(digitMet == true) {
								sdCommand.parameters[index] = p;
								sdCommand.parameterAmount++;
							}
							break; //end of a command
						}
						else if(' ' == c) {
							if(digitMet == true) {
								sdCommand.parameters[index] = p;
								sdCommand.parameterAmount++;
								break; //command number ends.
							}
							else {
								continue; //ignore continuous ' ' ahead of a parameter.
							}
						}
						else {
							//illegal character
							validCmd = false;
							break;
						}
					}
					if((0x0D == c) || !validCmd) {
						break;
					}
				}
				if(index >= SD_MAX_COMMAND_PARAMETERS) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_TOO_MANY_PARAMETERS);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					validCmd = false;
				}
			}
		}
	}

	if(!validCmd) {
		sdCommand.command = COMMAND_INVALID;
		sdCommand.parameterAmount = 0;
		clearInputBuffer();
		mmd_write_reply_header();
		writeOutputBufferString(STR_INVALID_COMMAND);
		writeOutputBufferString(STR_CARRIAGE_RETURN);
	}
	else {
		switch(cmd)
		{
		case COMMAND_QUREY_NAME:
		case COMMAND_DEVICE_POWER_QUERY:
		case COMMAND_DEVICE_FUSE_QUERY:
		case COMMAND_ECHO_COMMAND:
		case COMMAND_SOLENOID_ACTIVATE:
		{
			sdCommand.command = cmd;
			sdCommand.state = STARTING_COMMAND;
		}
		break;
		default:
		{
			mmd_write_reply_header();
			writeOutputBufferString(STR_UNKNOWN_COMMAND);
			writeOutputBufferString(STR_CARRIAGE_RETURN);
			sdCommand.command = COMMAND_INVALID;
			sdCommand.state = AWAITING_COMMAND;
		}
			break;
		}
	}
}

static void mmd_cancel_command(void)
{
	clearInputBuffer();
	sdCommand.state = AWAITING_COMMAND;
}

static void mmd_run_command(void)
{
	if(sdCommand.state == STARTING_COMMAND)
	{
		//prepare for execution.
		switch(sdCommand.command)
		{
			case COMMAND_QUREY_NAME:
			case COMMAND_DEVICE_POWER_QUERY:
			case COMMAND_DEVICE_FUSE_QUERY:
			{
				if(sdCommand.parameterAmount != 1) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					sdCommand.state = AWAITING_COMMAND;
				}
				else {
					sdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case COMMAND_ECHO_COMMAND:
			{
				if(sdCommand.parameterAmount != 2){
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					sdCommand.state = AWAITING_COMMAND;
				}
				else if(sdCommand.parameters[0] > 0x7FFF) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_EXCESSIVE_DELAY);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					sdCommand.state = AWAITING_COMMAND;
				}
				else {
					sdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case COMMAND_SOLENOID_ACTIVATE:
			{
				if(sdCommand.parameterAmount != 4){
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					sdCommand.state = AWAITING_COMMAND;
				}
				else if(sdCommand.parameters[0] >= SD_SOLENOID_AMOUNT) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_SOLENOID_INDEX_OUT_OF_SCOPE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					sdCommand.state = AWAITING_COMMAND;
				}
				else {
					sdCommand.solenoidData.solenoidIndex = sdCommand.parameters[0];
					sdCommand.solenoidData.activeClks = sdCommand.parameters[1];
					sdCommand.solenoidData.deactiveClks = sdCommand.parameters[2];
					sdCommand.solenoidData.phase = SD_PAHSE_START;
					
					sdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			default:
			{
				mmd_write_reply_header();
				writeOutputBufferString(STR_UNKNOWN_COMMAND);
				writeOutputBufferString(STR_CARRIAGE_RETURN);
				clearInputBuffer();
				sdCommand.state = AWAITING_COMMAND;

			}
			break;
		}
	}
	else if(sdCommand.state == EXECUTING_COMMAND)
	{
		switch(sdCommand.command)
		{
			case COMMAND_QUREY_NAME:
			{
				mmd_write_reply_header();
				writeOutputBufferString("\"name\":\""); writeOutputBufferString(SD_PRODUCT_NAME); writeOutputBufferString("\"");
				writeOutputBufferString(STR_CARRIAGE_RETURN);
				sdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_DEVICE_POWER_QUERY:
			{
				mmd_write_reply_header();
				writeOutputBufferString("\"state\":");
				if(SD_is_main_power_ok()) {
					writeOutputBufferString("\"powered on\"");
				}
				else {
					writeOutputBufferString("\"powered off\"");
				}
				writeOutputBufferString(STR_CARRIAGE_RETURN);
				sdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_DEVICE_FUSE_QUERY:
			{
				mmd_write_reply_header();
				writeOutputBufferString("\"state\":");
				if(SD_is_main_power_ok()) {
					writeOutputBufferString("\"main fuse is on\"");
				}
				else {
					writeOutputBufferString("\"main fuse is off\"");
				}
				writeOutputBufferString(STR_CARRIAGE_RETURN);
				sdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_ECHO_COMMAND:
			{
				if(sdCommand.parameters[0] != 0) {
					echoCommand = true;
				}
				else {
					echoCommand = false;
				}
				mmd_write_succeess_reply();
				sdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_SOLENOID_ACTIVATE:
			{
				if(!SD_is_main_power_ok())
				{
					mmd_write_reply_header();
					writeOutputBufferString(STR_POWER_UNAVAILABLE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					sdCommand.state = AWAITING_COMMAND;
				}
				else if(!SD_is_main_fuse_ok())
				{
					mmd_write_reply_header();
					writeOutputBufferString(STR_MAIN_FUSE_OFF);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					sdCommand.state = AWAITING_COMMAND;
				}	
				else
				{
					sd_run_solenoid();
				}			
			}
			break;
			
			default:
			{
				mmd_write_reply_header();
				writeOutputBufferString(STR_UNKNOWN_COMMAND);
				writeOutputBufferString(STR_CARRIAGE_RETURN);
				sdCommand.state = AWAITING_COMMAND;
			}
			break;
		}
	}
}

void ecd300SolenoidDriver(void)
{
	Invenco_init();
	SD_init();

	//PD0 works as indicator of host output
	PORTD_DIRSET = 0x01;
	
	while(1)
	{
		unsigned char key;
		
		pollScsDataExchange();
		
		if (getScsInputData(&key))
		{
			writeInputBuffer(key); //append to input buffer
			
			//toggle PD0 to indicate character reception.
			if(PORTD_IN&0x01) {
				PORTD_OUTCLR = 0x01;
			}
			else {
				PORTD_OUTSET = 0x01;
			}
			
			// 0x0D is command terminator
			if(key == 0x0D) 
			{
				if(sdCommand.state == AWAITING_COMMAND) {
					mmd_parse_command();
				}
				else {
					mmd_cancel_command();
				}
			}
		}

		//update current clock with system counter.
		sdCurrentClock = counter_get();

		mmd_run_command();
		
		sendOutputBufferToHost();
	}

	while(1)
	{
		;
	}
}

