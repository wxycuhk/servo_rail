/*
 * SCS.c
 * SCS串行舵机协议程序
 * 日期: 2022.3.29
 * 作者: 
 */

 #include <stdlib.h>
 #include "INST.h"
 #include "SCS.h"
 #include "SCS_Serial.h"
 
 static uint8_t Level =1;//舵机返回等级
 static uint8_t End = 0;//处理器大小端结构
 //static uint8_t Error = 0;//舵机状态
 uint8_t syncReadRxPacketIndex;
 uint8_t syncReadRxPacketLen;
 uint8_t *syncReadRxPacket;
 uint8_t *syncReadRxBuff;
 uint16_t syncReadRxBuffLen;
 uint16_t syncReadRxBuffMax;
 uint32_t syncTimeOut;
 
 //1个16位数拆分为2个8位数
 //DataL为低位，DataH为高位
 void Host2SCS(uint8_t *DataL, uint8_t* DataH, int Data)
 {
     if(End){
         *DataL = (Data>>8);
         *DataH = (Data&0xff);
     }else{
         *DataH = (Data>>8);
         *DataL = (Data&0xff);
     }
 }
 
 //2个8位数组合为1个16位数
 //DataL为低位，DataH为高位
 int SCS2Host(uint8_t DataL, uint8_t DataH)
 {
     int Data;
     if(End){
         Data = DataL;
         Data<<=8;
         Data |= DataH;
     }else{
         Data = DataH;
         Data<<=8;
         Data |= DataL;
     }
     return Data;
 }
 
 void writeBuf(uint8_t ID, uint8_t MemAddr, uint8_t *nDat, uint8_t nLen, uint8_t Fun, uint8_t uart)
 {
     uint8_t i;
     uint8_t msgLen = 2;
     uint8_t bBuf[6];
     uint8_t CheckSum = 0;
     bBuf[0] = 0xff;
     bBuf[1] = 0xff;
     bBuf[2] = ID;
     bBuf[4] = Fun;
     if(nDat){
         msgLen += nLen + 1;
         bBuf[3] = msgLen;
         bBuf[5] = MemAddr;
         writeSCS(bBuf, 6, uart);
         
     }else{
         bBuf[3] = msgLen;
         writeSCS(bBuf, 5, uart);
     }
     CheckSum = ID + msgLen + Fun + MemAddr;
     if(nDat){
         for(i=0; i<nLen; i++){
             CheckSum += nDat[i];
         }
         writeSCS(nDat, nLen, uart);
     }
     CheckSum = ~CheckSum;
     writeSCS(&CheckSum, 1, uart);
 }
 
 //普通写指令
 //舵机ID，MemAddr内存表地址，写入数据，写入长度
 int genWrite(uint8_t ID, uint8_t MemAddr, uint8_t *nDat, uint8_t nLen, uint8_t uart)
 {
     rFlushSCS(uart);
     writeBuf(ID, MemAddr, nDat, nLen, INST_WRITE, uart);
     wFlushSCS(uart);
     return Ack(ID, uart);
 }
 
 //异步写指令
 //舵机ID，MemAddr内存表地址，写入数据，写入长度
 int regWrite(uint8_t ID, uint8_t MemAddr, uint8_t *nDat, uint8_t nLen, uint8_t uart)
 {
     rFlushSCS(uart);
     writeBuf(ID, MemAddr, nDat, nLen, INST_REG_WRITE, uart);
     wFlushSCS(uart);
     return Ack(ID, uart);
 }
 
 //异步写执行行
 int regAction(uint8_t ID, uint8_t uart)
 {
     rFlushSCS(uart);
     writeBuf(ID, 0, NULL, 0, INST_REG_ACTION, uart);
     wFlushSCS(uart);
     return Ack(ID, uart);
 }
 
 //同步写指令
 //舵机ID[]数组，IDN数组长度，MemAddr内存表地址，写入数据，写入长度
 void syncWrite(uint8_t ID[], uint8_t IDN, uint8_t MemAddr, uint8_t *nDat, uint8_t nLen, uint8_t uart)
 {
     uint8_t mesLen = ((nLen+1)*IDN+4);
     uint8_t Sum = 0;
     uint8_t bBuf[7];
     uint8_t i, j;
     
     bBuf[0] = 0xff;
     bBuf[1] = 0xff;
     bBuf[2] = 0xfe;
     bBuf[3] = mesLen;
     bBuf[4] = INST_SYNC_WRITE;
     bBuf[5] = MemAddr;
     bBuf[6] = nLen;
     
     rFlushSCS(uart);
     writeSCS(bBuf, 7, uart);
 
     Sum = 0xfe + mesLen + INST_SYNC_WRITE + MemAddr + nLen;
 
     for(i=0; i<IDN; i++){
         writeSCS(&ID[i], 1, uart);
         writeSCS(nDat+i*nLen, nLen, uart);
         Sum += ID[i];
         for(j=0; j<nLen; j++){
             Sum += nDat[i*nLen+j];
         }
     }
     Sum = ~Sum;
     writeSCS(&Sum, 1, uart);
     wFlushSCS(uart);
 }
 
 int writeByte(uint8_t ID, uint8_t MemAddr, uint8_t bDat, uint8_t uart)
 {
     rFlushSCS(uart);
     writeBuf(ID, MemAddr, &bDat, 1, INST_WRITE, uart);
     wFlushSCS(uart);
     return Ack(ID, uart);
 }
 
 int writeWord(uint8_t ID, uint8_t MemAddr, uint16_t wDat, uint8_t uart)
 {
     uint8_t buf[2];
     Host2SCS(buf+0, buf+1, wDat);
     rFlushSCS(uart);
     writeBuf(ID, MemAddr, buf, 2, INST_WRITE, uart);
     wFlushSCS(uart);
     return Ack(ID, uart);
 }
 
 //读指令
 //舵机ID，MemAddr内存表地址，返回数据nData，数据长度nLen
 int Read(uint8_t ID, uint8_t MemAddr, uint8_t *nData, uint8_t nLen, uint8_t uart)
 {
     int Size;
     uint8_t bBuf[4];
     uint8_t calSum;
     uint8_t i;
     rFlushSCS(uart);
     writeBuf(ID, MemAddr, &nLen, 1, INST_READ, uart);
     wFlushSCS(uart);
     if(!checkHead(uart)){
         return 0;
     }
     //Error = 0;
     if(readSCS(bBuf, 3, uart)!=3){
         return 0;
     }
     Size = readSCS(nData, nLen, uart);
     if(Size!=nLen){
         return 0;
     }
     if(readSCS(bBuf+3, 1, uart)!=1){
         return 0;
     }
     calSum = bBuf[0]+bBuf[1]+bBuf[2];
     for(i=0; i<Size; i++){
         calSum += nData[i];
     }
     calSum = ~calSum;
     if(calSum!=bBuf[3]){
         return 0;
     }
     //Error = bBuf[2];
     return Size;
 }
 
 //读1字节，超时返回-1
 int readByte(uint8_t ID, uint8_t MemAddr, uint8_t uart)
 {
     uint8_t bDat;
     int Size = Read(ID, MemAddr, &bDat, 1, uart);
     if(Size!=1){
         return -1;
     }else{
         return bDat;
     }
 }
 
 //读2字节，超时返回-1
 int readWord(uint8_t ID, uint8_t MemAddr, uint8_t uart)
 {	
     uint8_t nDat[2];
     int Size;
     uint16_t wDat;
     Size = Read(ID, MemAddr, nDat, 2, uart);
     if(Size!=2)
         return -1;
     wDat = SCS2Host(nDat[0], nDat[1]);
     return wDat;
 }
 
 //Ping指令，返回舵机ID，超时返回-1
 int	Ping(uint8_t ID, uint8_t uart)
 {
     uint8_t bBuf[4];
     uint8_t calSum;
     rFlushSCS(uart);
     writeBuf(ID, 0, NULL, 0, INST_PING, uart);
     wFlushSCS(uart);
     //Error = 0;
     if(!checkHead(uart)){
         return -1;
     }
     
     if(readSCS(bBuf, 4, uart)!=4){
         return -1;
     }
     if(bBuf[0]!=ID && ID!=0xfe){
         return -1;
     }
     if(bBuf[1]!=2){
         return -1;
     }
     calSum = ~(bBuf[0]+bBuf[1]+bBuf[2]);
     if(calSum!=bBuf[3]){
         return -1;			
     }
     //Error = bBuf[2];
     return bBuf[0];
 }
 
 int checkHead(uint8_t uart)
 {
     uint8_t bDat;
     uint8_t bBuf[2] = {0, 0};
     uint8_t Cnt = 0;
     while(1){
         if(!readSCS(&bDat, 1, uart)){
             return 0;
         }
         bBuf[1] = bBuf[0];
         bBuf[0] = bDat;
         if(bBuf[0]==0xff && bBuf[1]==0xff){
             break;
         }
         Cnt++;
         if(Cnt>10){
             return 0;
         }
     }
     return 1;
 }
 
 //指令应答
 int	Ack(uint8_t ID, uint8_t uart)
 {
     uint8_t bBuf[4];
     uint8_t calSum;
     //Error = 0;
     if(ID!=0xfe && Level){
         if(!checkHead(uart)){
             return 0;
         }
         if(readSCS(bBuf, 4, uart)!=4){
             return 0;
         }
         if(bBuf[0]!=ID){
             return 0;
         }
         if(bBuf[1]!=2){
             return 0;
         }
         calSum = ~(bBuf[0]+bBuf[1]+bBuf[2]);
         if(calSum!=bBuf[3]){
             return 0;			
         }
         //Error = bBuf[2];
     }
     return 1;
 }
 
 int	syncReadPacketTx(uint8_t ID[], uint8_t IDN, uint8_t MemAddr, uint8_t nLen, uint8_t uart)
 {
     uint8_t checkSum;
     uint8_t i;
     rFlushSCS(uart);
     syncReadRxPacketLen = nLen;
     checkSum = (4+0xfe)+IDN+MemAddr+nLen+INST_SYNC_READ;
     writeByteSCS(0xff, uart);
     writeByteSCS(0xff, uart);
     writeByteSCS(0xfe, uart);
     writeByteSCS(IDN+4, uart);
     writeByteSCS(INST_SYNC_READ, uart);
     writeByteSCS(MemAddr, uart);
     writeByteSCS(nLen, uart);
     for(i=0; i<IDN; i++){
         writeByteSCS(ID[i], uart);
         checkSum += ID[i];
     }
     checkSum = ~checkSum;
     writeByteSCS(checkSum, uart);
     wFlushSCS(uart);
     
     syncReadRxBuffLen = readSCSTimeOut(syncReadRxBuff, syncReadRxBuffMax, syncTimeOut, uart);
     return syncReadRxBuffLen;
 }
 
 void syncReadBegin(uint8_t IDN, uint8_t rxLen, uint32_t TimeOut)
 {
     syncReadRxBuffMax = IDN*(rxLen+6);
     syncReadRxBuff = malloc(syncReadRxBuffMax);
     syncTimeOut = TimeOut;
 }
 
 void syncReadEnd(void)
 {
     if(syncReadRxBuff){
         free(syncReadRxBuff);
         syncReadRxBuff = NULL;
     }
 }
 
 int syncReadPacketRx(uint8_t ID, uint8_t *nDat)
 {
     uint16_t syncReadRxBuffIndex = 0;
	 	 uint8_t i = 0; 
     syncReadRxPacket = nDat;
     syncReadRxPacketIndex = 0;

     while((syncReadRxBuffIndex+6+syncReadRxPacketLen)<=syncReadRxBuffLen){
         uint8_t bBuf[] = {0, 0, 0};
         uint8_t calSum = 0;
         while(syncReadRxBuffIndex<syncReadRxBuffLen){
             bBuf[0] = bBuf[1];
             bBuf[1] = bBuf[2];
             bBuf[2] = syncReadRxBuff[syncReadRxBuffIndex++];
             if(bBuf[0]==0xff && bBuf[1]==0xff && bBuf[2]!=0xff){
                 break;
             }
         }
         if(bBuf[2]!=ID){
             continue;
         }
         if(syncReadRxBuff[syncReadRxBuffIndex++]!=(syncReadRxPacketLen+2)){
             continue;
         }
         //Error = syncReadRxBuff[syncReadRxBuffIndex++];
         calSum = ID+(syncReadRxPacketLen+2)+syncReadRxBuff[syncReadRxBuffIndex++];
         for(i=0; i<syncReadRxPacketLen; i++){
             syncReadRxPacket[i] = syncReadRxBuff[syncReadRxBuffIndex++];
             calSum += syncReadRxPacket[i];
         }
         calSum = ~calSum;
         if(calSum!=syncReadRxBuff[syncReadRxBuffIndex++]){
             return 0;
         }
         return syncReadRxPacketLen;
     }
     return 0;
 }
 
 int syncReadRxPacketToByte(void)
 {
     if(syncReadRxPacketIndex>=syncReadRxPacketLen){
         return -1;
     }
     return syncReadRxPacket[syncReadRxPacketIndex++];
 }
 
 int syncReadRxPacketToWord(uint8_t negBit)
 {
     if((syncReadRxPacketIndex+1)>=syncReadRxPacketLen){
         return -1;
     }
     int Word = SCS2Host(syncReadRxPacket[syncReadRxPacketIndex], syncReadRxPacket[syncReadRxPacketIndex+1]);
     syncReadRxPacketIndex += 2;
     if(negBit){
         if(Word&(1<<negBit)){
             Word = -(Word & ~(1<<negBit));
         }
     }
     return Word;
 }
