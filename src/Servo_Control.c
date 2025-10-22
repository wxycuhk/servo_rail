#include "Servo_Functions.h"
#include "SCS.h"
#include "INST.h"
#include "SCS_Serial.h"
#include "stdint.h"
#include "stdio.h"
#include "Servo_Control.h"

/* Here using RegWritePosEx as it can send moving command to any servo any time*/
/* Using Feedback as adjustment, add a timer interruption to read, better using RTOS*/
void Servo_MovePos(uint8_t ID, int16_t Position, uint16_t Speed, uint8_t acc, uint8_t uart)
{
    RegWritePosEx(ID, Position, Speed, acc, uart);
    RegWriteAction(uart);
}

void Multiple_Servo_MovePos(uint8_t* ID, int16_t Position[], uint16_t Speed[], uint8_t acc[], uint8_t uart)
{
    uint8_t i;
    //uint16_t V;
		size_t IDN = sizeof(ID)/sizeof(ID[0]);
    for(i = 0; i<IDN; i++)
    {
        RegWritePosEx(ID[i], Position[i], Speed[i], acc[i], uart);
    }
    RegWriteAction(uart);
}
