#ifndef __TIMER_H
#define __TIMER_H

#include "at32f423.h"
#include "stdio.h"

void TIM3_10kHz_Init(uint32_t rate_hz);
void  TMR3_GLOBAL_IRQHandler(void);

#endif
