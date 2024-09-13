/*
 * File:   main.c
 * Author: Jacek
 *
 * Created on 12 kwietnia 2018, 12:59
 */


#include "xc.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "main.h"
#include "system.h"
#include "paramether.h"
#include "user.h"
#include "debug.h"
#include "Commun.h"
#include "ioPorts.h"
#include "time.h"

// TODO Skalibrowac prad
// TODO Ustalic przy jakim pradzie ma dzialac komparator
// TODO dobrac parmetry PIDa

/******************************************************************************/
/* Global Variable Declaration                                                */
/******************************************************************************/
uint8_t myAddress = 0;
PARAMETER_t Parameter;

/******************************************************************************/
/* Main Program                                                               */

/******************************************************************************/
int main(void) {
	ConfigureOscillator();

	InitApp();

	setDefaultParameters();

	myAddress = DEVICE_ADDRESS_MOTOR_TYPE | (uint8_t) ((PORTB >> 8) & 0x01) | ((PORTC & 3) << 1) | ((PORTB << 1)&0x0008);
	DBG_PRINT("Device Adress is: 0x%X\r\n", myAddress);

	Parameter.Position = 0;
	Parameter.MotorCurrent = 0;

	Parameter.PositionRequired = 0;
	Parameter.StatFlags.Direction = 0;
	Parameter.StatFlags.MotorIsRunning = 0;
	Parameter.ControlFlags.MotorRun = 0;
	Parameter.error.all = 0;

	Parameter.ControlFlags.controlCurrent = 1;
	Parameter.ControlFlags.controlPosition = 1;
	Parameter.maxMotorCurrent = 15000;
	Parameter.ControlFlags.isEncoder = 0;

	RCONbits.SWDTEN = 1; // enable watchdog timer
	while (true) {
		ClrWdt();

		CommunicationTask();

		if (Parameter.StatFlags.MotorIsRunning != Parameter.ControlFlags.MotorRun) {
			if (Parameter.ControlFlags.MotorRun)
				RunMotor();
			else
				StopMotor();
		}
		if ((Parameter.error.all & ERROR_MASK_STOPS_MOTOR) != 0) {
			StopMotor();
			MotorFault = 0;
			Parameter.ControlFlags.MotorRun = 0;
		} else if (Parameter.error.all == 0) {
			MotorFault = 1;
		}

		CheckChangedParameter();
	}

	return 0;
}
