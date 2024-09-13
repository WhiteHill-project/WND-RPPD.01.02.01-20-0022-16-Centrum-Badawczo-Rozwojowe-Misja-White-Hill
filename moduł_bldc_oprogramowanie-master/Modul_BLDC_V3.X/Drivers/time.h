/* 
 * File:   time.h
 * Author: Jacek Kijas
 *
 * Created on 22 09 2017, 13:24
 */

#ifndef TIME_H
#define	TIME_H

#include <stdint.h>
#include <stddef.h>

#ifdef	__cplusplus
extern "C" {
#endif

	// ************************************* DEFINES **************************************************
#define TIMEOUTS_MAX	3 // max timeouts initialised at the same time


	// ************************************* TYPE DEFS ************************************************

	typedef struct _tim_t {
		volatile uint32_t counter;
		uint32_t interval;
		void (*handler)(void); // callback
		uint8_t lock; // lock if expired
	} tim_t;


	// ************************************ VARIABLES *************************************************
	extern volatile uint32_t systemTime;

	// ************************************ FUNCTION PROTOTYPES ***************************************
	tim_t* tim_initialize(void);
	void tim_destroy(tim_t *tim);
	uint32_t timeout_set_ms(tim_t * tim, uint32_t t, void (*handler)(void));
	void interval_set_ms(tim_t *tim, uint32_t t, void (*handler)(void));
	void timeout_clear(tim_t *tim);
	void TIME_HANDLER(void);
	
#ifdef	__cplusplus
}
#endif

#endif	/* TIME_H */

