#include "Invenco_lib.h"

#define MMD_LOCATOR_AMOUNT 5
#define MMD_STEPPERS_AMOUNT 5
#define MMD_DIRECT_CURRENT_MOTORS_AMOUNT 2
#define MMD_BI_DIRECTION_DIRECT_CURRENT_MOTORS_AMOUNT 6
#define MMD_MAX_COMMAND_PARAMETERS 6

enum MMD_command_e
{
	COMMAND_INVALID = 0,
	COMMAND_OPT_POWER_ON = 10,			// C 10
	COMMAND_OPT_POWER_OFF = 11,			// C 11
	COMMAND_OPT_POWER_QUERY = 12,		// C 12
	COMMAND_STEPPERS_POWER_ON = 20,		// C 13
	COMMAND_STEPPERS_POWER_OFF = 21,	// C 14
	COMMAND_STEPPERS_POWER_QUERY = 22,  // C 15
	COMMAND_DCM_POWER_ON = 30,			// C 30 dcmIndex
	COMMAND_DCM_POWER_OFF = 31,			// C 31 dcmIndex
	COMMAND_DCM_POWER_QUERY = 32,		// C 32 dcmIndex
	COMMAND_BDCS_POWER_ON = 40,			// C 40
	COMMAND_BDCS_POWER_OFF = 41,		// C 41
	COMMAND_BDCS_POWER_QUERY = 42,		// C 42
	COMMAND_BDC_COAST = 43,				// C 43 bdcIndex
	COMMAND_BDC_REVERSE = 44,			// C 44 bdcIndex
	COMMAND_BDC_FORWARD = 45,			// C 45 bdcIndex
	COMMAND_BDC_BREAK = 46,				// C 46 bdcIndex
	COMMAND_BDC_QUERY = 47,				// C 47 bdcIndex
	COMMAND_STEPPER_STEP_RESOLUTION_QUERY = 50,			// C 50
	COMMAND_STEPPER_CONFIG_STEP = 51,					// C 51 stepperIndex lowClks highClks
	COMMAND_STEPPER_ACCELERATION_BUFFER = 52,			// C 52 stepperIndex buffer
	COMMAND_STEPPER_ACCELERATION_BUFFER_DECREMENT = 53,	// C 53 stepperIndex decrement
	COMMAND_STEPPER_DECELERATION_BUFFER = 54,			// C 54 stepperIndex buffer
	COMMAND_STEPPER_DECELERATION_BUFFER_INCREMENT = 55,	// C 55 stepperIndex increment
	COMMAND_STEPPER_ENABLE = 56,		// C 56 stepperIndex 1/0
	COMMAND_STEPPER_DIR = 57,			// C 57 stepperIndex 1/0
	COMMAND_STEPPER_STEPS = 58,			// C 58 stepperIndex steps
	COMMAND_STEPPER_RUN = 59,			// c 59
	COMMAND_STEPPER_CONFIG_HOME = 60,	// C 60 stepperIndex locatorIndex lineNumberStart lineNumberTerminal
	COMMAND_STEPPER_QUREY = 61,			// C 61 stepperIndex
	COMMAND_LOCATOR_QUERY = 100			// C 100 locatorHubIndex
};

enum MMD_stepper_state
{
	STEPPER_STATE_UNKNOWN_POSITION = 0,
	STEPPER_STATE_APPROACHING_HOME,
	STEPPER_STATE_LEAVING_HOME,
	STEPPER_STATE_KNOWN_POSITION,
	STEPPER_STATE_ACCELERATING,
	STEPPER_STATE_CRUISING,
	STEPPER_STATE_DECELERATING
};

enum MMD_stepper_step_phase
{
	STEP_PHASE_CLk_LOW = 0,
	STEP_PHASE_CLK_HIGH,
	STEP_PHASE_DELAY,
	STEP_PHASE_FINISH
};

static const char * STR_UNKNOWN_COMMAND = "Unknown command\r\n";
static const char * STR_WRONG_COMMAND_FORMAT = "Wrong command format\r\n";
static const char * STR_INVALID_PARAMETER = "Invalid parameter\r\n";
static const char * STR_WRONG_PARAMETER_AMOUNT = "Wrong parameter amount\r\n";
static const char * STR_STEPPER_INDEX_OUT_OF_SCOPE = "Stepper index is out of scope\r\n";
static const char * STR_STEPPER_NOT_POSITIONED = "Stepper has not been positioned\r\n";
static const char * STR_LOCATOR_INDEX_OUT_OF_SCOPE = "Locator index is out of scope\r\n";
static const char * STR_LOCATOR_LINE_INDEX_OUT_OF_SCOPE = "Locator line index is out of scope\r\n";
static const char * STR_LOCATOR_LINE_INDEX_DUPLICATE = "Duplicated locator line index\r\n";
static const char * STR_DCM_INDEX_OUT_OF_SCOPE = "DCM index is out of scope\r\n";
static const char * STR_BDC_INDEX_OUT_OF_SCOPE = "BDC index is out of scope\r\n";

