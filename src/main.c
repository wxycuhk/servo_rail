/* Note: �˴���ֻ���ڵ��ԣ�����ṹ�����Ż� */
// Debug 2024/08/27: UR5 TX PB3 ���� mux10, rx PD2���� mux8, ����������������ΪPB4(ͬ����mux10���ɸ��� Mux7��Tx�л�������1)
// Debug 2024/09/04: �ⲿ���پ���Ƶ��Ϊ12MHz, ����at32f423_conf��hext��12000000
// Debug 2025/03/17: �ⲿʱ�Ӹ��ģ�Ϊ��֤ϵͳʱ��Ϊ144Mhz, system clock (sclk)   = (hext * pll_ns)/(pll_ms * pll_fr) / 2, ��Ӧ����pll_ns
// Debug 2024/10/18: ʹ���������ٷ����壬 ��ȡ������������ɸð�ADC���ߴ�������
// Debug 2025/01/06: add FreeRTOS system modified from openedv demo code
// For FreeRTOS System, adjust the priority of shift reg to highest, when task 2: ADC_Read and task 3: USART_printf in delay, enter the time counting of 74hc164 clock_high
// improve the sampling rate, if use FreeRTOS, use FreeRTOS_demo() in main loop only

// Debug 2025/02/10: Add USB Code

#include "python_print.h"
#include "at32f423_clock.h"
#include "stdio.h"
#include "delay.h"
#include "sys.h"
#include "freertos.h"
#include "task.h"
#include "USB_print.h"
#include "usb_conf.h"
#include "usb_core.h"
// #include "FreeRTOS_task.h"
#include "Servo_Control.h"
#include "Servo_Functions.h"
#include "SCS.h"
#include "usb_command_decoder.h"

//#define MS_TICK  (system_core_clock / 1000U)

// UR5 TX: PB3      RX: PD2
/**************** define print uart ******************/
/*
#define PRINT_UART                       USART5
#define PRINT_UART_CRM_CLK               CRM_USART5_PERIPH_CLOCK
#define PRINT_UART_TX_PIN                GPIO_PINS_3
#define PRINT_UART_TX_GPIO               GPIOB
#define PRINT_UART_TX_GPIO_CRM_CLK       CRM_GPIOB_PERIPH_CLOCK
#define PRINT_UART_TX_PIN_SOURCE         GPIO_PINS_SOURCE3
#define PRINT_UART_TX_PIN_MUX_NUM        GPIO_MUX_10
*/

int usb_div_flag = 0;
uint8_t i = 0;
int ur3_num = 7;
int ur5_num = 5;
int ur6_num = 6;
int ID_List[MAX_SERVO_NUM] = {0};
int motor_online_flag = 0;
extern volatile uint16_t data_len;
extern volatile uint8_t usb_buffer[512];
extern uint8_t servo_num;
unsigned char mode = 'p';


int main(void)
{
  system_clock_config();
	delay_init();
	
	#ifdef USE_DATA5_
		DATA5_UART_Init(1000000);
	#elif defined(USE_DATA6_)
		DATA6_UART_Init(1000000);
	#elif defined(USE_DATA3_)
		DATA3_UART_Init(1000000);
	#endif
	
	delay_ms(500);
	usb_init();
	//freertos_demo(); // if using FreeRTOS function, use this function only and comment all includings in while(1) afterwards
			/* Check motor ID */
	/*
	while(motor_online_flag < 1) // 线程卡死在舵机搜索上
	{
		for(int i = 1; i < servo_num; i++)
		{
			ID_List[i - 1] = Ping(i, ur3_num);
			if(ID_List[i - 1] != -1){
				usb_printf("Current motor ID is: %d\r\n", ID_List[i - 1]);
				motor_online_flag++;
			}
			else 
				usb_printf("Invalid motor, do not exist\r\n"); // 打印掉线舵机ID
		}
		usb_printf("Some of the motors offline\r\n");
	}
	*/
  while(1)
  {
		usb_recv_data();
		usb_parse_servo_command();
  }
}


