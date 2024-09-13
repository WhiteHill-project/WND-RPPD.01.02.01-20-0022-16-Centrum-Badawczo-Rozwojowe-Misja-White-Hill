/* 
 * File:   SVM.h
 * Author: Jacek Kijas
 *
 * Created on 5 stycznia 2017, 13:07
 */

#ifndef SVM_H
#define	SVM_H

#define	ALTDTR_DIV2	30		//Half of ALTDTRx register configured in InitMCPWM() function 

#define MAX_POS_VOLTS	(int)(PHASE1>>1) // 
#define MAX_NEG_VOLTS	(int)(-MAX_POS_VOLTS)


void SVM(int, unsigned int);

#endif	/* SVM_H */

