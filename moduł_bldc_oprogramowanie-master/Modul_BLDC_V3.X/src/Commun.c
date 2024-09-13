/* 
 * File:   Commun.c
 * Author: Jacek Kijas
 *
 * Created on 17 pazdziernika 2016, 09:03
 */

// <editor-fold defaultstate="collapsed" desc="Files to Include">
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "UartDrv.h"
#include "Commun.h"
#include "ecrc.h"
#include "ioPorts.h"
#include "time.h"
#include "main.h"
#include "user.h"
#include "debug.h"
//#include "sysTimer.h"
#include "paramether.h"

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="GLOBAL VARIABLES">
pUART_t commPort;
COMM_FRAME_T commReceivedFrame __attribute__((aligned(4)));

uint8_t commFrameToSend[sizeof (COMM_FRAME_T)];
volatile bool commSendInProgress = false;
tim_t *communTimeout;
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="Function prototypes">
void initDma1UartTx(void);
void UartSendByDMA(void *ptr, uint16_t size);

void CommandInterpreter(COMM_FRAME_T *receivedFrame);
void CreateHeader(COMM_HEADER_T *header, uint8_t command, uint8_t addressToSend, uint8_t dataBytes, uint8_t frameNumber);
void SendFrame(pUART_t uart, COMM_FRAME_T *frame);
void SendACK(pUART_t uart, COMM_FRAME_T *receivedFrame);
void SendDataByAddress(pUART_t uart, COMM_FRAME_T *receivedFrame);
void ResponseReceivedDataByAddress(pUART_t uart, COMM_FRAME_T *recFrame);
void SendData(pUART_t uart, COMM_FRAME_T *recFrame);
void ResponseReceivedData(pUART_t uart, COMM_FRAME_T *recFrame);
void SendErrorFrame(pUART_t uart, COMM_FRAME_T *receivedFrame, uint8_t errorNr);

// </editor-fold>

void __attribute__((interrupt, auto_psv)) _DMA1Interrupt(void) {
	_DMA1IF = 0; // Clear the DMA0 Interrupt Flag;
	commSendInProgress = false; // end of transmision
}

// <editor-fold defaultstate="collapsed" desc="Functions">

void CommandInterpreter(COMM_FRAME_T *receivedFrame) {
	uint16_t i, address;

	if (receivedFrame->header.receiverAddress == myAddress) { // if frame is addressed to me
		switch (receivedFrame->header.command) {
			case cmdACK:
				break;
			case cmdDATAbyADDR:
			{
				//				DBG_PRINT("\r\nCommand: DATAbyADDR");
				for (i = 0; i < receivedFrame->header.dataBytes; i += 2) { // get all data from frame
					address = receivedFrame->data[i];
					if (address < (&Parameter.lastAddr - Parameter.cTab) && address > ((uint16_t) & Parameter.MotorCurrent - (uint16_t) & Parameter))
						Parameter.cTab[address] = receivedFrame->data[i + 1];
				}
				//				SendACK(commPort, receivedFrame);
				ResponseReceivedDataByAddress(commPort, receivedFrame);
				ParamChanged = true;
				//				DBG_PRINT("\r\n New %d bytes arrived.\r\n", (uint16_t) receivedFrame->header.dataBytes);
				break;
			}
			case cmdGET_DATAbyADDR:
				//				DBG_PRINT("\r\nCommand: GET_DATAbyADDR");

				SendDataByAddress(commPort, receivedFrame);
				break;
			case cmdDATA:
				address = receivedFrame->data[0];
				i = 0;
				if (address < ((uint16_t) & Parameter.error - (uint16_t) & Parameter))
					i = ((uint16_t) & Parameter.error - address);
				for (; i < receivedFrame->header.dataBytes - 1; i++) {
					if ((address + i) < (&Parameter.lastAddr - Parameter.cTab))
						Parameter.cTab[address + i] = receivedFrame->data[i + 1];
				}
				//				DBG_PRINT("\r\nCommand: DATAfromADDR: %d BYTES: %d", address, receivedFrame->header.dataBytes - 1);

				//				SendACK(commPort, receivedFrame);
				ResponseReceivedData(commPort, receivedFrame);
				ParamChanged = true;
				break;
			case cmdGET_DATA:
				SendData(commPort, receivedFrame);
				//				DBG_PRINT("\r\nCommand: GET_DATAfromADDR:%d BYTES: %d", (uint16_t) receivedFrame->data[0], (uint16_t) receivedFrame->data[1]);
				break;
			case cmdPOSITION_CLEAR:
				DBG_PRINT("\r\nCommand: POSITION_CLEAR");

				MotorClearPosition();
				SendACK(commPort, receivedFrame);
			case cmdERROR:
				DBG_PRINT("\r\n Command: ERROR");
				break;
			default: // unknown command
				Parameter.error.communUnknownCommand = 1;
				DBG_PRINT("\r\n Unknown Command: %d\r\n", receivedFrame->header.command);
				break;
		}
	} else if (receivedFrame->header.receiverAddress == COMM_ADDRESS_GLOBAL) { // if frame is addressed to all
		switch (receivedFrame->header.command) {
			default: // unknown global command
				Parameter.error.communUnknownCommand = 1;
				DBG_PRINT("\r\n Unknown Global Command: %d\r\n", receivedFrame->header.command);
				break;
		}
	}
}

