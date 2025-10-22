#include "SCS_Serial.h"
#include "SCS.h"
#include "INST.h"
#include "python_print.h"
#include "string.h"
#include "Servo_Functions.h"
#include "stdint.h"

static uint8_t Mem[SMS_STS_PRESENT_CURRENT_H-SMS_STS_PRESENT_POSITION_L+1];
static int Err = 0;


int getErr(void)
{
	return Err;
}

int WritePosEx(uint8_t ID, int16_t Position, uint16_t Speed, uint8_t acc, uint8_t uart)
{
	uint8_t bBuf[7];
	if(Position<0){
		Position = -Position;
		Position |= (1<<15);
	}

	bBuf[0] = acc;
	Host2SCS(bBuf+1, bBuf+2, Position);
	Host2SCS(bBuf+3, bBuf+4, 0);
	Host2SCS(bBuf+5, bBuf+6, Speed);
	
	return genWrite(ID, SMS_STS_ACC, bBuf, 7, uart);
}

int RegWritePosEx(uint8_t ID, int16_t Position, uint16_t Speed, uint8_t acc, uint8_t uart)
{
	uint8_t bBuf[7];
	if(Position<0){
		Position = -Position;
		Position |= (1<<15);
	}

	bBuf[0] = acc;
	Host2SCS(bBuf+1, bBuf+2, Position);
	Host2SCS(bBuf+3, bBuf+4, 0);
	Host2SCS(bBuf+5, bBuf+6, Speed);
	
	return regWrite(ID, SMS_STS_ACC, bBuf, 7, uart);
}

void RegWriteAction(uint8_t uart)
{
	regAction(0xfe, uart);
}

void SyncWritePosEx(uint8_t ID[], uint8_t IDN, int16_t Position[], uint16_t Speed[], uint8_t acc[], uint8_t uart)
{
	uint8_t offbuf[32*7];
	uint8_t i;
	uint16_t V;
  for(i = 0; i<IDN; i++){
		if(Position[i]<0){
			Position[i] = -Position[i];
			Position[i] |= (1<<15);
		}

		if(Speed){
			V = Speed[i];
		}else{
			V = 0;
		}
		if(acc){
			offbuf[i*7] = acc[i];
		}else{
			offbuf[i*7] = 0;
		}
		Host2SCS(offbuf+i*7+1, offbuf+i*7+2, Position[i]);
    Host2SCS(offbuf+i*7+3, offbuf+i*7+4, 0);
    Host2SCS(offbuf+i*7+5, offbuf+i*7+6, V);
	}
  syncWrite(ID, IDN, SMS_STS_ACC, offbuf, 7, uart);
}

int WheelMode(uint8_t ID, uint8_t uart)
{
	return writeByte(ID, SMS_STS_MODE, 1, uart);		
}

int PWMMode(uint8_t ID, uint8_t uart)
{
	return writeByte(ID, SMS_STS_MODE, 2, uart);		
}

int WriteSpe(uint8_t ID, int16_t Speed, uint8_t acc, uint8_t uart)
{
	uint8_t bBuf[2];
	if(Speed<0){
		Speed = -Speed;
		Speed |= (1<<15);
	}
	bBuf[0] = acc;
	genWrite(ID, SMS_STS_ACC, bBuf, 1, uart);
	
	Host2SCS(bBuf+0, bBuf+1, Speed);

	genWrite(ID, SMS_STS_GOAL_SPEED_L, bBuf, 2, uart);
	return 1;
}

int EnableTorque(uint8_t ID, uint8_t Enable, uint8_t uart)
{
	return writeByte(ID, SMS_STS_TORQUE_ENABLE, Enable, uart);
}

int unLockEprom(uint8_t ID, uint8_t uart)
{
	return writeByte(ID, SMS_STS_LOCK, 0, uart);
}

int LockEprom(uint8_t ID, uint8_t uart)
{
	return writeByte(ID, SMS_STS_LOCK, 1, uart);
}

int CalibrationOfs(uint8_t ID, uint8_t uart)
{
	return writeByte(ID, SMS_STS_TORQUE_ENABLE, 128, uart);
}

