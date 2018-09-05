#include "Invenco_lib.h"

#define MMD_LOCATOR_HUB_AMOUNT 5
#define MMD_STEPPERS_AMOUNT 5
#define MMD_DIRECT_CURRENT_MOTORS_AMOUNT 2
#define MMD_BI_DIRECTION_DIRECT_CURRENT_MOTORS_AMOUNT 6
#define MMD_MAX_COMMAND_PARAMETERS 6

enum MMD_command_e
{
	INVALID_COMMAND = 0,
	OPT_POWER_ON = 10,			// C 10
	OPT_POWER_OFF = 11,			// C 11
	OPT_POWER_QUERY = 12,		// C 12
	STEPPERS_POWER_ON = 20,		// C 13
	STEPPERS_POWER_OFF = 21,	// C 14
	STEPPERS_POWER_QUERY = 22,  // C 15
	DCM_POWER_ON = 30,			// C 30 dcmIndex
	DCM_POWER_OFF = 31,			// C 31 dcmIndex
	DCM_POWER_QUERY = 32,		// C 32 dcmIndex
	BDCS_POWER_ON = 40,			// C 40
	BDCS_POWER_OFF = 41,		// C 41
	BDCS_POWER_QUERY = 42,		// C 42
	BDC_COAST = 43,				// C 43 bdcIndex
	BDC_REVERSE = 44,			// C 44 bdcIndex
	BDC_FORWARD = 45,			// C 45 bdcIndex
	BDC_BREAK = 46,				// C 46 bdcIndex
	BDC_QUERY = 47,				// C 47 bdcIndex
	STEPPER_STEP_RESOLUTION_QUERY = 50,
	STEPPER_STEP_DURATION = 51,
	STEPPER_ACCELERATION_BUFFER = 52,
	STEPPER_ACCELERATION_BUFFER_DECREMENT = 53,
	STEPPER_DECELERATION_BUFFER = 54,
	STEPPER_DECELERATION_BUFFER_INCREMENT = 55,
	STEPPER_ENABLE = 56,
	STEPPER_DIR = 57,
	STEPPER_STEPS = 58,
	STEPPER_HOME = 59,
	STEPPER_QUREY = 60,
	LOCATOR_QUERY = 100
};

enum MMD_stepper_speed
{
	STEPPER_IDLE = 0,
	STEPPER_ACCELERATING,
	STEPPER_CRUISING,
	STEPPER_DECELERATING
};

enum MMD_stepper_step_phase
{
	STEP_CLk_LOW = 0,
	STEP_CLK_HIGH,
	STEP_DELAY,
	STEP_FINISH
};

struct MMD_stepper_data
{
	bool enabled;
	bool forward;
	enum MMD_stepper_speed speed;

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
	unsigned char locatorHubs[MMD_LOCATOR_HUB_AMOUNT];
	
