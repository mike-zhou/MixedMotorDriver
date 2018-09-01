#include "Invenco_lib.h"

#define SMART_CARD_SWITCH_PRODUCT_NAME "ProductName: Smart Card Switch HV1.0 SV1.0"
#define SMART_CARD_SWITCH_SOLENOID_AMOUNT 4
#define SMART_CARD_SWITCH_DEFAULT_SOLENOID_DURATION_DIVISON 128

static struct SmartCardSwitch_CommandState
{
	enum CommandState state;

	unsigned char command;
	unsigned char param;
	unsigned char solenoidDurationDivision;

	union U
	{
		struct
		{
			unsigned char solenoidIndex;
			unsigned char solenoidPairIndex;
			unsigned char cycleIndex; // a cycle includes an activation period and a deactivation period.
			bool phaseActivationFinished;
			bool phaseDeactivationFinished;
			unsigned short initialCounter;
			unsigned short period;
		} solenoid;
	} u;
	
} smartCardSwitch_CommandState;

static struct SmartCardSwitch_MachineStatus
{
	//0: no smart card is connected
	//!0: number of the connected smart card
	unsigned short smartCardConnected;

	bool solenoidActivated[SMART_CARD_SWITCH_SOLENOID_AMOUNT];
	bool solenoidIsPowered[SMART_CARD_SWITCH_SOLENOID_AMOUNT];

	bool powerAvailable;
	bool powerFuseBroken;
} smartCardSwitch_status;


static void SmartCardSwitch_activate_status()
{
	PORTF_DIRSET = 0x01;
	PORTF_OUTSET = 0x01;
}

static void SmartCardSwitch_deactivate_status()
{
	PORTF_DIRCLR = 0x01;
	PORTF_OUTCLR = 0x01;
}

static void SmartCardSwitch_solenoid_deactivate_all(){
	PORTC_DIR=0x00;
	PORTC_OUT=0x00;
}

static void SmartCardSwitch_smartcard_disconnect_all()
{
	PORTA_DIR=0x00;
	PORTK_DIR=0x00;
	PORTA_OUT=0x00;
	PORTK_OUT=0x00;
	SmartCardSwitch_deactivate_status();
}

//return true if any solenoid is enabled, otherwise return false.
static bool SmartCardSwitch_solenoid_activate(unsigned char channel)
{
	SmartCardSwitch_solenoid_deactivate_all();
	
	switch(channel)
	{
		case 1: //pc7
		PORTC_OUT=0x80;
		PORTC_DIR=0x80;
		break;
		case 2: //pc5
		PORTC_OUT=0x20;
		PORTC_DIR=0x20;
		break;
		case 3: //pc3
		PORTC_OUT=0x08;
		PORTC_DIR=0x08;
		break;
		case 4: //pc1
		PORTC_OUT=0x02;
		PORTC_DIR=0x02;
		break;
		default:
		return false;
		break;
	}
	return true;
}

static bool SmartCardSwitch_solenoid_is_activated(unsigned char channel)
{
	switch(channel)
	{
		case 1: //pc7
		return PORTC_DIR&0x80;
		case 2: //pc5
		return PORTC_DIR&0x20;
		case 3: //pc3
		return PORTC_DIR&0x08;
		case 4: //pc1
		return PORTC_DIR&0x02;
		default:
		return false;
	}
}

// return true if the designated solenoid is activated
// return false if the designated solenoid is not activated
static bool SmartCardSwitch_solenoid_is_powered(unsigned char channel)
{
	switch(channel)
	{
		case 1: //pc6
		if(PORTC_IN & 0x40)
		return false;
		else
		return true;
		case 2: //pc4
		if(PORTC_IN & 0x10)
		return false;
		else
		return true;
		case 3: //pc2
		if(PORTC_IN & 0x04)
		return false;
		else
		return true;
		case 4: //pc0
		if(PORTC_IN & 0x01)
		return false;
		else
		return true;
		default:
		return false;
	}
}

