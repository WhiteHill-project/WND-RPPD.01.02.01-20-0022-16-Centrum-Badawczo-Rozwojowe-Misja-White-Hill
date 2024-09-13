#ifndef SYSTEM_H
#define SYSTEM_H
/******************************************************************************/
/* System Level #define Macros                                                */
/******************************************************************************/
/* Microcontroller MIPs (FCY) */
#define Fin		10000000UL
#define M		48
#define N1		2
#define N2		2
#define SYS_FREQ	Fin*M/(N1*N2)
#define FCY		SYS_FREQ/2ul	//FCY=((xtal*PLL)/2)


/******************************************************************************/
/* System Function Prototypes                                                 */
/******************************************************************************/

/**
 * @Param
	none
 * @Returns
	none
 * @Description
	Handles clock switching/osc initialization
 * @Example
	ConfigureOscillator();
 */
void ConfigureOscillator(void);

#define SYS_CLK_FrequencyPeripheralGet()	(FCY)
#define TICKS_PER_MICROSECONDS		(FCY/1000000ul)
#define TICKS_PER_MILISECONDS		(FCY/1000ul)

#endif // SYSTEM_H
