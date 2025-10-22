#ifndef _SERVO_CONTROL_H
#define _SERVO_CONTROL_H
#include "stdint.h"

void Servo_MovePos(uint8_t ID, int16_t Position, uint16_t Speed, uint8_t acc, uint8_t uart);
void Multiple_Servo_MovePos(uint8_t* ID, int16_t Position[], uint16_t Speed[], uint8_t acc[], uint8_t uart);

#endif // _SERVO_CONTROL_H
