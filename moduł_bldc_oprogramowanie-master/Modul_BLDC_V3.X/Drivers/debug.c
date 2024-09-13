#include "debug.h"

#ifdef DEBUG_MODE
#include <stdio.h>
#include "UartDrv.h"

pUART_t dbgUart;
#endif 

void DBG_Init(void) {
#ifdef DEBUG_MODE
	dbgUart = UartInit(serCOM2,
					DBG_BAUDRATE,
					serPARITY_NO,
					serSTOP_1,
					serBITS_8,
					serFLOW_NO,
					1,
					256);
	if (dbgUart == NULL) {
		// if initialize error
	}
#endif // DEBUG_MODE
}

void DBG_PRINT(char *fmt, ...) {
#ifdef DEBUG_MODE
	char buff[256];
	uint16_t numBytes;
	va_list arg_ptr;

	if (dbgUart) {
		va_start(arg_ptr, fmt);
		//		vprintf(fmt, arg_ptr);
		numBytes = vsprintf(buff, fmt, arg_ptr);
		va_end(arg_ptr);
		UartWrite(dbgUart, (uint8_t*) buff, numBytes);
	}
#endif // DEBUG_MODE
}


// EOF 
