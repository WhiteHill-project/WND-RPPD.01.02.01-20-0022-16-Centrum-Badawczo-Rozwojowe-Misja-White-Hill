/* 
 * File:   UartDrv.h
 * Author: Kijas Jacek
 *
 * Created on 14 luty 2016, 14:49
 */

#ifndef UARTDRV_H
#define	UARTDRV_H

/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/
// <editor-fold defaultstate="collapsed" desc="Files to Include">
/* Device header file */
#if defined(__XC16__)
#include <xc.h>
#elif defined(__C30__)
#if defined(__dsPIC33E__)
#include <p33Exxxx.h>
#elif defined(__dsPIC33F__)
#include <p33Fxxxx.h>
#endif
#endif
#include <stdbool.h>
#include <stdint.h>

#include "circularbuffer.h"
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="MACROS">
#define uartMAX_PORTS	( 2 )	// max number of serial ports we are supporting
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="Type definitions">

typedef enum {
	serCOM1,
	serCOM2,
	serCOM3,
	serCOM4,
	serCOM5,
	serCOM6,
	serCOM7,
	serCOM8
} eCOMPort;

typedef enum {
	serPARITY_NO,
	serPARITY_EVEN,
	serPARITY_ODD
} eParity;

typedef enum {
	serSTOP_1,
	serSTOP_2
} eStopBits;

typedef enum {
	serBITS_8,
	serBITS_9
} eDataBits;

typedef enum {
	serFLOW_NO,
	serFLOW_RTS_ONLY,
	serFLOW_RTS_CTS,
} eFlowControl;

typedef struct {
	const eCOMPort nr;
	PUART const pUartReg;
	pCircularBuffer_t rxBuf;
	pCircularBuffer_t txBuf;
} defUART, *pUART_t;
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="Function Prototypes">
/* Function:
	pUART_t UartInit(...)
  Summary:
	Function to configure uart port
	if intialisation succesful then return pointer to uart structure else return null pointer
 */
pUART_t UartInit(eCOMPort uartNr,
		uint32_t baudrate,
		eParity parity ,
		eStopBits stopBits,
		eDataBits dataBits,
		eFlowControl flow,
		uint16_t rxBufferSize,
		uint16_t txBufferSize);

/* Function:
	int16_t UartReadByte(pUART_t uart);
 Summary:
	Reads a byte of data from the UART
 Return:
	if no data return (-1) else return data
 */
int16_t UartReadByte(pUART_t uart);

/*  Function:
	uint16_t UartRead(pUART_t uart, uint8_t *buffer, const uint16_t numbytes)
  Summary:
	Returns the number of bytes read by the UART peripheral
 */
uint16_t UartRead(pUART_t uart, uint8_t *buffer, const uint16_t numbytes);

/* Function:
	uint8_t Uart_Peek(pUART_t uart, uint16_t offset)
  Summary:
	Returns the character in the read sequence at the offset provided, without
	extracting it
 */
uint8_t Uart_Peek(pUART_t uart, uint16_t offset);

/* Function:
	void UartWriteByte(pUART_t uart, const uint8_t byte)
  Summary:
	Writes a byte of data to the UART
 */
void UartWriteByte(pUART_t uart, const uint8_t byte);

/*  Function:
	uint16_t UartWrite(pUART_t uart, const uint8_t *buffer, const uint16_t numbytes)
  Summary:
	Returns the number of bytes written into the internal buffer
 */
uint16_t UartWrite(pUART_t uart, const uint8_t *buffer, const uint16_t numbytes);

/* Function:
  uint16_t UartRXBufferSizeGet(pUART_t uart)
  Summary:
	Returns the size of the receive buffer
 */
uint16_t UartRXBufferSizeGet(pUART_t uart);

/* Function:
  uint16_t UartTXBufferSizeGet(pUART_t uart)
  Summary:
	Returns the size of the transmit buffer
 */
uint16_t UartTXBufferSizeGet(pUART_t uart);

/* Function:
  bool UartRXBufferIsEmpty(pUART_t uart)
  Summary:
	Returns the status of the receive buffer
 */
bool UartRXBufferIsEmpty(pUART_t uart);

/* Function:
	bool UartTXBufferIsFull(pUART_t uart)
  Summary:
	Returns the status of the transmit buffer
 */
bool UartTXBufferIsFull(pUART_t uart);

/* Function:
	bool UartIsSending(pUART_t uart)
  Summary:
	Returns true if uart still sending data
 */
bool UartIsSending(pUART_t uart);

/* Function:
	uint16_t UartStatusGet(pUART_t uart)
  Summary:
	Returns the status of the transmit and receive
 */
uint16_t UartStatusGet(pUART_t uart);

// </editor-fold>

#endif	/* UARTDRV_H */

