
#include <xc.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <p33EP32MC204.h>

#include "user.h"
#include "paramether.h"
#include "system.h"
#include "ioPorts.h"
#include "ecrc.h"
#include "PID.h"
#include "time.h"
#include "Encoder.h"
#include "debug.h"
#include "Commun.h"

/** This variables holds present sector value, which is the rotor position*/
unsigned int Sector;
/** This variable holds the last sector value. This is critical to filter 
slow slew rate on the Hall effect sensors hardware*/
unsigned int LastSector;
/** This variable gets incremented each 100us, and is cleared everytime a new
 sector is detected. Used for ForceCommutation and MotorStalled 
 protection functions*/
unsigned int MotorStalledCounter = 0;
/** This array translates the hall state value read from the digital I/O to the
 proper sector.  Hall values of 0 or 7 represent illegal values and therefore
 return -1.*/
char SectorTable[] = {-1, 4, 2, 3, 0, 5, 1, -1};

/** Controller output, used as a voltage output, use its sign for the required direction*/
int ControlOutput = 0;

volatile int32_t PositionHall = 0;
volatile bool ParamChanged = false;
int16_t MotorCurrentOffset = 0;

PID_PARMS_t PID_Current = {.coeficients = &Parameter.CurrentCoeficients,
						   .integral = 0,
						   .error = 0,
						   .previousError = 0,
						   .output = 0,
						   .posLimit = MAX_POS_DUTY,
						   .negLimit = MAX_NEG_DUTY,
						   .scallingFactor = 2048};
PID_PARMS_t PID_Position = {.coeficients = &Parameter.PositionCoeficients,
							.integral = 0,
							.error = 0,
							.previousError = 0,
							.output = 0,
							.posLimit = MAX_POS_DUTY,
							.negLimit = MAX_NEG_DUTY,
							.scallingFactor = 1024};


//********************************************************************************************************
void InitMCPWM(void);
void InitOpAmp(void);
void InitADC(void);
void InitComparator(void);
void InitCNforHall(void);
void InitTMR1(void);
void InitTMR2(void);
void InitTMR3(void);
void InitTMR4(void);
void InitINT1(void);

//********************************************************************************************************

void InitApp(void) {
	Ports_Initialize();

	DBG_Init();
	DBG_PRINT("\r\nSystem initialization\r\n");
	//	DBG_PRINT("rcon reg: %#4X, \r\n", RCON);
	//	RCON = 0;

	InitTMR1();
	InitTMR2();
	InitTMR3();
	InitTMR4();

	InitCNforHall();
	InitMCPWM();
	InitOpAmp();
	InitADC();
	InitComparator();
	InitEncoder();

	ECRCInit(0x4C11DB7, // polynomial
			32, // polynomial order
			32, // data bit width
			0); // big endian
	// get initial value in indirect form
	seed = ECRCIndirectSeed(0xFFFFFFFF); //(0x0000ffff); //0x46AF6449;//
	// set initial seed value
	ECRCSetSeed(seed);

	uartCommunicationInit();

	InitINT1(); // interrupt for takt signal

	memset(Parameter.cTab, 0, sizeof (PARAMETER_t));
	DBG_PRINT("\r\nSystem initialized\r\n");
}

/** *******************************************************************
  @Function        void InitMCPWM(void)
  @PreCondition    None.
 
  @Input           None.
  @Output          None.
  @SideEffects    None.
  @Overview        InitMCPWM, intializes the PWM as follows:
				   1. FPWM = 20000 hz
				   2. Complementary PWMs with center aligned
				   3. Set Duty Cycle to 0 for complementary, which is half 
					  the period
				   4. Set ADC to be triggered by PWM special trigger
				   5. Configure deadtime to be 2 us	
  @Note            None.
 ******************************************************************* */
