#ifndef __USB_PRINT_H
#define __USB_PRINT_H
#include "at32f423_usb.h"


/* establish stream for usb receiving */
typedef struct {
    volatile uint8_t enabled;
    volatile uint32_t rate_hz;
} stream_state_t;

typedef struct {
    uint32_t t_ms;
    int16_t  pos[8];
    int16_t  vel[8];
} telem_t;

/* sampling frame line, ring buffer */
void usb_recv_data(void);
void usb_poll_rx_and_fill_ring(void);
void usb_send_data(uint8_t *data, uint32_t len);
void usb_clock48m_select(usb_clk48_s clk_s);
void usb_gpio_config(void);
void usb_low_power_wakeup_config(void);
void OTG_WKUP_HANDLER(void);
void OTG_IRQ_HANDLER(void);
void usb_init(void);
void usb_printf(char *fmt, ...);

#endif