/* DMA1 configuration to uart transfer */
void initDma1UartTx(void) {
	//  Associate DMA Channel 0 with UART Tx
	DMA1REQ = 0x000C; // Select UART1 Transmitter
	DMA1PAD = (volatile unsigned int) &U1TXREG;

	//  Configure DMA Channel 1 to:
	//  Transfer data from RAM to UART
	//  One-Shot mode
	//  Register Indirect with Post-Increment
	//  Using single buffer
	//  Transfer bytes
	DMA1CONbits.AMODE = 0;
	DMA1CONbits.MODE = 1;
	DMA1CONbits.DIR = 1;
	DMA1CONbits.SIZE = 1;
	DMA1CNT = 0; // 
	// Associate one buffer with Channel 0 for one-shot operation
	DMA1STAH = 0x0000;
	DMA1STAL = 0x0000;

	_DMA1IF = 0; // Clear DMA Interrupt Flag
	_DMA1IE = 1; // Enable DMA interrupt
}

void UartSendByDMA(void *ptr, uint16_t size) {
	if (commSendInProgress == false) {
		memcpy(commFrameToSend, ptr, size);
		DMA1STAH = 0x0000;
		DMA1STAL = (unsigned int) commFrameToSend;
		DMA1CNT = size - 1;

		U1STAbits.OERR = 0;
		DMA1CONbits.CHEN = 1; // Re-enable DMA1 Channel
		DMA1REQbits.FORCE = 1; // Manual mode: Kick-start the first transfer
		commSendInProgress = true;
	}
}

void uartCommunicationInit(void) {
	commPort = UartInit(serCOM1,
						COMMUN_BAUDRATE,
						serPARITY_NO,
						serSTOP_1,
						serBITS_8,
						serFLOW_NO,
						512,
						1);

	if (commPort == NULL) {
		// if UART initialization error
		DBG_PRINT("Uart 2 Initialization Error!\r\n");
	} else {
		commPort->pUartReg->uxsta |= _U1STA_UTXINV_MASK; // transmit pin should be inverted
		commPort->pUartReg->uxmode &= (~_U1MODE_UARTEN_MASK); // disable uart
		// Enable rts pin in simplex mode for easy control RS485 TX_enable pin
		commPort->pUartReg->uxmode |= _U1MODE_RTSMD_MASK; // UxRTS pin is in Simplex mode
		commPort->pUartReg->uxmode |= (1 << _U1MODE_UEN_POSITION); // UxTX, UxRX and UxRTS pins are enabled and used; UxCTS pin is controlled by PORT latches

		commPort->pUartReg->uxmode |= _U1MODE_UARTEN_MASK; // renable uart
		commPort->pUartReg->uxsta |= _U1STA_UTXEN_MASK; // enable uart tx
		initDma1UartTx();
		communTimeout = tim_initialize();
		if (communTimeout == NULL) {
			DBG_PRINT("nieudana inicjalizacja communTimeout");
			while (1);
		}
	}
}

