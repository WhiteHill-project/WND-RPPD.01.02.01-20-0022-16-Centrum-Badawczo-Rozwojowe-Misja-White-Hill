/* 
 * File:   Commun.h
 * Author: Jacek Kijas
 *
 * Created on 17 pazdziernika 2016, 09:03
 */

#ifndef COMMUN_H
#define	COMMUN_H


#define COMM_ADDRESS_GLOBAL 0xFF

//@ byte used to find start of frame
#define START_BYTE 0x55 

#define COMMUN_BAUDRATE		(125000)//(115200)
#define COMMUN_TIMEOUT_US	(8*1000000/(COMMUN_BAUDRATE/10))

typedef struct _COMM_HEADER_T {
	uint8_t startByte;
	uint8_t command;
	uint8_t receiverAddress;
	uint8_t senderAddress;
	uint8_t dataBytes;
	uint8_t frameNumber;
	uint8_t reserved;
	uint8_t crc;
} COMM_HEADER_T;

typedef union _COMM_FRAME_T {

	struct {
		COMM_HEADER_T header;
		uint8_t data[256 + 4]; // 256 data bytes + 2 bytes crc
	};
	uint8_t tab8[sizeof (COMM_HEADER_T) + 256 + 4];
	uint16_t tab16[(sizeof (COMM_HEADER_T) + 256 + 4)/sizeof(uint16_t)];
	uint32_t tab32[(sizeof (COMM_HEADER_T) + 256 + 4)/sizeof(uint32_t)];
} COMM_FRAME_T;

enum {
	cmdACK = 1,
	cmdDATAbyADDR,
	cmdGET_DATAbyADDR,
	cmdDATA,
	cmdGET_DATA,
	cmdPOSITION_CLEAR,
	cmdERROR,
	
	cmdMAX_COMMANDS // last command 
}; // COMMANDS

/**	------------------ FUNCTION PROTOTYPES ----------------------------------- */
void CommandInterpreter(COMM_FRAME_T* frame);
void uartCommunicationInit(void);
void CommunicationTask(void);

#endif	/* COMMUN_H */

