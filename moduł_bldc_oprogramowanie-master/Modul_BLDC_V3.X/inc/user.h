/* 
 * File:   user.h
 * Author: Jacek
 *
 * Created on 16 kwietnia 2018, 10:07
 */

#ifndef USER_H
#define	USER_H

#ifdef	__cplusplus
extern "C" {
#endif

#define CLOSED_LOOP      // if defined the speed controller will be enabled

#define DEVICE_ADDRESS_MOTOR_TYPE (0xA0)


#define FPWM	(20000)		 // 20 kHz, so that no audible noise is present.
#define PDIV	(256UL)

#define MAX_DUTY		((FCY/FPWM)-1)
#define MIN_POS_DUTY	(int)(45)
#define MIN_NEG_DUTY	(int)(-MIN_POS_DUTY)
#define MAX_POS_DUTY	(int)(MAX_DUTY>>1)
#define MAX_NEG_DUTY	(int)(-MAX_POS_DUTY)


#define CW	(0)		// Counter Clock Wise direction
#define CCW	(1)		// Clock Wise direction

	/* ALGORITHM SPECIFICS */
#define MOTOR_CURR_FILTER_CONST (100) /* the smaller the value, the higher the filter and the delay introduced */
#define MOTOR_CURR_SCALE	(16384)//(43000) // coeficient to scale current (16384 is a 1 multiplier)
#define MOTOR_CURR_LIMIT	(25000)  // maxMotorCurrent must be less than 25.00A
#define CRITICAL_CURRENT	(30000) // Max current 30.00A

	/** PID coeficients */
#define PID_CURRENT_KP		(3000)//(1200)
#define PID_CURRENT_KI		(15)//(1000)
#define PID_CURRENT_KD		(0)

#define PID_POSITION_KP		(25000)//(4000)
#define PID_POSITION_KI		(10)//(100)
#define PID_POSITION_KD		(0)

#define ERROR_MASK_DISABLE_MOTOR_RUN	(0xFFFE)  // Error mask which disable motor run
#define ERROR_MASK_STOPS_MOTOR			(0xFFFE)	// Error mask which stops motor if is run

#define _50MILISEC (50*10)	// Used as a timeout with no hall effect sensors
	// transitions and Forcing steps according to the
	// actual position of the motor
#define _1SEC (1000*10)      // after this time has elapsed, the motor is
	// consider stalled and it's stopped

	/*********************************************************************
	 GLOBAL VARIABLES                                                   
	 *********************************************************************/
	extern unsigned int Sector;
	extern unsigned int LastSector;

	extern unsigned int MotorStalledCounter;
	extern volatile bool ParamChanged;
	extern uint8_t myAddress;

	extern int ControlOutput;

	extern volatile int32_t PositionHall;


	extern int16_t MotorCurrentOffset;

	/******************************************************************************/
	/* User Function Prototypes                                                   */
	/******************************************************************************/
	void InitApp(void);

	inline char GetSector(void);
	void ForceCommutation(void);
	void CurrentMaxLimit(void);
	void StopMotor(void);
	void RunMotor(void);
	void ControlCurrentAndPosition(void);

	void CheckChangedParameter(void);
	int32_t MotorGetPosition(void);
	void MotorClearPosition(void);
	void setDefaultParameters(void);

#ifdef	__cplusplus
}
#endif

#endif	/* USER_H */