static bool SmartCardSwitch_smartCard_activate(unsigned char index)
{
	SmartCardSwitch_smartcard_disconnect_all();
	
	switch(index)
	{
		case 1:
		PORTA_OUTSET = 0x80;
		PORTA_DIRSET = 0x80;
		break;
		case 2:
		PORTA_OUTSET = 0x40;
		PORTA_DIRSET = 0x40;
		break;
		case 3:
		PORTA_OUTSET = 0x20;
		PORTA_DIRSET = 0x20;
		break;
		case 4:
		PORTA_OUTSET = 0x10;
		PORTA_DIRSET = 0x10;
		break;
		case 5:
		PORTA_OUTSET = 0x08;
		PORTA_DIRSET = 0x08;
		break;
		case 6:
		PORTA_OUTSET = 0x04;
		PORTA_DIRSET = 0x04;
		break;
		case 7:
		PORTA_OUTSET = 0x02;
		PORTA_DIRSET = 0x02;
		break;
		case 8:
		PORTA_OUTSET = 0x01;
		PORTA_DIRSET = 0x01;
		break;
		case 9:
		PORTK_OUTSET = 0x80;
		PORTK_DIRSET = 0x80;
		break;
		case 10:
		PORTK_OUTSET = 0x40;
		PORTK_DIRSET = 0x40;
		break;
		case 11:
		PORTK_OUTSET = 0x20;
		PORTK_DIRSET = 0x20;
		break;
		case 12:
		PORTK_OUTSET = 0x10;
		PORTK_DIRSET = 0x10;
		break;
		case 13:
		PORTK_OUTSET = 0x08;
		PORTK_DIRSET = 0x08;
		break;
		case 14:
		PORTK_OUTSET = 0x04;
		PORTK_DIRSET = 0x04;
		break;
		case 15:
		PORTK_OUTSET = 0x02;
		PORTK_DIRSET = 0x02;
		break;
		case 16:
		PORTK_OUTSET = 0x01;
		PORTK_DIRSET = 0x01;
		break;
		default:
		return false;
		break;
	}
	
	SmartCardSwitch_activate_status();
	return true;
}

static unsigned char SmartCardSwitch_smartCard_get_activated()
{
	if(PORTA_DIR & 0x80)
	return 1;
	if(PORTA_DIR & 0x40)
	return 2;
	if(PORTA_DIR & 0x20)
	return 3;
	if(PORTA_DIR & 0x10)
	return 4;
	if(PORTA_DIR & 0x08)
	return 5;
	if(PORTA_DIR & 0x04)
	return 6;
	if(PORTA_DIR & 0x02)
	return 7;
	if(PORTA_DIR & 0x01)
	return 8;
	if(PORTK_DIR & 0x80)
	return 9;
	if(PORTK_DIR & 0x40)
	return 10;
	if(PORTK_DIR & 0x20)
	return 11;
	if(PORTK_DIR & 0x10)
	return 12;
	if(PORTK_DIR & 0x08)
	return 13;
	if(PORTK_DIR & 0x04)
	return 14;
	if(PORTK_DIR & 0x02)
	return 15;
	if(PORTK_DIR & 0x01)
	return 16;
	
	return 0;
}

static bool test_smart_card_connection(void)
{
	unsigned char tmp;
	
	unsigned short initialCounter;
	unsigned short currentCounter;
	//wait for 1/8 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)/8) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)/8) {
				break;
			}
		}
	}
	
	PORTB_DIR = 0x0F;
	
	PORTB_OUT = 0x00;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}
	
	PORTB_OUT = 0x0F;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0xF0) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}
	
	PORTB_OUT = 0x01;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0x10) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	PORTB_OUT = 0x02;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0x20) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	PORTB_OUT = 0x04;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0x40) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	PORTB_OUT = 0x08;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0x80) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	PORTB_OUT = 0x0E;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0xE0) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	PORTB_OUT = 0x0D;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0xD0) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	PORTB_OUT = 0x0B;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0xB0) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	PORTB_OUT = 0x07;
	tmp = PORTB_IN;
	if((tmp & 0xF0) != 0x70) {
		writeOutputBufferChar('0' + (PORTB_OUT&0x0F));
		writeOutputBufferChar('0' + (tmp>>4));
		writeOutputBufferString("\r\n");
		return false;
	}

	return true;
}

