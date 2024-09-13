/* 
 * File:   PID.h
 * Author: Jacek
 *
 * Created on 25 stycznia 2017, 08:17
 */

#ifndef PID_H
#define	PID_H

#ifdef	__cplusplus
extern "C" {
#endif

	typedef struct _PID_COEFFICIENTS {
		int16_t kp;
		int16_t ki;
		int16_t kd;
		int16_t :16;
	} PID_COEFICIENTS;

	typedef struct _PID_PARMS {
		PID_COEFICIENTS *coeficients;
		int32_t integral;
		int16_t error;
		int16_t previousError;
		int16_t output;
		int32_t posLimit;
		int32_t negLimit;
		int16_t scallingFactor;
	} PID_PARMS_t;



	int16_t PidGetError(int32_t reference, int32_t messured);
	void PIDcalc(PID_PARMS_t *pidParms);

#ifdef	__cplusplus
}
#endif

#endif	/* PID_H */