void InitMCPWM(void) {
	PTCON = 0;

	PHASE1 = (FCY / FPWM - 1);
	PHASE2 = (FCY / FPWM - 1);
	PHASE3 = (FCY / FPWM - 1);
	// PWM outputs assignned to PWM module
	IOCON1 = 0xC300;
	IOCON2 = 0xC300;
	IOCON3 = 0xC300;

	DTR1 = 0x0000; // 2 us of dead time
	DTR2 = 0x0000; // 2 us of dead time
	DTR3 = 0x0000; // 2 us of dead time

	ALTDTR1 = 60; // 0.5us of dead time		
	ALTDTR2 = 60; // 0.5 us of dead time
	ALTDTR3 = 60; // 0.5 us of dead time

	//Note: ALTDTR_DIV2 define in svm.c should be half of ALTDTRx

	PTCON2 = 0x0000; // Divide by 1 to generate PWM

	PWMCON1 = 0x0604; // Enable PWM output pins and configure them as 
	PWMCON2 = 0x0204; // complementary mode
	PWMCON3 = 0x0204;

	PDC1 = PHASE1 / 2; // Initialize as 0 voltage
	PDC2 = PHASE2 / 2; // Initialize as 0 voltage
	PDC3 = PHASE3 / 2; // Initialize as 0 voltage

	// Enable triggering for ADC
	//	SEVTCMP = 1; 
	TRIG1 = PHASE1;
	TRGCON1 = 0;

	FCLCON1 = 0x2107; // Current-Limit Control is enabled and Signal Source is Comparator5
	FCLCON2 = 0x2107;
	FCLCON3 = 0x2107;

	PTCON = 0x8000; // start PWM

	return;
}

/*********************************************************************
  Function:        void InitTMR1(void)
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        Initialization of Timer 1 as the timebase for motor PID calculation.
  Note:            None.
 ********************************************************************/
void InitTMR1(void) {
	T1CON = 0;
	TMR1 = 0;
	PR1 = TICKS_PER_MILISECONDS / 10 - 1; // period for 100us
	_T1IF = 0;
	_T1IP = 7;
	T1CONbits.TON = 1; // Start Timer1
}

/*********************************************************************
  Function:        void InitTMR2(void)
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        Initialization of Timer 2 as the system timer with 1ms period.
  Note:            None.
 ********************************************************************/
void InitTMR2(void) {
	T2CON = 0;
	TMR2 = 0;
	PR2 = TICKS_PER_MILISECONDS - 1; // period for 1ms
	_T2IF = 0;
	_T2IE = 1;
	T2CONbits.TON = 1; // Start Timer1
}

/*********************************************************************
  Function:        void InitCNforHall(void)
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        Configure Hall sensor inputs, as Change Notification Interrupt.
  Note:            None.
 ********************************************************************/
void InitCNforHall(void) {
	CNENAbits.CNIEA8 = 1;
	CNENCbits.CNIEC6 = 1;
	CNENCbits.CNIEC7 = 1;

	return;
}

/*********************************************************************
  Function:        void InitTMR3(void)
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        Initialization of Timer 3 as the timebase for the capture 
				   channels for calculating the period of the halls.
  Note:            None.
 ********************************************************************/
void InitTMR3(void) {
	T3CON = 0x0030; // internal Tcy/256 clock
	TMR3 = 0;
	PR3 = 0xFFFF;
	T3CONbits.TON = 1; // turn on timer 3 
	return;
}

/*********************************************************************
  Function:        void InitTMR4(void)
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        Initialization of the timer 4 used to prevent false interrupts on a slowly rising or falling edge
  Note:            None.
 ********************************************************************/
void InitTMR4(void) {
	T4CON = 0;
	TMR4 = 0;
	PR4 = 2999; //50us

	_T4IF = 0; // clear interrupt flag
	_T4IE = 1; // enable interrupt
	return;
}