struct MMD_stepper_data
{
	enum MMD_stepper_state state;
	
	unsigned char locatorIndex;
	unsigned char locatorLineNumberStart;
	unsigned char locatorLineNumberTerminal;
	
	bool enabled;
	bool forward;
	unsigned short homeOffset;
	unsigned short totalSteps;
	unsigned short currentStepIndex;
	unsigned short decelerationStartingIndex; //from which step the deceleration starts.
	
	// acceleration 
	unsigned short accelerationBuffer;
	unsigned short accelerationDecrement;
	unsigned short accelerationLevel;
	// deceleration
	unsigned short decelerationBuffer;
	unsigned short decelerationIncrement;
	unsigned short decelerationLevel;
	
	// stepper is driven on the rising edge
	unsigned short counterPhaseStarting;
	unsigned short phaseLowClocks;
	unsigned short phaseHighClocks;
	enum MMD_stepper_step_phase stepPhase;
};

struct MMD_command
{
	enum CommandState state;
	enum MMD_command_e command;
	unsigned short parameterAmount;
	unsigned short parameters[MMD_MAX_COMMAND_PARAMETERS]; //command parameters
	
	struct MMD_stepper_data steppersData[MMD_STEPPERS_AMOUNT];
} mmdCommand;

enum MMD_BDCM_STATE
{
	BDCM_STATE_ERROR = 0,
	BDCM_STATE_COAST,
	BDCM_STATE_REVERSE,
	BDCM_STATE_FORWARD,
	BDCM_STATE_BREAK
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
	unsigned char locatorHubs[MMD_LOCATOR_AMOUNT];
	
	//steppers
	bool steppersArePowered;
	bool steppersEnabled[MMD_STEPPERS_AMOUNT];
	bool steppersForward[MMD_STEPPERS_AMOUNT];
	enum MMD_stepper_state steppersSpeed[MMD_STEPPERS_AMOUNT];
	
	//dcms
	bool dcmsPowered[MMD_DIRECT_CURRENT_MOTORS_AMOUNT];
	
	//bdcms
	bool bdcmsPowerOk;
	enum MMD_BDCM_STATE bdcmsState[MMD_BI_DIRECTION_DIRECT_CURRENT_MOTORS_AMOUNT];

	//command
	enum CommandState cmdState;
} mmdStatus;

