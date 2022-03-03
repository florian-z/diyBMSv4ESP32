#ifndef _MAIN_H_
#define _MAIN_H_

#define F_CPU 2000000UL
#include <avr/io.h>
#include <avr/builtins.h> // sli, cli, wdt-reset, ...
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h> // busy wait
#include <stdbool.h>

// A1 - TX
// A6 - LED BLUE
// A7 - REF. VOLTAGE
// B2 - LED RED
#define LED_BLU_ON PORTA |= _BV(PORTA6);
#define LED_RED_ON PORTB |= _BV(PORTB2);
#define REFVOLT_ON PORTA |= _BV(PORTA7);
#define LED_BLU_OFF PORTA &= (~_BV(PORTA6));
#define LED_RED_OFF PORTB &= (~_BV(PORTB2));
#define REFVOLT_OFF PORTA &= (~_BV(PORTA7));
#define READ_RX (PINA & _BV(PORTA2))
#define SET_TX PORTA |= _BV(PORTA1);
#define CLR_TX PORTA &= (~_BV(PORTA1));

void incoming_msg(const uint8_t * const msg, const uint8_t len);

void set_identify_module();
void set_go_sleeping();
void set_transfer_done();

#endif