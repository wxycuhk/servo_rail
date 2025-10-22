#include "usb_conf.h"
#include "usb_core.h"
#include "usbd_int.h"
#include "cdc_class.h"
#include "cdc_desc.h"
#include "string.h"
#include "delay.h"
#include "USB_print.h"
#include "string.h"
#include "stdarg.h"
#include "stdio.h"
#include <ctype.h>
#include "at32f423.h"

// #include "FreeRTOS.h"

/* usb global struct define */
otg_core_type otg_core_struct;
stream_state_t usb_rx_stream = {0};

#if defined ( __ICCARM__ ) /* iar compiler */
  #pragma data_alignment = 4
#endif
ALIGNED_HEAD volatile uint8_t usb_buffer[512] ALIGNED_TAIL;
volatile uint16_t data_len = 0;
uint32_t timeout = 0;
extern int usb_div_flag;
#define USB_USART_REC_LEN       200     /* USB���ڽ��ջ���������ֽ��� */
uint8_t g_usb_usart_printf_buffer[USB_USART_REC_LEN];


/* Part for usb receiving */

#define RX_RING_SIZE 1024
static uint8_t rx_ring[RX_RING_SIZE];
static volatile uint16_t rx_head = 0, rx_tail = 0;

static inline int rx_ring_put(uint8_t byte)
{
    uint16_t next = (uint16_t)(rx_head + 1) % RX_RING_SIZE;
    if(next == rx_tail) return 0; // ring full
    rx_ring[rx_head] = byte;
    rx_head = next; 
    return 1;
}

static inline int rx_ring_get(uint8_t *byte)
{
    if(rx_head == rx_tail) return 0; // ring empty
    *byte = rx_ring[rx_tail];
    rx_tail = (uint16_t)(rx_tail + 1) % RX_RING_SIZE;
    return 1;
}

/* sampling frame line, ring buffer */
#define TQ_SIZE 8 // must be power of 2
static telem_t tq[TQ_SIZE];
static volatile uint8_t tq_tail = 0, tq_head = 0;

static inline void tq_put(const telem_t *item)
{
    uint16_t next = (uint16_t)(tq_head + 1) & (TQ_SIZE - 1);
    if(next == tq_tail) tq_tail = (uint8_t)(tq_tail + 1) & (TQ_SIZE - 1); // ring full
    tq[tq_head] = *item;
    tq_head = next; 
}

static inline int tq_pop_main(telem_t *item)
{
    if(tq_head == tq_tail) return 0; // ring empty
    *item = tq[tq_tail];
    tq_tail = (uint8_t)(tq_tail + 1) & (TQ_SIZE - 1);
    return 1;
}

void usb_poll_rx_and_fill_ring(void)
{
  uint8_t buf[64];
  data_len = usb_vcp_get_rxdata(&otg_core_struct.dev, buf);
  for(int i = 0; i < data_len; i++)
  {
      (void)rx_ring_put(buf[i]);
  }
}

static uint16_t rx_take_one_line(uint8_t *buf, uint16_t max_len)
{
    if(max_len == 0) return 0;
    uint16_t len = 0;
    uint8_t ch;
    uint8_t seen_eol = 0;

    /*  */
    uint16_t t = rx_tail;
    while(t != rx_head) {
        uint8_t c = rx_ring[t];
        t = (uint16_t)((t + 1) % RX_RING_SIZE);
        if(c == '\r' || c == '\n') { seen_eol = 1; break; }
    }
    if(!seen_eol) return 0;

    /* keep receiving until we find a line end */
    while(rx_ring_get(&ch)) {
        if(ch == '\r' || ch == '\n') {
            /* discard the next line ending */
            uint8_t c2;
            if(rx_head != rx_tail) {
                if(rx_ring[rx_tail] == '\r' || rx_ring[rx_tail] == '\n') {
                    rx_ring_get(&c2);
                }
            }
            break;
        }
        if(len + 1 < max_len) buf[len++] = ch;
    }
    buf[len] = 0;
    return len;
}

void usb_recv_data(void)
{
    data_len = usb_vcp_get_rxdata(&otg_core_struct.dev, (uint8_t *)usb_buffer);
    if(data_len > 0)
    {
        //usb_vcp_send_data(&otg_core_struct.dev, (uint8_t *)usb_buffer, data_len);
    }
}


void usb_send_data(uint8_t *data, uint32_t len)
{   
    timeout = 5000000;
    do
    {
      if(usb_vcp_send_data(&otg_core_struct.dev, data, len) == SUCCESS){break;}
    }
    while(timeout--);

}
// return 1 if success, 0 if fail, do not block
static inline int usb_try_tx(uint8_t *buf, uint32_t len)
{
    if(usb_vcp_send_data(&otg_core_struct.dev, buf, len) == SUCCESS) return 1;
		else return 0;
}

