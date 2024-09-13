/* 
 * File:   ioPorts.c
 * Author: Jacek Kijas
 *
 * Created on 13 wrzesnia 2016, 10:58
 */

#include "ioPorts.h"
#include <pps.h>

void Ports_Initialize(void) {
	LATA = 0xFFEF;
	ANSELA = 0x0003; // RA0 and RA1 as analog
	TRISA = 0xFFEF;

	LATB = 0x03FF;
	ANSELB = 0x0009;
	TRISB = 0x017F;
	CNPUB = 0x0004; // pull up address pin

	LATC = 0xFFFB;
	ANSELC = 0x0000;
	TRISC = 0xFFFF;
	CNPUC = 0x0003; // pull up address pins

	CNPUAbits.CNPUA7 = 1; // mot limit pos
	CNPUAbits.CNPUA10 = 1; // mot home pos

	// PPS initialize output and input
	PPSUnLock;

	iPPSOutput(OUT_PIN_PPS_RP20, OUT_FN_PPS_U1TX); // UART 1 Transmit
	iPPSInput(IN_FN_PPS_U1RX, IN_PIN_PPS_RP36); // UART 1 Receive

	// Uart for debug communication
	iPPSOutput(OUT_PIN_PPS_RP37, OUT_FN_PPS_U2TX);
	iPPSInput(IN_FN_PPS_U2RX, IN_PIN_PPS_RP38);

	// interrupt from external position step signal
	iPPSInput(IN_FN_PPS_INT1, IN_PIN_PPS_RP56); // For external interrupt

	PPSLock;
}
