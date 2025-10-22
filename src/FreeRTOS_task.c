#include "python_print.h"
#include "at32f423_clock.h"
#include "freertos.h"
#include "task.h"
#include "FreeRTOS_task.h"
#include "usb_command_decoder.h" 
#include "usb_print.h"           
#include "delay.h"              
#include "semphr.h"
#include "Servo_Functions.h" 
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


/******************************************************************************************************/
extern volatile uint16_t data_len;
extern volatile uint8_t usb_buffer[256];
extern volatile uint8_t servo_num;
extern volatile uint8_t ID_list[MAX_SERVO_NUM];
extern volatile int16_t init_pos[MAX_SERVO_NUM];

static void trim(char *s) // as fifo involved, python do not need to send \r\n actually
{
    int n = strlen(s);
    while(n&& s[n-1] == '\n') s[--n] = 0; // remove trailing \n
}


static uint8_t read_int_list(int32_t *dst, uint8_t max_cnt)
{
    char *t;
    uint8_t n = 0;
		t = strtok(NULL,",");
    while(t && n < max_cnt){
				t = strtok(NULL,",");
        dst[n++] = strtol(t,NULL,0);
		}
    return n;
}
/*FreeRTOS配置*/

/* START_TASK 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
 
 
 // 定义任务优先级和栈大小
#define USB_TASK_PRIO   2
#define USB_STK_SIZE    128
#define SERVO_TASK_PRIO 1
#define SERVO_STK_SIZE  128
#define SERVO_TASK_PRIO_HIGH 4

// 定义任务句柄
TaskHandle_t UsbTask_Handler;
TaskHandle_t ServoTask_Handler;

// 定义信号量句柄
SemaphoreHandle_t xServoMoveSemaphore;

// 标志位，指示是否需要舵机移动
volatile uint8_t servo_move_pending = 0;

// 函数声明
void usb_task(void *pvParameters);
void servo_task(void *pvParameters);

// 初始化 FreeRTOS 任务
void init_freeRTOS_tasks(void)
{
    // 创建二进制信号量
    xServoMoveSemaphore = xSemaphoreCreateBinary();

    // 创建 USB 任务
    xTaskCreate(usb_task, "usb_task", USB_STK_SIZE, NULL, USB_TASK_PRIO, &UsbTask_Handler);
    
    // 创建舵机任务
    xTaskCreate(servo_task, "servo_task", SERVO_STK_SIZE, NULL, SERVO_TASK_PRIO, &ServoTask_Handler);
}

// USB 任务
void usb_task(void *pvParameters)
{
		char* sub = NULL;	
		uint8_t id = 2;
    if(data_len == 0) return; // no data to parse
    usb_buffer[data_len] = 0; // null terminate the string
    char *p = (char *)usb_buffer;
    trim(p); // remove trailing \n
    while(1)
    {
        // 接收 USB 数据
        usb_recv_data();
        
        // 解析舵机命令
				char *tok = strtok(p, ","); // get the first token
				if(!tok){data_len=0; return;} // no command found
				
				switch(toupper(tok[0])) // check the first character of the command
				{
						case 'P':
								for(int i = 0; i < servo_num; ++i) // id 2 to 8
								{
										id = ID_list[i];
										RegWritePosEx(id, init_pos[id-2], 32766, 0, 7 );
								}
								// RegWriteAction(7); // send action command to servo
								vTaskDelay(100); // wait for the action to complete
								usb_printf("Servo resetting done\r\n");
								break;
						case 'K':
						{
								int id = atoi(strtok(NULL,",")); // get the servo id
								int pos = atoi(strtok(NULL,",")); // get the position
								int speed = atoi(strtok(NULL,",")); // get the speed
								int acc = atoi(strtok(NULL,",")); // get the acceleration

								RegWritePosEx(id, pos, speed, acc, 7); // set the servo position
								// RegWriteAction(7); // send action command to servo
								usb_printf("Servo %d moving to %d with speed %d and acc %d\r\n", id, pos, speed, acc);
								break;
						}
						case 'U':
								for(int i = 0; i < servo_num; ++i) // id 2 to 8
								{
										id = ID_list[i];
										RegWritePosEx(id, init_pos[i] - 400, 8000, 0, 7 );
								}
								// RegWriteAction(7); // send action command to servo
								delay_ms(100); // wait for the action to complete
								usb_printf("Servo moving to %d\r\n", init_pos[0] - 400);
								break;
						case 'S':
								sub = strtok(NULL,",");
								if(!sub){usb_printf("ERROR MESSAGE: NO PARA COMMAND\r\n"); }
								if(!strcasecmp(sub, "num"))
								{
										int n = atoi(strtok(NULL, ","));
										servo_num = (uint8_t)n;
									usb_printf("ok, s:num = %d\r\n", servo_num);
								}
								else if(!strcasecmp(sub, "id"))
								{
										int32_t buf[MAX_SERVO_NUM];
										uint8_t n = read_int_list(buf, MAX_SERVO_NUM);
										if(n==0){ usb_printf("ERR S:ID empty\n"); break; }
										servo_num = n;
										for(uint8_t i=0;i<n;++i) ID_list[i]=(uint8_t)buf[i];
										usb_printf("OK S:ID set (%d)\r\n", servo_num);
								}
								else if(!strcasecmp(sub,"pos"))
								{
										int32_t buf[MAX_SERVO_NUM];
										uint8_t n = read_int_list(buf, MAX_SERVO_NUM);
										if(n==0){ usb_printf("ERR S:POS empty\n"); break; }
										servo_num = n;                          /* 按给定数量更新 */
										for(uint8_t i=0;i<n;++i) init_pos[i]=(int16_t)buf[i];
										usb_printf("OK S:POS set (%d)\r\n", servo_num);
								}
								else if(!strcasecmp(sub,"get"))
								{
										usb_printf("CFG num=%d\r\n", servo_num);
										usb_printf("CFG id=");
										for(uint8_t i=0;i<servo_num;++i) usb_printf("%d,", ID_list[i]);
										usb_printf("\r\nCFG pos=");
										for(uint8_t i=0;i<servo_num;++i) usb_printf("%d,", init_pos[i]);
										usb_printf("\r\n");
								}
								else
									usb_printf("Unknown setting command, please re-send\r\n");
						default:
								usb_printf("Unknown command: %s\r\n", p); // unknown command
								break;
				}
				servo_move_pending = 1;
				data_len = 0;

        // 如果有舵机运动请求
        if (servo_move_pending)
        {
            // 给舵机任务发送信号
            xSemaphoreGive(xServoMoveSemaphore);
            // 提升舵机任务优先级，确保其尽快执行
            vTaskPrioritySet(ServoTask_Handler, SERVO_TASK_PRIO_HIGH);
            // 重置标志位
            servo_move_pending = 0;
        }
        
        // 延时，避免占用 CPU 资源
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

// 舵机任务
void servo_task(void *pvParameters)
{
    while(1)
    {
        // 等待信号量，直到有舵机运动请求
        if(xSemaphoreTake(xServoMoveSemaphore, portMAX_DELAY) == pdTRUE)
        {
            // 调用舵机控制函数，执行运动 
            RegWriteAction(7);
            // 恢复舵机任务的优先级
            vTaskPrioritySet(ServoTask_Handler, SERVO_TASK_PRIO);
        }
    }
}
