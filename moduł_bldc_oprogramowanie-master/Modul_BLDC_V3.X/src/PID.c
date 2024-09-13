
#include <stdint.h>
#include <stddef.h>

#include "PID.h"

int16_t PidGetError(int32_t reference, int32_t messured) {
	int32_t error;

	error = reference - messured;
	/* check the error is not out of limit */
	if (error > INT16_MAX) error = INT16_MAX;
	else if (error < INT16_MIN) error = INT16_MIN;

	return (int16_t) error;
}

void PIDcalc(PID_PARMS_t *pidParms) {

	int32_t proportional_term;
	int32_t derivative_term;
	int32_t temp;


	if (pidParms != NULL) {
		proportional_term = (int32_t) pidParms->error * (int32_t) pidParms->coeficients->kp;

		derivative_term = (int32_t) (pidParms->error - pidParms->previousError) * (int32_t) pidParms->coeficients->kd;
		temp = proportional_term + derivative_term;

		/* if sum out of limit */
		if (temp > ((int32_t) pidParms->posLimit * pidParms->scallingFactor)
			|| temp < ((int32_t) pidParms->negLimit * pidParms->scallingFactor)) {
			pidParms->integral = 0;
		}

		pidParms->integral += ((int32_t) pidParms->error * (int32_t) pidParms->coeficients->ki);
		temp += pidParms->integral;

		/* if sum out of limit */
		if (temp > ((int32_t) pidParms->posLimit * pidParms->scallingFactor)) {
			temp = ((int32_t) pidParms->posLimit * pidParms->scallingFactor);
			if (pidParms->coeficients->ki)
				pidParms->integral = ((int32_t) pidParms->posLimit * pidParms->scallingFactor) - proportional_term - derivative_term;
		} else if (temp < ((int32_t) pidParms->negLimit * pidParms->scallingFactor)) {
			temp = ((int32_t) pidParms->negLimit * pidParms->scallingFactor);
			if (pidParms->coeficients->ki)
				pidParms->integral = ((int32_t) pidParms->negLimit * pidParms->scallingFactor) - proportional_term - derivative_term;
		}

		pidParms->previousError = pidParms->error;

		/* scale output */
		pidParms->output = (int16_t) (temp / pidParms->scallingFactor);
	}
}


