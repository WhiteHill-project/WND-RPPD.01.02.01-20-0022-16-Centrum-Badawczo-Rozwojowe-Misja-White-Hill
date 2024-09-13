/*************************************************************************
 *  © 2013 Microchip Technology Inc.
 *
 *  Project Name:    ECRC code example
 *  FileName:        ecrc.c
 *  Dependencies:    ecrc.h
 *  Processor:       PIC24, dsPIC
 *  Compiler:        MPLAB C30, XC16
 *  IDE:             MPLAB® IDE or MPLAB® X
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Description:     This file contains implementations for the ECRC functions.
 *
 *************************************************************************/
/**************************************************************************
 * MICROCHIP SOFTWARE NOTICE AND DISCLAIMER: You may use this software, and
 * any derivatives created by any person or entity by or on your behalf,
 * exclusively with Microchip's products in accordance with applicable
 * software license terms and conditions, a copy of which is provided for
 * your referencein accompanying documentation. Microchip and its licensors
 * retain all ownership and intellectual property rights in the
 * accompanying software and in all derivatives hereto.
 *
 * This software and any accompanying information is for suggestion only.
 * It does not modify Microchip's standard warranty for its products. You
 * agree that you are solely responsible for testing the software and
 * determining its suitability. Microchip has no obligation to modify,
 * test, certify, or support the software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE, ITS INTERACTION WITH
 * MICROCHIP'S PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE IN ANY
 * APPLICATION.
 *
 * IN NO EVENT, WILL MICROCHIP BE LIABLE, WHETHER IN CONTRACT, WARRANTY,
 * TORT (INCLUDING NEGLIGENCE OR BREACH OF STATUTORY DUTY), STRICT
 * LIABILITY, INDEMNITY, CONTRIBUTION, OR OTHERWISE, FOR ANY INDIRECT,
 * SPECIAL, PUNITIVE, EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE,
 * FOR COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE,
 * HOWSOEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY
 * OR THE DAMAGES ARE FORESEEABLE. TO THE FULLEST EXTENT ALLOWABLE BY LAW,
 * MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS
 * SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID
 * DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF
 * THESE TERMS.
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Author        	Date      	Comments on this revision
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Jyoti Shrinivas	27/07/09	First version of source file - v1
 * Anton Alkhimenok 22/05/13    Bugs are fixed, standard CRCs calculation
 *                              is added.
 *************************************************************************/

#include "ecrc.h"

volatile uint32_t polynomial;
volatile uint8_t polynomialOrder;
volatile uint8_t dataBitWidth;
volatile uint32_t seed = 0;

void ECRCInit( uint32_t _polynomial, uint8_t _polynomialOrder, uint8_t _dataBitWidth, uint8_t _littleEndian) {
	CRCCON1 = 0;
	CRCCON2 = 0;

	CRCCON1bits.CRCEN = 1; // enable CRC

	if (_littleEndian) {
		CRCCON1bits.LENDIAN = 1;
	}

	CRCCON1bits.CRCISEL = 0; // interrupt when data ready

	CRCCON2bits.DWIDTH = _dataBitWidth - 1; // data width

	CRCCON2bits.PLEN = _polynomialOrder - 1; // polynomial length

	CRCCON1bits.CRCGO = 1; // start CRC calculation

	polynomial = _polynomial;
	polynomialOrder = _polynomialOrder;
	dataBitWidth = _dataBitWidth;

	// set polynomial
	CRCXORL = ((__eds__ DATA32*) & _polynomial)->value16[0];
	CRCXORH = ((__eds__ DATA32*) & _polynomial)->value16[1];
}

uint32_t ECRCIndirectSeed(uint32_t seed) {
	uint8_t lsb;
	uint8_t i;
	uint32_t msbmask;

	if (seed == 0) {
		return 0;
	}

	msbmask = ((uint32_t) 1) << (polynomialOrder - 1);

	for (i = 0; i < polynomialOrder; i++) {
		lsb = seed & 1;
		if (lsb) seed ^= polynomial;
		seed >>= 1;
		if (lsb) seed |= msbmask;
	}

	return seed;
}

void ECRCSetSeed(uint32_t seed) {
	CRCWDATL = ((__eds__ DATA32*) & seed)->value16[0];
	CRCWDATH = ((__eds__ DATA32*) & seed)->value16[1];
}

void ECRCCalculateF(uint8_t* pData, uint16_t dataLength) {
	switch (dataBitWidth) {
		case 8:
		{
			uint8_t* pointer = (uint8_t*) pData;
			while (dataLength--) {
				// wait if FIFO is full
				while (CRCCON1bits.CRCFUL);

				// byte access to FIFO
				*((uint8_t*) & CRCDATL) = *pointer++;
			}
		}
			break;

		case 16:
		{
			uint16_t* pointer = (uint16_t*) pData;
			dataLength >>= 1;
			while (dataLength--) {
				// wait if FIFO is full
				while (CRCCON1bits.CRCFUL);

				// 16 bit word access to FIFO
				CRCDATL = *pointer++;
			}
		}
			break;

		case 32:
		{
			uint32_t* pointer = (uint32_t*) pData;
			dataLength >>= 2;
			while (dataLength--) {
				// wait if FIFO is full
				while (CRCCON1bits.CRCFUL);

				// 32 bit word access to FIFO
				CRCDATL = ((DATA32*) pointer)->value16[0];
				CRCDATH = ((DATA32*) pointer)->value16[1];

				// move to the next data
				pointer++;
			}
		}
			break;

		default:
			;
	}//end of switch
}

uint32_t ECRCGetCRC() {
	uint32_t crc = 0;

	while (!CRCCON1bits.CRCMPT);

	// Set data width to polynomial order
	CRCCON2bits.DWIDTH = polynomialOrder - 1;

	// Dummy data to shift out the CRC result
	switch (polynomialOrder) {
		case 8:
			*((uint8_t*) & CRCDATL) = 0;
			// Delay to shift out the result
			asm volatile ("repeat #4");
			asm volatile ("nop");
			crc = CRCWDATL & 0x00ff;
			break;
		case 16:
			CRCDATL = 0;
			// Delay to shift out the result
			asm volatile ("repeat #8");
			asm volatile ("nop");
			crc = CRCWDATL;
			break;
		case 32:
			CRCDATL = 0;
			CRCDATH = 0;
			// Delay to shift out the result
			asm volatile ("repeat #16");
			asm volatile ("nop");
			crc = ((DATA32*) & CRCWDATL)->value32;
			break;
		default:
			;
	}

	// Restore data width
	CRCCON2bits.DWIDTH = dataBitWidth - 1;

	return crc;
}

void ECRCChangeDataWidth(uint8_t _dataBitWidth) {
	while (!CRCCON1bits.CRCMPT);

	CRCCON2bits.DWIDTH = _dataBitWidth - 1;
	dataBitWidth = _dataBitWidth;
}

uint32_t ECRCReverse(uint32_t crc) {
	uint32_t maskin = 0;
	uint32_t maskout;
	uint32_t result = 0;
	uint8_t i;

	switch (polynomialOrder) {
		case 8:
			maskin = 0x80;
			maskout = 0x01;
			break;
		case 16:
			maskin = 0x8000;
			maskout = 0x0001;
			break;
		case 32:
			maskin = 0x80000000;
			maskout = 0x00000001;
			break;
		default:
			;
	}

	for (i = 0; i < polynomialOrder; i++) {
		if (crc & maskin) {
			result |= maskout;
		}
		maskin >>= 1;
		maskout <<= 1;
	}

	return result;
}
