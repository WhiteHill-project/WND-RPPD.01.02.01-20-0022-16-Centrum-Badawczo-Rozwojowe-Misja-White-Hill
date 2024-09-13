/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="INCLUDES">
#include <xc.h> /* Device header file */

#include "system.h"          /* variables/params used by system.c             */
// </editor-fold>

/******************************************************************************/
/* System Level Functions                                                     */
/*                                                                            */
/* Custom oscillator configuration funtions, reset source evaluation          */
/* functions, and other non-peripheral microcontroller initialization         */
/* functions get placed in system.c.                                          */
/*                                                                            */

/******************************************************************************/

void ConfigureOscillator(void) {
	/* Disable Watch Dog Timer */
	RCONbits.SWDTEN = 0;

	PLLFBD = (M - 2);
	CLKDIVbits.PLLPRE = (N1 - 2);
	CLKDIVbits.PLLPOST =
#if(N2==2)
			0;
#elif(N2==4)
			1;
#elif(N2==8)
			3;
#endif
	OSCTUN = 0; // Tune FRC oscillator, if FRC is used

	// Disable Watch Dog Timer
	RCONbits.SWDTEN = 0;
	CLKDIVbits.DOZE = 0;

	// Clock switch to incorporate PLL
	__builtin_write_OSCCONH(0x03); // Initiate Clock Switch to Primary
	// Oscillator with PLL (NOSC=0b011)
	__builtin_write_OSCCONL(0x01); // Start clock switching
	while (OSCCONbits.COSC != 0b011); // Wait for Clock switch to occur

	// Wait for PLL to lock
	while (OSCCONbits.LOCK != 1) {
	};
}