void CommunicationTask(void) {
	static bool startByteReceived = false;
	static uint16_t buffCounter, bytesToReceive;
#ifdef DEBUG_MODE
	static tim_t *dbgSendTime = NULL;
	if (dbgSendTime == NULL)
		dbgSendTime = tim_initialize();
#endif
	static uint32_t OK_frames, BAD_frames, BAD_Headers;
	uint32_t crc, frameCrc;
	uint16_t i, j;

	if (buffCounter > sizeof (COMM_FRAME_T))
		startByteReceived = false;
	if (startByteReceived == false)
		buffCounter = 0;
	if (!UartRXBufferIsEmpty(commPort)) {
		UartRead(commPort, (uint8_t*) & commReceivedFrame.tab8[buffCounter], 1);
		buffCounter++;
		//		timeStamp = SysMicrosGet();
		timeout_clear(communTimeout);
		switch (buffCounter) {
			case 1: // Reading START_BYTE
				if (commReceivedFrame.header.startByte == START_BYTE) {
					startByteReceived = true;
					//					buffCounter++;
				}
				break;
			case 2: // command
			case 3: // receiverAddress
			case 4: // senderAddress
			case 5: // dataBytes
			case 6: // frameNumber
			case 7: // reserved
				//				buffCounter++;
				break;
			case 8: // header crc
				frameCrc = commReceivedFrame.header.crc;
				commReceivedFrame.header.crc = 0;
				ECRCSetSeed(seed);
				ECRCCalculate(commReceivedFrame.tab8, sizeof (COMM_HEADER_T)&0xFFFC);
				crc = (unsigned long) ECRCGetCRC();
				commReceivedFrame.header.crc = (uint8_t) frameCrc;
				if (frameCrc == (uint8_t) (crc & 0xFF)) {
					if (commReceivedFrame.header.dataBytes > 0) { // is another data to read
						bytesToReceive = sizeof (COMM_HEADER_T) + commReceivedFrame.header.dataBytes + 2;
						//						buffCounter++;
					} else { // only header as a complete frame
						OK_frames++;
						CommandInterpreter(&commReceivedFrame);
						startByteReceived = false;
					}
				} else { // error crc
					BAD_Headers++;
					DBG_PRINT("\r\nBledne CRC Headera. BAD_frames:%d", BAD_frames);
					for (i = 1; i < buffCounter; i++) { // find next START_BYTE
						if (commReceivedFrame.tab8[i] == START_BYTE) break;
					}
					if (i < buffCounter) { // if new START_BYTE found then flush older data 
						j = buffCounter;
						buffCounter = 0;
						for (; i < j; i++)
							commReceivedFrame.tab8[buffCounter++] = commReceivedFrame.tab8[i];
					} else
						startByteReceived = false;
				}
				break;
			default: // rest of data 
				if (buffCounter >= bytesToReceive || buffCounter >= sizeof (COMM_FRAME_T)) {
					frameCrc = (((unsigned int) commReceivedFrame.tab8[bytesToReceive - 1] << 8) + commReceivedFrame.tab8[bytesToReceive - 2]);
					commReceivedFrame.tab8[bytesToReceive - 2] = 0;
					commReceivedFrame.tab8[bytesToReceive - 1] = 0;
					commReceivedFrame.tab8[bytesToReceive] = 0;
					commReceivedFrame.tab8[bytesToReceive + 1] = 0;
					ECRCSetSeed(seed);
					ECRCCalculate(commReceivedFrame.tab8, (sizeof (COMM_HEADER_T) + commReceivedFrame.header.dataBytes + 3)&0xFFFC);
					crc = (unsigned long) ECRCGetCRC();
					commReceivedFrame.tab8[bytesToReceive - 2] = frameCrc & 0xFF;
					commReceivedFrame.tab8[bytesToReceive - 1] = (frameCrc >> 8)&0xFF;
					if ((uint16_t) (crc & 0xFFFF) == frameCrc) {
						OK_frames++;
						CommandInterpreter(&commReceivedFrame);
					} else {
						BAD_frames++;
						DBG_PRINT("\r\nBledne CRC calej ramki");
					}
					startByteReceived = false;
				}
				break;
		}
	}
	// if timeout from last byte expired then flush received bytes
	if (startByteReceived == true && timeout_set_ms(communTimeout, COMMUN_TIMEOUT_US, NULL) == 0) {
		startByteReceived = false;
	}
#ifdef DEBUG_MODE
	if (timeout_set_ms(dbgSendTime, 100, NULL) == 0) {
		// messages with debug info, Comment if not need
//		DBG_PRINT("\033[0m\033[2J\033[?25l");
//		DBG_PRINT("\033[1;1HPOSITION: %ld ", Parameter.Position); // move currsor to position
//		DBG_PRINT("\033[2;1HCURRENT: %dmA \033[2;25HCURRENT OFFSET: %d", Parameter.MotorCurrent, MotorCurrentOffset);
//		DBG_PRINT("\033[3;1HFLAGS: %#X \033[3;25HERRORS: %#X ", Parameter.StatFlags.all, Parameter.error.all);
//		DBG_PRINT("\033[4;1HOK FRAMES: %ld \033[4;25HERROR HEADERS: %ld \033[4;50HERROR FRAMES: %ld ", OK_frames, BAD_Headers, BAD_frames);
	}
#endif
}

