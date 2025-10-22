#include "usb_command_decoder.h"
#include "USB_print.h"
#include "Servo_Functions.h"
#include "SCS.h"
#include "delay.h"
#include "sys.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "timer.h"

extern volatile uint16_t data_len;
extern volatile uint8_t usb_buffer[256];
// int init_pos[5] = {2067, 2227, 2217, 2327, 2300};
uint16_t set_time = 200;
int cur_mode = 0; // 0 - servo mode, 1 - wheel mode, 2 - pwm mode
uint8_t servo_num = 5; // default servo number is 5
uint8_t ID_list[MAX_SERVO_NUM] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14}; // default as 2 3 4 5 6
uint8_t valid_id_list[MAX_SERVO_NUM] = {0};
int16_t init_pos[MAX_SERVO_NUM] = {2067, 2227, 2217, 2327, 2300, 1900, 2047, 2047, 2047, 2147, 2047, 2397, 2447}; // default init pos for 2 3 4 5 6 7 8 9 10 11 12 13 14
stream_state_t rx_stream = {0, 0};
extern int ur3_num;


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

void handle_command_F(char *tof_after_F)
{
  uint32_t rate = 0;
  if(tof_after_F) rate = (uint32_t)atoi(tof_after_F);

  if(rate == 0){
    rx_stream.enabled = 0;
    rx_stream.rate_hz = 0;
    usb_printf("F,0\r\n");
    return;
  }

  if(rate > 1000) rate = 1000; // limit the max rate to 1000Hz

  rx_stream.enabled = 1;
  rx_stream.rate_hz = rate;
  TIM3_10kHz_Init(rate);
  usb_printf("F,%lu\r\n", rate);

}

