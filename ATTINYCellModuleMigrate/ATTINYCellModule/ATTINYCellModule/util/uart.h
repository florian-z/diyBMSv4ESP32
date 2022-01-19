#ifndef _UART_UTIL_H_
#define _UART_UTIL_H_

#include <stdint.h>

void outgoing_msg(const uint8_t * const msg, uint8_t len);
void send_tx0(const uint8_t* string);

#endif