static void MMD_init()
{
	unsigned char index;
	
	//command data
	mmdCommand.state = AWAITING_COMMAND;
	mmdCommand.parameterAmount = 0;
	for(index=0; index<MMD_STEPPERS_AMOUNT; index++)
	{
		mmdCommand.steppersData[index].enabled = false;
		mmdCommand.steppersData[index].forward = true;
		mmdCommand.steppersData[index].state = STEPPER_STATE_UNKNOWN_POSITION;

		mmdCommand.steppersData[index].totalSteps = 0;
		mmdCommand.steppersData[index].currentStepIndex = 0;
		mmdCommand.steppersData[index].decelerationStartingIndex = 0;

		mmdCommand.steppersData[index].accelerationBuffer = 0;
		mmdCommand.steppersData[index].accelerationDecrement = 1;
		mmdCommand.steppersData[index].accelerationLevel = 0;

		mmdCommand.steppersData[index].decelerationBuffer = 0;
		mmdCommand.steppersData[index].decelerationIncrement = 1;
		mmdCommand.steppersData[index].decelerationLevel = 0;

		mmdCommand.steppersData[index].counterPhaseStarting = 0;
		mmdCommand.steppersData[index].phaseLowClocks = 1;
		mmdCommand.steppersData[index].phaseHighClocks = 1;
		mmdCommand.steppersData[index].stepPhase = STEP_PHASE_FINISH;
	}

	//status
	mmdStatus.isMainPowerOk	= false;
	mmdStatus.isMainFuseOk = false;
	mmdStatus.isOptPowered = false;
	mmdStatus.isOptPowered = false;
	for(index=0; index<MMD_LOCATOR_AMOUNT; index++) {
		mmdStatus.locatorHubs[index] = 0;
	}
	mmdStatus.steppersArePowered = false;
	for(index=0; index<MMD_STEPPERS_AMOUNT; index++) {
		mmdStatus.steppersEnabled[index] = false;
		mmdStatus.steppersForward[index] = true;
		mmdStatus.steppersSpeed[index] = STEPPER_STATE_UNKNOWN_POSITION;
	}
	for(index=0; index<MMD_DIRECT_CURRENT_MOTORS_AMOUNT; index++){
		mmdStatus.dcmsPowered[index] = false;
	}
	mmdStatus.bdcmsPowerOk = false;
	for(index=0; index<MMD_BI_DIRECTION_DIRECT_CURRENT_MOTORS_AMOUNT; index++) {
		mmdStatus.bdcmsState[index] = BDCM_STATE_COAST;
	}
	mmdStatus.cmdState = AWAITING_COMMAND;
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

// stepper forwards if it leaves home
// stepper backwards if it approaches home
static void MMD_stepper_forward(unsigned char stepperIndex, bool forward)
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
			if(enable) {
				PORTQ_OUTSET = 0x01;
				PORTQ_DIRSET = 0x01;
			}
			else {
				PORTQ_OUTCLR = 0x01;
			}
		}
		break;
		
		case 1: //stepper 2
		{
			//Q8, PK5
			if(enable) {
				PORTK_OUTSET = 0x20;
				PORTK_DIRSET = 0x20;
			}
			else {
				PORTK_OUTCLR = 0x20;
			}
		}
		break;
		
		case 2: //stepper 3
		{
			//Q11, PK2
			if(enable) {
				PORTK_OUTSET = 0x04;
				PORTK_DIRSET = 0x04;
			}
			else {
				PORTK_OUTCLR = 0x04;
			}
		}
		break;
		
		case 3: //stepper 4
		{
			//Q14, PJ7
			if(enable) {
				PORTJ_OUTSET = 0x80;
				PORTJ_DIRSET = 0x80;
			}
			else {
				PORTJ_OUTCLR = 0x80;
			}
		}
		break;
		
		case 4: //stepper 5
		{
			//Q31, PJ4
			if(enable) {
				PORTJ_OUTSET = 0x10;
				PORTJ_DIRSET = 0x10;
			}
			else {
				PORTJ_OUTCLR = 0x10;
			}
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
			if(PORTQ_IN & 0x01) {
				return true;
			}
			else {
				return false;
			}
		}
		break;
		
		case 1: //stepper 2
		{
			//Q8, PK5
			if(PORTK_IN & 0x20) {
				return true;
			}
			else {
				return false;
			}
		}
		break;
		
		case 2: //stepper 3
		{
			//Q11, PK2
			if(PORTK_IN & 0x04) {
				return true;
			}
			else {
				return false;
			}
		}
		break;
		
		case 3: //stepper 4
		{
			//Q14, PJ7
			if(PORTJ_IN & 0x80) {
				return true;
			}
			else {
				return false;
			}
		}
		break;
		
		case 4: //stepper 5
		{
			//Q31, PJ4
			if(PORTJ_IN & 0x10) {
				return true;
			}
			else {
				return false;
			}
		}
		break;
		
		default:
		return false;
		break;
	}	
}

// return the resolution of step length in microseconds.
static unsigned short MMD_stepper_resolution()
{
	return counter_clock_length();
}

// set amount of clocks in a single step
static void MMD_stepper_config_step(unsigned char stepperIndex, unsigned short lowClocks, unsigned highClocks)
{
	if(stepperIndex >= MMD_STEPPERS_AMOUNT) {
		return;
	}

	mmdCommand.steppersData[stepperIndex].phaseLowClocks = lowClocks;
	mmdCommand.steppersData[stepperIndex].phaseHighClocks = highClocks;
}

static void MMD_stepper_set_acceleration_buffer(unsigned char stepperIndex, unsigned short clocks)
{
	if(stepperIndex >= MMD_STEPPERS_AMOUNT) {
		return;
	}
	
	mmdCommand.steppersData[stepperIndex].accelerationBuffer = clocks;
}

static void MMD_stepper_set_acceleration_buffer_decrement(unsigned char stepperIndex, unsigned short clocks)
{
	if(stepperIndex >= MMD_STEPPERS_AMOUNT) {
		return;
	}
	
	mmdCommand.steppersData[stepperIndex].accelerationDecrement = clocks;
}

