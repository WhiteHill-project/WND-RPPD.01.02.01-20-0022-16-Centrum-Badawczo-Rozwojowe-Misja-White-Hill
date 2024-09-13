
// <editor-fold defaultstate="collapsed" desc="INCLUDES">
#include <xc.h>

#include <stdint.h>        /* Includes uint16_t definition   */
#include <stdbool.h>
#include <stdlib.h>
#include <p33EP32MC204.h>

#include "SVM.h"
#include "user.h"
#include "time.h"
#include "paramether.h"
#include "ioPorts.h"

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="Interrupt Vector Names">
/******************************************************************************/
/* Interrupt Vector Options                                                   */
/******************************************************************************/
/*                                                                            */
/* Refer to the C30 (MPLAB C Compiler for PIC24F MCUs and dsPIC33F DSCs) User */
/* Guide for an up to date list of the available interrupt options.           */
/* Alternately these names can be pulled from the device linker scripts.      */
/*                                                                            */
/* dsPIC33F Primary Interrupt Vector Names:                                   */
/*                                                                            */
/* _INT0Interrupt      _C1Interrupt                                           */
/* _IC1Interrupt       _DMA3Interrupt                                         */
/* _OC1Interrupt       _IC3Interrupt                                          */
/* _T1Interrupt        _IC4Interrupt                                          */
/* _DMA0Interrupt      _IC5Interrupt                                          */
/* _IC2Interrupt       _IC6Interrupt                                          */
/* _OC2Interrupt       _OC5Interrupt                                          */
/* _T2Interrupt        _OC6Interrupt                                          */
/* _T3Interrupt        _OC7Interrupt                                          */
/* _SPI1ErrInterrupt   _OC8Interrupt                                          */
/* _SPI1Interrupt      _DMA4Interrupt                                         */
/* _U1RXInterrupt      _T6Interrupt                                           */
/* _U1TXInterrupt      _T7Interrupt                                           */
/* _ADC1Interrupt      _SI2C2Interrupt                                        */
/* _DMA1Interrupt      _MI2C2Interrupt                                        */
/* _SI2C1Interrupt     _T8Interrupt                                           */
/* _MI2C1Interrupt     _T9Interrupt                                           */
/* _CNInterrupt        _INT3Interrupt                                         */
/* _INT1Interrupt      _INT4Interrupt                                         */
/* _ADC2Interrupt      _C2RxRdyInterrupt                                      */
/* _DMA2Interrupt      _C2Interrupt                                           */
/* _OC3Interrupt       _DCIErrInterrupt                                       */
/* _OC4Interrupt       _DCIInterrupt                                          */
/* _T4Interrupt        _DMA5Interrupt                                         */
/* _T5Interrupt        _U1ErrInterrupt                                        */
/* _INT2Interrupt      _U2ErrInterrupt                                        */
/* _U2RXInterrupt      _DMA6Interrupt                                         */
/* _U2TXInterrupt      _DMA7Interrupt                                         */
/* _SPI2ErrInterrupt   _C1TxReqInterrupt                                      */
/* _SPI2Interrupt      _C2TxReqInterrupt                                      */
/* _C1RxRdyInterrupt                                                          */
/*                                                                            */
/* dsPIC33E Primary Interrupt Vector Names:                                   */
/*                                                                            */
/* _INT0Interrupt     _IC4Interrupt      _U4TXInterrupt                       */
/* _IC1Interrupt      _IC5Interrupt      _SPI3ErrInterrupt                    */
/* _OC1Interrupt      _IC6Interrupt      _SPI3Interrupt                       */
/* _T1Interrupt       _OC5Interrupt      _OC9Interrupt                        */
/* _DMA0Interrupt     _OC6Interrupt      _IC9Interrupt                        */
/* _IC2Interrupt      _OC7Interrupt      _PWM1Interrupt                       */
/* _OC2Interrupt      _OC8Interrupt      _PWM2Interrupt                       */
/* _T2Interrupt       _PMPInterrupt      _PWM3Interrupt                       */
/* _T3Interrupt       _DMA4Interrupt     _PWM4Interrupt                       */
/* _SPI1ErrInterrupt  _T6Interrupt       _PWM5Interrupt                       */
/* _SPI1Interrupt     _T7Interrupt       _PWM6Interrupt                       */
/* _U1RXInterrupt     _SI2C2Interrupt    _PWM7Interrupt                       */
/* _U1TXInterrupt     _MI2C2Interrupt    _DMA8Interrupt                       */
/* _AD1Interrupt      _T8Interrupt       _DMA9Interrupt                       */
/* _DMA1Interrupt     _T9Interrupt       _DMA10Interrupt                      */
/* _NVMInterrupt      _INT3Interrupt     _DMA11Interrupt                      */
/* _SI2C1Interrupt    _INT4Interrupt     _SPI4ErrInterrupt                    */
/* _MI2C1Interrupt    _C2RxRdyInterrupt  _SPI4Interrupt                       */
/* _CM1Interrupt      _C2Interrupt       _OC10Interrupt                       */
/* _CNInterrupt       _QEI1Interrupt     _IC10Interrupt                       */
/* _INT1Interrupt     _DCIEInterrupt     _OC11Interrupt                       */
/* _AD2Interrupt      _DCIInterrupt      _IC11Interrupt                       */
/* _IC7Interrupt      _DMA5Interrupt     _OC12Interrupt                       */
/* _IC8Interrupt      _RTCCInterrupt     _IC12Interrupt                       */
/* _DMA2Interrupt     _U1ErrInterrupt    _DMA12Interrupt                      */
/* _OC3Interrupt      _U2ErrInterrupt    _DMA13Interrupt                      */
/* _OC4Interrupt      _CRCInterrupt      _DMA14Interrupt                      */
/* _T4Interrupt       _DMA6Interrupt     _OC13Interrupt                       */
/* _T5Interrupt       _DMA7Interrupt     _IC13Interrupt                       */
/* _INT2Interrupt     _C1TxReqInterrupt  _OC14Interrupt                       */
/* _U2RXInterrupt     _C2TxReqInterrupt  _IC14Interrupt                       */
/* _U2TXInterrupt     _QEI2Interrupt     _OC15Interrupt                       */
/* _SPI2ErrInterrupt  _U3ErrInterrupt    _IC15Interrupt                       */
/* _SPI2Interrupt     _U3RXInterrupt     _OC16Interrupt                       */
/* _C1RxRdyInterrupt  _U3TXInterrupt     _IC16Interrupt                       */
/* _C1Interrupt       _USB1Interrupt     _ICDInterrupt                        */
/* _DMA3Interrupt     _U4ErrInterrupt    _PWMSpEventMatchInterrupt            */
/* _IC3Interrupt      _U4RXInterrupt     _PWMSecSpEventMatchInterrupt         */
/*                                                                            */
/* For alternate interrupt vector naming, simply add 'Alt' between the prim.  */
/* interrupt vector name '_' and the first character of the primary interrupt */
/* vector name.  There is no Alternate Vector or 'AIVT' for the 33E family.   */
/*                                                                            */
/* For example, the vector name _ADC2Interrupt becomes _AltADC2Interrupt in   */
/* the alternate vector table.                                                */
/*                                                                            */
/* Example Syntax:                                                            */
/*                                                                            */
/* void __attribute__((interrupt,auto_psv)) <Vector Name>(void)               */
/* {                                                                          */
/*     <Clear Interrupt Flag>                                                 */
/* }                                                                          */
/*                                                                            */
/* For more comprehensive interrupt examples refer to the C30 (MPLAB C        */
/* Compiler for PIC24 MCUs and dsPIC DSCs) User Guide in the                  */
/* <C30 compiler instal directory>/doc directory for the latest compiler      */
/* release.  For XC16, refer to the MPLAB XC16 C Compiler User's Guide in the */
/* <XC16 compiler instal directory>/doc folder.                               */
/*                                                                            */
/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/
// </editor-fold>

