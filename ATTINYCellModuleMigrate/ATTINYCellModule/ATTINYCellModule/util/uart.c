#include "uart.h"
#include "main.h"
#include <string.h>
#include "process_messages.h"

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



#define RX_BUF_LEN 100
#define TX_BUF_LEN 100
static volatile uint8_t rx_data[RX_BUF_LEN] = {0};
static volatile uint8_t rx_n = 0;
static volatile uint8_t rx_crc = 0;
void clear_rx_buffer() {
  memset(rx_data, 0, RX_BUF_LEN);
  rx_n = 0;
}

/* USART0, Start-Bit */
EMPTY_INTERRUPT(USART0_START_vect)

/* USART0, Rx Complete */
ISR(USART0_RX_vect) {
  // check parity
  if(UCSR0A & _BV(UPE0)) {
    // parity error
    clear_rx_buffer();
    const uint8_t data = UDR0;

  } else {
    // parity ok
    const uint8_t data = UDR0;
    rx_data[rx_n++] = data;

    if (MSG_END == data) {
      // received msg end, process msg
      LED_BLU_ON
      //incoming_msg(rx_data, rx_n);
      process_message(rx_data);
      clear_rx_buffer();
      LED_BLU_OFF

    } else if(MSG_START == data) {
      // received msg start, start over
      clear_rx_buffer();
      rx_data[rx_n++] = data;

    } else if (RX_BUF_LEN == rx_n) {
      // message too long / buffer full -> clear buffer
      clear_rx_buffer();
    }
  }
}

static volatile uint8_t tx_data[TX_BUF_LEN] = {0};
static volatile uint8_t tx_n = 0;
static volatile uint8_t tx_len = 0;

void outgoing_msg(const uint8_t * const msg, uint8_t len) {
  // wait for ongoing transmission to finish, if any
  while(UCSR0B & _BV(UDRIE0));
  
  // take next msg
  memcpy(tx_data, msg, len);
  tx_n = 0;
  tx_len = len;
  UCSR0B |= _BV(UDRIE0) | _BV(TXCIE0); // enable TX0 interrupts
}

/* USART0 Data Register Empty */
ISR(USART0_UDRE_vect) {
  if (!tx_len) {
    // transmit done
    UCSR0B &= ~_BV(UDRIE0); // disable TX0 DataRegisterEmpty interrupt
    return;
  }
  UDR0 = tx_data[tx_n++];
  if (tx_n == tx_len) {
    // transmit done
    //UCSR0B &= ~_BV(UDRIE0); // disable TX0 interrupt
    memset(tx_data, 0, TX_BUF_LEN);
    tx_n = 0;
    tx_len = 0;
  }
}

/* USART0, Tx Complete */
ISR(USART0_TX_vect) {
  UCSR0B &= ~_BV(TXCIE0); // disable TX0 TransferComplete interrupt
  set_transfer_done();
}