static void MMD_stepper_set_deceleration_buffer(unsigned char stepperIndex, unsigned short clocks)
{
	if(stepperIndex >= MMD_STEPPERS_AMOUNT) {
		return;
	}
	
	mmdCommand.steppersData[stepperIndex].decelerationBuffer = clocks;
}

static void MMD_stepper_set_deceleration_buffer_increment(unsigned char stepperIndex, unsigned short clocks)
{
	if(stepperIndex >= MMD_STEPPERS_AMOUNT) {
		return;
	}
	
	mmdCommand.steppersData[stepperIndex].decelerationIncrement = clocks;
}

// set amount of steps the stepper needs to move
static void MMD_stepper_set_steps(unsigned char stepperIndex, unsigned short steps)
{
	unsigned short decelerationSteps;
	
	if(stepperIndex >= MMD_STEPPERS_AMOUNT) {
		return;
	}

	mmdCommand.steppersData[stepperIndex].totalSteps += steps;

	//find out where deceleration should start
	decelerationSteps = mmdCommand.steppersData[stepperIndex].decelerationBuffer / mmdCommand.steppersData[stepperIndex].decelerationIncrement;
	if(decelerationSteps < (steps/2)) {
		mmdCommand.steppersData[stepperIndex].decelerationStartingIndex = decelerationSteps;
		mmdCommand.steppersData[stepperIndex].decelerationLevel = mmdCommand.steppersData[stepperIndex].decelerationIncrement;
	}
	else {
		mmdCommand.steppersData[stepperIndex].decelerationStartingIndex = steps/2;
		mmdCommand.steppersData[stepperIndex].decelerationLevel = mmdCommand.steppersData[stepperIndex].decelerationIncrement * (decelerationSteps - steps/2);
	}
}

// set clk line of stepper controller to high
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

void MMD_steppers_run()
{
	struct MMD_stepper_data * pStepper;
	
	for(unsigned char stepperIndex; stepperIndex < MMD_STEPPERS_AMOUNT; stepperIndex++)
	{
		pStepper = &(mmdCommand.steppersData[stepperIndex]);
		if(pStepper->currentStepIndex >= pStepper->totalSteps) {
			continue;
		}
		
	}
}

void MMD_stepper_query(unsigned char stepperIndex)
{

}