/*********************************************************************
  Function:        void __attribute__((__interrupt__)) _T1Interrupt (void)
  PreCondition:    The motor is running and is generating hall effect sensors
				   interrupts. Also, the actual direction of the motor used
				   in this interrupt is assumed to be previously calculated.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        In this ISR the Period, Phase Increment and MeasuredSpeed
				   are calculated based on the input capture of one of the
				   halls. The speed controller is also called in this ISR
				   to generate a new output voltage (ControlOutput). The 
				   Phase Advance is calculated based on the maximum allowed
				   phase advance (MAX_PH_ADV) and the actual speed of the 
				   motor. The last thing done in this ISR is the forced 
				   commutation, which happens each time the motor doesn't
				   generate a new hall interrupt after a programmed period 
				   of time. If the timeout for generating hall ISR is too much
				   (i.e. 100 ms) the motor is then stopped.
  Note:            The MeasuredSpeed Calculation is made in assembly to take 
				   advantage of the signed fractional division.
 ********************************************************************/
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
	_T1IF = 0;

	Parameter.Position = MotorGetPosition();
	if (Parameter.Position >= (Parameter.PositionRequired - Parameter.PositionOkOutPrecision)
		&& Parameter.Position <= (Parameter.PositionRequired + Parameter.PositionOkOutPrecision)) {
		MotorPending = 0;
		MotorStalledCounter = 0;
	} else {
		MotorPending = 1;
	}

	ControlCurrentAndPosition();

	// We increment a timeout variable to see if the motor is too slow (not generating hall effect
	// sensors interrupts frequently enough) or if the motor is stalled. This variable is cleared
	// in halls ISRs
	MotorStalledCounter++;
	if ((MotorStalledCounter % _50MILISEC) == 0) {
		ForceCommutation(); // Force Commutation if no hall sensor changes have occured in specified timeout.
	} else if (MotorStalledCounter >= _1SEC) {
		Parameter.error.motorStalled = 1;
	}
}

