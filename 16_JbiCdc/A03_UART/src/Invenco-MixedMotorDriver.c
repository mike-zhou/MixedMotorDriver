#include "Invenco_lib.h"

#define MMD_PRODUCT_NAME "Mixed_Motor_Drivers_HV1.0_SV1.0"

#define MMD_LOCATOR_AMOUNT 8
#define MMD_STEPPERS_AMOUNT 5
#define MMD_DIRECT_CURRENT_MOTORS_AMOUNT 2
#define MMD_BI_DIRECTION_DIRECT_CURRENT_MOTORS_AMOUNT 6
#define MMD_MAX_COMMAND_PARAMETERS 6
#define MMD_ABSOLUTE_HOME_OFFSET_STEPS 40

enum MMD_command_e
{
	COMMAND_INVALID = 0,
	COMMAND_QUREY_NAME = 1,				// C 1 cmdId
	COMMAND_DEVICE_POWER_QUERY,			// C 2 cmdId
	COMMAND_DEVICE_FUSE_QUERY,			// C 3 cmdId
	COMMAND_DEVICE_DELAY,				// C 4 clks cmdId
	COMMAND_LOCATOR_INTERVAL,			// C 5 clks cmdId
	COMMAND_OPT_POWER_ON = 10,			// C 10 cmdId
	COMMAND_OPT_POWER_OFF = 11,			// C 11 cmdId
	COMMAND_OPT_POWER_QUERY = 12,		// C 12 cmdId
	COMMAND_STEPPERS_POWER_ON = 20,		// C 20 cmdId
	COMMAND_STEPPERS_POWER_OFF = 21,	// C 21 cmdId
	COMMAND_STEPPERS_POWER_QUERY = 22,  // C 22 cmdId
	COMMAND_DCM_POWER_ON = 30,			// C 30 dcmIndex cmdId
	COMMAND_DCM_POWER_OFF = 31,			// C 31 dcmIndex cmdId
	COMMAND_DCM_POWER_QUERY = 32,		// C 32 dcmIndex cmdId
	COMMAND_BDCS_POWER_ON = 40,			// C 40 cmdId
	COMMAND_BDCS_POWER_OFF = 41,		// C 41 cmdId
	COMMAND_BDCS_POWER_QUERY = 42,		// C 42 cmdId
	COMMAND_BDC_COAST = 43,				// C 43 bdcIndex cmdId
	COMMAND_BDC_REVERSE = 44,			// C 44 bdcIndex lowClks highClks cycles cmdId
	COMMAND_BDC_FORWARD = 45,			// C 45 bdcIndex lowClks highClks cycles cmdId
	COMMAND_BDC_BREAK = 46,				// C 46 bdcIndex cmdId
	COMMAND_BDC_QUERY = 47,				// C 47 bdcIndex cmdId
	COMMAND_STEPPER_STEP_RESOLUTION_QUERY = 50,			// C 50 cmdId
	COMMAND_STEPPER_CONFIG_STEP = 51,					// C 51 stepperIndex lowClks highClks cmdId
	COMMAND_STEPPER_ACCELERATION_BUFFER = 52,			// C 52 stepperIndex buffer cmdId
	COMMAND_STEPPER_ACCELERATION_BUFFER_DECREMENT = 53,	// C 53 stepperIndex decrement cmdId
	COMMAND_STEPPER_DECELERATION_BUFFER = 54,			// C 54 stepperIndex buffer cmdId
	COMMAND_STEPPER_DECELERATION_BUFFER_INCREMENT = 55,	// C 55 stepperIndex increment cmdId
	COMMAND_STEPPER_ENABLE = 56,		// C 56 stepperIndex 1/0 cmdId
	COMMAND_STEPPER_DIR = 57,			// C 57 stepperIndex 1/0 cmdId
	COMMAND_STEPPER_STEPS = 58,			// C 58 stepperIndex steps cmdId
	COMMAND_STEPPER_RUN = 59,			// c 59 stepperIndex cmdId
	COMMAND_STEPPER_CONFIG_HOME = 60,	// C 60 stepperIndex locatorIndex lineNumberStart lineNumberTerminal cmdId
	COMMAND_STEPPER_QUREY = 61,			// C 61 stepperIndex cmdId
	COMMAND_STEPPER_SET_STATE = 62,		// C 62 stepperIndex state cmdId
	COMMAND_LOCATOR_QUERY = 100			// C 100 locatorHubIndex cmdId
};

enum MMD_stepper_state
{
	STEPPER_STATE_UNKNOWN_POSITION = 0,
	STEPPER_STATE_APPROACHING_HOME_LOCATOR,
	STEPPER_STATE_LEAVING_HOME_LOCATOR,
	STEPPER_STATE_GO_HOME,
	STEPPER_STATE_KNOWN_POSITION,
	STEPPER_STATE_ACCELERATING,
	STEPPER_STATE_CRUISING,
	STEPPER_STATE_DECELERATING
};

enum MMD_stepper_step_phase
{
	STEP_PHASE_FINISH = 0,
	STEP_PHASE_CLK_LOW,
	STEP_PHASE_CLK_HIGH,
	STEP_PHASE_DELAY
};

static const char * STR_CARRIAGE_RETURN = "\r\n";
static const char * STR_INVALID_COMMAND = "\"error\":\"invalid command\"";
static const char * STR_TOO_MANY_PARAMETERS = "\"error\":\"too many parameters\"";
static const char * STR_UNKNOWN_COMMAND = "\"error\":\"unknown command\"";
static const char * STR_WRONG_COMMAND_FORMAT = "\"error\":\"wrong command format\"";
static const char * STR_INVALID_PARAMETER = "\"error\":\"invalid parameter\"";
static const char * STR_WRONG_PARAMETER_AMOUNT = "\"error\":\"wrong parameter amount\"";
static const char * STR_EXCESSIVE_DELAY = "\"error\":\"delay value exceeds limit\"";
static const char * STR_BDC_WRONG_PHASE_STATE = "\"error\":\"wrong phase state\"";
static const char * STR_STEPPER_INDEX_OUT_OF_SCOPE = "\"error\":\"stepper index is out of scope\"";
static const char * STR_STEPPER_NOT_POSITIONED = "\"error\":\"stepper has not been positioned\"";
static const char * STR_STEPPER_WRONG_STATE = "\"error\":\"wrong stepper state\"";
static const char * STR_STEPPER_UNDER_SCOPE = "\"error\":\"stepper is under scope\"";
static const char * STR_STEPPER_OVER_SCOPE = "\"error\":\"stepper is over scope\"";
static const char * STR_STEPPER_STATE = "\"state\":";
static const char * STR_STEPPER_LOCATOR_INDEX = "\"locatorIndex\":";
static const char * STR_STEPPER_LOCATOR_LINE_NUMBER_START = "\"locatorLineNumberStart\":";
static const char * STR_STEPPER_LOCATOR_LINE_NUMBER_TERMINAL = "\"locatorLineNumberTerminal\":";
static const char * STR_STEPPER_IS_ENABLED = "\"enabled\":";
static const char * STR_STEPPER_FORWARD = "\"forward\":";
static const char * STR_STEPPER_HOME_OFFSET = "\"homeOffset\":";
static const char * STR_STEPPER_TOTAL_STEPS = "\"totalSteps\":";
static const char * STR_STEPPER_CURRENT_STEP_INDEX = "\"currentStepIndex\":";
static const char * STR_STEPPER_DECELERATION_STARTING_INDEX = "\"decelerationStartingIndex\":";
static const char * STR_STEPPER_ACCELERATION_BUFFER = "\"accelerationBuffer\":";
static const char * STR_STEPPER_ACCELERATION_DECREMENT = "\"accelerationDecrement\":";
static const char * STR_STEPPER_DECELERATION_BUFFER = "\"decelerationBuffer\":";
static const char * STR_STEPPER_DECELERATION_INCREMENT = "\"decelerationIncrement\":";
static const char * STR_STEPPER_PHASE_LOW_CLOCKS = "\"lowClks\":";
static const char * STR_STEPPER_PHASE_HIGH_CLOCKS = "\"highClks\":";
static const char * STR_LOCATOR_INDEX_OUT_OF_SCOPE = "\"error\":\"locator index is out of scope\"";
static const char * STR_LOCATOR_LINE_INDEX_OUT_OF_SCOPE = "\"error\":\"locator line index is out of scope\"";
static const char * STR_LOCATOR_LINE_INDEX_DUPLICATE = "\"error\":\"duplicated locator line index\"";
static const char * STR_DCM_INDEX_OUT_OF_SCOPE = "\"error\":\"DCM index is out of scope\"";
static const char * STR_BDC_INDEX_OUT_OF_SCOPE = "\"error\":\"BDC index is out of scope\"";


static const char * EVENT_MAIN_POWER_IS_ON = "\"event\":\"main power is on\"\r\n";
static const char * EVENT_MAIN_POWER_IS_OFF = "\"event\":\"main fuse is off\"\r\n";
static const char * EVENT_MAIN_FUSE_IS_ON = "\"event\":\"main fuse is on\"\r\n";
static const char * EVENT_MAIN_FUSE_IS_OFF = "\"event\":\"main fuse is off\"\r\n";
static const char * EVENT_OPT_IS_POWERED_ON = "\"event\":\"OPT is powered on\"\r\n";
static const char * EVENT_OPT_IS_POWERED_OFF = "\"event\":\"OPT is powered off\"\r\n";
static const char * EVENT_DCM_IS_POWERED_ON = "\"event\":\"DCM is powered on\"";
static const char * EVENT_DCM_IS_POWERED_OFF = "\"event\":\"DCM is powered off\"";
static const char * EVENT_BDCS_ARE_POWERED_ON = "\"event\":\"BDCs are powered on\"\r\n";
static const char * EVENT_BDCS_ARE_POWERED_OFF = "\"event\":\"BDCs are powered off\"\r\n";
static const char * EVENT_BDC_COAST = "\"event\":\"BDC coast\"";
static const char * EVENT_BDC_REVERSE = "\"event\":\"BDC reverse\"";
static const char * EVENT_BDC_FORWARD = "\"event\":\"BDC forward\"";
static const char * EVENT_BDC_BREAK = "\"event\":\"BDC break\"";
static const char * EVENT_BDC_WRONT_STATE = "\"event\":\"BDC wrong state\"";
static const char * EVENT_STEPPERS_ARE_POWERED_ON = "\"event\":\"steppers are powered on\"\r\n";
static const char * EVENT_STEPPERS_ARE_POWERED_OFF = "\"event\":\"steppers are powered off\"\r\n";
static const char * EVENT_STEPPER_IS_ENABLED = "\"event\":\"stepper is enabled\"";
static const char * EVENT_STEPPER_IS_DISABLED = "\"event\":\"stepper is disabled\"";
static const char * EVENT_STEPPER_FORWORD = "\"event\":\"stepper forward\"";
static const char * EVENT_STEPPER_BACKWORD = "\"event\":\"stepper backward\"";
static const char * EVENT_STEPPER_STATE_UNKNOWN_POSITION = "\"event\":\"stepper unknown position\"";
static const char * EVENT_STEPPER_STATE_APPROACH_HOME = "\"event\":\"stepper approach home locator\"";
static const char * EVENT_STEPPER_STATE_LEAVE_HOME = "\"event\":\"stepper leave home locator\"";
static const char * EVENT_STEPPER_STATE_GO_HOME = "\"event\":\"stepper go home\"";
static const char * EVENT_STEPPER_STATE_KNOWN_POSITION = "\"event\":\"stepper known position\"";
static const char * EVENT_STEPPER_STATE_ACCELERATE = "\"event\":\"stepper accelerate\"";
static const char * EVENT_STEPPER_STATE_CRUISE = "\"event\":\"stepper cruise\"";
static const char * EVENT_STEPPER_STATE_DECELERATE = "\"event\":\"stepper decelerate\"";
static const char * EVENT_STEPPER_STATE_WRONG_STATE = "\"event\":\"stepper wrong state\"";
static const char * EVENT_LOCATOR = "\"event\":\"locator\"";