/*---------------------------------------------------------------------
  Function Name: InitOpAmp
  Description:   Initialize operational amplifier module
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void InitOpAmp(void) {
	CM2CON = 0;
	CM2MSKSRC = 0;
	CM2MSKCON = 0;
	CM2FLTR = 0;

	//	ANSELAbits.ANSA0 = 1; // RA0 as OpAmp output
	ANSELAbits.ANSA1 = 1; // RA1 as opAmp input
	ANSELBbits.ANSB0 = 1; // RB0 as opAmp input

	// errata if using opAmp then bit 11 in apropriate CMxCON register must be set
	CM2CON = 0x0800;
	CM2CONbits.CCH = 0; // Inverting input of op amp/comparator connects to CxIN1- pin

	CM2CONbits.OPMODE = 1; // enabla as an Op Amp

	CM2CONbits.CON = 1; // Op Amp is enabled
}

/*---------------------------------------------------------------------
  Function Name: InitADC
  Description:   Initialize ADC module
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void InitADC(void) {
	/* Output Format is Integer */
	AD1CON1bits.FORM = 0;
	/* Internal Counter Ends Sampling and Starts Conversion */
	AD1CON1bits.SSRC = 0;
	AD1CON1bits.SSRCG = 1;
	/* Sampling begins immediately after last conversion */
	AD1CON1bits.ASAM = 1;
	/* Select 12-bit, 1 channel ADC operation */
	AD1CON1bits.AD12B = 1;

	/* Channel scan for CH0+, Use MUX A, SMPI = 1 per interrupt, Vref = AVdd/AVss */
	AD1CON2 = 0x0000;

	/* Set Samples and bit conversion time */
	AD1CON3 = 0x0303;

	/* Disable DMA */
	AD1CON4 = 0x0000;

	/* No Channels to Scan */
	AD1CSSH = 0x0000;
	AD1CSSL = 0x0000;

	/* Channel select: positive input is the output of AN0 */
	AD1CHS0 = 25;

	/* Reset ADC interrupt flag */
	_AD1IF = 0;

	/* Enable ADC interrupts, disable this interrupt if the DMA is enabled */
	_AD1IE = 1;

	/* Turn on ADC module */
	AD1CON1bits.ADON = 1;
}

/*---------------------------------------------------------------------
  Function Name: InitComparator
  Description:   Initialize comparator module
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void InitComparator(void) {
	CMSTAT = 0;
	CM1CON = 0;
	CM1MSKSRC = 0;
	CM1MSKCON = 0;
	CM1FLTR = 0;
	CVRCON = 0;

	ANSELBbits.ANSB3 = 1; // RB3 as comparator input

	CM1CONbits.COE = 0; //0 = Comparator output is internal only
	/* Comparator Output Polarity Select bit */
	CM1CONbits.CPOL = 1;
	CM1CONbits.CREF = 1; // VIN+ input connects to internal CVREFIN voltage
	CM1CONbits.CCH = 0; //  Inverting input of op amp/comparator connects to CxIN1- pin

	CVRCONbits.VREFSEL = 0; // CVREFIN is generated by the resistor network
	CVRCONbits.CVREN = 1; // Comparator voltage reference circuit is powered on
	CVRCONbits.CVR1OE = 0; // Voltage level is disconnected from the CVREF1O pin
	/* CVRR: Comparator Voltage Reference Range Selection bits
		1 = CVRSRC/24 step-size,  CVREFIN = (CVR<3:0>/24) * (CVRSRC)
		0 = CVRSRC/32 step-size, CVREFIN = (CVRSRC/4) + (CVR<3:0>/32) * (CVRSRC)*/
	CVRCONbits.CVRR = 1;

	CVRCONbits.CVRSS = 0; // Comparator voltage reference source, CVRSRC = AVDD ? AVSS
	/* CVR<3:0> Comparator Voltage Reference Value Selection 0 <= CVR<3:0> >= 15 bits */
	CVRCONbits.CVR = 14;

	CVRCONbits.VREFSEL = 0; // Reference source for inverting input is from CVR1

	CM1CONbits.CON = 1; // Comparator is enabled
}

/*---------------------------------------------------------------------
  Function Name: InitINT1
  Description:   Initialize external interrupt to capture rising edge on "TAKT" input
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void InitINT1(void) {
	_INT1EP = 0; //Interrupt on positive edge

	_INT1IF = 0;
	_INT1IE = 1;
}

/*********************************************************************
  Function:        void CurrentMaxLimit(void) 
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        This subroutine control max allowed current for the board
  Note:            None.
 ********************************************************************/
void CurrentMaxLimit(void) {
	if (Parameter.StatFlags.MotorIsRunning != 0) {
		if (Parameter.MotorCurrent > CRITICAL_CURRENT) {
			Parameter.error.criticalOvercurrent = 1;
			return;
		}
	}
}