void usb_printf(char *fmt, ...)
{
    uint16_t i;
    va_list ap;
    va_start(ap, fmt);
    int n = vsprintf((char *)g_usb_usart_printf_buffer, fmt, ap);
    va_end(ap);
    if(n <= 0) return;
    i = strlen((const char *)g_usb_usart_printf_buffer);    /* �˴η������ݵĳ��� */
    (void)usb_try_tx(g_usb_usart_printf_buffer, i);          /* �������� */
}

void usb_init()
{
		usb_div_flag = 9;
		crm_periph_clock_enable(OTG_CLOCK, TRUE);
		usb_div_flag = 8;
		usb_clock48m_select(USB_CLK_HEXT);
		nvic_irq_enable(OTG_IRQ, 0, 0);
		usbd_init(&otg_core_struct,
						USB_FULL_SPEED_CORE_ID,
						USB_ID,
						&cdc_class_handler,
						&cdc_desc_handler);
}
/**
  * @brief  usb 48M clock select
  * @param  clk_s:USB_CLK_HICK, USB_CLK_HEXT
  * @retval none
  */
  void usb_clock48m_select(usb_clk48_s clk_s)
  {
		
    if(clk_s == USB_CLK_HICK)
    {
			usb_div_flag = 1;
      crm_usb_clock_source_select(CRM_USB_CLOCK_SOURCE_HICK);
  
      /* enable the acc calibration ready interrupt */
      crm_periph_clock_enable(CRM_ACC_PERIPH_CLOCK, TRUE);
  
      /* update the c1\c2\c3 value */
      acc_write_c1(7980);
      acc_write_c2(8000);
      acc_write_c3(8020);
  
      /* open acc calibration */
      acc_calibration_mode_enable(ACC_CAL_HICKTRIM, TRUE);
    }
    else
    {
      switch(system_core_clock)
      {
        /* 48MHz */
        case 48000000:
          crm_usb_clock_div_set(CRM_USB_DIV_2);
          break;
  
        /* 72MHz */
        case 72000000:
          crm_usb_clock_div_set(CRM_USB_DIV_3);
          break;
  
        /* 96MHz */
        case 96000000:
          crm_usb_clock_div_set(CRM_USB_DIV_4);
          break;
  
        /* 120MHz */
        case 120000000:
          crm_usb_clock_div_set(CRM_USB_DIV_5);
          break;
        
        /* 144MHz */
        case 144000000:
          crm_usb_clock_div_set(CRM_USB_DIV_6);
          break;
				
				/* 216MHz */
				
  
        default: 
          break;
  
      }
    }
  }
  
  /**
    * @brief  this function config gpio.
    * @param  none
    * @retval none
    */
  void usb_gpio_config(void)
  {
    gpio_init_type gpio_init_struct;
  
    crm_periph_clock_enable(OTG_PIN_GPIO_CLOCK, TRUE);
    gpio_default_para_init(&gpio_init_struct);
  
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  
  #ifdef USB_SOF_OUTPUT_ENABLE
    crm_periph_clock_enable(OTG_PIN_SOF_GPIO_CLOCK, TRUE);
    gpio_init_struct.gpio_pins = OTG_PIN_SOF;
    gpio_init(OTG_PIN_SOF_GPIO, &gpio_init_struct);
    gpio_pin_mux_config(OTG_PIN_GPIO, OTG_PIN_SOF_SOURCE, OTG_PIN_MUX);
  #endif
  
    /* otgfs use vbus pin */
  #ifndef USB_VBUS_IGNORE
    gpio_init_struct.gpio_pins = OTG_PIN_VBUS;
    gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
    gpio_pin_mux_config(OTG_PIN_GPIO, OTG_PIN_VBUS_SOURCE, OTG_PIN_MUX);
    gpio_init(OTG_PIN_GPIO, &gpio_init_struct);
  #endif
  
  
  }
  #ifdef USB_LOW_POWER_WAKUP
  /**
    * @brief  usb low power wakeup interrupt config
    * @param  none
    * @retval none
    */
  void usb_low_power_wakeup_config(void)
  {
    exint_init_type exint_init_struct;
  
    crm_periph_clock_enable(CRM_SCFG_PERIPH_CLOCK, TRUE);
    exint_default_para_init(&exint_init_struct);
  
    exint_init_struct.line_enable = TRUE;
    exint_init_struct.line_mode = EXINT_LINE_INTERRUPT;
    exint_init_struct.line_select = OTG_WKUP_EXINT_LINE;
    exint_init_struct.line_polarity = EXINT_TRIGGER_RISING_EDGE;
    exint_init(&exint_init_struct);
  
    nvic_irq_enable(OTG_WKUP_IRQ, 0, 0);
  }
  
  /**
    * @brief  this function handles otgfs wakup interrupt.
    * @param  none
    * @retval none
    */
  void OTG_WKUP_HANDLER(void)
  {
    exint_flag_clear(OTG_WKUP_EXINT_LINE);
  }
  
  #endif
  
  /**
    * @brief  this function handles otgfs interrupt.
    * @param  none
    * @retval none
    */
  void OTG_IRQ_HANDLER(void)
  {
    usbd_irq_handler(&otg_core_struct);
  }

