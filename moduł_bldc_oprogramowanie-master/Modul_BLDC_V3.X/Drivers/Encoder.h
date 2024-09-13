/* 
 * File:   Encoder.h
 * Author: Jacek
 *
 * Created on 23 kwietnia 2018, 08:34
 */

#ifndef ENCODER_H
#define	ENCODER_H

#ifdef	__cplusplus
extern "C" {
#endif


void InitEncoder(void);
unsigned long EncoderGetPosition(void);
void EnkoderClearPosition(void);

#ifdef	__cplusplus
}
#endif

#endif	/* ENCODER_H */

