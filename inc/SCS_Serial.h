#ifndef __SCS_SERIAL_H
#define __SCS_SERIAL_H
#include "stdint.h"

int readSCSTimeOut(unsigned char *nDat, int nLen, uint32_t TimeOut, uint8_t uart);
int readSCS(unsigned char *nDat, int nLen, uint8_t uart);
int writeSCS(uint8_t *nDat, int nLen, uint8_t uart);
int writeByteSCS(uint8_t nDat, uint8_t uart);
void rFlushSCS(uint8_t uart);
void wFlushSCS(uint8_t uart);
void nopDelay(void);

#endif // __SCS_SERIAL_H
