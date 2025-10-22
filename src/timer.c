#include "timer.h"
#include "sys.h"
#include "delay.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

crm_clocks_freq_type crm_clocks_freq_struct = {0};

void TIM3_10kHz_Init(uint32_t rate_hz)
{
    /* get system clock */
    crm_clocks_freq_get(&crm_clocks_freq_struct);
    /* enable TIM3 clock */
    crm_periph_clock_enable(CRM_TMR3_PERIPH_CLOCK, TRUE); 
    /* tmr3 configuration */
    if(rate_hz < 1) rate_hz = 1;
    if(rate_hz > 10000) rate_hz = 10000;
    uint32_t arr = 10000 / rate_hz - 1;
    tmr_base_init(TMR3, arr, (crm_clocks_freq_struct.apb1_freq / 10000) - 1);
    tmr_cnt_dir_set(TMR3, TMR_COUNT_UP); // count up

    tmr_interrupt_enable(TMR3, TMR_OVF_INT, TRUE);
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_irq_enable(TMR3_GLOBAL_IRQn, 2, 0);
    
    tmr_counter_enable(TMR3, TRUE); // enable TMR3
}

void  TMR3_GLOBAL_IRQHandler(void)
{
    if(tmr_flag_get(TMR3, TMR_OVF_FLAG) == SET) // overflow interrupt
    {
        tmr_flag_clear(TMR3, TMR_OVF_FLAG);
    }
}