static bool test_smart_card_connection_slow(void)
{
	unsigned short initialCounter;
	unsigned short currentCounter;
	
	PORTB_DIR = 0x0F;
	
	PORTB_OUT = 0x00;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0) {
		return false;
	}
	
	PORTB_OUT = 0x0F;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0xF0) {
		return false;
	}
	
	PORTB_OUT = 0x01;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0x10) {
		return false;
	}

	PORTB_OUT = 0x02;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0x20) {
		return false;
	}

	PORTB_OUT = 0x04;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0x40) {
		return false;
	}

	PORTB_OUT = 0x08;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0x80) {
		return false;
	}

	PORTB_OUT = 0x0E;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0xE0) {
		return false;
	}

	PORTB_OUT = 0x0D;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0xD0) {
		return false;
	}

	PORTB_OUT = 0x0B;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0xB0) {
		return false;
	}

	PORTB_OUT = 0x07;
	//wait for 1 second
	initialCounter = tc_read_count(&TCC0);
	for(;;)
	{
		currentCounter = tc_read_count(&TCC0);
		
		if(currentCounter > initialCounter) {
			if((currentCounter - initialCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
		else if(currentCounter < initialCounter) {
			// a wrap around
			if(((0xffff - initialCounter) + currentCounter) > tc_get_resolution(&TCC0)) {
				break;
			}
		}
	}
	if((PORTB_IN & 0xF0) != 0x70) {
		return false;
	}

	return true;
}

// is power available
bool SmartCardSwitch_is_power_ok()
{
	//PD5
	if(PORTD_IN	& 0x20) {
		return false;
	}
	else {
		return true;
	}
}

bool SmartCardSwitch_is_power_fuse_ok()
{
	//PD4
	if(PORTD_IN & 0x10)
	return true;
	else
	return false;
}

static void SmartCardSwitch_init_status()
{
	unsigned char solenoidIndex;

	smartCardSwitch_status.smartCardConnected = 0;
	
	for(solenoidIndex = 1; solenoidIndex <= SMART_CARD_SWITCH_SOLENOID_AMOUNT; solenoidIndex++){
		smartCardSwitch_status.solenoidActivated[solenoidIndex - 1] = false;
		smartCardSwitch_status.solenoidIsPowered[solenoidIndex - 1] = false;
	}
	
	smartCardSwitch_status.powerAvailable = false;
	smartCardSwitch_status.powerFuseBroken = false;
}

static void SmartCardSwitch_check_status()
{
	unsigned char activatedSmartCard;
	bool powerAvailable;
	bool powerFuseBroken;

	//check which smart card is connected.
	activatedSmartCard = SmartCardSwitch_smartCard_get_activated();
	if(smartCardSwitch_status.smartCardConnected != activatedSmartCard) {
		//smart card status changed.
		if(smartCardSwitch_status.smartCardConnected != 0) {
			writeOutputBufferString("SC 0x");
			writeOutputBufferHex(smartCardSwitch_status.smartCardConnected);
			writeOutputBufferString(" disconnected\r\n");
		}
		smartCardSwitch_status.smartCardConnected = activatedSmartCard;
		if(smartCardSwitch_status.smartCardConnected != 0) {
			writeOutputBufferString("SC 0x");
			writeOutputBufferHex(smartCardSwitch_status.smartCardConnected);
			writeOutputBufferString(" connected\r\n");
		}
	}

	//check 24V and its fuse
	powerAvailable = SmartCardSwitch_is_power_ok();
	if(smartCardSwitch_status.powerAvailable != powerAvailable)
	{
		smartCardSwitch_status.powerAvailable = powerAvailable;
		if(powerAvailable) {
			writeOutputBufferString("Power connected\r\n");
		}
		else {
			writeOutputBufferString("Power disconnected\r\n");
		}
	}
	powerFuseBroken = !SmartCardSwitch_is_power_fuse_ok();
	if(smartCardSwitch_status.powerFuseBroken != powerFuseBroken) {
		smartCardSwitch_status.powerFuseBroken = powerFuseBroken;
		if(powerFuseBroken) {
			writeOutputBufferString("PowerFuse is broken\r\n");
		}
		else {
			writeOutputBufferString("PowerFuse is OK\r\n");
		}
	}
	
	//check solenoids
	for(unsigned char solenoidIndex = 1; solenoidIndex <= SMART_CARD_SWITCH_SOLENOID_AMOUNT; solenoidIndex++)
	{
		if(smartCardSwitch_status.solenoidActivated[solenoidIndex - 1] != SmartCardSwitch_solenoid_is_activated(solenoidIndex)) {
			smartCardSwitch_status.solenoidActivated[solenoidIndex - 1] = SmartCardSwitch_solenoid_is_activated(solenoidIndex);
			if(smartCardSwitch_status.solenoidActivated[solenoidIndex - 1]) {
				writeOutputBufferString("Solenoid 0x");
				writeOutputBufferHex(solenoidIndex);
				writeOutputBufferString(" is activated\r\n");
			}
			else {
				writeOutputBufferString("Solenoid 0x");
				writeOutputBufferHex(solenoidIndex);
				writeOutputBufferString(" is not activated\r\n");
			}
		}

		if(smartCardSwitch_status.solenoidIsPowered[solenoidIndex - 1] != SmartCardSwitch_solenoid_is_powered(solenoidIndex)) {
			smartCardSwitch_status.solenoidIsPowered[solenoidIndex - 1] = SmartCardSwitch_solenoid_is_powered(solenoidIndex);
			if(smartCardSwitch_status.solenoidIsPowered[solenoidIndex - 1]) {
				writeOutputBufferString("Solenoid 0x");
				writeOutputBufferHex(solenoidIndex);
				writeOutputBufferString(" is powered\r\n");
			}
			else {
				writeOutputBufferString("Solenoid 0x");
				writeOutputBufferHex(solenoidIndex);
				writeOutputBufferString(" is not powered\r\n");
			}
		}
	}
}

static void SmartCardSwitch_write_status()
{
	//Power
	if(smartCardSwitch_status.powerAvailable) {
		writeOutputBufferString("Power connected\r\n");
	}
	else {
		writeOutputBufferString("Power disconnected\r\n");
	}
	if(smartCardSwitch_status.powerFuseBroken) {
		writeOutputBufferString("PowerFuse is broken\r\n");
	}
	else {
		writeOutputBufferString("PowerFuse is OK\r\n");
	}

	//smart card
	if(smartCardSwitch_status.smartCardConnected != 0) {
		writeOutputBufferString("SC 0x");
		writeOutputBufferHex(smartCardSwitch_status.smartCardConnected);
		writeOutputBufferString(" connected\r\n");
	}
	else {
		writeOutputBufferString("No smart card connected\r\n");
	}

	//solenoids
	for(unsigned char solenoidIndex = 1; solenoidIndex <= SMART_CARD_SWITCH_SOLENOID_AMOUNT; solenoidIndex++)
	{
		writeOutputBufferString("Solenoid 0x");
		writeOutputBufferHex(solenoidIndex);
		if(smartCardSwitch_status.solenoidActivated[solenoidIndex - 1]) {
			writeOutputBufferString(" is activated\r\n");
		}
		else {
			writeOutputBufferString(" is not activated\r\n");
		}

		writeOutputBufferString("Solenoid 0x");
		writeOutputBufferHex(solenoidIndex);
		if(smartCardSwitch_status.solenoidIsPowered[solenoidIndex - 1]) {
			writeOutputBufferString(" is powered\r\n");
		}
		else {
			writeOutputBufferString(" is not powered\r\n");
		}
	}
}

void SmartCardSwitch_run_command()
{
	if(smartCardSwitch_CommandState.state == STARTING_COMMAND)
	{
		switch(smartCardSwitch_CommandState.command)
		{
			case 'I'://insert smart card
			smartCardSwitch_CommandState.u.solenoid.solenoidIndex = 1;
			smartCardSwitch_CommandState.u.solenoid.solenoidPairIndex = 2;
			smartCardSwitch_CommandState.u.solenoid.cycleIndex = 0;
			smartCardSwitch_CommandState.u.solenoid.phaseActivationFinished = false;
			smartCardSwitch_CommandState.u.solenoid.phaseDeactivationFinished = false;
			smartCardSwitch_CommandState.u.solenoid.initialCounter = counter_get();
			smartCardSwitch_CommandState.u.solenoid.period = tc_get_resolution(&TCC0)/smartCardSwitch_CommandState.solenoidDurationDivision;
			SmartCardSwitch_solenoid_activate(smartCardSwitch_CommandState.u.solenoid.solenoidIndex);
			smartCardSwitch_CommandState.state = EXECUTING_COMMAND;
			break;

			case 'P'://pull out smart card
			smartCardSwitch_CommandState.u.solenoid.solenoidIndex = 3;
			smartCardSwitch_CommandState.u.solenoid.solenoidPairIndex = 0;
			smartCardSwitch_CommandState.u.solenoid.cycleIndex = 0;
			smartCardSwitch_CommandState.u.solenoid.phaseActivationFinished = false;
			smartCardSwitch_CommandState.u.solenoid.phaseDeactivationFinished = false;
			smartCardSwitch_CommandState.u.solenoid.initialCounter = counter_get();
			smartCardSwitch_CommandState.u.solenoid.period = tc_get_resolution(&TCC0)/smartCardSwitch_CommandState.solenoidDurationDivision;
			SmartCardSwitch_solenoid_activate(smartCardSwitch_CommandState.u.solenoid.solenoidIndex);
			smartCardSwitch_CommandState.state = EXECUTING_COMMAND;
			break;

			case 'C'://connect smart card
			SmartCardSwitch_smartCard_activate(smartCardSwitch_CommandState.param);
			smartCardSwitch_CommandState.state = AWAITING_COMMAND;
			break;

			case 'Q'://query
			writeOutputBufferString(SMART_CARD_SWITCH_PRODUCT_NAME);
			writeOutputBufferString("\r\n");
			writeOutputBufferString("SolenoidDuration 1/0x");
			writeOutputBufferHex(smartCardSwitch_CommandState.solenoidDurationDivision);
			writeOutputBufferString(" second\r\n");
			SmartCardSwitch_write_status();
			smartCardSwitch_CommandState.state = AWAITING_COMMAND;
			break;

			case 'D'://division
			if(smartCardSwitch_CommandState.param > 0) {
				smartCardSwitch_CommandState.solenoidDurationDivision = smartCardSwitch_CommandState.param;
			}
			else {
				writeOutputBufferString("Invalid parameter of 'D' command. Restore division to default value\r\n");
				smartCardSwitch_CommandState.solenoidDurationDivision = SMART_CARD_SWITCH_DEFAULT_SOLENOID_DURATION_DIVISON;
			}
			smartCardSwitch_CommandState.state = AWAITING_COMMAND;
			break;
			
			default:
			writeOutputBufferString("Unknown command at STARTING_COMMAND\r\n");
			SmartCardSwitch_solenoid_deactivate_all();
			SmartCardSwitch_smartcard_disconnect_all();
			smartCardSwitch_CommandState.state = AWAITING_COMMAND;
			break;
		}
	}

	if(smartCardSwitch_CommandState.state == EXECUTING_COMMAND)
	{
		switch(smartCardSwitch_CommandState.command)
		{
			case 'I': // fall through
			case 'P':
			{
				unsigned short currentCounter;
				unsigned short initialCounter = smartCardSwitch_CommandState.u.solenoid.initialCounter;
				bool periodExpired = false;

				currentCounter = counter_get();
				if(currentCounter > smartCardSwitch_CommandState.u.solenoid.initialCounter) {
					if((currentCounter - smartCardSwitch_CommandState.u.solenoid.initialCounter) > smartCardSwitch_CommandState.u.solenoid.period) {
						periodExpired = true;
					}
				}
				else if(currentCounter < smartCardSwitch_CommandState.u.solenoid.initialCounter) {
					// a wrap around
					if(((0xffff - smartCardSwitch_CommandState.u.solenoid.initialCounter) + currentCounter) > smartCardSwitch_CommandState.u.solenoid.period) {
						periodExpired = true;
					}
				}
				
				if(periodExpired) {
					if(smartCardSwitch_CommandState.u.solenoid.phaseActivationFinished == false) {
						//activation phase finished
						smartCardSwitch_CommandState.u.solenoid.phaseActivationFinished = true;
						SmartCardSwitch_solenoid_deactivate_all();
						if(smartCardSwitch_CommandState.u.solenoid.solenoidPairIndex != 0) {
							//activate the other solenoid
							SmartCardSwitch_solenoid_activate(smartCardSwitch_CommandState.u.solenoid.solenoidPairIndex);
						}
						smartCardSwitch_CommandState.u.solenoid.initialCounter = currentCounter;
					}
					else if(smartCardSwitch_CommandState.u.solenoid.phaseDeactivationFinished == false) {
						//deactivation phase finished
						SmartCardSwitch_solenoid_deactivate_all();
						smartCardSwitch_CommandState.u.solenoid.phaseDeactivationFinished = true;
					}
				}

				if((smartCardSwitch_CommandState.u.solenoid.phaseActivationFinished == true) &&
				(smartCardSwitch_CommandState.u.solenoid.phaseDeactivationFinished == true))
				{
					//one cycleIndex finished
					smartCardSwitch_CommandState.u.solenoid.cycleIndex++;
					if(smartCardSwitch_CommandState.u.solenoid.cycleIndex < smartCardSwitch_CommandState.param)
					{
						//start a new cycleIndex
						smartCardSwitch_CommandState.u.solenoid.phaseActivationFinished = false;
						smartCardSwitch_CommandState.u.solenoid.phaseDeactivationFinished = false;
						smartCardSwitch_CommandState.u.solenoid.initialCounter = currentCounter;
						SmartCardSwitch_solenoid_activate(smartCardSwitch_CommandState.u.solenoid.solenoidIndex);
					}
					else
					{
						//all cycleIndexs have finished.
						smartCardSwitch_CommandState.state = AWAITING_COMMAND;
					}
				}
			}
			break;

			default:
			{
				writeOutputBufferString("Unknown command at EXECUTING_COMMAND\r\n");
				SmartCardSwitch_solenoid_deactivate_all();
				SmartCardSwitch_smartcard_disconnect_all();
				smartCardSwitch_CommandState.state = AWAITING_COMMAND;
			}
			break;
		}
	}
}

void ecd300SmartCardSwitch(void)
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

	smartCardSwitch_CommandState.solenoidDurationDivision = SMART_CARD_SWITCH_DEFAULT_SOLENOID_DURATION_DIVISON;
	
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
			if((key == 0x0D) && (smartCardSwitch_CommandState.state == AWAITING_COMMAND))
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
					SmartCardSwitch_solenoid_deactivate_all();
					SmartCardSwitch_smartcard_disconnect_all();
				}
				else
				{
					switch(cmd)
					{
						case 'I':
						case 'i':
						// Insert smart card.
						smartCardSwitch_CommandState.state = STARTING_COMMAND;
						smartCardSwitch_CommandState.command = 'I';
						smartCardSwitch_CommandState.param = param;
						break;
						
						case 'P':
						case 'p':
						// Pullout smart card.
						smartCardSwitch_CommandState.state = STARTING_COMMAND;
						smartCardSwitch_CommandState.command = 'P';
						smartCardSwitch_CommandState.param = param;
						break;
						
						case 'C':
						case 'c':
						//connect smart card
						smartCardSwitch_CommandState.state = STARTING_COMMAND;
						smartCardSwitch_CommandState.command = 'C';
						smartCardSwitch_CommandState.param = param;
						break;
						
						case 'D':
						case 'd':
						//solenoid duration division
						smartCardSwitch_CommandState.state = STARTING_COMMAND;
						smartCardSwitch_CommandState.command = 'D';
						smartCardSwitch_CommandState.param = param;
						break;
						
						case 'Q':
						case 'q':
						// Query
						smartCardSwitch_CommandState.state = STARTING_COMMAND;
						smartCardSwitch_CommandState.command = 'Q';
						break;
						default:
						writeOutputBufferString("Invalid command\r\n");
						SmartCardSwitch_solenoid_deactivate_all();
						SmartCardSwitch_smartcard_disconnect_all();
						break;
					}
				}
			}
		}

		SmartCardSwitch_run_command();
		
		SmartCardSwitch_check_status();

		sendOutputBufferToHost();
	}

	while(1)
	{
		;
	}
}