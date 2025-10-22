/*Currently only UR6 used for 30 servos*/
/*ur5 TR: PB3 PD2 MUX10*/
/*ur6 TR: PC6 PC7 MUX8*/
/*UR3 TR: PC4 PC5 MUX7*/
#ifndef PYTHON_PRINT_H
#define PYTHON_PRINT_H

#include "at32f423.h"
#include "stdio.h"
#include "delay.h"
#include "at32f423_usart.h"

#define DATA5_UART                       USART5
#define DATA5_UART_GPIO                  GPIOB
#define DATA5_UART_CRM_CLK               CRM_USART5_PERIPH_CLOCK
#define DATA5_UART_GPIO_CRM_CLK          CRM_GPIOB_PERIPH_CLOCK
#define DATA5_UART_TX_PIN                GPIO_PINS_3
#define DATA5_UART_TX_PIN_SOURCE         GPIO_PINS_SOURCE3
#define DATA5_UART_RX_PIN                GPIO_PINS_2
#define DATA5_UART_RX_PIN_SOURCE         GPIO_PINS_SOURCE2
#define DATA5_UART_PIN_MUX_NUM           GPIO_MUX_10

#define DATA6_UART                       USART6
#define DATA6_UART_GPIO                  GPIOC
#define DATA6_UART_CRM_CLK               CRM_USART6_PERIPH_CLOCK
#define DATA6_UART_GPIO_CRM_CLK          CRM_GPIOC_PERIPH_CLOCK
#define DATA6_UART_TX_PIN                GPIO_PINS_7
#define DATA6_UART_TX_PIN_SOURCE         GPIO_PINS_SOURCE7
#define DATA6_UART_RX_PIN                GPIO_PINS_6
#define DATA6_UART_RX_PIN_SOURCE         GPIO_PINS_SOURCE6
#define DATA6_UART_PIN_MUX_NUM           GPIO_MUX_8

#define DATA3_UART                       USART3
#define DATA3_UART_GPIO                  GPIOC
#define DATA3_UART_CRM_CLK               CRM_USART3_PERIPH_CLOCK
#define DATA3_UART_GPIO_CRM_CLK          CRM_GPIOC_PERIPH_CLOCK
#define DATA3_UART_TX_PIN                GPIO_PINS_5
#define DATA3_UART_TX_PIN_SOURCE         GPIO_PINS_SOURCE5
#define DATA3_UART_RX_PIN                GPIO_PINS_4
#define DATA3_UART_RX_PIN_SOURCE         GPIO_PINS_SOURCE4
#define DATA3_UART_PIN_MUX_NUM           GPIO_MUX_7

void DATA5_UART_Init(uint32_t baudrate);
void DATA6_UART_Init(uint32_t baudrate);
void DATA3_UART_Init(uint32_t baudrate);
void Uart_Flush(uint8_t uart);
int16_t Uart_Read(uint8_t uart);
void Uart_Send(uint8_t ur_num, uint8_t *data, uint16_t len);



#define USE_DATA3_

#endif // !PYTHON_PRINT_H