void usb_parse_servo_command(void)
{
		char* sub = NULL;	
		uint8_t id = 2;
		static uint8_t servo_num_pinged = 0; 
		uint8_t desired_id = 1; 
    uint8_t current_id = 1;
    if(data_len == 0) return; // no data to parse
    usb_buffer[data_len] = 0; // null terminate the string
    char *p = (char *)usb_buffer;
    trim(p); // remove trailing \n

    char *tok = strtok(p, ","); // get the first token
    if(!tok){data_len=0; return;} // no command found

    switch(toupper(tok[0])) // check the first character of the command
    {
        // reset all servos to initial position      
        case 'P':
            for(int i = 0; i < servo_num; ++i) // id 2 to 8
            {
								id = ID_list[i];
                RegWritePosEx(id, init_pos[id-2], 32766, 0, 7 );
            }
            RegWriteAction(7); // send action command to servo
            delay_ms(100); // wait for the action to complete
            usb_printf("Servo resetting done\r\n");
            break;
        // move single servo to position with speed and acceleration
        /* switch mode of servo, command: M, id, mode */
        // mode: 0 - servo mode, 1 - wheel mode, 2 - pwm mode
        case 'M':
        {
          int id = atoi(strtok(NULL,",")); // get the servo id
          int mode = atoi(strtok(NULL,",")); // get the mode
          int mode_result = 0;
          cur_mode = mode;
          if(mode == 0){
            unLockEprom(id, ur3_num); // unlock the eprom
            mode_result = writeByte(id, SMS_STS_MODE, 0, ur3_num); // set the servo to servo mode
            delay_ms(5);
          } 
          else if(mode == 1)
            mode_result = WheelMode(id, 7); // set the servo to wheel mode
          else if(mode == 2)
            mode_result = PWMMode(id, 7); // set the servo to pwm mode
          else
            usb_printf("Unknown mode %d\r\n", mode);

          usb_printf("M, %d, %d, %d\r\n", id, mode, mode_result);
          break;
        }

        // move single servo to position with speed and acceleration under normal servo mode
        case 'K':
        {
            if (cur_mode != 0)
            {
                usb_printf("Error: Servo not in servo mode, please switch to servo mode first\r\n");
                break;
            }
            int id = atoi(strtok(NULL,",")); // get the servo id
            int pos = atoi(strtok(NULL,",")); // get the position
            int speed = atoi(strtok(NULL,",")); // get the speed
            int acc = atoi(strtok(NULL,",")); // get the acceleration

            RegWritePosEx(id, pos, speed, acc, 7); // set the servo position
            RegWriteAction(7); // send action command to servo
            // usb_printf("Servo %d moving to %d with speed %d and acc %d\r\n", id, pos, speed, acc);
            break;
        }
        
        // move single servo to speed with acceleration under wheel mode
        case 'W': // move single servo with speed and acceleration under wheel mode
        {
            if (cur_mode != 1)
            {
                usb_printf("Error: Servo not in wheel mode, please switch to wheel mode first\r\n");
                break;
            }
            int id = atoi(strtok(NULL,",")); // get the servo id
            int speed = atoi(strtok(NULL,",")); // get the speed
            int acc = atoi(strtok(NULL,",")); // get the acceleration

            WriteSpe(id, speed, acc, ur3_num); // set the servo speed
            break;
        }

        // disable torque of given servo
				case 'D':
				{
					int id = atoi(strtok(NULL,",")); // get the servo

					EnableTorque(id, 0, ur3_num);
					delay_ms(1);
					usb_printf("D, %d\r\n", id);
					
				}

        // calibrate servo and enable torque 
				case 'R':
				{
					int id = atoi(strtok(NULL,","));
					EnableTorque(id, 128, ur3_num);
					delay_ms(5);
					EnableTorque(id, 1, ur3_num);
					delay_ms(5);
					usb_printf("R, %d\r\n", id);
					break;
				}
        				
        // set servo number and id list
				case 'S':
						sub = strtok(NULL,",");
						if(!sub){usb_printf("ERROR MESSAGE: NO PARA COMMAND\r\n"); }
						if(!strcasecmp(sub, "num"))
						{
								int n = atoi(strtok(NULL, ","));
								servo_num = (uint8_t)n;
								usb_printf("ok, s:num = %d\r\n", servo_num);
						}

						else if(!strcasecmp(sub, "id")) // adjust the id sequence in the id_list array
						{
								int32_t buf[MAX_SERVO_NUM];
								uint8_t n = read_int_list(buf, MAX_SERVO_NUM);
                if(n==0){ usb_printf("ERR S:ID empty\n"); break; }
                servo_num = n;
                for(uint8_t i=0;i<n;++i) ID_list[i]=(uint8_t)buf[i];
                usb_printf("OK S:ID set (%d)\r\n", servo_num);
						}
            
						else if(!strcasecmp(sub,"pos")) // update the initial position list
            {
                int32_t buf[MAX_SERVO_NUM];
                uint8_t n = read_int_list(buf, MAX_SERVO_NUM);
                if(n==0){ usb_printf("ERR S:POS empty\n"); break; }
                servo_num = n;                          /* Update the servo number */
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

        // function for ping all servos and update
        case 'C':
            for(int i = 0; i < MAX_SERVO_NUM; i++)
            {
                valid_id_list[i] = 0;
            }
            servo_num_pinged = 0;
            for(int i = 1; i <= MAX_SERVO_NUM; i++)
            {
                int temple_id = Ping(i, ur3_num);
                if(temple_id != -1)
                {
                    valid_id_list[servo_num_pinged] = temple_id;
                    servo_num_pinged ++;
                }
            }
            usb_printf("C,%d", servo_num_pinged);
            for(int i = 0; i < servo_num_pinged; i++)
            {
                usb_printf(",%d", valid_id_list[i]);
            }
            usb_printf("\r\n");

        /* Modify the ID of servos*/
				case 'I':
            current_id = atoi(strtok(NULL, ","));
						desired_id = atoi(strtok(NULL, ","));

            // check if the desired id is already in use
            for(int i = 0; i < servo_num_pinged; i++)
            {
                if(valid_id_list[i] == desired_id)
                {
                    usb_printf("C: ID %d is already in use\r\n", desired_id);
                    return;
                }
            }

            unLockEprom(current_id, ur3_num);
            int id_result = writeByte(current_id, SMS_STS_ID, desired_id, ur3_num);
            if(id_result == 1){
              usb_printf("Motor ID modified from %d to %d\r\n", current_id, desired_id);
              LockEprom(desired_id, ur3_num);
            }
            else
              usb_printf("ID modification failed! \r\n");

        /* send the feedback pos and vel */
				default:
    
            break;
    }
    data_len = 0; // reset the data length after processing the command
    
}