static unsigned short mmdCurrentClock;
static unsigned short locatorCheckStamp;
static unsigned short locatorCheckInterval;

struct MMD_stepper_data
{
	enum MMD_stepper_state state;
	unsigned char stepperIndex;
	
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
	unsigned short stepPhaseStartingClock;
	unsigned short stepPhaseLowClocks;
	unsigned short stepPhaseHighClocks;
	enum MMD_stepper_step_phase stepPhase;
};

struct MMD_DeviceDelay
{
	unsigned short totalClks;
	unsigned short startingClk;
};

enum MMD_bdc_phase
{
	BDC_PHASE_FINISH,
	BDC_PHASE_LOW,
	BDC_PHASE_HIGH
};

struct MMD_bdc_data
{
	unsigned char bdcIndex;
	unsigned short lowClks;
	unsigned short highClks;
	unsigned short cycles;
	
	enum MMD_bdc_phase phase;
	unsigned short phaseStartClk;
	unsigned short cycleIndex;
};

struct MMD_command
{
	enum CommandState state;
	enum MMD_command_e command;
	unsigned short parameterAmount;
	unsigned short parameters[MMD_MAX_COMMAND_PARAMETERS]; //command parameters
	
	struct MMD_stepper_data steppersData[MMD_STEPPERS_AMOUNT];
	struct MMD_DeviceDelay deviceDelay;
	struct MMD_bdc_data bdcData;
} mmdCommand;

enum MMD_BDC_STATE
{
	BDC_STATE_ERROR = 0,
	BDC_STATE_COAST,
	BDC_STATE_REVERSE,
	BDC_STATE_FORWARD,
	BDC_STATE_BREAK
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
	bool areSteppersPowered;
	bool stepperEnabled[MMD_STEPPERS_AMOUNT];
	bool stepperForward[MMD_STEPPERS_AMOUNT];
	enum MMD_stepper_state stepperState[MMD_STEPPERS_AMOUNT];
	
	//dcms
	bool dcmPowered[MMD_DIRECT_CURRENT_MOTORS_AMOUNT];
	
	//bdcs
	bool areBdcsPowered;
	enum MMD_BDC_STATE bdcState[MMD_BI_DIRECTION_DIRECT_CURRENT_MOTORS_AMOUNT];

	//command
	enum CommandState cmdState;
} mmdStatus;

static void MMD_pull_up_gpio_inputs(void)
{
	//PORTA pull up
	PORTA_PIN0CTRL = PORT_OPC_PULLUP_gc;
	PORTA_PIN1CTRL = PORT_OPC_PULLUP_gc;
	PORTA_PIN2CTRL = PORT_OPC_PULLUP_gc;
	PORTA_PIN3CTRL = PORT_OPC_PULLUP_gc;
	PORTA_PIN4CTRL = PORT_OPC_PULLUP_gc;
	PORTA_PIN5CTRL = PORT_OPC_PULLUP_gc;
	PORTA_PIN6CTRL = PORT_OPC_PULLUP_gc;
	PORTA_PIN7CTRL = PORT_OPC_PULLUP_gc;

	//PORTB pull up
	PORTB_PIN0CTRL = PORT_OPC_PULLUP_gc;
	PORTB_PIN1CTRL = PORT_OPC_PULLUP_gc;
	PORTB_PIN2CTRL = PORT_OPC_PULLUP_gc;
	PORTB_PIN3CTRL = PORT_OPC_PULLUP_gc;
	PORTB_PIN4CTRL = PORT_OPC_PULLUP_gc;
	PORTB_PIN5CTRL = PORT_OPC_PULLUP_gc;
	PORTB_PIN6CTRL = PORT_OPC_PULLUP_gc;
	PORTB_PIN7CTRL = PORT_OPC_PULLUP_gc;
	
	//PORTC pull up
	PORTC_PIN0CTRL = PORT_OPC_PULLUP_gc;
	PORTC_PIN1CTRL = PORT_OPC_PULLUP_gc;
	PORTC_PIN2CTRL = PORT_OPC_PULLUP_gc;
	PORTC_PIN3CTRL = PORT_OPC_PULLUP_gc;
	PORTC_PIN4CTRL = PORT_OPC_PULLUP_gc;
	PORTC_PIN5CTRL = PORT_OPC_PULLUP_gc;
	PORTC_PIN6CTRL = PORT_OPC_PULLUP_gc;
	PORTC_PIN7CTRL = PORT_OPC_PULLUP_gc;

	//PORTD pull up
	PORTD_PIN4CTRL = PORT_OPC_PULLUP_gc;
	PORTD_PIN5CTRL = PORT_OPC_PULLUP_gc;
	
	//PORTE
	PORTE_PIN0CTRL = PORT_OPC_PULLUP_gc;
	PORTE_PIN1CTRL = PORT_OPC_PULLUP_gc;
	PORTE_PIN2CTRL = PORT_OPC_PULLUP_gc;
	PORTE_PIN3CTRL = PORT_OPC_PULLUP_gc;
	PORTE_PIN4CTRL = PORT_OPC_PULLUP_gc;
	PORTE_PIN5CTRL = PORT_OPC_PULLUP_gc;
	
	//PORTF pull up
	PORTF_PIN0CTRL = PORT_OPC_PULLUP_gc;
	PORTF_PIN1CTRL = PORT_OPC_PULLUP_gc;
	PORTF_PIN2CTRL = PORT_OPC_PULLUP_gc;
	PORTF_PIN3CTRL = PORT_OPC_PULLUP_gc;
	
	//PORTH pull up
	PORTH_PIN0CTRL = PORT_OPC_PULLUP_gc;
	PORTH_PIN2CTRL = PORT_OPC_PULLUP_gc;
	PORTH_PIN4CTRL = PORT_OPC_PULLUP_gc;
	PORTH_PIN6CTRL = PORT_OPC_PULLUP_gc;
	
	//PORTJ pull up
	PORTJ_PIN0CTRL = PORT_OPC_PULLUP_gc;
	PORTJ_PIN1CTRL = PORT_OPC_PULLUP_gc;

	//PORTQ pull up
	PORTQ_PIN2CTRL = PORT_OPC_PULLUP_gc;
	PORTQ_PIN3CTRL = PORT_OPC_PULLUP_gc;

	//PORTR pull up
	PORTR_PIN0CTRL = PORT_OPC_PULLUP_gc;
	PORTR_PIN1CTRL = PORT_OPC_PULLUP_gc;
}