/** \brief	Przerwanie zglaszane co 1 ms, zwiekszany licznik czasu systemowego,
 * 			pozostale liczniki uaktualniane w procedurze UpdateTimeCounters()
 */
void __attribute__((interrupt, auto_psv)) _T2Interrupt(void) {
	_T2IF = 0; // Clear interrupt flag

	systemTime++;

	TIME_HANDLER();

	// for test only
#if 0
	static int16_t znak = 1, ms = 10;
	if (--ms == 0) {
		ms = 2;
		Parameter.PositionRequired += znak;
	}
	if ((systemTime % 5000) == 0) {
		znak--;
		if (znak == (-2)) znak = 1;
	}
#endif
}

/*********************************************************************
  Function:        void __attribute__((__interrupt__)) _PWM1Interrupt (void)
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        in this ISR the sinewave is generated. If the current motor
				   direction of rotation is different from the required 
				   direction then the motor is operated	in braking mode and 
				   step commutation is performed. Once both directions are 
				   equal then the sinewave is fed into the motor windings.
				   If PHASE_ADVANCE is defined, a value corresponding to the
				   multiplication of the actual speed * maximum phase advance
				   is added to the sine wave phase to produce the phase shift
  Note:            None.
 ********************************************************************/
void __attribute__((interrupt, auto_psv)) _PWM1Interrupt(void) {

	_PWM1IF = 0; // Clear interrupt flag
	if (Sector != (-1)) {
		SVM(ControlOutput, (Sector) % 6);
	}
#ifdef RTDM
	/********************* DMCI Dynamic Data Views  ***************************/
	/********************** RECORDING MOTOR PHASE VALUES ***************/
	if (DMCIFlags.Recorder) {
		if (SnapShotDelayCnt++ == SnapShotDelay) {
			SnapShotDelayCnt = 0;
			*PtrRecBuffer1++ = SNAP1;
			*PtrRecBuffer2++ = SNAP2;
			*PtrRecBuffer3++ = SNAP3;
			*PtrRecBuffer4++ = SNAP4;

			if (PtrRecBuffer4 > RecBuffUpperLimit) {
				PtrRecBuffer1 = RecorderBuffer1;
				PtrRecBuffer2 = RecorderBuffer2;
				PtrRecBuffer3 = RecorderBuffer3;
				PtrRecBuffer4 = RecorderBuffer4;
				DMCIFlags.Recorder = 0;
			}
		}
	}
#endif
}