void CreateHeader(COMM_HEADER_T *header, uint8_t command, uint8_t addressToSend, uint8_t dataBytes, uint8_t frameNumber) {
	header->startByte = START_BYTE;
	header->command = command;
	header->receiverAddress = addressToSend;
	header->senderAddress = myAddress;
	header->dataBytes = dataBytes;
	header->frameNumber = frameNumber;
	header->reserved = 0;
	header->crc = 0;
}

void SendFrame(pUART_t uart, COMM_FRAME_T *frame) {
	uint32_t crc;
	if (uart == NULL) return;

	// Calculate crc for header
	frame->header.crc = 0;
	ECRCSetSeed(seed);
	ECRCCalculate(&frame->header, sizeof (COMM_HEADER_T)&0xFFFC);
	crc = ECRCGetCRC();
	frame->header.crc = (uint8_t) crc;
	if (frame->header.dataBytes == 0) { // send only header
		UartSendByDMA(frame->tab8, sizeof (COMM_HEADER_T));
	} else { // some data to send
		// Calculate crc for all frame
		frame->data[frame->header.dataBytes] = 0; // clear crc for proper count crc
		frame->data[frame->header.dataBytes + 1] = 0;
		frame->data[frame->header.dataBytes + 2] = 0;
		ECRCSetSeed(seed);
		ECRCCalculate(frame->tab8, (sizeof (COMM_HEADER_T) + frame->header.dataBytes + 3)&0xFFFC);
		crc = ECRCGetCRC();
		frame->data[frame->header.dataBytes] = (uint8_t) crc & 0xFF;
		frame->data[frame->header.dataBytes + 1] = (uint8_t) (crc >> 8)&0xFF;
		UartSendByDMA(frame->tab8, sizeof (COMM_HEADER_T) + frame->header.dataBytes + 2);
	}
}

void SendACK(pUART_t uart, COMM_FRAME_T *receivedFrame) {
	COMM_FRAME_T frameToSend;

	CreateHeader(&frameToSend.header,
				cmdACK,
				receivedFrame->header.senderAddress,
				0,
				receivedFrame->header.frameNumber);

	SendFrame(uart, &frameToSend);
}