static void MMD_init(void)
{
	unsigned char index;
	
	MMD_pull_up_gpio_inputs();
	
	//command data
	mmdCommand.state = AWAITING_COMMAND;
	mmdCommand.parameterAmount = 0;
	for(index=0; index<MMD_STEPPERS_AMOUNT; index++)
	{
		mmdCommand.steppersData[index].state = STEPPER_STATE_UNKNOWN_POSITION;
		mmdCommand.steppersData[index].stepperIndex = index;
		
		mmdCommand.steppersData[index].locatorIndex =0;
		mmdCommand.steppersData[index].locatorLineNumberTerminal = 0xff;
		mmdCommand.steppersData[index].locatorLineNumberStart = 0xff;

		mmdCommand.steppersData[index].enabled = false;
		mmdCommand.steppersData[index].forward = true;

		mmdCommand.steppersData[index].totalSteps = 0;
		mmdCommand.steppersData[index].currentStepIndex = 0;
		mmdCommand.steppersData[index].decelerationStartingIndex = 0;

		mmdCommand.steppersData[index].accelerationBuffer = 0;
		mmdCommand.steppersData[index].accelerationDecrement = 1;
		mmdCommand.steppersData[index].accelerationLevel = 0;

		mmdCommand.steppersData[index].decelerationBuffer = 0;
		mmdCommand.steppersData[index].decelerationIncrement = 1;
		mmdCommand.steppersData[index].decelerationLevel = 0;

		mmdCommand.steppersData[index].stepPhaseStartingClock = 0;
		mmdCommand.steppersData[index].stepPhaseLowClocks = 1;
		mmdCommand.steppersData[index].stepPhaseHighClocks = 1;
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
	mmdStatus.areSteppersPowered = false;
	for(index=0; index<MMD_STEPPERS_AMOUNT; index++) {
		mmdStatus.stepperEnabled[index] = false;
		mmdStatus.stepperForward[index] = true;
		mmdStatus.stepperState[index] = STEPPER_STATE_UNKNOWN_POSITION;
	}
	for(index=0; index<MMD_DIRECT_CURRENT_MOTORS_AMOUNT; index++){
		mmdStatus.dcmPowered[index] = false;
	}
	mmdStatus.areBdcsPowered = false;
	for(index=0; index<MMD_BI_DIRECTION_DIRECT_CURRENT_MOTORS_AMOUNT; index++) {
		mmdStatus.bdcState[index] = BDC_STATE_COAST;
	}
	mmdStatus.cmdState = AWAITING_COMMAND;
}

// return (mmdCurrentClk - clkStamp)
static unsigned short MMD_elapsed_clocks(unsigned short clkStamp)
{
	if(mmdCurrentClock >= clkStamp) {
		return (mmdCurrentClock - clkStamp);
	}
	else {
		return (0xFFFF - clkStamp + mmdCurrentClock);
	}
}

static inline unsigned short MMD_current_clock(void)
{
	return mmdCurrentClock;
}

// check main power status
// returned value:
//		true: main power is applied
//		false: no main power
static bool MMD_is_main_power_ok(void)
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
static bool MMD_is_main_fuse_ok(void)
{
	//PJ0
	if(PORTJ_IN & 0x01) {
		return true;
	}
	else {
		return false;
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
		{
			if((PORTD_IN & 0x10) == 0)
				return 1;
			else if((PORTD_IN & 0x20) == 0)
				return 2;
			else
				return 0;
		}
		
		case 1: //LC2
		{
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
				return 7 - n + 1;
			}
		}
		
		case 2: //LC3
		{
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
				return 7 - n + 1;
			}
			
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
				return 7 - n + 1;
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
				return 7 - n + 1;
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
				return 7 - n + 1;
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
				return 7 - n + 1;
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
				return 7 - n + 1;
			}
		
		default:
		return 0;
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
static bool MMD_is_opt_powered_on(void)
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
				PORTH_OUTSET = 0x08;
				PORTH_DIRSET = 0x08;
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
				PORTH_OUTCLR = 0x08;
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
			if(PORTH_IN & 0x04) {
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

static bool MMD_are_steppers_powered_on(void)
{
	//PH4
	if(PORTH_IN & 0x10) {
		return false;
	}
	else {
		return true;
	}
}

// stepper forwards if it departs from home
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
			mmdCommand.steppersData[stepperIndex].forward = forward;
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
			mmdCommand.steppersData[stepperIndex].forward = forward;
		}
		break;
		
		case 2: // stepper 3
		{
			//Q12, PK1
			if(forward) {
				PORTK_OUTCLR = 0x02;
			}
			else {
				PORTK_OUTSET = 0x02;
				PORTK_DIRSET = 0x02;
			}
			mmdCommand.steppersData[stepperIndex].forward = forward;
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
			mmdCommand.steppersData[stepperIndex].forward = forward;
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
			mmdCommand.steppersData[stepperIndex].forward = forward;
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
			if(PORTK_IN & 0x02) {
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
	if(stepperIndex >= MMD_STEPPERS_AMOUNT) {
		return;
	}
	
	switch(stepperIndex) 
	{
		case 0: //stepper 1
		{
			//Q5, PQ1
			if(enable) {
				PORTQ_OUTCLR = 0x02;
			}
			else {
				PORTQ_OUTSET = 0x02;
				PORTQ_DIRSET = 0x02;
			}
			mmdCommand.steppersData[stepperIndex].enabled = enable;
			if(!enable) {
				mmdCommand.steppersData[stepperIndex].state = STEPPER_STATE_UNKNOWN_POSITION;
			}
		}
		break;
		
		case 1: //stepper 2
		{
			//Q8, PK5
			if(enable) {
				PORTK_OUTCLR = 0x20;
			}
			else {
				PORTK_OUTSET = 0x20;
				PORTK_DIRSET = 0x20;
			}
			mmdCommand.steppersData[stepperIndex].enabled = enable;
			if(!enable) {
				mmdCommand.steppersData[stepperIndex].state = STEPPER_STATE_UNKNOWN_POSITION;
			}
		}
		break;
		
		case 2: //stepper 3
		{
			//Q11, PK2
			if(enable) {
				PORTK_OUTCLR = 0x04;
			}
			else {
				PORTK_OUTSET = 0x04;
				PORTK_DIRSET = 0x04;
			}
			mmdCommand.steppersData[stepperIndex].enabled = enable;
			if(!enable) {
				mmdCommand.steppersData[stepperIndex].state = STEPPER_STATE_UNKNOWN_POSITION;
			}
		}
		break;
		
		case 3: //stepper 4
		{
			//Q14, PJ7
			if(enable) {
				PORTJ_OUTCLR = 0x80;
			}
			else {
				PORTJ_OUTSET = 0x80;
				PORTJ_DIRSET = 0x80;
			}
			mmdCommand.steppersData[stepperIndex].enabled = enable;
			if(!enable) {
				mmdCommand.steppersData[stepperIndex].state = STEPPER_STATE_UNKNOWN_POSITION;
			}
		}
		break;
		
		case 4: //stepper 5
		{
			//Q31, PJ4
			if(enable) {
				PORTJ_OUTCLR = 0x10;
			}
			else {
				PORTJ_OUTSET = 0x10;
				PORTJ_DIRSET = 0x10;
			}
			mmdCommand.steppersData[stepperIndex].enabled = enable;
			if(!enable) {
				mmdCommand.steppersData[stepperIndex].state = STEPPER_STATE_UNKNOWN_POSITION;
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
			if(PORTQ_IN & 0x02) {
				return false;
			}
			else {
				return true;
			}
		}
		break;
		
		case 1: //stepper 2
		{
			//Q8, PK5
			if(PORTK_IN & 0x20) {
				return false;
			}
			else {
				return true;
			}
		}
		break;
		
		case 2: //stepper 3
		{
			//Q11, PK2
			if(PORTK_IN & 0x04) {
				return false;
			}
			else {
				return true;
			}
		}
		break;
		
		case 3: //stepper 4
		{
			//Q14, PJ7
			if(PORTJ_IN & 0x80) {
				return false;
			}
			else {
				return true;
			}
		}
		break;
		
		case 4: //stepper 5
		{
			//Q31, PJ4
			if(PORTJ_IN & 0x10) {
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

// return the resolution of step length in microseconds.
static unsigned short MMD_stepper_resolution(void)
{
	return counter_clock_length();
}

// set amount of clocks in a single step
static void MMD_stepper_config_step(unsigned char stepperIndex, unsigned short lowClocks, unsigned highClocks)
{
	if(stepperIndex >= MMD_STEPPERS_AMOUNT) {
		return;
	}

	mmdCommand.steppersData[stepperIndex].stepPhaseLowClocks = lowClocks;
	mmdCommand.steppersData[stepperIndex].stepPhaseHighClocks = highClocks;
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
	if(clocks == 0) {
		clocks = 1;
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

	mmdCommand.steppersData[stepperIndex].totalSteps = steps;

	//find out where deceleration should start
	decelerationSteps = mmdCommand.steppersData[stepperIndex].decelerationBuffer / mmdCommand.steppersData[stepperIndex].decelerationIncrement;
	if(decelerationSteps <= (steps/2)) {
		//stepper can cruise
		mmdCommand.steppersData[stepperIndex].decelerationStartingIndex = (steps - decelerationSteps);
		mmdCommand.steppersData[stepperIndex].decelerationLevel = mmdCommand.steppersData[stepperIndex].decelerationIncrement;
	}
	else {
		//no cruise
		mmdCommand.steppersData[stepperIndex].decelerationStartingIndex = steps/2;
		mmdCommand.steppersData[stepperIndex].decelerationLevel = mmdCommand.steppersData[stepperIndex].decelerationIncrement * (decelerationSteps - steps/2);
	}
	
	mmdCommand.steppersData[stepperIndex].state = STEPPER_STATE_ACCELERATING;
	mmdCommand.steppersData[stepperIndex].stepPhase = STEP_PHASE_FINISH;
	mmdCommand.steppersData[stepperIndex].currentStepIndex = 0;
	mmdCommand.steppersData[stepperIndex].accelerationLevel = mmdCommand.steppersData[stepperIndex].accelerationBuffer;
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
static void MMD_power_on_bdcs(bool on)
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

static bool MMD_are_bdcs_powered_on(void)
{
	//read PF6
	return (PORTF_IN & 0x40) == 0;
}

static void MMD_bdc_set_state(unsigned char index, enum MMD_BDC_STATE state)
{
	switch(index)
	{
		case 0: {
			//bdcm1: PE4, PE5
			switch(state)
			{
				case BDC_STATE_COAST:
				{
					//IN1: 0; IN2: 0
					PORTF_OUTCLR = 0x30;
					PORTF_DIRSET = 0x30;
				}
				break;
			
				case BDC_STATE_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTF_OUTCLR = 0x20; 
					PORTF_OUTSET = 0x10;
					PORTF_DIRSET = 0x30;
				}
				break;
			
				case BDC_STATE_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTF_OUTSET = 0x20;
					PORTF_OUTCLR = 0x10;
					PORTF_DIRSET = 0x30;
				}
				break;
			
				case BDC_STATE_BREAK:
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
			//bdcm2: PF2, PF3
			switch(state)
			{
				case BDC_STATE_COAST:
				{
					//IN1: 0; IN2: 0
					PORTF_OUTCLR = 0x0C;
					PORTF_DIRSET = 0x0C;
				}
				break;
				
				case BDC_STATE_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTF_OUTCLR = 0x08;
					PORTF_OUTSET = 0x04;
					PORTF_DIRSET = 0x0C;
				}
				break;
				
				case BDC_STATE_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTF_OUTSET = 0x08;
					PORTF_OUTCLR = 0x04;
					PORTF_DIRSET = 0x0C;
				}
				break;
				
				case BDC_STATE_BREAK:
				{
					//IN1: 1; IN2: 1
					PORTF_OUTSET = 0x0C;
					PORTF_DIRSET = 0x0C;
				}
				break;
				
				default:
				break;
			}
		}
		break;		

		case 2: {
			//bdcm3: PF0, PF1
			switch(state)
			{
				case BDC_STATE_COAST:
				{
					//IN1: 0; IN2: 0
					PORTF_OUTCLR = 0x03;
					PORTF_DIRSET = 0x03;
				}
				break;
				
				case BDC_STATE_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTF_OUTCLR = 0x02;
					PORTF_OUTSET = 0x01;
					PORTF_DIRSET = 0x03;
				}
				break;
				
				case BDC_STATE_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTF_OUTSET = 0x02;
					PORTF_OUTCLR = 0x01;
					PORTF_DIRSET = 0x03;
				}
				break;
				
				case BDC_STATE_BREAK:
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
			//bdcm4: PE4, PE5
			switch(state)
			{
				case BDC_STATE_COAST:
				{
					//IN1: 0; IN2: 0
					PORTE_OUTCLR = 0x30;
					PORTE_DIRSET = 0x30;
				}
				break;
				
				case BDC_STATE_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTE_OUTCLR = 0x20;
					PORTE_OUTSET = 0x10;
					PORTE_DIRSET = 0x30;
				}
				break;
				
				case BDC_STATE_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTE_OUTSET = 0x20;
					PORTE_OUTCLR = 0x10;
					PORTE_DIRSET = 0x30;
				}
				break;
				
				case BDC_STATE_BREAK:
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
			//bdcm5: PE2, PE3
			switch(state)
			{
				case BDC_STATE_COAST:
				{
					//IN1: 0; IN2: 0
					PORTE_OUTCLR = 0x0C;
					PORTE_DIRSET = 0x0C;
				}
				break;
				
				case BDC_STATE_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTE_OUTCLR = 0x08;
					PORTE_OUTSET = 0x04;
					PORTE_DIRSET = 0x0C;
				}
				break;
				
				case BDC_STATE_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTE_OUTSET = 0x08;
					PORTE_OUTCLR = 0x04;
					PORTE_DIRSET = 0x0C;
				}
				break;
				
				case BDC_STATE_BREAK:
				{
					//IN1: 1; IN2: 1
					PORTE_OUTSET = 0x0C;
					PORTE_DIRSET = 0x0C;
				}
				break;
				
				default:
				break;
			}
		}
		break;

		case 5: {
			//bdcm6: PE0, PE1
			switch(state)
			{
				case BDC_STATE_COAST:
				{
					//IN1: 0; IN2: 0
					PORTE_OUTCLR = 0x03;
					PORTE_DIRSET = 0x03;
				}
				break;
				
				case BDC_STATE_REVERSE:
				{
					//IN1: 0; IN2: 1
					PORTE_OUTCLR = 0x02;
					PORTE_OUTSET = 0x01;
					PORTE_DIRSET = 0x03;
				}
				break;
				
				case BDC_STATE_FORWARD:
				{
					//IN1: 1; IN2: 0
					PORTE_OUTSET = 0x02;
					PORTE_OUTCLR = 0x01;
					PORTE_DIRSET = 0x03;
				}
				break;
				
				case BDC_STATE_BREAK:
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

static enum MMD_BDC_STATE MMD_bdc_get_state(unsigned char index)
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
			return BDC_STATE_ERROR;
	}
	
	switch(data)
	{
		case 0:
		{
			return BDC_STATE_COAST;
		}
		case 1:
		{
			return BDC_STATE_REVERSE;
		}
		case 2:
		{
			return BDC_STATE_FORWARD;
		}
		default:
		{
			return BDC_STATE_BREAK;
		}
	}
}

static void mmd_stepper_out_of_scope(struct MMD_stepper_data * pData)
{
	if(pData == NULL) {
		return;
	}
	
	MMD_stepper_enable(pData->stepperIndex, false);
	mmdCommand.state = AWAITING_COMMAND;
}

//write command and parameters to output buffer.
static void mmd_write_succeess_reply() 
{
	writeOutputBufferString("\"command\":\"");
	writeOutputBufferHex(mmdCommand.command);
	writeOutputBufferString("\",\"params\":[");
	//the first parameter
	writeOutputBufferChar('"');
	writeOutputBufferHex(mmdCommand.parameters[0] >> 8);
	writeOutputBufferHex(mmdCommand.parameters[0] & 0xff);
	writeOutputBufferChar('"');
	//other parameters
	for(int i=1; i<mmdCommand.parameterAmount; i++) {
		writeOutputBufferChar(',');
		writeOutputBufferChar('"');
		writeOutputBufferHex(mmdCommand.parameters[i] >> 8);
		writeOutputBufferHex(mmdCommand.parameters[i] & 0xff);
		writeOutputBufferChar('"');
	}
	writeOutputBufferChar(']');
	writeOutputBufferString(STR_CARRIAGE_RETURN);
}

static void mmd_write_reply_header()
{
	writeOutputBufferString("\"command\":\"");
	writeOutputBufferHex(mmdCommand.command);
	writeOutputBufferString("\",\"params\":[");
	//the first parameter
	writeOutputBufferChar('"');
	writeOutputBufferHex(mmdCommand.parameters[0] >> 8);
	writeOutputBufferHex(mmdCommand.parameters[0] & 0xff);
	writeOutputBufferChar('"');
	//other parameters
	for(int i=1; i<mmdCommand.parameterAmount; i++) {
		writeOutputBufferChar(',');
		writeOutputBufferChar('"');
		writeOutputBufferHex(mmdCommand.parameters[i] >> 8);
		writeOutputBufferHex(mmdCommand.parameters[i] & 0xff);
		writeOutputBufferChar('"');
	}
	writeOutputBufferChar(']');
	writeOutputBufferChar(',');
}

// check if stepper is under scope or over scope,
// and cancel current command if out of scope happens.
static void mmd_stepper_check_scope(struct MMD_stepper_data * pData)
{
	if(pData == NULL) {
		return;
	}
	
	switch(pData->state)
	{
		case STEPPER_STATE_UNKNOWN_POSITION:
		case STEPPER_STATE_APPROACHING_HOME_LOCATOR:
		case STEPPER_STATE_LEAVING_HOME_LOCATOR:
		case STEPPER_STATE_GO_HOME:
			//no need to check stepper scope
		break;
		
		default:
		{
			unsigned char lineNumber = MMD_locator_get(pData->locatorIndex);
			
			if(pData->locatorLineNumberStart == lineNumber) {
				// under scope 
				mmd_stepper_out_of_scope(pData);
				
				mmd_write_reply_header();
				writeOutputBufferString(STR_STEPPER_UNDER_SCOPE);
				writeOutputBufferString(",\"index\":\"");
				writeOutputBufferHex(pData->stepperIndex);
				writeOutputBufferChar('"');
				writeOutputBufferString(STR_CARRIAGE_RETURN);
			}
			else if(pData->locatorLineNumberTerminal == lineNumber) {
				// over scope
				mmd_stepper_out_of_scope(pData);
				
				mmd_write_reply_header();
				writeOutputBufferString(STR_STEPPER_OVER_SCOPE);
				writeOutputBufferString(",\"index\":\"");
				writeOutputBufferHex(pData->stepperIndex);
				writeOutputBufferChar('"');
				writeOutputBufferString(STR_CARRIAGE_RETURN);
			}
		}
		break;
	}
}

static void mmd_stepper_approach_home_locator(struct MMD_stepper_data * pData)
{
	if(pData == NULL) {
		return;
	}

	switch(pData->stepPhase)
	{
		case STEP_PHASE_FINISH:
		{
			if(pData->locatorLineNumberStart == MMD_locator_get(pData->locatorIndex)) {
				//home locator is triggered, then switch to the next state
				MMD_stepper_forward(pData->stepperIndex, true);
				pData->state = STEPPER_STATE_LEAVING_HOME_LOCATOR;
			}
			else {
				//continue moving to home
				MMD_stepper_clock_high(pData->stepperIndex, false);
				pData->stepPhase = STEP_PHASE_CLK_LOW;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;

		case STEP_PHASE_CLK_LOW:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= pData->stepPhaseLowClocks) {
				//trigger a rising edge at the end of lower phase
				MMD_stepper_clock_high(pData->stepperIndex, true);
				pData->stepPhase = STEP_PHASE_CLK_HIGH;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;

		case STEP_PHASE_CLK_HIGH:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= pData->stepPhaseHighClocks) {
				MMD_stepper_clock_high(pData->stepperIndex, false);
				pData->stepPhase = STEP_PHASE_DELAY;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;

		case STEP_PHASE_DELAY:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= (pData->accelerationBuffer + pData->decelerationBuffer)) {
				//stepper moved and stopped.
				pData->stepPhase = STEP_PHASE_FINISH;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;

		default:
		break;
	}
}

static void mmd_stepper_leave_home_locator(struct MMD_stepper_data * pData)
{
	if(pData == NULL) {
		return;
	}

	switch(pData->stepPhase)
	{
		case STEP_PHASE_FINISH:
		{
			if(pData->locatorLineNumberStart != MMD_locator_get(pData->locatorIndex)) {
				//stepper home line just changes to high, then switch to the next state
				MMD_stepper_forward(pData->stepperIndex, true);
				pData->state = STEPPER_STATE_GO_HOME;
				pData->currentStepIndex = 0;
			}
			else {
				//continue leaving home locator
				MMD_stepper_clock_high(pData->stepperIndex, false);
				pData->stepPhase = STEP_PHASE_CLK_LOW;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;
		
		case STEP_PHASE_CLK_LOW:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= pData->stepPhaseLowClocks) {
				//trigger a rising edge at the end of lower phase
				MMD_stepper_clock_high(pData->stepperIndex, true);
				pData->stepPhase = STEP_PHASE_CLK_HIGH;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;

		case STEP_PHASE_CLK_HIGH:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= pData->stepPhaseHighClocks) {
				MMD_stepper_clock_high(pData->stepperIndex, false);
				pData->stepPhase = STEP_PHASE_DELAY;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;

		case STEP_PHASE_DELAY:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= (pData->accelerationBuffer + pData->decelerationBuffer)) {
				pData->stepPhase = STEP_PHASE_FINISH;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;

		default:
		break;
	}
}

static void mmd_stepper_go_home(struct MMD_stepper_data * pData)
{
	if(pData == NULL) {
		return;
	}

	switch(pData->stepPhase)
	{
		case STEP_PHASE_FINISH:
		{
			if(pData->currentStepIndex >= MMD_ABSOLUTE_HOME_OFFSET_STEPS) {
				//arrive at home position
				MMD_stepper_forward(pData->stepperIndex, true);
				pData->homeOffset = 0;
				pData->totalSteps = 0;
				pData->currentStepIndex = 0;
				pData->state = STEPPER_STATE_KNOWN_POSITION;
			}
			else {
				//continue moving to home
				MMD_stepper_clock_high(pData->stepperIndex, false);
				pData->stepPhase = STEP_PHASE_CLK_LOW;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;
		
		case STEP_PHASE_CLK_LOW:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= pData->stepPhaseLowClocks) {
				//trigger a rising edge at the end of lower phase
				MMD_stepper_clock_high(pData->stepperIndex, true);
				pData->stepPhase = STEP_PHASE_CLK_HIGH;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;

		case STEP_PHASE_CLK_HIGH:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= pData->stepPhaseHighClocks) {
				MMD_stepper_clock_high(pData->stepperIndex, false);
				pData->stepPhase = STEP_PHASE_DELAY;
			}
		}
		break;

		case STEP_PHASE_DELAY:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= (pData->accelerationBuffer + pData->decelerationBuffer)) {
				pData->stepPhase = STEP_PHASE_FINISH;
				pData->stepPhaseStartingClock = MMD_current_clock();
				pData->currentStepIndex++;
			}
		}
		break;

		default:
		break;
	}
}

static void mmd_stepper_accelerate(struct MMD_stepper_data * pData)
{
	if(pData == NULL) {
		return;
	}

	// STEP_PHASE_FINISH >> STEP_PHASE_CLK_LOW >> STEP_PHASE_CLK_HIGH >> STEP_PHASE_DELAY 
	//        |_________________________________<<______________________________|

	switch(pData->stepPhase)
	{
		case STEP_PHASE_FINISH:
		{
			if(pData->currentStepIndex < pData->totalSteps) 
			{
				if(pData->currentStepIndex >= pData->decelerationStartingIndex) {
					// no cruise state, change to state of deceleration
					pData->state = STEPPER_STATE_DECELERATING;
				}
				else if(pData->accelerationLevel > 0) {
					//still in acceleration state
					MMD_stepper_clock_high(pData->stepperIndex, false);
					pData->stepPhase = STEP_PHASE_CLK_LOW;
					pData->stepPhaseStartingClock = MMD_current_clock();
				}
				else {
					//acceleration finishes, change to state of cruising
					pData->state = STEPPER_STATE_CRUISING;
				}
			}
			else {
				//change to state of known position.
				pData->totalSteps = 0;
				pData->currentStepIndex = 0;
				pData->accelerationLevel = pData->accelerationBuffer;
				pData->state = STEPPER_STATE_KNOWN_POSITION;
			}
		}
		break;
		
		case STEP_PHASE_CLK_LOW:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= pData->stepPhaseLowClocks) {
				//trigger a rising edge at the end of lower phase
				MMD_stepper_clock_high(pData->stepperIndex, true);
				pData->stepPhase = STEP_PHASE_CLK_HIGH;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;

		case STEP_PHASE_CLK_HIGH:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= pData->stepPhaseHighClocks) {
				//lower down the clock at the end of high phase
				MMD_stepper_clock_high(pData->stepperIndex, false);
				pData->stepPhase = STEP_PHASE_DELAY;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;

		case STEP_PHASE_DELAY:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= pData->accelerationLevel) 
			{
				if(pData->accelerationLevel > pData->accelerationDecrement) {
					pData->accelerationLevel -= pData->accelerationDecrement;
				}
				else {
					pData->accelerationLevel = 0;
				}
				
				//increase step index
				pData->currentStepIndex++;
				if(pData->forward) {
					pData->homeOffset++;
				}
				else {
					pData->homeOffset--;
				}
				
				pData->stepPhase = STEP_PHASE_FINISH;
			}
		}
		break;

		default:
		break;
	}
}

static void mmd_stepper_cruise(struct MMD_stepper_data * pData)
{
	if(pData == NULL) {
		return;
	}

	switch(pData->stepPhase)
	{
		case STEP_PHASE_FINISH:
		{
			if(pData->currentStepIndex < pData->totalSteps) 
			{
				if(pData->currentStepIndex >= pData->decelerationStartingIndex) {
					// change to state of deceleration
					pData->state = STEPPER_STATE_DECELERATING;
				}
				else {
					//still in cruising state
					MMD_stepper_clock_high(pData->stepperIndex, false);
					pData->stepPhase = STEP_PHASE_CLK_LOW;
					pData->stepPhaseStartingClock = MMD_current_clock();
				}
			}
			else 
			{
				//change to state of known position.
				pData->totalSteps = 0;
				pData->currentStepIndex = 0;
				pData->accelerationLevel = pData->accelerationBuffer;
				pData->state = STEPPER_STATE_KNOWN_POSITION;
			}
		}
		break;
		
		case STEP_PHASE_CLK_LOW:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= pData->stepPhaseLowClocks) {
				//trigger a rising edge at the end of lower phase
				MMD_stepper_clock_high(pData->stepperIndex, true);
				pData->stepPhase = STEP_PHASE_CLK_HIGH;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;

		case STEP_PHASE_CLK_HIGH:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= pData->stepPhaseHighClocks) {
				MMD_stepper_clock_high(pData->stepperIndex, false);
				//increase step index
				pData->currentStepIndex++;
				if(pData->forward) {
					pData->homeOffset++;
				}
				else {
					pData->homeOffset--;
				}
				
				pData->stepPhase = STEP_PHASE_FINISH;
			}
		}
		break;

		case STEP_PHASE_DELAY:
		{
			//no delay phase in cruising state
		}
		break;

		default:
		break;
	}
}

static void mmd_stepper_decelerate(struct MMD_stepper_data * pData)
{
	if(pData == NULL) {
		return;
	}

	switch(pData->stepPhase)
	{
		case STEP_PHASE_FINISH:
		{
			if(pData->currentStepIndex < pData->totalSteps) {
				//still in deceleration state
				MMD_stepper_clock_high(pData->stepperIndex, false);
				pData->stepPhase = STEP_PHASE_CLK_LOW;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
			else {
				//change to state of known position.
				pData->totalSteps = 0;
				pData->currentStepIndex = 0;
				pData->accelerationLevel = pData->accelerationBuffer;
				pData->state = STEPPER_STATE_KNOWN_POSITION;
			}
		}
		break;
		
		case STEP_PHASE_CLK_LOW:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= pData->stepPhaseLowClocks) {
				//trigger a rising edge at the end of lower phase
				MMD_stepper_clock_high(pData->stepperIndex, true);
				pData->stepPhase = STEP_PHASE_CLK_HIGH;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;

		case STEP_PHASE_CLK_HIGH:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= pData->stepPhaseHighClocks) {
				MMD_stepper_clock_high(pData->stepperIndex, false);
				pData->stepPhase = STEP_PHASE_DELAY;
				pData->stepPhaseStartingClock = MMD_current_clock();
			}
		}
		break;

		case STEP_PHASE_DELAY:
		{
			if(MMD_elapsed_clocks(pData->stepPhaseStartingClock) >= pData->decelerationLevel) 
			{
				//increase step index
				pData->currentStepIndex++;
				if(pData->forward) {
					pData->homeOffset++;
				}
				else {
					pData->homeOffset--;
				}
				//adjust delay
				pData->decelerationLevel += pData->decelerationIncrement;
				
				pData->stepPhase = STEP_PHASE_FINISH;
			}
		}
		break;

		default:
		break;
	}
}

static void mmd_steppers_run(void)
{
	struct MMD_stepper_data * pStepper;
	
	for(unsigned char stepperIndex = 0; stepperIndex < MMD_STEPPERS_AMOUNT; stepperIndex++)
	{
		pStepper = &(mmdCommand.steppersData[stepperIndex]);
		switch(pStepper->state)
		{
			case STEPPER_STATE_UNKNOWN_POSITION:
			{
				//do nothing.
			}
			break;
			
			case STEPPER_STATE_APPROACHING_HOME_LOCATOR:
			{
				mmd_stepper_approach_home_locator(pStepper);
			}
			break;

			case STEPPER_STATE_LEAVING_HOME_LOCATOR:
			{
				mmd_stepper_leave_home_locator(pStepper);
			}
			break;
			
			case STEPPER_STATE_GO_HOME:
			{
				mmd_stepper_go_home(pStepper);
			}
			break;

			case STEPPER_STATE_KNOWN_POSITION:
			{
				//do nothing
				if(stepperIndex == mmdCommand.parameters[0]) 
				{
					mmd_write_reply_header();
					writeOutputBufferString("\"position\":\"");
					writeOutputBufferHex(pStepper->homeOffset >> 8);
					writeOutputBufferHex(pStepper->homeOffset & 0xff);
					writeOutputBufferChar('"');
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					
					mmdCommand.state = AWAITING_COMMAND;
				}
			}
			break;

			case STEPPER_STATE_ACCELERATING:
			{
				mmd_stepper_accelerate(pStepper);
			}
			break;
			
			case STEPPER_STATE_CRUISING:
			{
				mmd_stepper_cruise(pStepper);
			}
			break;
			
			case STEPPER_STATE_DECELERATING:
			{
				mmd_stepper_decelerate(pStepper);
			}
			break;

			default:
			break;
		}
		
		mmd_stepper_check_scope(pStepper);
	}
}

static void mmd_stepper_query(unsigned char stepperIndex)
{
	struct MMD_stepper_data * pData;
	
	pData = &(mmdCommand.steppersData[stepperIndex]);
	
	mmd_write_reply_header();
	//stepper state
	writeOutputBufferString(STR_STEPPER_STATE);
	writeOutputBufferChar('"');
	switch(pData->state)
	{
	case STEPPER_STATE_UNKNOWN_POSITION:
		writeOutputBufferString("unknown position");
		break;
	case STEPPER_STATE_APPROACHING_HOME_LOCATOR:
		writeOutputBufferString("approaching home locator");
		break;
	case STEPPER_STATE_LEAVING_HOME_LOCATOR:
		writeOutputBufferString("leaving home locator");
		break;
	case STEPPER_STATE_GO_HOME:
		writeOutputBufferString("going home");
		break;
	case STEPPER_STATE_KNOWN_POSITION:
		writeOutputBufferString("known position");
		break;
	case STEPPER_STATE_ACCELERATING:
		writeOutputBufferString("accelerating");
		break;
	case STEPPER_STATE_CRUISING:
		writeOutputBufferString("cruising");
		break;
	case STEPPER_STATE_DECELERATING:
		writeOutputBufferString("decelerating");
		break;
	default:
		writeOutputBufferString("unknown state");
		break;
	}
	writeOutputBufferString("\",");
	
	//enabled
	writeOutputBufferString(STR_STEPPER_IS_ENABLED);
	writeOutputBufferChar('"');
	writeOutputBufferHex(pData->enabled);
	writeOutputBufferString("\",");
	
	//forward
	writeOutputBufferString(STR_STEPPER_FORWARD);
	writeOutputBufferChar('"');
	writeOutputBufferHex(pData->forward);
	writeOutputBufferString("\",");
	
	//locator
	writeOutputBufferString(STR_STEPPER_LOCATOR_INDEX);
	writeOutputBufferChar('"');
	writeOutputBufferHex(pData->locatorIndex);
	writeOutputBufferString("\",");
	
	//locator line number start
	writeOutputBufferString(STR_STEPPER_LOCATOR_LINE_NUMBER_START);
	writeOutputBufferChar('"');
	writeOutputBufferHex(pData->locatorLineNumberStart);
	writeOutputBufferString("\",");
	
	//locator line number terminal
	writeOutputBufferString(STR_STEPPER_LOCATOR_LINE_NUMBER_TERMINAL);
	writeOutputBufferChar('"');
	writeOutputBufferHex(pData->locatorLineNumberTerminal);
	writeOutputBufferString("\",");
	
	//home offset
	writeOutputBufferString(STR_STEPPER_HOME_OFFSET);
	writeOutputBufferChar('"');
	writeOutputBufferHex(pData->homeOffset >> 8);
	writeOutputBufferHex(pData->homeOffset & 0xff);
	writeOutputBufferString("\",");
	
	//step phase low clocks
	writeOutputBufferString(STR_STEPPER_PHASE_LOW_CLOCKS);
	writeOutputBufferChar('"');
	writeOutputBufferHex(pData->stepPhaseLowClocks >> 8);
	writeOutputBufferHex(pData->stepPhaseLowClocks & 0xff);
	writeOutputBufferString("\",");

	//step phase high clocks
	writeOutputBufferString(STR_STEPPER_PHASE_HIGH_CLOCKS);
	writeOutputBufferChar('"');
	writeOutputBufferHex(pData->stepPhaseHighClocks >> 8);
	writeOutputBufferHex(pData->stepPhaseHighClocks & 0xff);
	writeOutputBufferString("\",");
	
	//acceleration buffer
	writeOutputBufferString(STR_STEPPER_ACCELERATION_BUFFER);
	writeOutputBufferChar('"');
	writeOutputBufferHex(pData->accelerationBuffer >> 8);
	writeOutputBufferHex(pData->accelerationBuffer & 0xff);
	writeOutputBufferString("\",");

	//acceleration decrement
	writeOutputBufferString(STR_STEPPER_ACCELERATION_DECREMENT);
	writeOutputBufferChar('"');
	writeOutputBufferHex(pData->accelerationDecrement >> 8);
	writeOutputBufferHex(pData->accelerationDecrement & 0xff);
	writeOutputBufferString("\",");

	//deceleration buffer
	writeOutputBufferString(STR_STEPPER_DECELERATION_BUFFER);
	writeOutputBufferChar('"');
	writeOutputBufferHex(pData->decelerationBuffer >> 8);
	writeOutputBufferHex(pData->decelerationBuffer & 0xff);
	writeOutputBufferString("\",");

	//deceleration increment
	writeOutputBufferString(STR_STEPPER_DECELERATION_INCREMENT);
	writeOutputBufferChar('"');
	writeOutputBufferHex(pData->decelerationIncrement >> 8);
	writeOutputBufferHex(pData->decelerationIncrement & 0xff);
	writeOutputBufferChar('"');

	writeOutputBufferString(STR_CARRIAGE_RETURN);
}

static void mmd_locator_query(unsigned char locatorIndex)
{
	unsigned char locator = MMD_locator_get(locatorIndex);
	
	mmd_write_reply_header();
	writeOutputBufferString("\"lowInput\":\"");
	writeOutputBufferHex(locator);
	writeOutputBufferChar('"');
	writeOutputBufferString(STR_CARRIAGE_RETURN);
}

static void mmd_bdc_forward()
{
	switch(mmdCommand.bdcData.phase)
	{
		case BDC_PHASE_FINISH:
		{
			if(mmdCommand.bdcData.cycleIndex >= mmdCommand.bdcData.cycles)
			{
				MMD_bdc_set_state(mmdCommand.bdcData.bdcIndex, BDC_STATE_FORWARD);
				mmd_write_succeess_reply();
				mmdCommand.state = AWAITING_COMMAND;
			}
			else
			{
				mmdCommand.bdcData.phase = BDC_PHASE_LOW;
				MMD_bdc_set_state(mmdCommand.bdcData.bdcIndex, BDC_STATE_FORWARD); //activate BDC
				mmdCommand.bdcData.phaseStartClk = MMD_current_clock();
			}
		}
		break;
		
		case BDC_PHASE_LOW:
		{
			if(MMD_elapsed_clocks(mmdCommand.bdcData.phaseStartClk) >= mmdCommand.bdcData.lowClks) {
				mmdCommand.bdcData.phase = BDC_PHASE_HIGH;
				MMD_bdc_set_state(mmdCommand.bdcData.bdcIndex, BDC_STATE_BREAK); //deactivate BDC
				mmdCommand.bdcData.phaseStartClk = MMD_current_clock();
			}
		}
		break;
		
		case BDC_PHASE_HIGH:
		{
			if(MMD_elapsed_clocks(mmdCommand.bdcData.phaseStartClk) >= mmdCommand.bdcData.highClks) {
				mmdCommand.bdcData.phase = BDC_PHASE_FINISH;
				mmdCommand.bdcData.cycleIndex++;
			}
		}
		break;
		
		default:
		{
			//wrong state
			mmd_write_reply_header();
			writeOutputBufferString(STR_BDC_WRONG_PHASE_STATE);
			writeOutputBufferString(STR_CARRIAGE_RETURN);
			mmdCommand.state = AWAITING_COMMAND;
		}
		break;
	}
}

static void mmd_bdc_reverse()
{
	switch(mmdCommand.bdcData.phase)
	{
		case BDC_PHASE_FINISH:
		{
			if(mmdCommand.bdcData.cycleIndex >= mmdCommand.bdcData.cycles)
			{
				MMD_bdc_set_state(mmdCommand.bdcData.bdcIndex, BDC_STATE_REVERSE);
				mmd_write_succeess_reply();
				mmdCommand.state = AWAITING_COMMAND;
			}
			else
			{
				mmdCommand.bdcData.phase = BDC_PHASE_LOW;
				MMD_bdc_set_state(mmdCommand.bdcData.bdcIndex, BDC_STATE_REVERSE); //activate BDC
				mmdCommand.bdcData.phaseStartClk = MMD_current_clock();
			}
		}
		break;
		
		case BDC_PHASE_LOW:
		{
			if(MMD_elapsed_clocks(mmdCommand.bdcData.phaseStartClk) >= mmdCommand.bdcData.lowClks) {
				mmdCommand.bdcData.phase = BDC_PHASE_HIGH;
				MMD_bdc_set_state(mmdCommand.bdcData.bdcIndex, BDC_STATE_BREAK); //deactivate BDC
				mmdCommand.bdcData.phaseStartClk = MMD_current_clock();
			}
		}
		break;
		
		case BDC_PHASE_HIGH:
		{
			if(MMD_elapsed_clocks(mmdCommand.bdcData.phaseStartClk) >= mmdCommand.bdcData.highClks) {
				mmdCommand.bdcData.phase = BDC_PHASE_FINISH;
				mmdCommand.bdcData.cycleIndex++;
			}
		}
		break;
		
		default:
		{
			//wrong state
			mmd_write_reply_header();
			writeOutputBufferString(STR_BDC_WRONG_PHASE_STATE);
			writeOutputBufferString(STR_CARRIAGE_RETURN);
			mmdCommand.state = AWAITING_COMMAND;
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
								mmdCommand.parameters[index] = p;
								mmdCommand.parameterAmount++;
							}
							break; //end of a command
						}
						else if(' ' == c) {
							if(digitMet == true) {
								mmdCommand.parameters[index] = p;
								mmdCommand.parameterAmount++;
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
				if(index >= MMD_MAX_COMMAND_PARAMETERS) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_TOO_MANY_PARAMETERS);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					validCmd = false;
				}
			}
		}
	}

	if(!validCmd) {
		mmdCommand.command = COMMAND_INVALID;
		mmdCommand.parameterAmount = 0;
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
		case COMMAND_DEVICE_DELAY:
		case COMMAND_LOCATOR_INTERVAL:
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
		case COMMAND_STEPPER_SET_STATE:
		case COMMAND_LOCATOR_QUERY:
		{
			mmdCommand.command = cmd;
			mmdCommand.state = STARTING_COMMAND;
		}
		break;
		default:
		{
			mmd_write_reply_header();
			writeOutputBufferString(STR_UNKNOWN_COMMAND);
			writeOutputBufferString(STR_CARRIAGE_RETURN);
			mmdCommand.command = COMMAND_INVALID;
			mmdCommand.state = AWAITING_COMMAND;
		}
			break;
		}
	}
}