/*********************************************************************
  Function:        void __attribute__((__interrupt__)) _CNInterrupt (void)
  PreCondition:    The inputs of the hall effect sensors should have low pass
				   filters. A simple RC network works.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        This interrupt represent Hall A, B and C ISR. 
				   This is generated by the input change notification .
				   The purpose of this ISR is to Calculate the actual 
				   mechanical direction of rotation of the motor, and get the
				   sector 
				   the Phase variable depending on the sector the rotor is in.
  Note 1:          The sector is validated in order to avoid any spurious
				   interrupt due to a slow slew rate on the halls inputs due to
				   hardware filtering.
 ********************************************************************/
void __attribute__((interrupt, auto_psv)) _CNInterrupt(void) {
	_CNIF = 0; // clear interrupt flag	
	Sector = GetSector();

	if (Sector != LastSector) {
		// Since a new sector is detected, clear variable that would stop 
		// the motor if stalled.
		MotorStalledCounter = 0;

		if ((Sector == 5 && LastSector == 0) || (LastSector - Sector == 1)) {
			Parameter.StatFlags.Direction = CCW; // Current_Direction = CCW;
			PositionHall--;
		} else {
			Parameter.StatFlags.Direction = CW; // Current_Direction = CW;
			PositionHall++;
		}
		LastSector = Sector; // Update last sector		
	}
}

/*********************************************************************
  Function:        void __attribute__((__interrupt__)) _AD1Interrupt (void)
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        
  Note:            None.
 ********************************************************************/
void __attribute__((interrupt, auto_psv)) _AD1Interrupt(void) {
	static long CurrentStateVar;
	static int CurrentFilter;
	static int last10;
	static int history[10] = {0}, hIdx;

	_AD1IF = 0; // Clear interrupt flag
	if (Parameter.StatFlags.MotorIsRunning == 0 && 3000 > systemTime) {
		MotorCurrentOffset = ((MotorCurrentOffset + ADC1BUF0) >> 1);
	}

	history[hIdx] = (ADC1BUF0 - MotorCurrentOffset);
	last10 += history[hIdx++];
	hIdx %= 10;
	last10 -= history[hIdx];

	CurrentStateVar += ((long) last10 - CurrentFilter) * MOTOR_CURR_FILTER_CONST; // filter ADC value
	CurrentFilter = (int) (CurrentStateVar >> 15);

	Parameter.MotorCurrent = (int16_t) (__builtin_mulsu(CurrentFilter, Parameter.MotorCurrentScale) >> 14);

	CurrentMaxLimit();

	return;
}

/*********************************************************************
  Function:        void __attribute__((__interrupt__)) _INT1Interrupt (void)
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        
  Note:            None.
 ********************************************************************/
void __attribute__((interrupt, auto_psv))_INT1Interrupt(void) {
	_INT1IF = 0; // clear interrupt flag

	// run timer for prevent false interrupt
	T4CONbits.TON = 1;
}

/*********************************************************************
  Function:        void __attribute__((__interrupt__)) _T4Interrupt (void)
  PreCondition:    None.
 
  Input:           None.
  Output:          None.
  Side Effects:    None.
  Overview:        
  Note:            None.
 ********************************************************************/
void __attribute__((interrupt, auto_psv)) _T4Interrupt(void) {
	_T4IF = 0;

	T4CONbits.TON = 0; //turn off timer
	TMR4 = 0;

	// check state of "TAKT" input
	if (_RC8 == 1) {
		if (Parameter.StatFlags.MotorIsRunning) {
			if (MotorDirection)
				Parameter.PositionRequired += Parameter.PositionStep;
			else
				Parameter.PositionRequired -= Parameter.PositionStep;
		}
	}
}