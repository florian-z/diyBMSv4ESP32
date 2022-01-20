#ifndef _PROCESS_MESSAGES_H_
#define _PROCESS_MESSAGES_H_

#include <stdint.h>
#include <stdbool.h>


// example:
// ! <module> <cmd> <data> <...> * <crc> \n

#define MSG_CMD 1
#define MSG_MOD_CNT 3

#define MSG_START '!'
#define MSG_CRC '*'
#define MSG_END '\n'

typedef enum msg_command { GET_BATT_VOLT=0, GET_TEMP=1, IDENTIFY_MODULE=2, SET_BATT_VOLT_CALIB=3 } msg_command_t;

uint8_t calc_crc(const uint8_t* msg);
bool is_msg_syntax_valid(const uint8_t* const msg);
void process_message(uint8_t* const msg);

#endif