void MMD_locator_query(unsigned char hubIndex)
{

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
				case BDCM_STATE_COAST:
				{
					//IN1: 0; IN2: 0
					PORTF_OUTCLR = 0x30;
					PORTF_DIRSET = 0x30;
				}
				break;
			
				case BDCM_STATE_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTF_OUTCLR = 0x20; 
					PORTF_OUTSET = 0x10;
					PORTF_DIRSET = 0x30;
				}
				break;
			
				case BDCM_STATE_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTF_OUTSET = 0x20;
					PORTF_OUTCLR = 0x10;
					PORTF_DIRSET = 0x30;
				}
				break;
			
				case BDCM_STATE_BREAK:
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
				case BDCM_STATE_COAST:
				{
					//IN1: 0; IN2: 0
					PORTF_OUTCLR = 0xC0;
					PORTF_DIRSET = 0xC0;
				}
				break;
				
				case BDCM_STATE_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTF_OUTCLR = 0x80;
					PORTF_OUTSET = 0x40;
					PORTF_DIRSET = 0xC0;
				}
				break;
				
				case BDCM_STATE_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTF_OUTSET = 0x80;
					PORTF_OUTCLR = 0x40;
					PORTF_DIRSET = 0xC0;
				}
				break;
				
				case BDCM_STATE_BREAK:
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
				case BDCM_STATE_COAST:
				{
					//IN1: 0; IN2: 0
					PORTF_OUTCLR = 0x03;
					PORTF_DIRSET = 0x03;
				}
				break;
				
				case BDCM_STATE_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTF_OUTCLR = 0x02;
					PORTF_OUTSET = 0x01;
					PORTF_DIRSET = 0x03;
				}
				break;
				
				case BDCM_STATE_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTF_OUTSET = 0x02;
					PORTF_OUTCLR = 0x01;
					PORTF_DIRSET = 0x03;
				}
				break;
				
				case BDCM_STATE_BREAK:
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
				case BDCM_STATE_COAST:
				{
					//IN1: 0; IN2: 0
					PORTE_OUTCLR = 0x30;
					PORTE_DIRSET = 0x30;
				}
				break;
				
				case BDCM_STATE_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTE_OUTCLR = 0x20;
					PORTE_OUTSET = 0x10;
					PORTE_DIRSET = 0x30;
				}
				break;
				
				case BDCM_STATE_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTE_OUTSET = 0x20;
					PORTE_OUTCLR = 0x10;
					PORTE_DIRSET = 0x30;
				}
				break;
				
				case BDCM_STATE_BREAK:
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
				case BDCM_STATE_COAST:
				{
					//IN1: 0; IN2: 0
					PORTE_OUTCLR = 0xC0;
					PORTE_DIRSET = 0xC0;
				}
				break;
				
				case BDCM_STATE_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTE_OUTCLR = 0x80;
					PORTE_OUTSET = 0x40;
					PORTE_DIRSET = 0xC0;
				}
				break;
				
				case BDCM_STATE_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTE_OUTSET = 0x80;
					PORTE_OUTCLR = 0x40;
					PORTE_DIRSET = 0xC0;
				}
				break;
				
				case BDCM_STATE_BREAK:
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
				case BDCM_STATE_COAST:
				{
					//IN1: 0; IN2: 0
					PORTE_OUTCLR = 0x03;
					PORTE_DIRSET = 0x03;
				}
				break;
				
				case BDCM_STATE_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTE_OUTCLR = 0x02;
					PORTE_OUTSET = 0x01;
					PORTE_DIRSET = 0x03;
				}
				break;
				
				case BDCM_STATE_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTE_OUTSET = 0x02;
					PORTE_OUTCLR = 0x01;
					PORTE_DIRSET = 0x03;
				}
				break;
				
				case BDCM_STATE_BREAK:
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
			return BDCM_STATE_ERROR;
	}
	
	switch(data)
	{
		case 0:
		{
			return BDCM_STATE_COAST;
		}
		case 1:
		{
			return BDCM_STATE_REVERSE;
		}
		case 2:
		{
			return BDCM_STATE_FORWARD;
		}
		default:
		{
			return BDCM_STATE_BREAK;
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
	unsigned char tag;
	unsigned char cmd = 0;
	unsigned char data;
	unsigned short param;
	bool validCmd = true;

	//command format:
	// C cmdNumber param0 param1 param2 param3 param4 param5

	//1st char
	tag = readInputBuffer();
	printString("TAG:");printHex(cmd);printString("\r\n");

	if((tag == 'C') || (tag == 'c'))
	{
		//2nd char
		data = readInputBuffer();
		if(data == ' ')
		{
			unsigned char c;
			
			mmdCommand.command = COMMAND_INVALID;
			mmdCommand.parameterAmount = 0;
			for(unsigned char i = 0; i<MMD_MAX_COMMAND_PARAMETERS; i++) {
				mmdCommand.parameters[i] = 0;
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
				for(index=0; index<MMD_MAX_COMMAND_PARAMETERS; index++)
				{
					unsigned short p = 0;
					//read a parameter
					for(;;)
					{
						c = readInputBuffer();
						if((c >= '0') && (c <= '9')) {
							p =  p * 10 + c - '0';
						}
						else if(0x0D == c) {
							mmdCommand.parameters[index] = p;
							mmdCommand.parameterAmount++;
							break; //end of a command
						}
						else if(' ' == c) {
							mmdCommand.parameters[index] = p;
							mmdCommand.parameterAmount++;
							break; //command number ends.
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
				if(index >= MMD_MAX_COMMAND_PARAMETERS) {
					writeOutputBufferString("Too many parameters\r\n");
					validCmd = false;
				}
			}
		}
		else {
			validCmd = false;
		}
	}

	if(!validCmd) {
		mmdCommand.command = COMMAND_INVALID;
		mmdCommand.parameterAmount = 0;
		clearInputBuffer();
		writeOutputBufferString("Invalid command\r\n");
	}
	else {
		switch(cmd)
		{
		case COMMAND_OPT_POWER_ON:
		case COMMAND_OPT_POWER_OFF:
		case COMMAND_OPT_POWER_QUERY:
		case COMMAND_STEPPERS_POWER_ON:
		case COMMAND_STEPPERS_POWER_OFF:
		case COMMAND_STEPPERS_POWER_QUERY:
		case COMMAND_DCM_POWER_ON:
		case COMMAND_DCM_POWER_OFF:
		case COMMAND_DCM_POWER_QUERY:
		case COMMAND_BDCS_POWER_ON:
		case COMMAND_BDCS_POWER_OFF:
		case COMMAND_BDCS_POWER_QUERY:
		case COMMAND_BDC_COAST:
		case COMMAND_BDC_REVERSE:
		case COMMAND_BDC_FORWARD:
		case COMMAND_BDC_BREAK:
		case COMMAND_BDC_QUERY:
		case COMMAND_STEPPER_STEP_RESOLUTION_QUERY:
		case COMMAND_STEPPER_CONFIG_STEP:
		case COMMAND_STEPPER_ACCELERATION_BUFFER:
		case COMMAND_STEPPER_ACCELERATION_BUFFER_DECREMENT:
		case COMMAND_STEPPER_DECELERATION_BUFFER:
		case COMMAND_STEPPER_DECELERATION_BUFFER_INCREMENT:
		case COMMAND_STEPPER_ENABLE:
		case COMMAND_STEPPER_DIR:
		case COMMAND_STEPPER_STEPS:
		case COMMAND_STEPPER_RUN:
		case COMMAND_STEPPER_CONFIG_HOME:
		case COMMAND_STEPPER_QUREY:
		case COMMAND_LOCATOR_QUERY:
			mmdCommand.command = cmd;
			mmdCommand.state = STARTING_COMMAND;
			break;
		default:
			mmdCommand.command = COMMAND_INVALID;
			mmdCommand.state = AWAITING_COMMAND;
			writeOutputBufferString("Unknown command\r\n");
			break;
		}
	}
}

static void MMD_run_command()
{
	if(mmdCommand.state == STARTING_COMMAND)
	{
		//prepare for execution.
		switch(mmdCommand.command)
		{
			case COMMAND_OPT_POWER_ON:
			case COMMAND_OPT_POWER_OFF:
			case COMMAND_OPT_POWER_QUERY:
			case COMMAND_STEPPERS_POWER_ON:
			case COMMAND_STEPPERS_POWER_OFF:
			case COMMAND_STEPPERS_POWER_QUERY:
			{
				if(mmdCommand.parameterAmount > 0) {
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case COMMAND_DCM_POWER_ON:
			case COMMAND_DCM_POWER_OFF:
			case COMMAND_DCM_POWER_QUERY:
			{
				if(mmdCommand.parameterAmount != 1){
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_DIRECT_CURRENT_MOTORS_AMOUNT) {
					writeOutputBufferString(STR_DCM_INDEX_OUT_OF_SCOPE);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case COMMAND_BDCS_POWER_ON:
			case COMMAND_BDCS_POWER_OFF:
			case COMMAND_BDCS_POWER_QUERY:
			{
				if(mmdCommand.parameterAmount > 0) {
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case COMMAND_BDC_COAST:
			case COMMAND_BDC_REVERSE:
			case COMMAND_BDC_FORWARD:
			case COMMAND_BDC_BREAK:
			case COMMAND_BDC_QUERY:
			{
				if(mmdCommand.parameterAmount != 1){
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_BI_DIRECTION_DIRECT_CURRENT_MOTORS_AMOUNT) {
					writeOutputBufferString(STR_BDC_INDEX_OUT_OF_SCOPE);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case COMMAND_STEPPER_STEP_RESOLUTION_QUERY:
			{
				if(mmdCommand.parameterAmount > 0) {
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;

			case COMMAND_STEPPER_CONFIG_STEP:
			{
				if(mmdCommand.parameterAmount != 3){
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_STEPPERS_AMOUNT) {
					writeOutputBufferString(STR_STEPPER_INDEX_OUT_OF_SCOPE);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;

			case COMMAND_STEPPER_ACCELERATION_BUFFER:
			case COMMAND_STEPPER_ACCELERATION_BUFFER_DECREMENT:
			case COMMAND_STEPPER_DECELERATION_BUFFER:
			case COMMAND_STEPPER_DECELERATION_BUFFER_INCREMENT:
			case COMMAND_STEPPER_ENABLE:
			case COMMAND_STEPPER_DIR:
			{
				if(mmdCommand.parameterAmount != 2){
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_STEPPERS_AMOUNT) {
					writeOutputBufferString(STR_STEPPER_INDEX_OUT_OF_SCOPE);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;

			case COMMAND_STEPPER_STEPS:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				
				if(mmdCommand.parameterAmount != 2){
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(stepperIndex >= MMD_STEPPERS_AMOUNT) {
					writeOutputBufferString(STR_STEPPER_INDEX_OUT_OF_SCOPE);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.steppersData[stepperIndex].state == STEPPER_STATE_UNKNOWN_POSITION) {
					writeOutputBufferString(STR_STEPPER_NOT_POSITIONED);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;

			case COMMAND_STEPPER_RUN:
			{
				if(mmdCommand.parameterAmount > 0) {
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;

			case COMMAND_STEPPER_CONFIG_HOME:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				unsigned char locatorIndex = mmdCommand.parameters[1];
				unsigned char lineNumberStart = mmdCommand.parameters[2];
				unsigned char lineNumberTerminal = mmdCommand.parameters[3];
				
				if(mmdCommand.parameterAmount != 4){
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(stepperIndex >= MMD_STEPPERS_AMOUNT) {
					writeOutputBufferString(STR_STEPPER_INDEX_OUT_OF_SCOPE);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(locatorIndex >= MMD_LOCATOR_AMOUNT) {
					writeOutputBufferString(STR_LOCATOR_INDEX_OUT_OF_SCOPE);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if((lineNumberStart < 1) || (lineNumberStart > 8)) {
					writeOutputBufferString(STR_LOCATOR_LINE_INDEX_OUT_OF_SCOPE);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if((lineNumberTerminal < 1) || (lineNumberTerminal > 8)) {
					writeOutputBufferString(STR_LOCATOR_LINE_INDEX_OUT_OF_SCOPE);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(lineNumberStart == lineNumberTerminal) {
					writeOutputBufferString(STR_LOCATOR_LINE_INDEX_DUPLICATE);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			
			case COMMAND_STEPPER_QUREY:
			{
				if(mmdCommand.parameterAmount != 1){
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_STEPPERS_AMOUNT) {
					writeOutputBufferString(STR_STEPPER_INDEX_OUT_OF_SCOPE);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case COMMAND_LOCATOR_QUERY:
			{
				if(mmdCommand.parameterAmount != 1){
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_LOCATOR_AMOUNT) {
					writeOutputBufferString(STR_LOCATOR_LINE_INDEX_OUT_OF_SCOPE);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;

			default:
			{
				writeOutputBufferString("Unknown command\r\n");
				clearInputBuffer();
				mmdCommand.state = AWAITING_COMMAND;

			}
			break;
		}
	}
	else if(mmdCommand.state == EXECUTING_COMMAND)
	{
		switch(mmdCommand.command)
		{
			case COMMAND_OPT_POWER_ON:
			{
				MMD_power_on_opt(true);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_OPT_POWER_OFF:
			{
				MMD_power_on_opt(false);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_OPT_POWER_QUERY:
			{
				if(MMD_is_opt_powered_on())
					writeOutputBufferString("OPT is powered on\r\n");
				else
					writeOutputBufferString("OPT is powered off\r\n");
					
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_STEPPERS_POWER_ON:
			{
				MMD_power_on_steppers(true);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_STEPPERS_POWER_OFF:
			{
				MMD_power_on_steppers(false);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_STEPPERS_POWER_QUERY:
			{
				if(MMD_are_steppers_powered_on())
					writeOutputBufferString("Steppers are powered on\r\n");
				else
					writeOutputBufferString("Steppers are powered off\r\n");
					
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_DCM_POWER_ON:
			{
				MMD_power_on_dcm(mmdCommand.parameters[0], true);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_DCM_POWER_OFF:
			{
				MMD_power_on_dcm(mmdCommand.parameters[0], false);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_DCM_POWER_QUERY:
			{
				writeOutputBufferString("DCM ");
				writeOutputBufferHex(mmdCommand.parameters[0] & 0xff);
				if(MMD_is_dcm_powered_on(mmdCommand.parameters[0])) {
					writeOutputBufferString(" is powered on\r\n");
				}
				else {
					writeOutputBufferString(" is powered off\r\n");
				}
			}
			break;
			
			case COMMAND_BDCS_POWER_ON:
			{
				MMD_power_on_bdcms(true);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_BDCS_POWER_OFF:
			{
				MMD_power_on_bdcms(false);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_BDCS_POWER_QUERY:
			{
				if(MMD_are_bdcms_powered_on()) {
					writeOutputBufferString("BDCMs are powered on\r\n");
				}
				else {
					writeOutputBufferString("BDCMs are powered off\r\n");
				}
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_BDC_COAST:
			{
				MMD_set_bdcm_state(mmdCommand.parameters[0], BDCM_STATE_COAST);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_BDC_REVERSE:
			{
				MMD_set_bdcm_state(mmdCommand.parameters[0], BDCM_STATE_REVERSE);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_BDC_FORWARD:
			{
				MMD_set_bdcm_state(mmdCommand.parameters[0], BDCM_STATE_FORWARD);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_BDC_BREAK:
			{
				MMD_set_bdcm_state(mmdCommand.parameters[0], BDCM_STATE_BREAK);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_BDC_QUERY:
			{
				writeOutputBufferString("BDCM ");
				writeOutputBufferHex(mmdCommand.parameters[0]);
				writeOutputBufferString(" state: ");
				switch(MMD_get_bdcm_state(mmdCommand.parameters[0]))
				{
					case BDCM_STATE_COAST:
						writeOutputBufferString("COAST\r\n");
						break;
					case BDCM_STATE_REVERSE:
						writeOutputBufferString("REVERSE\r\n");
						break;
					case BDCM_STATE_FORWARD:
						writeOutputBufferString("FORWARD\r\n");
						break;
					case BDCM_STATE_BREAK:
						writeOutputBufferString("BREAK\r\n");
						break;
					default:
						writeOutputBufferString("ERROR\r\n");
						break;
				}
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_STEPPER_STEP_RESOLUTION_QUERY:
			{
				unsigned short resolution = MMD_stepper_resolution();

				writeOutputBufferString("Resolution: ");
				writeOutputBufferHex(resolution >> 8);
				writeOutputBufferString(" ");
				writeOutputBufferHex(resolution & 0xff);
				writeOutputBufferString("\r\n");
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_CONFIG_STEP:
			{
				MMD_stepper_config_step(mmdCommand.parameters[0], mmdCommand.parameters[1], mmdCommand.parameters[2]);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_ACCELERATION_BUFFER:
			{
				MMD_stepper_set_acceleration_buffer(mmdCommand.parameters[0], mmdCommand.parameters[1]);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_ACCELERATION_BUFFER_DECREMENT:
			{
				MMD_stepper_set_acceleration_buffer_decrement(mmdCommand.parameters[0], mmdCommand.parameters[1]);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_DECELERATION_BUFFER:
			{
				MMD_stepper_set_deceleration_buffer(mmdCommand.parameters[0], mmdCommand.parameters[1]);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_DECELERATION_BUFFER_INCREMENT:
			{
				MMD_stepper_set_deceleration_buffer_increment(mmdCommand.parameters[0], mmdCommand.parameters[1]);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_ENABLE:
			{
				unsigned char index = mmdCommand.parameters[0];
				bool enable = mmdCommand.parameters[1] != 0;
				
				MMD_stepper_enable(index, enable);
				mmdCommand.steppersData[index].enabled = enable;
				if(!enable) {
					mmdCommand.steppersData[index].state = STEPPER_STATE_UNKNOWN_POSITION;
				}
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_DIR:
			{
				MMD_stepper_forward(mmdCommand.parameters[0], mmdCommand.parameters[1] != 0);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_STEPS:
			{
				MMD_stepper_set_steps(mmdCommand.parameters[0], mmdCommand.parameters[1] != 0);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_RUN:
			{
				MMD_steppers_run();
			}
			break;

			case COMMAND_STEPPER_CONFIG_HOME:
			{
				mmdCommand.steppersData[mmdCommand.parameters[0]].locatorIndex = mmdCommand.parameters[1];
				mmdCommand.steppersData[mmdCommand.parameters[0]].locatorLineNumberStart = mmdCommand.parameters[2];
				mmdCommand.steppersData[mmdCommand.parameters[0]].locatorLineNumberTerminal = mmdCommand.parameters[3];
				mmdCommand.steppersData[mmdCommand.parameters[0]].state = STEPPER_STATE_APPROACHING_HOME;
			}
			break;

			case COMMAND_STEPPER_QUREY:
			{
				MMD_stepper_query(mmdCommand.parameters[0]);
			}
			break;

			case COMMAND_LOCATOR_QUERY:
			{
				MMD_locator_query(mmdCommand.parameters[0]);
			}
			break;

			default:
			{
				writeOutputBufferString(STR_UNKNOWN_COMMAND);
			}
			break;
		}
	}
}

static void MMD_check_status()
{
	
}

void ecd300MixedMotorDrivers()
{
	unsigned char c;

	Invenco_init();
	MMD_init();

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

