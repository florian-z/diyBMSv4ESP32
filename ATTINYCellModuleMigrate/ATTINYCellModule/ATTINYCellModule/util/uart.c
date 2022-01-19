#include "uart.h"
#include "main.h"

void putc_tx0(const uint8_t singlebyte) {
	while(!(UCSR0A & _BV(UDRE0)));
	UDR0 = singlebyte;
}

void send_tx0(const uint8_t* string) {
  UCSR0A |= _BV(TXC0); // clear any pending transmit complete flags
	while(*string) {
		putc_tx0(*string++);
	}
  while(!(UCSR0A & _BV(TXC0))); // busy wait until last character is fully sent
}