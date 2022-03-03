/*
 * ATTINYCellModule.c
 *
 * Created: 13.01.2022 22:28:58
 * Author : user
 */ 
#include "main.h"

//#include <stddef.h>
#include <stdio.h>

#include <avr/pgmspace.h>
#include <avr/wdt.h>


#include <string.h>

//#include <avr/boot.h>
//USING CLOCKWISE PIN MAPPINGS

#include "adc.h"
#include "pwrmgmt.h"
#include "uart.h"
#include "steinhart.h"
#include "eeprom.h"

void setup();
void loop_test_blinky();
void loop_test_uart();
void loop_test_adc();
void loop();

int main(void) {
  setup();
  set_identify_module();
  while(1) {
    //loop_test_blinky();
	  //loop_test_uart();
	  //loop_test_adc();
    loop();
  }
}

void setup() {
  wdt_enable(WDTO_8S);
  WDTCSR |= _BV(WDIE); // execute Watchdog interrupt instead of reset
  
  /* 
   * Boot up will be in 1Mhz CKDIV8 mode from external 8MHz crystal. Swap to /4 to change speed to 2Mhz.
   * Below 2Mhz is required for running ATTINY at low voltages (less than 2V)
   */
  //CCP – Configuration Change Protection Register
  // protected: CLKPR, MCUCR, WDTCSR
  CCP = 0xD8;
  //CLKPR – Clock Prescale Register
  CLKPR = _BV(CLKPS1);
  // 2MHz clock ...
  

  //PUEA – Port A Pull-Up Enable Control Register (All disabled)
  PUEA = 0;
  //PUEB – Port B Pull-Up Enable Control Register (All disabled)
  PUEB = 0;

  //DDRA – Port A Data Direction Register
  //When DDAn is set, the pin PAn is configured as an output. When DDAn is cleared, the pin is configured as an input
  DDRA |= _BV(DDA1) | _BV(DDA6) | _BV(DDA7); // TX, LED_BLU, REF_VOLT
  //DDRB – Port B Data Direction Register
  //Spare pin is output
  DDRB |= _BV(DDB2); // LED_RED

  //Set the extra high sink capability of pin PA7 is enabled.
  //PHDE |= _BV(PHDEA1);
  
  // config UART0
  //UCSR0A = 0x00; // default: ensure double-speed-off and multi-processor-mode-off
  UCSR0B = _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0); // non default: TXCIE0 RXCIE0 TXEN0 RXEN0 UDRIE0
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01) | _BV(UPM01); // default: async, 8E1
  
  // todo: // wake up mcu from all sleep modes on incoming RX-data
  // note: UCSR0D = UCSR0D | _BV(RXS0); // needs to be cleared by writing a logical one
  UCSR0D = _BV(RXSIE0) | _BV(SFDE0); // non-default: RXSIE0, SFDE0
  
  
  
  // set baudrate
  UBRR0L = 12; // 9600 baud @ 2MHz
  
  __builtin_avr_wdr();
  __builtin_avr_sei();
}

void loop_test_blinky() {
  _delay_ms(4000);
  LED_BLU_ON
  _delay_ms(4000);
  REFVOLT_ON
  _delay_ms(4000);
  SET_TX
  _delay_ms(4000);
  if(READ_RX) {
    LED_RED_ON
  }
  _delay_ms(4000);

  LED_BLU_OFF
  CLR_TX
  REFVOLT_OFF
  _delay_ms(1);
  if(!READ_RX) {
    LED_RED_OFF;
  }
  _delay_ms(1000);
}

void loop_test_uart() {
	LED_BLU_ON
	send_tx0("test1 ... ");
	_delay_ms(500);
	send_tx0("test2\n");
	LED_BLU_OFF
	_delay_ms(500);
	LED_RED_ON
	LED_BLU_ON
	send_tx0("test3 ... ");
	_delay_ms(500);
	send_tx0("test4\n");
	LED_BLU_OFF
	_delay_ms(500);
	LED_RED_OFF
}

ISR(BADISR_vect) {
  while(1) {
    LED_BLU_OFF
    LED_RED_ON
    _delay_ms(50);
    LED_BLU_ON
    LED_RED_OFF
    _delay_ms(50);
  }
}

// printf %f needs linker library printf_flt (huge size)
void loop_test_adc() {
  
  for(uint8_t i=0; i<4;i++) {
    //LED_RED_ON
    float adc_result = read_adc_channel_multiple(i, 10);
    //float adc_result = read_adc_channel(i); 
    //LED_RED_OFF

	  // ADC = Vin * 1024 / Vref
	  float volt_calib = 1.25 / 1023.0; // 0x03FF is VREF minus one LSB
	  float voltage = adc_result * volt_calib;
    
	  if(BATT == i) {
  	  voltage *= 3.5185185;
	  }
  
    char data[100] = {0};
    if(AREF == i || BATT == i) {
	    snprintf(data, 100, "%d %.2f %.3fV\n", i, adc_result, voltage);
    } else {
      snprintf(data, 100, "%d %.2f %.3fV %dC\n", i, adc_result, voltage, thermistorToCelcius(3950, adc_result));
    }
    //send_tx0(data);
    outgoing_msg(data, strlen(data));
  }
  deinit_adc();
  //send_tx0("\n");
  
  _delay_ms(1000);
  
  //set_config(VOLT_CALIB, 1234);
  //_delay_ms(1000);
  char data[100] = {0};
  snprintf(data, 100, "EEPROM %f\n", get_config(VOLT_CALIB));
  //send_tx0(data);
  outgoing_msg(data, strlen(data));
  //_delay_ms(1000);
}

#include <string.h>

#define MSG_IN_LEN 10
static uint8_t msg_in[MSG_IN_LEN] = {0};
static bool new_msg_in = false;

void incoming_msg(const uint8_t * const msg, const uint8_t len) {
  memcpy(msg_in, msg, len);
  new_msg_in = true;

  // todo remove
  outgoing_msg(msg, len);
}

static uint8_t flag_identify_module = 0;
static uint8_t flag_go_deepsleep = 0;
static uint8_t flag_processing_done = 0;

void set_identify_module() {
  flag_identify_module = 16;
}
void set_disable_deepsleep() {
  flag_go_deepsleep = 0;
}
void set_enable_deepsleep() {
  flag_go_deepsleep = 1;
}
void set_processing_start() {
  flag_processing_done = 0;
}
void set_processing_done() {
  flag_processing_done = 1;
}


void go_sleep_idle();
void go_sleep_powerdown();

void loop() {
  __builtin_avr_wdr();
  if(flag_identify_module) {
    flag_identify_module--;
    LED_RED_ON
    _delay_ms(50);
    LED_RED_OFF
    _delay_ms(50);
    
  // "else if" will ensure, that identify module has priority over sleep
  } else if (flag_processing_done) {
    if (flag_go_deepsleep) {
      go_sleep_powerdown();
    } else {
      go_sleep_idle();
    }
  } else {
    go_sleep_idle();
  }
}

void go_sleep_idle() {
  deinit_adc();
  LED_RED_OFF
  LED_BLU_ON
  pwrmgmt_sleep_idle();
  LED_BLU_OFF
}

void go_sleep_powerdown() {
  deinit_adc();
  LED_RED_ON
  LED_BLU_OFF
  pwrmgmt_sleep_standby(); // oscillator will keep running, otherwise mcu startup is too slow to get the next incoming message
  LED_RED_OFF
}

/* Watchdog Time-out Interrupt */
ISR(WDT_vect) {
  WDTCSR |= _BV(WDIE); // execute Watchdog interrupt instead of reset
}