/*********************************************************************
  Function:        void CheckChangedParameter(void) 
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        This subroutine check changed parameter
  Note:            None.
 ********************************************************************/
void CheckChangedParameter(void) {
	if (ParamChanged == true) {
		if (Parameter.maxMotorCurrent > MOTOR_CURR_LIMIT)
			Parameter.maxMotorCurrent = MOTOR_CURR_LIMIT;

		ParamChanged = false;
	}
}

/*********************************************************************
  Function:        int32_t MotorGetPosition(void)
  PreCondition:    None.
 
  Input:           None.
  Output:          Position from encoder or hall sensors.
  Side Effects:    None.
  Overview:        Get position from encoder or hall sensors depend on configuration bits 
  Note:            None.
 ********************************************************************/
int32_t MotorGetPosition(void) {
	if (Parameter.ControlFlags.isEncoder)
		return EncoderGetPosition();
	else
		return PositionHall;
}

/*********************************************************************
  Function:        void MotorClearPosition(void)
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        This subroutine 
  Note:            None.
 ********************************************************************/
void MotorClearPosition(void) {
	if (Parameter.ControlFlags.isEncoder) {
		Parameter.PositionRequired = 0;
		EnkoderClearPosition();
	} else {
		Parameter.PositionRequired = 0;
		PositionHall = 0;
	}
}

/*********************************************************************
  Function:        void setDefaultParameters(void)
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        Set default value for all parameters
  Note:            None.
 ********************************************************************/
void setDefaultParameters(void) {
	Parameter.ControlFlags.isEncoder = 0;
	Parameter.ControlFlags.controlCurrent = 1;
	Parameter.ControlFlags.controlPosition = 1;
	Parameter.ControlFlags.MotorRun = 0;

	Parameter.PositionRequired = 0;
	Parameter.PositionStep = 1;
	Parameter.PositionOkOutPrecision = 5;


	Parameter.MotorCurrentScale = MOTOR_CURR_SCALE;
	Parameter.maxMotorCurrent = 1000; // 10A

	Parameter.error.all = 0;

	MotorClearPosition();

	Parameter.CurrentCoeficients.kp = PID_CURRENT_KP;
	Parameter.CurrentCoeficients.ki = PID_CURRENT_KI;
	Parameter.CurrentCoeficients.kd = PID_CURRENT_KD;

	Parameter.PositionCoeficients.kp = PID_POSITION_KP;
	Parameter.PositionCoeficients.ki = PID_POSITION_KI;
	Parameter.PositionCoeficients.kd = PID_POSITION_KD;
}

/*********************************************************************
  Function:			inline unsigned int GetHallValue(void)
  PreCondition:		None
 
  Input:			None
  Output:			Returns current Sector
  Side Effects:		None
  Overview:			Get Sector from table based on Hall
  Note:				None
 ********************************************************************/
inline char GetSector(void) {
	unsigned int HallValue;
	HallValue = (unsigned int) ((PORTA & 0x0100) >> 8 | (PORTC & 0x00C0) >> 5); // Read halls

	return (char) (SectorTable[HallValue]);
}

/*********************************************************************
  Function:        void ForceCommutation(void)
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        This function is called each time the motor doesn't 
				   generate	hall change interrupt, which means that the motor
				   running too slow or is stalled. If it is stalled, the motor
				   is stopped, but if it is only slow, this function is called
				   and forces a commutation based on the actual hall sensor 
				   position and the required direction of rotation.
  Note:            None.
 ********************************************************************/
void ForceCommutation(void) {
	Sector = GetSector(); // Read sector based on halls
	if (Sector != -1) // If the sector is invalid don't do anything
	{
	}
	return;
}

/*********************************************************************
  Function:        void StopMotor(void)
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        Call this subroutine whenever the user want to stop the 
				   motor. This subroutine will clear interrupts properly, and
				   will also turn OFF all PWM channels.
  Note:            None.
 ********************************************************************/
