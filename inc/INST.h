/* command package format: starter 0xff 0xff + ID_num + Data_length + command package + parameters + check_sum */
/* starter: 2 0xff means command package incoming */
/* ID: servo ID from 0-253, transfered to hex 0x00 - 0xFD */
/* Data_length: number of paras waiting for sending N + 2 */
/* Command package: written downwards*/
/* Parameters: other command information for supprt*/
/* check_sum: Check Sum = ~ (ID + Length + Instruction + Parameter1 + ... Parameter N)*/

 #ifndef _INST_H
 #define _INST_H
 
 #include <stdint.h>
 
 #ifndef NULL
 #define NULL ((void *)0)
 #endif
 
 // define the command types
 #define INST_PING 0x01
 #define INST_READ 0x02 // length of para = 2
 #define INST_WRITE 0x03 // length of para >=1
 #define INST_REG_WRITE 0x04 // length of para >=2
 #define INST_REG_ACTION 0x05 // trigger the action of reg_write
 #define INST_SYNC_READ 0x82 // length of para >= 3
 #define INST_SYNC_WRITE 0x83 // length of para >= 2
 #define INST_RECOVERY 0x06 // length of para = 1
 #define INST_RESET 0x0A // length of para = 1
 
 #endif
