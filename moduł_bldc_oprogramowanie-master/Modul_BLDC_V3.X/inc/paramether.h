/* 
 * File:   paramether.h
 * Author: Jacek
 *
 * Created on 9 kwietnia 2018, 08:28
 */

#ifndef PARAMETHER_H
#define	PARAMETHER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
	
#include "PID.h"
	
	typedef union STAT_FLAGS {

		struct {
			volatile uint16_t MotorIsRunning : 1; // This bit is 1 if motor running
			volatile uint16_t Direction : 1; // mechanical motor direction of rotation 
		};
		uint16_t all;
	} STAT_FLAGS_t;

	typedef union _CONTROL_FLAGS {

		struct {
			uint16_t MotorRun : 1; // This bit is 1 if motor running
			uint16_t Direction : 1; // mechanical motor direction of rotation 
			uint16_t isEncoder : 1;
			uint16_t controlCurrent : 1;
			uint16_t controlPosition : 1;
		};
		uint16_t all;
	} CONTROL_FLAGS_t;

	typedef union _ERROR_BITS {

		struct {
			volatile uint16_t communUnknownCommand : 1; // received unknown command
			volatile uint16_t overcurrent : 1; // over current exceeds 1 second
			volatile uint16_t criticalOvercurrent : 1; // overrun of critical current
			volatile uint16_t motorStalled : 1; // position goes to the limit switch
		};
		volatile uint16_t all;
	} ERRORS_t;

	typedef union _PARAMETER_t {
		uint8_t cTab[256];

		struct {
			STAT_FLAGS_t StatFlags;
			volatile int16_t MotorCurrent; // in mA for read only
			volatile int32_t Position;

			ERRORS_t error;

			CONTROL_FLAGS_t ControlFlags;

			int32_t PositionRequired;
			int16_t PositionStep; // position changed on every pulse on input pin
			uint16_t PositionOkOutPrecision; // +- precision of output

			uint16_t maxMotorCurrent;
			uint16_t MotorCurrentScale;

			PID_COEFICIENTS CurrentCoeficients;
			PID_COEFICIENTS PositionCoeficients;

			uint8_t lastAddr;
		};
	} PARAMETER_t;

	extern PARAMETER_t Parameter;



#ifdef	__cplusplus
}
#endif

#endif	/* PARAMETHER_H */

