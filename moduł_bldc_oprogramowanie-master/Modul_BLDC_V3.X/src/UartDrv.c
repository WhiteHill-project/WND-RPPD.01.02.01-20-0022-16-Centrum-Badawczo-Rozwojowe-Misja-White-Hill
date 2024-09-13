
// <editor-fold defaultstate="collapsed" desc="Files to Include">
#include <stddef.h>
#include <stdbool.h>

#include "UartDrv.h"
#include "system.h"
#include "user.h"

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="GLOBAL VARIABLES">
static defUART uarts[uartMAX_PORTS] = {
	{.nr = serCOM1, .pUartReg = (PUART) & U1MODE, .rxBuf = NULL, .txBuf = NULL},
	{.nr = serCOM2, .pUartReg = (PUART) & U2MODE, .rxBuf = NULL, .txBuf = NULL},
};
// </editor-fold>

static uint16_t UartBrgValueCalc(uint32_t baudrate, uint16_t baudspeed);

// ********************* FUNCTIONS **********************************************

pUART_t UartInit(eCOMPort uartNr,
				 uint32_t baudrate,
				 eParity parity,
				 eStopBits stopBits,
				 eDataBits dataBits,
				 eFlowControl flow,
				 uint16_t rxBufferSize,
				 uint16_t txBufferSize) {
	pUART_t ptrUart = NULL;
	uint16_t brgValue;
	if (uartNr < uartMAX_PORTS) {
		ptrUart = &uarts[uartNr];
		ptrUart->pUartReg->uxmode = 0;
		ptrUart->pUartReg->uxsta = 0;
		//check baudrate
		ptrUart->pUartReg->uxmode &= (~_U1MODE_BRGH_MASK); // BRGH=0
		brgValue = UartBrgValueCalc(baudrate, 16);
		if (brgValue == 0) {
			ptrUart->pUartReg->uxmode |= (_U1MODE_BRGH_MASK);
			brgValue = UartBrgValueCalc(baudrate, 4);
			if (brgValue == 0) return NULL;
		}
		ptrUart->pUartReg->uxbrg = brgValue;
		ptrUart->pUartReg->uxmode |= (flow << _U1MODE_UEN_POSITION);
		if (dataBits == serBITS_9)
			ptrUart->pUartReg->uxmode |= (3 << _U1MODE_PDSEL_POSITION);
		else
			ptrUart->pUartReg->uxmode |= (parity << _U1MODE_PDSEL_POSITION);
		ptrUart->pUartReg->uxmode |= (stopBits << _U1MODE_STSEL_POSITION);

		ptrUart->rxBuf = Cb_initialize(rxBufferSize, sizeof (uint8_t), NULL);
		ptrUart->txBuf = Cb_initialize(txBufferSize, sizeof (uint8_t), NULL);
		if (ptrUart->rxBuf == NULL || ptrUart->txBuf == NULL) {
			// if some errors
		}
		switch (ptrUart->nr) {
			case serCOM1:
				_U1RXIF = 0;
				_U1RXIE = 1;
				break;
			case serCOM2:
				_U2RXIF = 0;
				_U2RXIE = 1;
				break;
			default:
				break;
		}
		ptrUart->pUartReg->uxmode |= _U1MODE_UARTEN_MASK;
		ptrUart->pUartReg->uxsta |= _U1STA_UTXEN_MASK;
		return ptrUart;
	}
	return NULL;
}

static uint16_t UartBrgValueCalc(uint32_t baudrate, uint16_t baudspeed) {
	uint16_t brgVal;
	uint32_t actualBaudrate, error, errorPercent;

	brgVal = (uint16_t) ((uint64_t) SYS_CLK_FrequencyPeripheralGet() / baudrate / baudspeed) - 1;
	actualBaudrate = (uint32_t) ((uint64_t) SYS_CLK_FrequencyPeripheralGet() / (baudspeed * (brgVal + 1)));
	error = (actualBaudrate > baudrate ? actualBaudrate - baudrate : baudrate - actualBaudrate);
	errorPercent = (((error * 100)+(baudrate / 2)) / baudrate);
	// if error less than 2%
	if (errorPercent < 2) {
		return brgVal;
	}
	return 0;
}

int16_t UartReadByte(pUART_t uart) {
	int16_t data = 0;

	if (Cb_get(uart->rxBuf, &data) == true)
		return (int16_t) data;
	else
		return (-1);
}

uint16_t UartRead(pUART_t uart, uint8_t *buffer, const uint16_t numbytes) {
	uint16_t numBytesRead = 0;
	int16_t data;

	while (numBytesRead < (numbytes)) {
		data = UartReadByte(uart);
		if ((-1 != data)) {
			buffer[numBytesRead++] = data;
		} else // no more data in buff
			break;
	}
	return numBytesRead;
}

