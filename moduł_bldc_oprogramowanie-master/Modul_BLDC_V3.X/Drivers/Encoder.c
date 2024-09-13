
#include <xc.h>
#include <stdint.h>

#include "Encoder.h"

/*********************************************************************
  Function:			void initEncoder(void)
  PreCondition:		None.
 
  Input:			None.
  Output:			None.
  Side Effects:		None.
  Overview:			Initialization of QEI1 interface to read position
					of motor
  Note:				None.
 ********************************************************************/
void InitEncoder(void) {

}

/*********************************************************************
  Function:			unsigned long long EncoderGetPosition(void)
  PreCondition:		Must be set STEPS_PER_ROTATE.
 
  Input:			None.
  Output:			Current position.
  Side Effects:		None.
  Overview:			Calculate position based on rotation and current rotor position
  Note:				None.
 ********************************************************************/
unsigned long EncoderGetPosition(void) {
	unsigned long position;

	return (unsigned long) (position);
}

void EnkoderClearPosition(void) {
	
}
