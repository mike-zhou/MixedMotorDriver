#include "Invenco_lib.h"

#define MMD_LOCATOR_HUB_AMOUNT 5
#define MMD_STEPPERS_AMOUNT 5
#define MMD_DIRECT_CURRENT_MOTORS_AMOUNT 2
#define MMD_BI_DIRECTION_DIRECT_CURRENT_MOTORS_AMOUNT 6

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

enum MMD_STEPPER_PHASE
{
	STEPPER_ACCERATING = 0,
	STEPPER_NORMAL,
	STEPPER_DECELATING
};

struct MMD_STEPPER_DATA
{
	unsigned short totalSteps;
	unsigned short currentStepIndex;
	
	// acceleration 
	unsigned short accelerationBuffer;
	unsigned short accelerationDecrement;
	unsigned short accelerationLevel;
	// deceleration
	unsigned short decelerationBuffer;
	unsigned short decelerationIncrement;
	unsigned short decelerationLevel;
	unsigned short decelerationStartingIndex; //from which step the deceleration starts.
	
	// stepper is driven on the rising edge
	bool phaseLow;
	unsigned short counterPhaseStarting;
	unsigned short phaseCounts;
};

struct MMD_command_state
{
	enum CommandState state;
	enum MMD_command_e command;
	
	struct MMD_STEPPER_DATA steppersData[MMD_STEPPERS_AMOUNT];
} mmdCommand;

enum MMD_BDCM_STATE
{
	BDCM_ERROR = 0,
	BDCM_COAST,
	BDCM_REVERSE,
	BDCM_FORWARD,
	BDCM_BREAK
};

struct MMD_status
{
	//power status
	bool isMainPowerOk;
	bool isMainFuseOk;
	
	//OPT
	bool isOptPowered;
	
	//locator hubs: 
	//	0: no input is low
	//	1-8: the designated input is low 
	//	other value: invalid
	unsigned char locatorHubs[MMD_LOCATOR_HUB_AMOUNT];
	
	//steppers
	bool steppersArePowered;
	bool steppersEnabled[MMD_STEPPERS_AMOUNT];
	bool steppersForward[MMD_STEPPERS_AMOUNT];
	bool steppersWorking[MMD_STEPPERS_AMOUNT];
	
	//dcms
	bool dcmsPowered[MMD_DIRECT_CURRENT_MOTORS_AMOUNT];
	
	//bdcms
	bool bdcmsPowerOk;
	enum MMD_BDCM_STATE bdcmsState[MMD_BI_DIRECTION_DIRECT_CURRENT_MOTORS_AMOUNT];
} mmd_status;

static void MMD_init()
{
	
}