	//steppers
	bool steppersArePowered;
	bool steppersEnabled[MMD_STEPPERS_AMOUNT];
	bool steppersForward[MMD_STEPPERS_AMOUNT];
	enum MMD_stepper_speed steppersSpeed[MMD_STEPPERS_AMOUNT];
	
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
		mmdCommand.steppersData[index].speed = STEPPER_IDLE;

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
		mmdCommand.steppersData[index].stepPhase = STEP_FINISH;
	}

	//status
	mmdStatus.isMainPowerOk	= false;
	mmdStatus.isMainFuseOk = false;
	mmdStatus.isOptPowered = false;
	mmdStatus.isOptPowered = false;
	for(index=0; index<MMD_LOCATOR_HUB_AMOUNT; index++) {
		mmdStatus.locatorHubs[index] = 0;
	}
	mmdStatus.steppersArePowered = false;
	for(index=0; index<MMD_STEPPERS_AMOUNT; index++) {
		mmdStatus.steppersEnabled[index] = false;
		mmdStatus.steppersForward[index] = true;
		mmdStatus.steppersSpeed[index] = STEPPER_IDLE;
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
			mmdCommand.steppersData[0].forward = forward;
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
			mmdCommand.steppersData[1].forward = forward;
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
			mmdCommand.steppersData[2].forward = forward;
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
			mmdCommand.steppersData[3].forward = forward;
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
			mmdCommand.steppersData[4].forward = forward;
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
			mmdCommand.steppersData[0].enabled = enable;
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
			mmdCommand.steppersData[1].enabled = enable;
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
			mmdCommand.steppersData[2].enabled = enable;
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
			mmdCommand.steppersData[3].enabled = enable;
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
			mmdCommand.steppersData[4].enabled = enable;
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
static void MMD_stepper_clocks(unsigned char stepperIndex, unsigned short steps)
{
	unsigned short decelerationSteps;
	
	if(stepperIndex >= MMD_STEPPERS_AMOUNT) {
		return;
	}

	mmdCommand.steppersData[stepperIndex].totalSteps = steps;

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
			
			mmdCommand.command = INVALID_COMMAND;
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
		mmdCommand.command = INVALID_COMMAND;
		mmdCommand.parameterAmount = 0;
		clearInputBuffer();
		writeOutputBufferString("Invalid command\r\n");
	}
	else {
		switch(cmd)
		{
		case OPT_POWER_ON:
		case OPT_POWER_OFF:
		case OPT_POWER_QUERY:
		case STEPPERS_POWER_ON:
		case STEPPERS_POWER_OFF:
		case STEPPERS_POWER_QUERY:
		case DCM_POWER_ON:
		case DCM_POWER_OFF:
		case DCM_POWER_QUERY:
		case BDCS_POWER_ON:
		case BDCS_POWER_OFF:
		case BDCS_POWER_QUERY:
		case BDC_COAST:
		case BDC_REVERSE:
		case BDC_FORWARD:
		case BDC_BREAK:
		case BDC_QUERY:
		case STEPPER_STEP_RESOLUTION_QUERY:
		case STEPPER_STEP_DURATION:
		case STEPPER_ACCELERATION_BUFFER:
		case STEPPER_ACCELERATION_BUFFER_DECREMENT:
		case STEPPER_DECELERATION_BUFFER:
		case STEPPER_DECELERATION_BUFFER_INCREMENT:
		case STEPPER_ENABLE:
		case STEPPER_DIR:
		case STEPPER_STEPS:
		case STEPPER_HOME:
		case STEPPER_QUREY:
		case LOCATOR_QUERY:
			mmdCommand.command = cmd;
			mmdCommand.state = STARTING_COMMAND;
			break;
		default:
			mmdCommand.command = INVALID_COMMAND;
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
			case OPT_POWER_ON:
			case OPT_POWER_OFF:
			case OPT_POWER_QUERY:
			case STEPPERS_POWER_ON:
			case STEPPERS_POWER_OFF:
			case STEPPERS_POWER_QUERY:
			{
				if(mmdCommand.parameterAmount > 0) {
					writeOutputBufferString("Invalid parameter\r\n");
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case DCM_POWER_ON:
			case DCM_POWER_OFF:
			case DCM_POWER_QUERY:
			{
				if(mmdCommand.parameterAmount != 1){
					writeOutputBufferString("Invalid parameter\r\n");
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_DIRECT_CURRENT_MOTORS_AMOUNT) {
					writeOutputBufferString("Invalid parameter\r\n");
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case BDCS_POWER_ON:
			case BDCS_POWER_OFF:
			case BDCS_POWER_QUERY:
			{
				if(mmdCommand.parameterAmount > 0) {
					writeOutputBufferString("Invalid parameter\r\n");
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case BDC_COAST:
			case BDC_REVERSE:
			case BDC_FORWARD:
			case BDC_BREAK:
			case BDC_QUERY:
			{
				if(mmdCommand.parameterAmount != 1){
					writeOutputBufferString("Invalid parameter\r\n");
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_BI_DIRECTION_DIRECT_CURRENT_MOTORS_AMOUNT) {
					writeOutputBufferString("Invalid parameter\r\n");
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case STEPPER_STEP_RESOLUTION_QUERY:
			case STEPPER_STEP_DURATION:
			case STEPPER_ACCELERATION_BUFFER:
			case STEPPER_ACCELERATION_BUFFER_DECREMENT:
			case STEPPER_DECELERATION_BUFFER:
			case STEPPER_DECELERATION_BUFFER_INCREMENT:
			case STEPPER_ENABLE:
			case STEPPER_DIR:
			case STEPPER_STEPS:
			case STEPPER_HOME:
			case STEPPER_QUREY:
			case LOCATOR_QUERY:
		}
	}
	else if(mmdCommand.state == EXECUTING_COMMAND)
	{
		switch(mmdCommand.command)
		{
			case OPT_POWER_ON:
			{
				MMD_power_on_opt(true);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case OPT_POWER_OFF:
			{
				MMD_power_on_opt(false);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case OPT_POWER_QUERY:
			{
				if(MMD_is_opt_powered_on())
					writeOutputBufferString("OPT is powered on\r\n");
				else
					writeOutputBufferString("OPT is powered off\r\n");
					
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case STEPPERS_POWER_ON:
			{
				MMD_power_on_steppers(true);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case STEPPERS_POWER_OFF:
			{
				MMD_power_on_steppers(false);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case STEPPERS_POWER_QUERY:
			{
				if(MMD_are_steppers_powered_on())
					writeOutputBufferString("Steppers are powered on\r\n");
				else
					writeOutputBufferString("Steppers are powered off\r\n");
					
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case DCM_POWER_ON:
			{
				MMD_power_on_dcm(mmdCommand.parameters[0], true);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case DCM_POWER_OFF:
			{
				MMD_power_on_dcm(mmdCommand.parameters[0], false);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case DCM_POWER_QUERY:
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
			
			case BDCS_POWER_ON:
			{
				MMD_power_on_bdcms(true);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case BDCS_POWER_OFF:
			{
				MMD_power_on_bdcms(false);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case BDCS_POWER_QUERY:
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
			
			case BDC_COAST:
			{
				MMD_set_bdcm_state(mmdCommand.parameters[0], BDCM_STATE_COAST);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case BDC_REVERSE:
			{
				MMD_set_bdcm_state(mmdCommand.parameters[0], BDCM_STATE_REVERSE);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case BDC_FORWARD:
			{
				MMD_set_bdcm_state(mmdCommand.parameters[0], BDCM_STATE_FORWARD);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case BDC_BREAK:
			{
				MMD_set_bdcm_state(mmdCommand.parameters[0], BDCM_STATE_BREAK);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case BDC_QUERY:
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
			}
			break;
			
			case STEPPER_STEP_RESOLUTION_QUERY:
			case STEPPER_STEP_DURATION:
			case STEPPER_ACCELERATION_BUFFER:
			case STEPPER_ACCELERATION_BUFFER_DECREMENT:
			case STEPPER_DECELERATION_BUFFER:
			case STEPPER_DECELERATION_BUFFER_INCREMENT:
			case STEPPER_ENABLE:
			case STEPPER_DIR:
			case STEPPER_STEPS:
			case STEPPER_HOME:
			case STEPPER_QUREY:
			case LOCATOR_QUERY:
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