static void mmd_cancel_command(void)
{
	clearInputBuffer();
	mmdCommand.state = AWAITING_COMMAND;
}

static void mmd_run_command(void)
{
	if(mmdCommand.state == STARTING_COMMAND)
	{
		//prepare for execution.
		switch(mmdCommand.command)
		{
			case COMMAND_QUREY_NAME:
			case COMMAND_DEVICE_POWER_QUERY:
			case COMMAND_DEVICE_FUSE_QUERY:
			case COMMAND_OPT_POWER_ON:
			case COMMAND_OPT_POWER_OFF:
			case COMMAND_OPT_POWER_QUERY:
			case COMMAND_STEPPERS_POWER_ON:
			case COMMAND_STEPPERS_POWER_OFF:
			case COMMAND_STEPPERS_POWER_QUERY:
			{
				if(mmdCommand.parameterAmount != 1) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case COMMAND_DEVICE_DELAY:
			case COMMAND_LOCATOR_INTERVAL:
			{
				if(mmdCommand.parameterAmount != 2){
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] > 0x7FFF) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_EXCESSIVE_DELAY);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
					mmdCommand.deviceDelay.totalClks = mmdCommand.parameters[0];
					mmdCommand.deviceDelay.startingClk = MMD_current_clock();
				}
			}
			break;
			
			case COMMAND_DCM_POWER_ON:
			case COMMAND_DCM_POWER_OFF:
			case COMMAND_DCM_POWER_QUERY:
			{
				if(mmdCommand.parameterAmount != 2){
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_DIRECT_CURRENT_MOTORS_AMOUNT) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_DCM_INDEX_OUT_OF_SCOPE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
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
				if(mmdCommand.parameterAmount != 1) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case COMMAND_BDC_COAST:
			case COMMAND_BDC_BREAK:
			case COMMAND_BDC_QUERY:
			{
				if(mmdCommand.parameterAmount != 2){
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_BI_DIRECTION_DIRECT_CURRENT_MOTORS_AMOUNT) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_BDC_INDEX_OUT_OF_SCOPE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case COMMAND_BDC_REVERSE:
			case COMMAND_BDC_FORWARD:
			{
				if(mmdCommand.parameterAmount != 5){
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_BI_DIRECTION_DIRECT_CURRENT_MOTORS_AMOUNT) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_BDC_INDEX_OUT_OF_SCOPE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.bdcData.bdcIndex = mmdCommand.parameters[0];
					mmdCommand.bdcData.lowClks = mmdCommand.parameters[1];
					mmdCommand.bdcData.highClks = mmdCommand.parameters[2];
					mmdCommand.bdcData.cycles = mmdCommand.parameters[3];
					mmdCommand.bdcData.cycleIndex = 0;
					mmdCommand.bdcData.phase = BDC_PHASE_FINISH;
					
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case COMMAND_STEPPER_STEP_RESOLUTION_QUERY:
			{
				if(mmdCommand.parameterAmount != 1) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
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
				if(mmdCommand.parameterAmount != 4){
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_STEPPERS_AMOUNT) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_STEPPER_INDEX_OUT_OF_SCOPE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
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
				if(mmdCommand.parameterAmount != 3){
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_STEPPERS_AMOUNT) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_STEPPER_INDEX_OUT_OF_SCOPE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
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
				unsigned short steps = mmdCommand.parameters[1];
				
				if(mmdCommand.parameterAmount != 3){
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(stepperIndex >= MMD_STEPPERS_AMOUNT) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_STEPPER_INDEX_OUT_OF_SCOPE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.steppersData[stepperIndex].state != STEPPER_STATE_KNOWN_POSITION) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_STEPPER_NOT_POSITIONED);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(steps < 1) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_INVALID_PARAMETER);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
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
				if(mmdCommand.parameterAmount != 2) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
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
				
				if(mmdCommand.parameterAmount != 5){
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(stepperIndex >= MMD_STEPPERS_AMOUNT) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_STEPPER_INDEX_OUT_OF_SCOPE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(locatorIndex >= MMD_LOCATOR_AMOUNT) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_LOCATOR_INDEX_OUT_OF_SCOPE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if((lineNumberStart < 1) || (lineNumberStart > 8)) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_LOCATOR_LINE_INDEX_OUT_OF_SCOPE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if((lineNumberTerminal < 1) || (lineNumberTerminal > 8)) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_LOCATOR_LINE_INDEX_OUT_OF_SCOPE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(lineNumberStart == lineNumberTerminal) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_LOCATOR_LINE_INDEX_DUPLICATE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case COMMAND_STEPPER_QUREY:
			{
				if(mmdCommand.parameterAmount != 2){
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_STEPPERS_AMOUNT) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_STEPPER_INDEX_OUT_OF_SCOPE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else {
					mmdCommand.state = EXECUTING_COMMAND;
				}
			}
			break;
			
			case COMMAND_STEPPER_SET_STATE:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				unsigned char state = mmdCommand.parameters[1];
				
				if(mmdCommand.parameterAmount != 3){
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(stepperIndex >= MMD_STEPPERS_AMOUNT) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_STEPPER_INDEX_OUT_OF_SCOPE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(state > STEPPER_STATE_DECELERATING) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_STEPPER_WRONG_STATE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
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
				if(mmdCommand.parameterAmount != 2){
					mmd_write_reply_header();
					writeOutputBufferString(STR_WRONG_PARAMETER_AMOUNT);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
					clearInputBuffer();
					mmdCommand.state = AWAITING_COMMAND;
				}
				else if(mmdCommand.parameters[0] >= MMD_LOCATOR_AMOUNT) {
					mmd_write_reply_header();
					writeOutputBufferString(STR_LOCATOR_LINE_INDEX_OUT_OF_SCOPE);
					writeOutputBufferString(STR_CARRIAGE_RETURN);
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
				mmd_write_reply_header();
				writeOutputBufferString(STR_UNKNOWN_COMMAND);
				writeOutputBufferString(STR_CARRIAGE_RETURN);
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
			case COMMAND_QUREY_NAME:
			{
				mmd_write_reply_header();
				writeOutputBufferString("\"name\":\""); writeOutputBufferString(MMD_PRODUCT_NAME); writeOutputBufferString("\"");
				writeOutputBufferString(STR_CARRIAGE_RETURN);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_DEVICE_POWER_QUERY:
			{
				mmd_write_reply_header();
				writeOutputBufferString("\"state\":");
				if(MMD_is_main_power_ok()) {
					writeOutputBufferString("\"powered on\"");
				}
				else {
					writeOutputBufferString("\"powered off\"");
				}
				writeOutputBufferString(STR_CARRIAGE_RETURN);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_DEVICE_FUSE_QUERY:
			{
				mmd_write_reply_header();
				writeOutputBufferString("\"state\":");
				if(MMD_is_main_power_ok()) {
					writeOutputBufferString("\"main fuse is on\"");
				}
				else {
					writeOutputBufferString("\"main fuse is off\"");
				}
				writeOutputBufferString(STR_CARRIAGE_RETURN);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_DEVICE_DELAY:
			{
				if(MMD_elapsed_clocks(mmdCommand.deviceDelay.startingClk) >= mmdCommand.deviceDelay.totalClks) {
					mmd_write_succeess_reply();
					mmdCommand.state = AWAITING_COMMAND;
				}
			}
			break;
			
			case COMMAND_LOCATOR_INTERVAL:
			{
				locatorCheckInterval = mmdCommand.deviceDelay.totalClks;
				mmd_write_succeess_reply();
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_OPT_POWER_ON:
			{
				MMD_power_on_opt(true);
				mmd_write_succeess_reply();			
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_OPT_POWER_OFF:
			{
				MMD_power_on_opt(false);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_OPT_POWER_QUERY:
			{
				mmd_write_reply_header();
				if(MMD_is_opt_powered_on())
					writeOutputBufferString("\"state\":\"OPT is powered on\"");
				else
					writeOutputBufferString("\"state\":\"OPT is powered off\"");
				writeOutputBufferString(STR_CARRIAGE_RETURN);
					
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_STEPPERS_POWER_ON:
			{
				MMD_power_on_steppers(true);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_STEPPERS_POWER_OFF:
			{
				MMD_power_on_steppers(false);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_STEPPERS_POWER_QUERY:
			{
				mmd_write_reply_header();
				
				if(MMD_are_steppers_powered_on())
					writeOutputBufferString("\"state\":\"steppers are powered on\"");
				else
					writeOutputBufferString("\"state\":\"steppers are powered off\"");
				writeOutputBufferString(STR_CARRIAGE_RETURN);
					
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_DCM_POWER_ON:
			{
				unsigned char index = mmdCommand.parameters[0];
				MMD_power_on_dcm(index, true);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_DCM_POWER_OFF:
			{
				unsigned char index = mmdCommand.parameters[0];
				MMD_power_on_dcm(index, false);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_DCM_POWER_QUERY:
			{
				unsigned char index = mmdCommand.parameters[0];
				
				mmd_write_reply_header();
				if(MMD_is_dcm_powered_on(index)) {
					writeOutputBufferString("\"state\":\"dcm is powered on\"");
				}
				else {
					writeOutputBufferString("\"state\":\"dcm is powered off\"");
				}
				writeOutputBufferString(STR_CARRIAGE_RETURN);
				
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_BDCS_POWER_ON:
			{
				MMD_power_on_bdcs(true);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_BDCS_POWER_OFF:
			{
				MMD_power_on_bdcs(false);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_BDCS_POWER_QUERY:
			{
				mmd_write_reply_header();
				if(MMD_are_bdcs_powered_on()) {
					writeOutputBufferString("\"state\":\"BDCs are powered on\"");
				}
				else {
					writeOutputBufferString("\"state\":\"BDCs are powered off\"");
				}
				writeOutputBufferString(STR_CARRIAGE_RETURN);
				
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_BDC_COAST:
			{
				unsigned char index = mmdCommand.parameters[0];
				MMD_bdc_set_state(index, BDC_STATE_COAST);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_BDC_REVERSE:
			{
				mmd_bdc_reverse();
			}
			break;
			
			case COMMAND_BDC_FORWARD:
			{
				mmd_bdc_forward();
			}
			break;
			
			case COMMAND_BDC_BREAK:
			{
				unsigned char index = mmdCommand.parameters[0];
				MMD_bdc_set_state(index, BDC_STATE_BREAK);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_BDC_QUERY:
			{
				unsigned char index = mmdCommand.parameters[0];
				
				mmd_write_reply_header();
				switch(MMD_bdc_get_state(index))
				{
					case BDC_STATE_COAST:
						writeOutputBufferString("\"state\":\"coast\"");
						break;
					case BDC_STATE_REVERSE:
						writeOutputBufferString("\"state\":\"reverse\"");
						break;
					case BDC_STATE_FORWARD:
						writeOutputBufferString("\"state\":\"forward\"");
						break;
					case BDC_STATE_BREAK:
						writeOutputBufferString("\"state\":\"break\"");
						break;
					default:
						writeOutputBufferString("\"error\":\"wrong BDC state\"");
						break;
				}
				writeOutputBufferString(STR_CARRIAGE_RETURN);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_STEPPER_STEP_RESOLUTION_QUERY:
			{
				unsigned short resolution = MMD_stepper_resolution();

				mmd_write_reply_header();
				writeOutputBufferString("\"resolution\":\"");
				writeOutputBufferHex(resolution >> 8);
				writeOutputBufferHex(resolution & 0xff);
				writeOutputBufferString("\"");
				writeOutputBufferString(STR_CARRIAGE_RETURN);
				
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_CONFIG_STEP:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				unsigned short lowClocks = mmdCommand.parameters[1];
				unsigned short highClocks = mmdCommand.parameters[2];
				
				MMD_stepper_config_step(stepperIndex, lowClocks, highClocks);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_ACCELERATION_BUFFER:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				unsigned short clocks = mmdCommand.parameters[1];
				
				MMD_stepper_set_acceleration_buffer(stepperIndex, clocks);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_ACCELERATION_BUFFER_DECREMENT:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				unsigned short clocks = mmdCommand.parameters[1];
				
				MMD_stepper_set_acceleration_buffer_decrement(stepperIndex, clocks);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_DECELERATION_BUFFER:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				unsigned short clocks = mmdCommand.parameters[1];

				MMD_stepper_set_deceleration_buffer(stepperIndex, clocks);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_DECELERATION_BUFFER_INCREMENT:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				unsigned short clocks = mmdCommand.parameters[1];

				MMD_stepper_set_deceleration_buffer_increment(stepperIndex, clocks);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_ENABLE:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				bool enable = mmdCommand.parameters[1] != 0;
				
				MMD_stepper_enable(stepperIndex, enable);
				if(!enable) {
					mmdCommand.steppersData[stepperIndex].state = STEPPER_STATE_UNKNOWN_POSITION;
				}
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_DIR:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				bool forward = mmdCommand.parameters[1] != 0;

				MMD_stepper_forward(stepperIndex, forward);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_STEPS:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				unsigned short steps = mmdCommand.parameters[1];
				
				MMD_stepper_set_steps(stepperIndex,  steps);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_RUN:
			{
				mmd_steppers_run();
			}
			break;

			case COMMAND_STEPPER_CONFIG_HOME:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				
				mmdCommand.steppersData[stepperIndex].locatorIndex = mmdCommand.parameters[1];
				mmdCommand.steppersData[stepperIndex].locatorLineNumberStart = mmdCommand.parameters[2];
				mmdCommand.steppersData[stepperIndex].locatorLineNumberTerminal = mmdCommand.parameters[3];
				mmdCommand.steppersData[stepperIndex].state = STEPPER_STATE_APPROACHING_HOME_LOCATOR;
				mmdCommand.steppersData[stepperIndex].stepPhase = STEP_PHASE_FINISH;
				//reverse to home
				MMD_stepper_forward(stepperIndex, false);
				MMD_stepper_enable(stepperIndex, true);
				mmd_write_succeess_reply();	
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_STEPPER_QUREY:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				
				mmd_stepper_query(stepperIndex);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
			
			case COMMAND_STEPPER_SET_STATE:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				unsigned char state = mmdCommand.parameters[1];
				
				mmdCommand.steppersData[stepperIndex].state = state;
				mmdCommand.steppersData[stepperIndex].homeOffset = 0;
				mmdCommand.steppersData[stepperIndex].totalSteps = 0;
				
				mmd_write_succeess_reply();
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			case COMMAND_LOCATOR_QUERY:
			{
				unsigned char stepperIndex = mmdCommand.parameters[0];
				
				mmd_locator_query(stepperIndex);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;

			default:
			{
				mmd_write_reply_header();
				writeOutputBufferString(STR_UNKNOWN_COMMAND);
				writeOutputBufferString(STR_CARRIAGE_RETURN);
				mmdCommand.state = AWAITING_COMMAND;
			}
			break;
		}
	}
}

static void mmd_check_status(void)
{
	bool b;
	unsigned char index;
	
	// main power
	b = MMD_is_main_power_ok();
	if(mmdStatus.isMainPowerOk != b) {
		if(b) {
			writeOutputBufferString(EVENT_MAIN_POWER_IS_ON);
		}
		else {
			writeOutputBufferString(EVENT_MAIN_POWER_IS_OFF);
		}
		mmdStatus.isMainPowerOk = b;
	}
	
	//main fuse
	b = MMD_is_main_fuse_ok();
	if(mmdStatus.isMainFuseOk != b) {
		if(b) {
			writeOutputBufferString(EVENT_MAIN_FUSE_IS_ON);
		}
		else {
			writeOutputBufferString(EVENT_MAIN_FUSE_IS_OFF);
		}
		mmdStatus.isMainFuseOk = b;
	}
	
	//opt power
	b = MMD_is_opt_powered_on();
	if(mmdStatus.isOptPowered != b) {
		if(b) {
			writeOutputBufferString(EVENT_OPT_IS_POWERED_ON);
		}
		else {
			writeOutputBufferString(EVENT_OPT_IS_POWERED_OFF);
		}
		mmdStatus.isOptPowered = b;
	}
	
	//dcms
	for(index = 0; index < MMD_DIRECT_CURRENT_MOTORS_AMOUNT; index++)
	{
		b = MMD_is_dcm_powered_on(index);
		if(mmdStatus.dcmPowered[index] != b) 
		{
			if(b) {
				writeOutputBufferString(EVENT_DCM_IS_POWERED_ON);
			}
			else {
				writeOutputBufferString(EVENT_DCM_IS_POWERED_OFF);
			}
			writeOutputBufferString(",\"index\":\"");
			writeOutputBufferHex(index);
			writeOutputBufferChar('"');
			writeOutputBufferString(STR_CARRIAGE_RETURN);
			mmdStatus.dcmPowered[index] = b;
		}
	}
	
	//bdcs
	b = MMD_are_bdcs_powered_on();
	if(mmdStatus.areBdcsPowered != b) {
		if(b) {
			writeOutputBufferString(EVENT_BDCS_ARE_POWERED_ON);
		}
		else {
			writeOutputBufferString(EVENT_BDCS_ARE_POWERED_OFF);
		}
		mmdStatus.areBdcsPowered = b;
	}
	for(index = 0; index < MMD_BI_DIRECTION_DIRECT_CURRENT_MOTORS_AMOUNT; index++)
	{
		if(mmdCommand.state != AWAITING_COMMAND) {
			break; //report BDC status change only after command finishes to avoid excessive state changes in BDC FORWARD and REVERSE
		}
		
		enum MMD_BDC_STATE state = MMD_bdc_get_state(index);
		if(mmdStatus.bdcState[index] != state) 
		{
			switch(state)
			{
				case BDC_STATE_COAST:
				{
					writeOutputBufferString(EVENT_BDC_COAST);
				}
				break;
				
				case BDC_STATE_REVERSE:
				{
					writeOutputBufferString(EVENT_BDC_REVERSE);
				}
				break;
				
				case BDC_STATE_FORWARD:
				{
					writeOutputBufferString(EVENT_BDC_FORWARD);
				}
				break;
				
				case BDC_STATE_BREAK:
				{
					writeOutputBufferString(EVENT_BDC_BREAK);
				}
				break;
				
				default:
				{
					writeOutputBufferString(EVENT_BDC_WRONT_STATE);
				}
				break;
			}
			writeOutputBufferString(",\"index\":\"");
			writeOutputBufferHex(index);
			writeOutputBufferChar('"');
			writeOutputBufferString(STR_CARRIAGE_RETURN);
			
			mmdStatus.bdcState[index] = state;
		}
	}
	
	//steppers
	b = MMD_are_steppers_powered_on();
	if(mmdStatus.areSteppersPowered != b){
		if(b) {
			writeOutputBufferString(EVENT_STEPPERS_ARE_POWERED_ON);
		}
		else {
			writeOutputBufferString(EVENT_STEPPERS_ARE_POWERED_OFF);
		}
		mmdStatus.areSteppersPowered = b;
	}
	for(index = 0; index < MMD_STEPPERS_AMOUNT; index++)
	{
		enum MMD_stepper_state state;
		
		b = MMD_is_stepper_enabled(index);
		if(mmdStatus.stepperEnabled[index] != b) {
			if(b) {
				writeOutputBufferString(EVENT_STEPPER_IS_ENABLED);
			}
			else {
				writeOutputBufferString(EVENT_STEPPER_IS_DISABLED);
			}
			writeOutputBufferString(",\"index\":\"");
			writeOutputBufferHex(index);
			writeOutputBufferChar('"');
			writeOutputBufferString(STR_CARRIAGE_RETURN);
			
			mmdStatus.stepperEnabled[index] = b;
		}
		
		b = MMD_is_stepper_forward(index);
		if(mmdStatus.stepperForward[index] != b) {
			if(b) {
				writeOutputBufferString(EVENT_STEPPER_FORWORD);
			}
			else {
				writeOutputBufferString(EVENT_STEPPER_BACKWORD);
			}
			writeOutputBufferString(",\"index\":\"");
			writeOutputBufferHex(index);
			writeOutputBufferChar('"');
			writeOutputBufferString(STR_CARRIAGE_RETURN);
			
			mmdStatus.stepperForward[index] = b;
		}
		
		state = mmdCommand.steppersData[index].state;
		if(mmdStatus.stepperState[index] != state)
		{
			switch(state)
			{
				case STEPPER_STATE_UNKNOWN_POSITION:
				{
					writeOutputBufferString(EVENT_STEPPER_STATE_UNKNOWN_POSITION);
				}
				break;
				
				case STEPPER_STATE_APPROACHING_HOME_LOCATOR:
				{
					writeOutputBufferString(EVENT_STEPPER_STATE_APPROACH_HOME);
				}
				break;

				case STEPPER_STATE_LEAVING_HOME_LOCATOR:
				{
					writeOutputBufferString(EVENT_STEPPER_STATE_LEAVE_HOME);
				}
				break;

				case STEPPER_STATE_GO_HOME:
				{
					writeOutputBufferString(EVENT_STEPPER_STATE_GO_HOME);
				}
				break;

				case STEPPER_STATE_KNOWN_POSITION:
				{
					writeOutputBufferString(EVENT_STEPPER_STATE_KNOWN_POSITION);
				}
				break;

				case STEPPER_STATE_ACCELERATING:
				{
					writeOutputBufferString(EVENT_STEPPER_STATE_ACCELERATE);
				}
				break;

				case STEPPER_STATE_CRUISING:
				{
					writeOutputBufferString(EVENT_STEPPER_STATE_CRUISE);
				}
				break;

				case STEPPER_STATE_DECELERATING:
				{
					writeOutputBufferString(EVENT_STEPPER_STATE_DECELERATE);
				}
				break;
				
				default:
				{
					writeOutputBufferString(EVENT_STEPPER_STATE_WRONG_STATE);
				}
				break;
			}
			writeOutputBufferString(",\"index\":\"");
			writeOutputBufferHex(index);
			writeOutputBufferChar('"');
			writeOutputBufferString(STR_CARRIAGE_RETURN);
			
			mmdStatus.stepperState[index] = state;
		}
	}
	
	//locator
	if(MMD_elapsed_clocks(locatorCheckStamp) >= locatorCheckInterval)
	{
		locatorCheckStamp = MMD_current_clock();
			
		index = MMD_locator_get(0);
		if(mmdStatus.locatorHubs[0] != index)
		{
			writeOutputBufferString(EVENT_LOCATOR);
			writeOutputBufferString(",\"index\":\"");
			writeOutputBufferHex(0);
			writeOutputBufferString("\",\"input\":\"");
			writeOutputBufferHex(index);
			writeOutputBufferChar('"');
			writeOutputBufferString(STR_CARRIAGE_RETURN);
		
			mmdStatus.locatorHubs[0] = index;
		}
		for(index = 1; index < MMD_LOCATOR_AMOUNT; index++) 
		{
			unsigned char locator = MMD_locator_get(index);
			if(mmdStatus.locatorHubs[index] != locator) {
				writeOutputBufferString(EVENT_LOCATOR);
				writeOutputBufferString(",\"index\":\"");
				writeOutputBufferHex(index);
				writeOutputBufferString("\",\"input\":\"");
				writeOutputBufferHex(locator);
				writeOutputBufferChar('"');
				writeOutputBufferString(STR_CARRIAGE_RETURN);
		
				mmdStatus.locatorHubs[index] = locator;
			}
		}
	}
}

void ecd300MixedMotorDrivers(void)
{
	Invenco_init();
	MMD_init();

	//PD0 works as indicator of host output
	PORTD_DIRSET = 0x01;
	
	udc_start();
	
	locatorCheckInterval = 33; //about 1 millisecond
	locatorCheckStamp = counter_get();
	
	while(1)
	{
		unsigned char key;
		
		if (udi_cdc_is_rx_ready())
		{
			//read a char from USB buffer.
			key = (unsigned char)udi_cdc_getc();
			
			//echo to serial port for debug
			{
				unsigned char serialBuf[4];
				
				printHex(key);printString(":");
				serialBuf[0] = key;
				serialBuf[1] = '\r';
				serialBuf[2] = '\n';
				serialBuf[3] = 0;
				printString(serialBuf);
			}
			
			
			writeInputBuffer(key); //append to input buffer
			writeOutputBufferChar(key); //echo char to host
			if(key == 0x0D) {
				writeOutputBufferChar(0x0A); //append a new line.
			}
			
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
				if(mmdCommand.state == AWAITING_COMMAND) {
					mmd_parse_command();
				}
				else {
					mmd_cancel_command();
				}
			}
		}

		//update current clock with system counter.
		mmdCurrentClock = counter_get();

		mmd_run_command();
		
		mmd_check_status();

		sendOutputBufferToHost();
	}

	while(1)
	{
		;
	}
}