int FeedBack(int ID, uint8_t uart)
{
	int nLen = Read(ID, SMS_STS_PRESENT_POSITION_L, Mem, sizeof(Mem), uart);
	if(nLen!=sizeof(Mem)){
		Err = 1;
		return -1;
	}
	Err = 0;
	return nLen;
}

int ReadPos(int ID, uint8_t uart)
{
	int Pos = -1;
	if(ID==-1){
		Pos = Mem[SMS_STS_PRESENT_POSITION_H-SMS_STS_PRESENT_POSITION_L];
		Pos <<= 8;
		Pos |= Mem[SMS_STS_PRESENT_POSITION_L-SMS_STS_PRESENT_POSITION_L];
	}else{
		Err = 0;
		Pos = readWord(ID, SMS_STS_PRESENT_POSITION_L, uart);
		if(Pos==-1){
			Err = 1;
		}
	}
	if(!Err && Pos&(1<<15)){
		Pos = -(Pos&~(1<<15));
	}	
	return Pos;
}

int ReadSpeed(int ID, uint8_t uart)
{
	int Speed = -1;
	if(ID==-1){
		Speed = Mem[SMS_STS_PRESENT_SPEED_H-SMS_STS_PRESENT_POSITION_L];
		Speed <<= 8;
		Speed |= Mem[SMS_STS_PRESENT_SPEED_L-SMS_STS_PRESENT_POSITION_L];
	}else{
		Err = 0;
		Speed = readWord(ID, SMS_STS_PRESENT_SPEED_L, uart);
		if(Speed==-1){
			Err = 1;
			return -1;
		}
	}
	if(!Err && Speed&(1<<15)){
		Speed = -(Speed&~(1<<15));
	}	
	return Speed;
}

int ReadLoad(int ID, uint8_t uart)
{
	int Load = -1;
	if(ID==-1){
		Load = Mem[SMS_STS_PRESENT_LOAD_H-SMS_STS_PRESENT_POSITION_L];
		Load <<= 8;
		Load |= Mem[SMS_STS_PRESENT_LOAD_L-SMS_STS_PRESENT_POSITION_L];
	}else{
		Err = 0;
		Load = readWord(ID, SMS_STS_PRESENT_LOAD_L, uart);
		if(Load==-1){
			Err = 1;
		}
	}
	if(!Err && Load&(1<<10)){
		Load = -(Load&~(1<<10));
	}	
	return Load;
}

int ReadVoltage(int ID, uint8_t uart)
{
	int Voltage = -1;
	if(ID==-1){
		Voltage = Mem[SMS_STS_PRESENT_VOLTAGE-SMS_STS_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Voltage = readByte(ID, SMS_STS_PRESENT_VOLTAGE, uart);
		if(Voltage==-1){
			Err = 1;
		}
	}
	return Voltage;
}

int ReadTemper(int ID, uint8_t uart)
{
	int Temper = -1;
	if(ID==-1){
		Temper = Mem[SMS_STS_PRESENT_TEMPERATURE-SMS_STS_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Temper = readByte(ID, SMS_STS_PRESENT_TEMPERATURE, uart);
		if(Temper==-1){
			Err = 1;
		}
	}
	return Temper;
}

int ReadMove(int ID, uint8_t uart)
{
	int Move = -1;
	if(ID==-1){
		Move = Mem[SMS_STS_MOVING-SMS_STS_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Move = readByte(ID, SMS_STS_MOVING, uart);
		if(Move==-1){
			Err = 1;
		}
	}
	return Move;
}

int ReadCurrent(int ID, uint8_t uart)
{
	int Current = -1;
	if(ID==-1){
		Current = Mem[SMS_STS_PRESENT_CURRENT_H-SMS_STS_PRESENT_POSITION_L];
		Current <<= 8;
		Current |= Mem[SMS_STS_PRESENT_CURRENT_L-SMS_STS_PRESENT_POSITION_L];
	}else{
		Err = 0;
		Current = readWord(ID, SMS_STS_PRESENT_CURRENT_L, uart);
		if(Current==-1){
			Err = 1;
			return -1;
		}
	}
	if(!Err && Current&(1<<15)){
		Current = -(Current&~(1<<15));
	}	
	return Current;
}
