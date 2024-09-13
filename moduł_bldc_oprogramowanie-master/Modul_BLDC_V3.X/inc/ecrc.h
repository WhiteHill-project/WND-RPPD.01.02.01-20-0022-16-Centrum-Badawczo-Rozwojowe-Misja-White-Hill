/*************************************************************************
 *  © 2013 Microchip Technology Inc.
 *
 *  Project Name:    ECRC code example
 *  FileName:        ecrc.h
 *  Dependencies:    stdint.h, xc.h
 *  Processor:       PIC24, dsPIC
 *  Compiler:        MPLAB C30, XC16
 *  IDE:             MPLAB® IDE or MPLAB® X
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Description:     This file contains declarations for the ECRC functions.
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

#ifndef __ECRC_H
#define __ECRC_H

#include <xc.h>
#include <stdint.h>

typedef union {
	uint8_t value8[4];
	uint16_t value16[2];
	uint32_t value32;
} DATA32;

extern volatile uint32_t seed;

/****************************************************************************
  Function:
    void ECRCInit(uint32_t _polynomial, uint8_t _polynomialOrder, uint8_t _dataBitWidth, uint8_t _littleEndian)

  Summary:
    Initializes ECRC module.

  Description:
    Initializes ECRC module.

  Precondition:
    None.

  Parameters:
    _polynomial - CRC polynomial (32 bits).
    _polynomialOrder - CRC polynomial order.
    _dataBitWidth - data width in bits (valid values are 8,16 and 32).
    _littleEndian - if not zero then little endian.

  Returns:
    None.

  Remarks:
    None.
  ***************************************************************************/
void ECRCInit(uint32_t _polynomial, uint8_t _polynomialOrder, uint8_t _dataBitWidth, uint8_t _littleEndian);

/****************************************************************************
  Function:
    uint32_t ECRCIndirectSeed(uint32_t seed)

  Summary:
    Calculates indirect CRC initial value.

  Description:
    Calculates indirect CRC initial value.

  Precondition:
    None.

  Parameters:
    seed - initial CRC value.

  Returns:
    Returns indirect CRC initial value.

  Remarks:
    None.
 ****************************************************************************/
uint32_t ECRCIndirectSeed(uint32_t seed);

/****************************************************************************
  Function:
    void  ECRCSetSeed(uint32_t seed)

  Summary:
    Sets initial value of the CRC shift register.

  Description:
    Sets initial value of the CRC shift register.

  Precondition:
    The module must be initialized with ECRCInit().

  Parameters:
    seed - initial CRC value.

  Returns:
    None.

  Remarks:
    None.
 ****************************************************************************/
void  ECRCSetSeed(uint32_t seed);

/****************************************************************************
  Macro:
    ECRCCalculate(pData, dataLength)

  Summary:
    Calculates the CRC of the data stored in the buffer.

  Description:
    Calculates the CRC of the data stored in the buffer.

  Precondition:
    The module must be initialized with ECRCInit().

  Parameters:
    pData - pointer to the buffer with data.
    dataLength - data length in bytes.

  Returns:
    None.

  Remarks:
    To get CRC result use ECRCGetCRC() function.The data buffer address must
    be aligned by:
    1 byte for  DATA WIDTH == 8
    2 bytes for DATA WIDTH == 16
    4 bytes for DATA WIDTH == 32
    To do this use __attribute__((aligned(n))).
  ***************************************************************************/
void ECRCCalculateF(uint8_t* pData, uint16_t dataLength);
#define ECRCCalculate(pData, dataLength) \
ECRCCalculateF((uint8_t*) pData, (uint16_t) dataLength)

/****************************************************************************
  Function:
    uint32_t  ECRCGetCRC()

  Summary:
    Returns the CRC result.

  Description:
    Returns the CRC result.

  Precondition:
    The module must be initialized with ECRCInit().

  Parameters:
    None.

  Returns:
    Returns the CRC result.

  Remarks:
    The valid result is returned only one time. The second call of this function
    returns not correct value.
  ***************************************************************************/
uint32_t ECRCGetCRC();

/****************************************************************************
  Function:
    ECRCChangeDataWidth(uint8_t _dataBitWidth)

  Summary:
    Sets a new data width.

  Description:
    Sets a new data bit width without reinitialization of the ECRC.

  Precondition:
    The module must be initialized with ECRCInit().

  Parameters:
    _dataBitWidth - new data bit width to be set.

  Returns:
    None.

  Remarks:
    None.
 ****************************************************************************/
void ECRCChangeDataWidth(uint8_t _dataBitWidth);

/****************************************************************************
  Function:
    uint32_t ECRCReverse(uint32_t crc)

  Summary:
    Returns the bit reversed CRC result.

  Description:
    Returns the bit reversed CRC result.

  Precondition:
    None.

  Parameters:
    crc - CRC value to be reversed.

  Returns:
    Returns the bit revered CRC value.

  Remarks:
 None.
  ***************************************************************************/
uint32_t ECRCReverse(uint32_t crc);

#endif
