#ifndef __DELAY_H
#define __DELAY_H

#include "sys.h"

void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 nus);
uint8_t delay_get_fac_us(void);

#endif
