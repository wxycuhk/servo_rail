/*
 * SCServo.c
 * 飞特舵机硬件接口层程序
 * 日期: 2022.3.29
 * 作者: 
 */

 #include "at32f423.h"
 #include "python_print.h"
 #include "SCS_Serial.h"
 
 uint32_t IOTimeOut = 5000;//输入输出超时
 uint8_t wBuf[3][128];
 uint8_t wLen[3] = {0, 0, 0};
 
 int readSCSTimeOut(unsigned char *nDat, int nLen, uint32_t TimeOut, uint8_t uart)
 {
     int Size = 0;
     int ComData;
     uint32_t t_user = 0;
     while(1){
         ComData = Uart_Read(uart);
         if(ComData!=-1){
             if(nDat){
                 nDat[Size] = ComData;
             }
             Size++;
         }
         if(Size>=nLen){
             break;
         }
         t_user++;
         if(t_user>TimeOut){
             break;
         }
     }
     return Size;
 }
 
 //UART 接收数据接口
 int readSCS(unsigned char *nDat, int nLen, uint8_t uart)
 {
     int Size = 0;
     int ComData;
     uint32_t t_user = 0;
     while(1){
         ComData = Uart_Read(uart);
         if(ComData!=-1){
             if(nDat){
                 nDat[Size] = ComData;
             }
             Size++;
             t_user = 0;
         }
         if(Size>=nLen){
             break;
         }
         t_user++;
         if(t_user>IOTimeOut){
             break;
         }
     }
     return Size;
 }

 
int writeByteSCS(unsigned char bDat, uint8_t uart)
{
    if (uart < 5 || uart > 7) {
        return -1; // Invalid UART range
    }

    if (wLen[uart - 5] < sizeof(wBuf[uart - 5])) {
        wBuf[uart - 5][wLen[uart - 5]] = bDat;
        wLen[uart - 5]++;
    }
    return wLen[uart - 5];
}
 
//UART 发送数据接口
int writeSCS(unsigned char *nDat, int nLen, uint8_t uart)
{
    if (uart < 5 || uart > 7) {
        return -1; // Invalid UART range
    }

    while (nLen--) {
        if (wLen[uart - 5] < sizeof(wBuf[uart - 5])) {
            wBuf[uart - 5][wLen[uart - 5]] = *nDat;
            wLen[uart - 5]++;
            nDat++;
        }
    }
    return wLen[uart - 5];
}
 //等待舵机总线切换(约20us)
 void nopDelay(void)
 {
     uint16_t i = 300;
     while(i--);
 }
 
 //接收缓冲区刷新
 void rFlushSCS(uint8_t uart)
 {
    nopDelay();
    Uart_Flush(uart);
 }
 
 //发送缓冲区刷新
void wFlushSCS(uint8_t uart)
{
	  uint8_t index = uart - 5;
    if (uart < 5 || uart > 7) {
        return; // Invalid UART range
    }
    if (wLen[index] > 0) {
        Uart_Send(uart, wBuf[index], wLen[index]);
        wLen[index] = 0;
    }
}
