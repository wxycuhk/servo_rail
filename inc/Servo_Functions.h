#ifndef _SERVO_FUNCTIONS_H
#define _SERVO_FUNCTIONS_H

#include "stdint.h"

#define	SMS_STS_1M 0
#define	SMS_STS_0_5M 1
#define	SMS_STS_250K 2
#define	SMS_STS_128K 3
#define	SMS_STS_115200 4
#define	SMS_STS_76800	5
#define	SMS_STS_57600	6
#define	SMS_STS_38400	7

//�ڴ������
//-------EPROM(ֻ��)--------
#define SMS_STS_MODEL_L 3
#define SMS_STS_MODEL_H 4

//-------EPROM(��д)--------
#define SMS_STS_ID 5
#define SMS_STS_BAUD_RATE 6
#define SMS_STS_MIN_ANGLE_LIMIT_L 9
#define SMS_STS_MIN_ANGLE_LIMIT_H 10
#define SMS_STS_MAX_ANGLE_LIMIT_L 11
#define SMS_STS_MAX_ANGLE_LIMIT_H 12
#define MS_STS_PHASE_MOD 18
#define SMS_STS_CW_DEAD 26
#define SMS_STS_CCW_DEAD 27
#define SMS_STS_OFS_L 31
#define SMS_STS_OFS_H 32
#define SMS_STS_MODE 33

//-------SRAM(��д)--------
#define SMS_STS_TORQUE_ENABLE 40
#define SMS_STS_ACC 41
#define SMS_STS_GOAL_POSITION_L 42
#define SMS_STS_GOAL_POSITION_H 43
#define SMS_STS_GOAL_TIME_L 44
#define SMS_STS_GOAL_TIME_H 45
#define SMS_STS_GOAL_SPEED_L 46
#define SMS_STS_GOAL_SPEED_H 47
#define MAX_TORQUE_RANGE 48
#define SMS_STS_LOCK 55

//-------SRAM(ֻ��)--------
#define SMS_STS_PRESENT_POSITION_L 56
#define SMS_STS_PRESENT_POSITION_H 57
#define SMS_STS_PRESENT_SPEED_L 58
#define SMS_STS_PRESENT_SPEED_H 59
#define SMS_STS_PRESENT_LOAD_L 60
#define SMS_STS_PRESENT_LOAD_H 61
#define SMS_STS_PRESENT_VOLTAGE 62
#define SMS_STS_PRESENT_TEMPERATURE 63
#define SMS_STS_MOVING 66
#define SMS_STS_PRESENT_CURRENT_L 69
#define SMS_STS_PRESENT_CURRENT_H 70

int getErr(void);
int WritePosEx(uint8_t ID, int16_t Position, uint16_t Speed, uint8_t acc, uint8_t uart);
int RegWritePosEx(uint8_t ID, int16_t Position, uint16_t Speed, uint8_t acc, uint8_t uart);
void RegWriteAction(uint8_t uart);
void SyncWritePosEx(uint8_t ID[], uint8_t IDN, int16_t Position[], uint16_t Speed[], uint8_t acc[], uint8_t uart);
int WheelMode(uint8_t ID, uint8_t uart);
int WriteSpe(uint8_t ID, int16_t Speed, uint8_t acc, uint8_t uart);
int EnableTorque(uint8_t ID, uint8_t Enable, uint8_t uart);
int unLockEprom(uint8_t ID, uint8_t uart);
int LockEprom(uint8_t ID, uint8_t uart);
int CalibrationOfs(uint8_t ID, uint8_t uart);
int FeedBack(int ID, uint8_t uart);
int ReadPos(int ID, uint8_t uart);
int ReadSpeed(int ID, uint8_t uart);
int ReadLoad(int ID, uint8_t uart);
int ReadVoltage(int ID, uint8_t uart);
int ReadTemper(int ID, uint8_t uart);
int ReadMove(int ID, uint8_t uart);
int ReadCurrent(int ID, uint8_t uart);
int PWMMode(uint8_t ID, uint8_t uart);

#endif

