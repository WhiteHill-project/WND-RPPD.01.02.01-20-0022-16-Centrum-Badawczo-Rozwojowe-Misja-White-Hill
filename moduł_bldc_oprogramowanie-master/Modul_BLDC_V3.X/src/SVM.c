/* 
 * File:   SVM.c
 * Author: Jacek Kijas
 *
 * Created on 5 stycznia 2017, 13:07
 */
#include <xc.h>
#include <stdint.h>
#include <dsp.h>

#include "SVM.h"


#define oNEG	Q15(-1.0)
#define oPOS	Q15(1.0)
#define oZER	Q15(0.0)

typedef struct _LOKUP {
	int16_t u;
	int16_t v;
	int16_t w;
} LOKUP_t;

LOKUP_t lokupTab[6] = {
	{.u = oZER, .v = oPOS, .w = oNEG},
	{.u = oPOS, .v = oZER, .w = oNEG},
	{.u = oPOS, .v = oNEG, .w = oZER},
	{.u = oZER, .v = oNEG, .w = oPOS},
	{.u = oNEG, .v = oZER, .w = oPOS},
	{.u = oNEG, .v = oPOS, .w = oZER},
};

void SVM(int volts, unsigned int sector) {
	unsigned int PDC1Latch, PDC2Latch, PDC3Latch;
	unsigned int tpwm, u, v, w;

	tpwm = PHASE1 / 2;

	// Limit volts input to avoid overmodulation.
	if (volts > MAX_POS_VOLTS) volts = MAX_POS_VOLTS;
	if (volts < (MAX_NEG_VOLTS)) volts = MAX_NEG_VOLTS;


	u = lokupTab[sector].u;
	v = lokupTab[sector].v;
	w = lokupTab[sector].w;

	if (u == oZER) {
		IOCON1 = 0xC300;
		PDC1Latch = 0;
	} else {
		IOCON1 = 0xC000;
		PDC1Latch = tpwm + (int) (__builtin_mulss(volts, u) >> 15); //(__builtin_mulss(voltstpwm, u) >> 15);
	}
	if (v == oZER) {
		IOCON2 = 0xC300;
		PDC2Latch = 0;
	} else {
		IOCON2 = 0xC000;
		PDC2Latch = tpwm + (int) (__builtin_mulss(volts, v) >> 15); //(__builtin_mulss(voltstpwm, v) >> 15);
	}
	if (w == oZER) {
		IOCON3 = 0xC300;
		PDC3Latch = 0;
	} else {
		IOCON3 = 0xC000;
		PDC3Latch = tpwm + (int) (__builtin_mulss(volts, w) >> 15); //(__builtin_mulss(voltstpwm, w) >> 15);
	}

	if (PDC1Latch > ALTDTR_DIV2) PDC1 = PDC1Latch; // - ALTDTR_DIV2;
	else PDC1 = ALTDTR_DIV2;

	if (PDC2Latch > ALTDTR_DIV2) PDC2 = PDC2Latch; // - ALTDTR_DIV2;
	else PDC2 = ALTDTR_DIV2;

	if (PDC3Latch > ALTDTR_DIV2) PDC3 = PDC3Latch; // - ALTDTR_DIV2;
	else PDC3 = ALTDTR_DIV2;

	return;

}

