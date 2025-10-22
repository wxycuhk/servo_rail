#include "delay.h"

/* 由正点原子delay.c移植, 删除ucos相关内容*/
/* 由正点原子delay */
/* clock source --> system core clock， PLL enable， system_core_clock = 144MHz， 正确性已验证*/
static u8  fac_us=0;							//us延时倍乘数
static u16 fac_ms=0;							//ms延时倍乘数

//初始化延迟函数
//当使用SysTick的时候,此函数会初始化SysTick的时钟,并设置SysTick的中断优先级
//当使用TIM的时候,此函数会初始化TIM的时钟
//SYSCLK:系统时钟频率
void delay_init(void)
{
    systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_DIV8);	//选择外部时钟  HCLK/8
    fac_us=system_core_clock/(8000000U);									//为系统时钟的1/8
    fac_ms=(u16)fac_us*1000U;							//非常重要,在ucos下,不能缺少
}
void delay_us(u32 nus)
{		
    u32 temp;	    	 
    SysTick->LOAD=nus*fac_us; 					//时间加载
    SysTick->VAL=0x00;        					//清空计数器
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          //开始倒数
    do
    {
        temp=SysTick->CTRL;
    }while((temp&0x01)&&!(temp&(1<<16)));	//等待时间到达
    SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       //关闭计数器
    SysTick->VAL =0X00;       //清空计数器
}
void delay_ms(u16 nms)
{	 		  	  
    u32 temp;		   
    SysTick->LOAD=(u32)nms*fac_ms;				//时间加载(SysTick->LOAD为24bit)
    SysTick->VAL =0x00;            //清空计数器
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          //开始倒数
    do
    {
        temp=SysTick->CTRL;
    }while((temp&0x01)&&!(temp&(1<<16)));	//等待时间到达
    SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       //关闭计数器
    SysTick->VAL =0X00;       //清空计数器
}
uint8_t delay_get_fac_us(void)
{
	return fac_us;
}
