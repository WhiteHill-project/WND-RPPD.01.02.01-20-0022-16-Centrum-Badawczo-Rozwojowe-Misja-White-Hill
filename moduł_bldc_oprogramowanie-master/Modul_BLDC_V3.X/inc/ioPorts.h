/* 
 * File:   ioPorts.h
 * Author: Jacek Kijas
 *
 * Created on 13 wrzesnia 2016, 10:58
 */

#ifndef IOPORTS_H
#define	IOPORTS_H

#include <xc.h>

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0


#define MotorFault		(_LATB7)
#define MotorDirection	(_RC9)
#define MotorPending	(_LATB9)

#define Krancowka_A		(_RA7)
#define Krancowka_B		(_RA10)

/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/
/**
 * @Param
	none
 * @Returns
	none
 * @Description
	GPIO and peripheral I/O initialization
 * @Example
	SetupPorts();
 */
void Ports_Initialize(void);

#endif	/* IOPORTS_H */

