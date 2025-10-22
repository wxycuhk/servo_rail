// 2025/02/20 Modified from Feetech STM32F4xx example code, add usart5 and usart6
// 傻叉代码再改是
#include "at32f423_clock.h"
#include "stdio.h"
#include "delay.h"
#include "python_print.h"

// UART read data buffers and indices for USART5, USART6, and usart3
__IO uint8_t uartBuf[3][128];
__IO int head[3] = {0, 0, 0}, tail[3] = {0, 0, 0};

void Uart_Flush(uint8_t uart)
{
  if (uart >= 5 && uart <= 7)
  {
    head[uart - 5] = tail[uart - 5] = 0;
  }
}

int16_t Uart_Read(uint8_t uart)
{
  if (uart >= 5 && uart <= 7)
  {
    int index = uart - 5;
    if (head[index] != tail[index])
    {
      uint8_t data = uartBuf[index][head[index]];
      head[index] = (head[index] + 1) % 128;
      return data;
    }
  }
  return -1;
}

#ifdef USE_DATA5_
void DATA5_UART_Init(uint32_t baudrate)
{
  gpio_init_type gpio_init_struct;

  /* enable the usart5 & gpio clock */
  crm_periph_clock_enable(DATA5_UART_CRM_CLK, TRUE);
  crm_periph_clock_enable(DATA5_UART_GPIO_CRM_CLK, TRUE);

  gpio_default_para_init(&gpio_init_struct);
  /* configure the usart5 tx, rx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = DATA5_UART_TX_PIN | DATA5_UART_RX_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(DATA5_UART_GPIO, &gpio_init_struct);
  gpio_pin_mux_config(DATA5_UART_GPIO, DATA5_UART_TX_PIN_SOURCE, DATA5_UART_PIN_MUX_NUM);
  gpio_pin_mux_config(DATA5_UART_GPIO, DATA5_UART_RX_PIN_SOURCE, DATA5_UART_PIN_MUX_NUM);

  /* configure usart param */
  usart_init(DATA5_UART, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_transmitter_enable(DATA5_UART, TRUE); // enable tx
  usart_receiver_enable(DATA5_UART, TRUE); // enable rx
  usart_parity_config(DATA5_UART, USART_PARITY_NONE); // no parity  

  /* enable usart5 interrupt rxne*/
  usart_interrupt_enable(DATA5_UART, USART_RDBF_INT, TRUE);
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_0);
  nvic_irq_enable(USART5_IRQn, 0, 0);

  usart_enable(DATA5_UART, TRUE);
}
void USART5_IRQHandler(void)
{
  if (usart_flag_get(DATA5_UART, USART_RDBF_FLAG) != RESET) // Check if data is received
  {
    int index = 0; // USART5 corresponds to index 0 in uartBuf, head, and tail
    uartBuf[index][tail[index]] = usart_data_receive(DATA5_UART);
    tail[index] = (tail[index] + 1) % 128;
  }
}
#endif

#ifdef USE_DATA6_
void DATA6_UART_Init(uint32_t baudrate)
{
  gpio_init_type gpio_init_struct;

  /* enable the usart6 & gpio clock */
  crm_periph_clock_enable(DATA6_UART_CRM_CLK, TRUE);
  crm_periph_clock_enable(DATA6_UART_GPIO_CRM_CLK, TRUE);

  /* configure the usart6 tx, rx pin */
  gpio_default_para_init(&gpio_init_struct);
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = DATA6_UART_TX_PIN | DATA6_UART_RX_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(DATA6_UART_GPIO, &gpio_init_struct);
  gpio_pin_mux_config(DATA6_UART_GPIO, DATA6_UART_TX_PIN_SOURCE, DATA6_UART_PIN_MUX_NUM);
  gpio_pin_mux_config(DATA6_UART_GPIO, DATA6_UART_RX_PIN_SOURCE, DATA6_UART_PIN_MUX_NUM);

  /* configure usart param */
  usart_init(DATA6_UART, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_transmitter_enable(DATA6_UART, TRUE); // enable tx
  usart_receiver_enable(DATA6_UART, TRUE); // enable rx
  usart_parity_selection_config(DATA6_UART, USART_PARITY_NONE); // no parity

  /* enable usart6 interrupt rxne*/
  usart_interrupt_enable(DATA6_UART, USART_RDBF_INT, TRUE);
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_0);
  nvic_irq_enable(USART6_IRQn, 0, 0);

  usart_enable(DATA6_UART, TRUE);
}
void USART6_IRQHandler(void)
{
  if (usart_flag_get(DATA6_UART, USART_RDBF_FLAG) != RESET) // Check if data is received
  {
    int index = 1; // USART6 corresponds to index 1 in uartBuf, head, and tail
    uartBuf[index][tail[index]] = usart_data_receive(DATA6_UART);
    tail[index] = (tail[index] + 1) % 128;
  }
}
#endif
#ifdef USE_DATA3_
void DATA3_UART_Init(uint32_t baudrate)
{
  gpio_init_type gpio_init_struct;

  /* enable the usart3 & gpio clock */
  crm_periph_clock_enable(DATA3_UART_CRM_CLK, TRUE);
  crm_periph_clock_enable(DATA3_UART_GPIO_CRM_CLK, TRUE);

  /* configure the usart3 tx, rx pin */
  gpio_default_para_init(&gpio_init_struct);
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = DATA3_UART_TX_PIN | DATA3_UART_RX_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(DATA3_UART_GPIO, &gpio_init_struct);
  gpio_pin_mux_config(DATA3_UART_GPIO, DATA3_UART_TX_PIN_SOURCE, DATA3_UART_PIN_MUX_NUM);
  gpio_pin_mux_config(DATA3_UART_GPIO, DATA3_UART_RX_PIN_SOURCE, DATA3_UART_PIN_MUX_NUM);

  /* configure usart param */
  usart_init(DATA3_UART, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_transmitter_enable(DATA3_UART, TRUE); // enable tx
  usart_receiver_enable(DATA3_UART, TRUE); // enable rx
  usart_parity_selection_config(DATA3_UART, USART_PARITY_NONE); // no parity

  /* enable usart3 interrupt rxne*/
  usart_interrupt_enable(DATA3_UART, USART_RDBF_INT, TRUE);
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_0);
  nvic_irq_enable(USART3_IRQn, 0, 0);

  usart_enable(DATA3_UART, TRUE);
}
void USART3_IRQHandler(void)
{
  if (usart_flag_get(DATA3_UART, USART_RDBF_FLAG) != RESET) // Check if data is received
  {
    int index = 2; // usart3 corresponds to index 2 in uartBuf, head, and tail
    uartBuf[index][tail[index]] = usart_data_receive(DATA3_UART);
    tail[index] = (tail[index] + 1) % 128;
  }
}
#endif
void Uart_Send(uint8_t ur_num, uint8_t *data, uint16_t len)
{
  usart_type *usart;
	uint16_t i = 0;
  if (ur_num == 5)
    usart = DATA5_UART;
  else if (ur_num == 6)
    usart = DATA6_UART;
  else if (ur_num == 7)
    usart = DATA3_UART;
  else
    return;

  for (i = 0; i < len; i++)
  {
    usart_data_transmit(usart, data[i]);
    while (usart_flag_get(usart, USART_TDBE_FLAG) == RESET); // Wait until the data is sent
  }
}
