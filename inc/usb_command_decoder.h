#ifndef __USB_COMMAND_DECODER_H
#define __USB_COMMAND_DECODER_H

#include <stdint.h>

#define MAX_SERVO_NUM 35

void usb_parse_servo_command(void);
void handle_command_F(char *tof_after_F);

#endif