void SendDataByAddress(pUART_t uart, COMM_FRAME_T *recFrame) {
	COMM_FRAME_T frameToSend;
	uint16_t i, j, address;
	//	uint32_t crc;

	if (recFrame->header.dataBytes > 0) {
		i = (recFrame->header.dataBytes * 2);
		if (i > 255) i = 255;
		CreateHeader(&frameToSend.header,
					cmdDATAbyADDR,
					recFrame->header.senderAddress,
					i,
					recFrame->header.frameNumber);

		for (i = 0, j = 0; i < recFrame->header.dataBytes && j <= frameToSend.header.dataBytes; ++i) {
			address = recFrame->data[i];
			if (address < (&Parameter.lastAddr - Parameter.cTab)) {
				frameToSend.data[j++] = address;
				frameToSend.data[j++] = Parameter.cTab[address];
			}
		}

		SendFrame(uart, &frameToSend);
	}
}

void ResponseReceivedDataByAddress(pUART_t uart, COMM_FRAME_T *recFrame) {
	COMM_FRAME_T frameToSend;
	uint16_t i, j, address;
	//	uint32_t crc;

	if (recFrame->header.dataBytes > 0) {
		CreateHeader(&frameToSend.header,
					cmdDATAbyADDR,
					recFrame->header.senderAddress,
					recFrame->header.dataBytes,
					recFrame->header.frameNumber);

		for (i = 0, j = 0; i < recFrame->header.dataBytes && j <= frameToSend.header.dataBytes; ++i) {
			address = recFrame->data[i];
			frameToSend.data[j++] = address;
			frameToSend.data[j++] = Parameter.cTab[address];
		}

		SendFrame(uart, &frameToSend);
	}
}

void SendData(pUART_t uart, COMM_FRAME_T *recFrame) {
	COMM_FRAME_T frameToSend;
	uint8_t i, addr, bytes;
	//	uint32_t crc;

	if (recFrame->header.dataBytes > 0) {
		addr = recFrame->data[0];
		bytes = recFrame->data[1];

		if (((uint16_t) addr + bytes) > 256)
			bytes = 256 - addr;

		CreateHeader(&frameToSend.header,
					cmdDATA,
					recFrame->header.senderAddress,
					bytes + 1,
					recFrame->header.frameNumber);
		if ((addr) < (&Parameter.lastAddr - Parameter.cTab))
			frameToSend.data[0] = addr;
		for (i = 0; i < bytes; ++i) {
			if ((addr + i) < (&Parameter.lastAddr - Parameter.cTab))
				frameToSend.data[i + 1] = Parameter.cTab[addr + i];
		}

		SendFrame(uart, &frameToSend);
	}
}

void ResponseReceivedData(pUART_t uart, COMM_FRAME_T *recFrame) {
	COMM_FRAME_T frameToSend;
	uint8_t i, addr, bytes;

	if (recFrame->header.dataBytes > 0) {
		addr = recFrame->data[0];
		bytes = (recFrame->header.dataBytes - 1);

		if (((uint16_t) addr + bytes) > 256)
			bytes = 256 - addr;

		CreateHeader(&frameToSend.header,
					cmdDATA,
					recFrame->header.senderAddress,
					bytes + 1,
					recFrame->header.frameNumber);
		frameToSend.data[0] = addr;
		for (i = 0; i < bytes; ++i) {
			frameToSend.data[i + 1] = Parameter.cTab[addr + i];
		}
		SendFrame(uart, &frameToSend);
	}
}

void SendErrorFrame(pUART_t uart, COMM_FRAME_T *receivedFrame, uint8_t errorNr) {
	COMM_FRAME_T frameToSend;

	CreateHeader(&frameToSend.header,
				cmdERROR,
				receivedFrame->header.senderAddress,
				1,
				receivedFrame->header.frameNumber);

	frameToSend.data[0] = errorNr;

	SendFrame(uart, &frameToSend);
}




// EOF
