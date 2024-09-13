/* 
 * File:   debug.h
 * Author: Jacek Kijas
 *
 * Created on 17 listopada 2016, 14:20
 */

#ifndef DEBUG_H
#define	DEBUG_H

#ifdef	__cplusplus
extern "C" {
#endif

//#define DEBUG_MODE // uncomment if need debug info
#define DBG_BAUDRATE (115200)

	void DBG_Init(void);
	void DBG_PRINT(char *fmt, ...);

#ifdef	__cplusplus
}
#endif

#endif	/* DEBUG_H */

