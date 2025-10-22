#ifndef _SCS_H
#define _SCS_H

#include <stdint.h>
#include "SCS_Serial.h"
#include "INST.h"

void Host2SCS(uint8_t *DataL, uint8_t* DataH, int Data);
int SCS2Host(uint8_t DataL, uint8_t DataH);
void writeBuf(uint8_t ID, uint8_t MemAddr, uint8_t *nDat, uint8_t nLen, uint8_t Fun, uint8_t uart);
int Read(uint8_t ID, uint8_t MemAddr, uint8_t *nData, uint8_t nLen, uint8_t uart);
int readByte(uint8_t ID, uint8_t MemAddr, uint8_t uart);
int readWord(uint8_t ID, uint8_t MemAddr, uint8_t uart);
int writeByte(uint8_t ID, uint8_t MemAddr, uint8_t bDat, uint8_t uart);
int writeWord(uint8_t ID, uint8_t MemAddr, uint16_t wDat, uint8_t uart);
int genWrite(uint8_t ID, uint8_t MemAddr, uint8_t *nDat, uint8_t nLen, uint8_t uart);
int regWrite(uint8_t ID, uint8_t MemAddr, uint8_t *nDat, uint8_t nLen, uint8_t uart);
int regAction(uint8_t ID, uint8_t uart);
void syncWrite(uint8_t ID[], uint8_t IDN, uint8_t MemAddr, uint8_t *nDat, uint8_t nLen, uint8_t uart);
int Ping(uint8_t ID, uint8_t uart);
int Ack(uint8_t ID, uint8_t uart);
int checkHead(uint8_t uart);
int syncReadPacketTx(uint8_t ID[], uint8_t IDN, uint8_t MemAddr, uint8_t nLen, uint8_t uart);
void SyncReadBegin(uint8_t IDN, uint8_t rxlen, uint32_t Timeout);
void syncReadEnd(void);
int syncReadPacketRx(uint8_t ID, uint8_t *nDat);
int syncReadRxPacketToByte(void);
int syncReadRxPacketToWord(uint8_t negBit);

#endif // _SCS_H