// check main power status
// returned value:
//		true: main power is applied
//		false: no main power
static bool MMD_is_main_power_ok()
{
	//PJ1
	if(PORTJ_IN & 0x02) {
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
static bool MMD_is_main_fuse_ok()
{
	//PJ0
	if(PORTJ_IN & 0x01) {
		return true;
	}
	else {
		return false;
	}
}

// power on OPT
static void MMD_power_on_opt(bool on)
{
	//controlled by PH7
	if(on) {
		PORTH_OUTSET = 0x80;
		PORTH_DIRSET = 0x80;
	}
	else {
		PORTH_OUTCLR = 0x80;
	}
}

// return whether OPT is powered on
// return value:
//		true: OPT is powered on
//		false: OPT isn't powered on
static bool MMD_is_opt_powered_on()
{
	// PH6
	if(PORTH_IN & 0x40) {
		return false;
	}
	else {
		return true;
	}
}

// power on direct current motor
static void MMD_power_on_dcm(unsigned char dcmIndex, bool on)
{
	if(on)
	{
		switch(dcmIndex)
		{
			case 0: //DCM1
			{
				//PH3
				PORTH_OUTSET = 0x04;
				PORTH_DIRSET = 0x04;
			}
			break;
			
			case 1: //DCM2
			{
				//PH1
				PORTH_OUTSET = 0x02;
				PORTH_DIRSET = 0x02;
			}
			break;
			
			default:
				break;
		}
	}
	else {
		switch(dcmIndex)
		{
			case 0: //DCM1
			{
				//PH3
				PORTH_OUTCLR = 0x04;
			}
			break;
			
			case 1: //DCM2
			{
				//PH1
				PORTH_OUTCLR = 0x02;
			}
			break;
			
			default:
				break;
		}
	}
}

static bool MMD_is_dcm_powered_on(unsigned char dcmIndex)
{
	switch(dcmIndex)
	{
		case 0: //DCM1
		{
			//PH2
			if(PORTH_IN & 0x02) {
				return false;
			}
			else {
				return true;
			}
		}
		break;
		
		case 1: //DCM2
		{
			//PH0
			if(PORTH_IN & 0x01) {
				return false;
			}
			else {
				return  true;
			}
		}
		break;
		
		default:
			return false;
	}
}

// power on all steppers
static void MMD_power_on_steppers(bool on)
{
	//PH5
	if(on) {
		PORTH_OUTSET = 0x20;
		PORTH_DIRSET = 0x20;
	}
	else {
		PORTH_OUTCLR = 0x20;
	}
}

static bool MMD_are_steppers_powered_on()
{
	//PH4
	if(PORTH_IN & 0x10) {
		return false;
	}
	else {
		return true;
	}
}

static void MMD_stepper_dir(unsigned char stepperIndex, bool forward)
{
	switch(stepperIndex)
	{
		case 0: // stepper 1
		{
			//Q6: PK7
			if(forward) {
				PORTK_OUTCLR = 0x80;
			}
			else {
				PORTK_OUTSET = 0x80;
				PORTK_DIRSET = 0x80;
			}
		}
		break;
		
		case 1: // stepper 2
		{
			//Q9: PK4
			if(forward) {
				PORTK_OUTCLR = 0x10;
			}
			else {
				PORTK_OUTSET = 0x10;
				PORTK_DIRSET = 0x10;
			}
		}
		break;
		
		case 2: // stepper 3
		{
			//Q12, PK1
			if(forward) {
				PORTK_OUTCLR = 0x01;
			}
			else {
				PORTK_OUTSET = 0x01;
				PORTK_DIRSET = 0x01;
			}
		}
		break;
		
		case 3: //stepper 4
		{
			//Q15, PJ6
			if(forward) {
				PORTJ_OUTCLR = 0x40;
			}
			else {
				PORTJ_OUTSET = 0x40;
				PORTJ_DIRSET = 0x40;
			}
		}
		break;
		
		case 4: //stepper 5
		{
			//Q32, PJ3
			if(forward) {
				PORTJ_OUTCLR = 0x08;
			}
			else {
				PORTJ_OUTSET = 0x08;
				PORTJ_DIRSET = 0x08;
			}
		}
		break;
		
		default:
		break;
	}
}

static bool MMD_is_stepper_forward(unsigned char stepperIndex)
{
	switch(stepperIndex)
	{
		case 0: // stepper 1
		{
			//Q6: PK7
			if(PORTK_IN & 0x80) {
				return false;
			}
			else {
				return true;
			}
		}
		break;
		
		case 1: // stepper 2
		{
			//Q9: PK4
			if(PORTK_IN & 0x10) {
				return false;
			}
			else {
				return true;
			}
		}
		break;
		
		case 2: // stepper 3
		{
			//Q12, PK1
			if(PORTK_IN & 0x01) {
				return false;
			}
			else {
				return true;
			}
		}
		break;
		
		case 3: //stepper 4
		{
			//Q15, PJ6
			if(PORTJ_IN & 0x40) {
				return false;
			}
			else {
				return true;
			}
		}
		break;
		
		case 4: //stepper 5
		{
			//Q32, PJ3
			if(PORTJ_IN & 0x08) {
				return false;
			}
			else {
				return true;
			}
		}
		break;
		
		default:
		return false;
		break;
	}
}

static void MMD_stepper_enable(unsigned char stepperIndex, bool enable)
{
	switch(stepperIndex) 
	{
		case 0: //stepper 1
		{
			//Q5, PQ1
		}
		break;
		
		case 1: //stepper 2
		{
			//Q8, PK5
		}
		break;
		
		case 2: //stepper 3
		{
			//Q11, PK2
		}
		break;
		
		case 3: //stepper 4
		{
			//Q14, PJ7
		}
		break;
		
		case 4: //stepper 5
		{
			//Q31, PJ4
		}
		break;
		
		default:
		break;
	}
}

static bool MMD_is_stepper_enabled(unsigned char stepperIndex)
{
	switch(stepperIndex)
	{
		case 0: //stepper 1
		{
			//Q5, PQ1
		}
		break;
		
		case 1: //stepper 2
		{
			//Q8, PK5
		}
		break;
		
		case 2: //stepper 3
		{
			//Q11, PK2
		}
		break;
		
		case 3: //stepper 4
		{
			//Q14, PJ7
		}
		break;
		
		case 4: //stepper 5
		{
			//Q31, PJ4
		}
		break;
		
		default:
		break;
	}	
}

// return the resolution of step length in microseconds.
static unsigned short MMD_stepper_resolution()
{
	
}

static void MMD_stepper_set_step_duration(unsigned char stepperIndex, unsigned short duration)
{
	
}

static void MMD_stepper_set_acceleration_buffer(unsigned char stepperIndex, unsigned short buffer)
{
	
}

static void MMD_stepper_set_acceleration_buffer_decrement(unsigned char stepperIndex, unsigned short decrement)
{
	
}

static void MMD_stepper_set_deceleration_buffer(unsigned char stepperIndex, unsigned short buffer)
{
	
}

static void MMD_stepper_set_deceleration_buffer_increment(unsigned char stepperIndex, unsigned short increment)
{
	
}

static bool MMD_stepper_clocks(unsigned char stepperIndex, unsigned short steps)
{
	
}

static void MMD_stepper_clock_high(unsigned char stepperIndex, bool high)
{
	switch(stepperIndex)
	{
		case 0: //stepper 1
		{
			//Q7, PK6
			if(high) {
				PORTK_OUTCLR = 0x40;
			}
			else {
				PORTK_OUTSET = 0x40;
				PORTK_DIRSET = 0x40;
			}
		}
		break;
			
		case 1: //stepper 2
		{
			//Q10, PK3
			if(high) {
				PORTK_OUTCLR = 0x08;
			}
			else {
				PORTK_OUTSET = 0x08;
				PORTK_DIRSET = 0x08;
			}
		}
		break;
			
		case 2: //stepper 3
		{
			//Q13, PK0
			if(high) {
				PORTK_OUTCLR = 0x01;
			}
			else {
				PORTK_OUTSET = 0x01;
				PORTK_DIRSET = 0x01;
			}
		}
		break;
			
		case 3: //stepper 4
		{
			//Q16, PJ5
			if(high) {
				PORTJ_OUTCLR = 0x20;
			}
			else {
				PORTJ_OUTSET = 0x20;
				PORTJ_DIRSET = 0x20;
			}
		}
		break;
			
		case 4: //stepper 5
		{
			//Q33, PJ2
			if(high) {
				PORTJ_OUTCLR = 0x04;
			}
			else {
				PORTJ_OUTSET = 0x04;
				PORTJ_DIRSET = 0x04;
			}
		}
		break;
			
		default:
		break;
	}
}

// power on all bi-direction direct current motor
static void MMD_power_on_bdcms(bool on)
{
	if(on) {
		//activate PF7
		PORTF_OUTSET = 0x80;
		PORTF_DIRSET = 0x80;
	}
	else {
		PORTF_DIRCLR = 0x80;
	}
}

static bool MMD_are_bdcms_powered_on()
{
	//read PF6
	return (PORTF_IN & 0x40) != 0;
}

static void MMD_set_bdcm_state(unsigned char index, enum MMD_BDCM_STATE state)
{
	switch(index)
	{
		case 0: {
			//bdcm1
			switch(state)
			{
				case BDCM_COAST:
				{
					//IN1: 0; IN2: 0
					PORTF_OUTCLR = 0x30;
					PORTF_DIRSET = 0x30;
				}
				break;
			
				case BDCM_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTF_OUTCLR = 0x20; 
					PORTF_OUTSET = 0x10;
					PORTF_DIRSET = 0x30;
				}
				break;
			
				case BDCM_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTF_OUTSET = 0x20;
					PORTF_OUTCLR = 0x10;
					PORTF_DIRSET = 0x30;
				}
				break;
			
				case BDCM_BREAK:
				{
					//IN1: 1; IN2: 1
					PORTF_OUTSET = 0x30;
					PORTF_DIRSET = 0x30;
				}
				break;
			
				default:
					break;
			}
		}
		break;

		case 1: {
			//bdcm2
			switch(state)
			{
				case BDCM_COAST:
				{
					//IN1: 0; IN2: 0
					PORTF_OUTCLR = 0xC0;
					PORTF_DIRSET = 0xC0;
				}
				break;
				
				case BDCM_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTF_OUTCLR = 0x80;
					PORTF_OUTSET = 0x40;
					PORTF_DIRSET = 0xC0;
				}
				break;
				
				case BDCM_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTF_OUTSET = 0x80;
					PORTF_OUTCLR = 0x40;
					PORTF_DIRSET = 0xC0;
				}
				break;
				
				case BDCM_BREAK:
				{
					//IN1: 1; IN2: 1
					PORTF_OUTSET = 0xC0;
					PORTF_DIRSET = 0xC0;
				}
				break;
				
				default:
				break;
			}
		}
		break;		

		case 2: {
			//bdcm3
			switch(state)
			{
				case BDCM_COAST:
				{
					//IN1: 0; IN2: 0
					PORTF_OUTCLR = 0x03;
					PORTF_DIRSET = 0x03;
				}
				break;
				
				case BDCM_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTF_OUTCLR = 0x02;
					PORTF_OUTSET = 0x01;
					PORTF_DIRSET = 0x03;
				}
				break;
				
				case BDCM_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTF_OUTSET = 0x02;
					PORTF_OUTCLR = 0x01;
					PORTF_DIRSET = 0x03;
				}
				break;
				
				case BDCM_BREAK:
				{
					//IN1: 1; IN2: 1
					PORTF_OUTSET = 0x03;
					PORTF_DIRSET = 0x03;
				}
				break;
				
				default:
				break;
			}
		}
		break;	

		case 3: {
			//bdcm4
			switch(state)
			{
				case BDCM_COAST:
				{
					//IN1: 0; IN2: 0
					PORTE_OUTCLR = 0x30;
					PORTE_DIRSET = 0x30;
				}
				break;
				
				case BDCM_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTE_OUTCLR = 0x20;
					PORTE_OUTSET = 0x10;
					PORTE_DIRSET = 0x30;
				}
				break;
				
				case BDCM_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTE_OUTSET = 0x20;
					PORTE_OUTCLR = 0x10;
					PORTE_DIRSET = 0x30;
				}
				break;
				
				case BDCM_BREAK:
				{
					//IN1: 1; IN2: 1
					PORTE_OUTSET = 0x30;
					PORTE_DIRSET = 0x30;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 4: {
			//bdcm5
			switch(state)
			{
				case BDCM_COAST:
				{
					//IN1: 0; IN2: 0
					PORTE_OUTCLR = 0xC0;
					PORTE_DIRSET = 0xC0;
				}
				break;
				
				case BDCM_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTE_OUTCLR = 0x80;
					PORTE_OUTSET = 0x40;
					PORTE_DIRSET = 0xC0;
				}
				break;
				
				case BDCM_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTE_OUTSET = 0x80;
					PORTE_OUTCLR = 0x40;
					PORTE_DIRSET = 0xC0;
				}
				break;
				
				case BDCM_BREAK:
				{
					//IN1: 1; IN2: 1
					PORTE_OUTSET = 0xC0;
					PORTE_DIRSET = 0xC0;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 5: {
			//bdcm6
			switch(state)
			{
				case BDCM_COAST:
				{
					//IN1: 0; IN2: 0
					PORTE_OUTCLR = 0x03;
					PORTE_DIRSET = 0x03;
				}
				break;
				
				case BDCM_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTE_OUTCLR = 0x02;
					PORTE_OUTSET = 0x01;
					PORTE_DIRSET = 0x03;
				}
				break;
				
				case BDCM_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTE_OUTSET = 0x02;
					PORTE_OUTCLR = 0x01;
					PORTE_DIRSET = 0x03;
				}
				break;
				
				case BDCM_BREAK:
				{
					//IN1: 1; IN2: 1
					PORTE_OUTSET = 0x03;
					PORTE_DIRSET = 0x03;
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

static enum MMD_BDCM_STATE MMD_get_bdcm_state(unsigned char index)
{
	unsigned char data = 0;
	
	switch(index)
	{
		case 0: //bdcm1
		{
			data = (PORTF_IN >> 4) & 0x03;
		}
		break;
		
		case 1: //bdcm2
		{
			data = (PORTF_IN >> 2) & 0x03;
		}
		break;
		
		case 2: //bdcm3
		{
			data = (PORTF_IN) & 0x03;
		}
		break;
		
		case 3: //bdcm4
		{
			data = (PORTE_IN >> 4) & 0x03;
		}
		break;

		case 4: //bdcm5
		{
			data = (PORTE_IN >> 2) & 0x03;
		}
		break;
		
		case 5: //bdcm6
		{
			data = (PORTE_IN) & 0x03;
		}
		break;
		
		default:
			return BDCM_ERROR;
	}
	
	switch(data)
	{
		case 0:
		{
			return BDCM_COAST;
		}
		case 1:
		{
			return BDCM_REVERSE;
		}
		case 2:
		{
			return BDCM_FORWARD;
		}
		default:
		{
			return BDCM_BREAK;
		}
	}
}


// check which input line is triggered
// return value:
//	0: no input line is triggered
//  1 - 8: an input line is triggered
//	others: error occurs.
static unsigned char MMD_locator_get(unsigned char locatorIndex)
{
	switch(locatorIndex)
	{
		case 0: //LC1
			if((PORTD_IN & 0x10) == 0)
				return 1;
			else if((PORTD_IN & 0x20) == 0)
				return 2;
			else 
				return 0;
		
		case 1: //LC2
			if((PORTC_IN & 0x80) != 0)
				return 0;
			else {
				unsigned char n = 0;
				
				if((PORTC_IN & 0x40) != 0)
					n += 1;
				if((PORTC_IN & 0x20) != 0)
					n += 2;
				if((PORTC_IN & 0x10) != 0)
					n += 4;
				return n;
			}
		
		case 2: //LC3
			if((PORTC_IN & 0x08) != 0)
				return 0;
			else {
				unsigned char n = 0;
				
				if((PORTC_IN & 0x04) != 0)
					n += 1;
				if((PORTC_IN & 0x02) != 0)
					n += 2;
				if((PORTC_IN & 0x01) != 0)
					n += 4;
				return n;
			}
		
		case 3: //LC4
			if((PORTB_IN & 0x80) != 0)
				return 0;
			else {
				unsigned char n = 0;
			
				if((PORTB_IN & 0x40) != 0)
					n += 1;
				if((PORTB_IN & 0x20) != 0)
					n += 2;
				if((PORTB_IN & 0x10) != 0)
					n += 4;
				return n;
			}
		
		case 4: //LC5
			if((PORTB_IN & 0x08) != 0)
				return 0;
			else {
				unsigned char n = 0;
			
				if((PORTB_IN & 0x04) != 0)
					n += 1;
				if((PORTB_IN & 0x02) != 0)
					n += 2;
				if((PORTB_IN & 0x01) != 0)
					n += 4;
				return n;
			}
				
		case 5: //LC6
			if((PORTA_IN & 0x80) != 0)
				return 0;
			else {
				unsigned char n = 0;
				
				if((PORTA_IN & 0x40) != 0)
					n += 1;
				if((PORTA_IN & 0x20) != 0)
					n += 2;
				if((PORTA_IN & 0x10) != 0)
					n += 4;
				return n;
			}
		
		case 6: //LC7
			if((PORTA_IN & 0x08) != 0)
				return 0;
			else {
				unsigned char n = 0;
				
				if((PORTA_IN & 0x04) != 0)
					n += 1;
				if((PORTA_IN & 0x02) != 0)
					n += 2;
				if((PORTA_IN & 0x01) != 0)
					n += 4;
				return n;
			}
		
		case 7: //LC8
			if((PORTR_IN & 0x02) != 0)
				return 0;
			else {
				unsigned char n = 0;
				
				if((PORTR_IN & 0x01) != 0)
					n += 1;
				if((PORTQ_IN & 0x08) != 0)
					n += 2;
				if((PORTQ_IN & 0x04) != 0)
					n += 4;
				return n;
			}
		
		default:
			return 0;
	}
}

static void MMD_parse_command()
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

static void MMD_run_command()
{
	
	
}

static void MMD_check_status()
{
	
}

void ecd300MixedMotorDrivers()
{
	unsigned char c;

	Invenco_init();

	//PD0 works as indicator of host output
	PORTD_DIRSET = 0x01;
	
	udc_start();
	
	while(1)
	{
		unsigned char key;
		
		if (udi_cdc_is_rx_ready())
		{
			//read a command from USB buffer.
			key = (unsigned char)udi_cdc_getc();
			
			writeInputBuffer(key);
			writeOutputBufferChar(key);
			if(key == 0x0D) {
				writeOutputBufferChar(0x0A); //append a line feed.
			}
			
			//toggle PD0 to indicate character reception.
			if(PORTD_IN&0x01) {
				PORTD_OUTCLR = 0x01;
			}
			else {
				PORTD_OUTSET = 0x01;
			}
			
			// 0x0D is command terminator
			if((key == 0x0D) && (mmdCommand.state == AWAITING_COMMAND))
			{
				MMD_parse_command();
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