void StopMotor(void) {
	IOCON1 = 0xC300;
	IOCON2 = 0xC300;
	IOCON3 = 0xC300;

	// disable all interrupts
	__asm__ volatile ("DISI #0x3FFF");
	CM1CONbits.CON = 0; // Comparator is disabled

	_T1IE = 0; // Disable interrupts for timer 1
	_CNIE = 0; // Disable CN interrupt
	_PWM1IE = 0; // Disable PWM interrupts
	// enable all interrupts
	DISICNT = 0;

	Parameter.StatFlags.MotorIsRunning = 0;
	return;
}

/*********************************************************************

  Function:        void RunMotor(void)

  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        Call this subroutine when first trying to run the motor and
				   the motor is previously stopped. RunMotor will charge 
				   bootstrap caps, will initialize application variables, and
				   will enable all ISRs.
  Note:            None.
 ********************************************************************/
void RunMotor(void) {
	// if no critical errors then run motor
	if ((Parameter.error.all & ERROR_MASK_DISABLE_MOTOR_RUN) == 0) {
		LastSector = Sector = GetSector(); // Initialize Sector variable
		if (Sector == (-1)) {
			DBG_PRINT("Hall position error - can't start motor\r\n");
			return;
		}

		//	ChargeBootstraps();
		PDC1 = PHASE1 / 2; // Initialize as 0 voltage
		PDC2 = PHASE2 / 2; // Initialize as 0 voltage
		PDC3 = PHASE3 / 2; // Initialize as 0 voltage

		IOCON1 = 0xC040; //TODO: check errate to correct turnoff override
		IOCON2 = 0xC040;
		IOCON3 = 0xC040;

		TMR1 = 0; // Reset timer 1 for speed control

		ControlOutput = 0; // Initial output voltage

		MotorStalledCounter = 0; // Reset motor stalled protection counter

		// Clear all interrupts flags
		_T1IF = 0; // Clear timer 1 flag
		_CNIF = 0; // Clear interrupt Flag
		_PWM1IF = 0; // Clear interrupt flag

		// disable all interrupts
		__asm__ volatile ("DISI #0x3FFF");

		CM1CONbits.CON = 1; // Comparator is enabled

		_T1IE = 1; // Enable interrupts for timer 1
		_CNIE = 1; // Enable interrupt on CN
		_PWM1IE = 1; // Enable PWM interrupts
		// enable all interrupts
		DISICNT = 0;

		Parameter.StatFlags.MotorIsRunning = 1; // Indicate that the motor is running
	}
	return;
}

/*********************************************************************
  Function:        void ControlPosition(void)
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        This subroutine implements a PID position and current control
  Note:            None.
 ********************************************************************/
void ControlCurrentAndPosition(void) {
	if (Parameter.ControlFlags.controlCurrent == 0 && Parameter.ControlFlags.controlPosition == 0) {
		StopMotor();
		return;
	}

	if (Parameter.ControlFlags.controlCurrent) {
		PID_Current.error = PidGetError((int32_t) Parameter.maxMotorCurrent, (int32_t) Parameter.MotorCurrent);
		PIDcalc(&PID_Current);
	} else PID_Current.output = 0;

	if (Parameter.ControlFlags.controlPosition) {
		PID_Position.error = PidGetError((int32_t) Parameter.PositionRequired, (int32_t) MotorGetPosition());
		PIDcalc(&PID_Position);
	} else PID_Position.output = 1500;

	if (PID_Current.output < 0) {
		if ((Parameter.ControlFlags.controlPosition && Parameter.PositionRequired < Parameter.Position) ||
			(!Parameter.ControlFlags.controlPosition && Parameter.ControlFlags.Direction == CCW)) {
			ControlOutput = PID_Position.output - PID_Current.output;
		} else {
			ControlOutput = PID_Position.output + PID_Current.output;
		}
	} else {
		ControlOutput = PID_Position.output;
	}

#ifndef CLOSED_LOOP
	ControlOutput = Parameter.maxMotorCurrent;
#endif

	if (ControlOutput > MAX_POS_DUTY)
		ControlOutput = MAX_POS_DUTY;
	if (ControlOutput < MAX_NEG_DUTY)
		ControlOutput = MAX_NEG_DUTY;
	return;
}