uint8_t Uart_Peek(pUART_t uart, uint16_t offset) {
	uint8_t data = 0;
	Cb_peek(uart->rxBuf, &data, offset);
	return data;
}

void UartWriteByte(pUART_t uart, const uint8_t byte) {
	Cb_put(uart->txBuf, (uint8_t*) & byte);
	switch (uart->nr) {
		case serCOM1:
			if (_U1TXIE == false) {
				_U1TXIE = true;
				_U1TXIF = true;
			}
			break;
		case serCOM2:
			if (_U2TXIE == false) {
				_U2TXIE = true;
				_U2TXIF = true;
			}
			break;
		default:
			break;
	}
}

uint16_t UartWrite(pUART_t uart, const uint8_t *buffer, const uint16_t numbytes) {
	uint16_t numBytesWritten = 0;

	while (numBytesWritten < (numbytes)) {
		if (CbIsFull(uart->txBuf)) {
			break;
		} else {

			UartWriteByte(uart, buffer[numBytesWritten++]);
		}
	}
	return numBytesWritten;
}

uint16_t UartRXBufferSizeGet(pUART_t uart) {
	return CbAvailableData(uart->rxBuf);
}

uint16_t UartTXBufferSizeGet(pUART_t uart) {
	return CbAvailableData(uart->txBuf);
}

bool UartRXBufferIsEmpty(pUART_t uart) {
	return CbIsEmpty(uart->rxBuf);
}

bool UartTXBufferIsFull(pUART_t uart) {
	return CbIsFull(uart->txBuf);
}

bool UartIsSending(pUART_t uart) {
	if ((uart->pUartReg->uxsta & (_U1STA_TRMT_MASK)) == 0 ||
		CbAvailableData(uart->txBuf) > 0) {
		return true;
	} else
		return false;
}

uint16_t UartStatusGet(pUART_t uart) {
	return uart->pUartReg->uxsta;
}

// <editor-fold defaultstate="collapsed" desc="Interrupts Rutines">
// UART1 Interrupts

void __attribute__((interrupt, auto_psv)) _U1TXInterrupt(void) {
	uint8_t byteToTx;
	if (CbIsEmpty(uarts[serCOM1].txBuf)) {
		_U1TXIE = false;
		return;
	}
	_U1TXIF = false;
	while ((uarts[serCOM1].pUartReg->uxsta & _U1STA_UTXBF_MASK) == 0) {
		if (Cb_get(uarts[serCOM1].txBuf, &byteToTx))
			uarts[serCOM1].pUartReg->uxtxreg = byteToTx;
		else
			break;
	}
}

void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void) {
	uint8_t rxByte;

	while ((uarts[serCOM1].pUartReg->uxsta & _U1STA_URXDA_MASK) != 0) { 
		rxByte = uarts[serCOM1].pUartReg->uxrxreg;
		if (Cb_put(uarts[serCOM1].rxBuf, &rxByte) == false) break;
	}
	_U1RXIF = false;
}

void __attribute__((interrupt, auto_psv)) _U1ErrInterrupt(void) {
	uint16_t temp;

	_U1EIF = false;

	uarts[serCOM1].pUartReg->uxsta &= (~(_U1STA_OERR_MASK | _U1STA_FERR_MASK | _U1STA_PERR_MASK));
	temp = uarts[serCOM1].pUartReg->uxrxreg;
}

// UART2 Interrupts

void __attribute__((interrupt, auto_psv)) _U2TXInterrupt(void) {
	uint8_t byteToTx;
	if (CbIsEmpty(uarts[serCOM2].txBuf)) {
		_U2TXIE = false;
		return;
	}
	_U2TXIF = false;
	while ((uarts[serCOM2].pUartReg->uxsta & _U2STA_UTXBF_MASK) == 0) {
		if (Cb_get(uarts[serCOM2].txBuf, &byteToTx))
			uarts[serCOM2].pUartReg->uxtxreg = byteToTx;
		else
			break;
	}
}

void __attribute__((interrupt, auto_psv)) _U2RXInterrupt(void) {
	uint8_t rxByte;

	while ((uarts[serCOM2].pUartReg->uxsta & _U2STA_URXDA_MASK) != 0) { 
		rxByte = uarts[serCOM2].pUartReg->uxrxreg;
		if (Cb_put(uarts[serCOM2].rxBuf, &rxByte) == false) break;
	}
	_U2RXIF = false;
}

void __attribute__((interrupt, auto_psv)) _U2ErrInterrupt(void) {
	uint16_t temp;

	_U2EIF = false;

	uarts[serCOM2].pUartReg->uxsta &= (~(_U2STA_OERR_MASK | _U2STA_FERR_MASK | _U2STA_PERR_MASK));
	temp = uarts[serCOM2].pUartReg->uxrxreg;
}

// </editor-fold>

