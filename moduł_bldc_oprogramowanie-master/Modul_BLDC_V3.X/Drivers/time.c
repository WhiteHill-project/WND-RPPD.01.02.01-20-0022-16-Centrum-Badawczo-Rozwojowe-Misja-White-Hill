/* 
 * File:   time.c
 * Author: Jacek Kijas
 *
 * Created on 22 09 2017, 13:24
 */
#include<stdlib.h>

#include "time.h"

// ************************************ VARIABLES *************************************************
volatile uint32_t systemTime;
tim_t *timTab[TIMEOUTS_MAX] = {NULL};

// ************************************ FUNCTIONS *************************************************

tim_t* tim_initialize(void) {
	uint16_t i;
	tim_t *tInit = NULL;

	for (i = 0; i < TIMEOUTS_MAX; i++) { // if is free place in the table
		if (timTab[i] == NULL)break;
	}
	if (i >= TIMEOUTS_MAX) return tInit;

	tInit = malloc(sizeof (tim_t));

	if (tInit) {
		tInit->counter = 0;
		tInit->lock = 0;
		tInit->handler = NULL;
		tInit->interval = 0;
	}
	if (timTab[i] == NULL) {
		timTab[i] = tInit;
		return (tim_t*) tInit;
	}
	return NULL;
}

void tim_destroy(tim_t *tim) {
	uint16_t i;

	if (tim) {
		for (i = 0; i < TIMEOUTS_MAX; i++) {
			if (timTab[i] == tim)timTab[i] = NULL;
		}
		free(tim);
	}
}

uint32_t timeout_set_ms(tim_t * tim, uint32_t t, void (*handler)(void)) {

	if (!t || !tim)
		return 0;

	if (!tim->counter && !tim->lock) {
		tim->counter = t;
		tim->lock = 1;
		tim->handler = handler;
		return t;
	}
	if (!tim->counter && tim->lock)
		tim->lock = 0;

	return tim->counter;
}

void interval_set_ms(tim_t *tim, uint32_t t, void (*handler)(void)) {

	if (!t || !handler || !tim)
		return;

	tim->interval = tim->counter = t;
	tim->handler = handler;
}

void timeout_clear(tim_t *tim) {
	tim->counter = 0;
	tim->lock = 0;
	tim->handler = NULL;
	tim->interval = 0;
}

void TIME_HANDLER(void) {
	uint16_t i;

	for (i = 0; i < TIMEOUTS_MAX; ++i) {
		if (timTab[i]) {
			if (timTab[i]->counter) {
				timTab[i]->counter--;
				if (!timTab[i]->counter) {
					if (timTab[i]->handler)
						timTab[i]->handler();
				}
			}
			if (timTab[i]->interval&&!timTab[i]->counter)
				timTab[i]->counter = timTab[i]->interval;
		}
	}
}

/*
 End of File
 */
