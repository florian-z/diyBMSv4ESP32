/*
 * ATTINYCellModule.c
 *
 * Created: 13.01.2022 22:28:58
 * Author : user
 */ 

#define F_CPU 2000000UL

#include <avr/io.h>
#include <avr/builtins.h> // sli, cli, wdt-reset, ...


//#include <inttypes.h>
//#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/power.h> // power consumption
#include <avr/sleep.h> // power consumption
#include <avr/cpufunc.h> // nop, ccp-register

#include <util/delay.h> // busy wait
//#include <util/setbaud.h> // baud rate calculator macros

//#include <avr/boot.h>
//USING CLOCKWISE PIN MAPPINGS

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

void setup();
void loop_test_blinky();
void loop_test_uart();
void loop_test_adc();



// uart header
void send_tx0(const uint8_t* string);
// uart header end

void putc_tx0(uint8_t singlebyte) {
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

int main(void) {
  setup();
  while(1) {
    //loop_test_blinky();
	//loop_test_uart();
	loop_test_adc();
  }
}

void setup() {
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
  UCSR0B = _BV(TXEN0) | _BV(RXEN0); // non default: TXCIE0 RXCIE0 TXEN0 RXEN0 UDRIE0
  //UCSR0C = _BV(UCSZ00) | _BV(UCSZ01); // default: async, 8N1
  
  // wake up mcu from all sleep modes on incoming RX-data
  // note: UCSR0D = UCSR0D | _BV(RXS0); // needs to be cleared by writing a logical one
  //UCSR0D = _BV(RXSIE0) | _BV(SFDE0); // non-default: RXSIE0, SFDE0
  
  // set baudrate
  UBRR0L = 12; // 9600 baud @ 2MHz
  
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

// pwrmgmt header
void pwrmgmt_sleep_idle();
void pwrmgmt_sleep_adcnoisereduction();
// pwrmgmt header end
void pwrmgmt_sleep_idle() {
  MCUCR = _BV(SE); // enable sleep instruction, select idle
  __builtin_avr_sleep(); // start sleep (sleep instruction)
  MCUCR &= ~_BV(SE); // disable sleep instruction

//Idle Mode
//This sleep mode basically halts clkCPU and clkFLASH, while allowing other clocks to run. In Idle Mode, the CPU is stopped
//but the following peripherals continue to operate:
//? Watchdog and interrupt system
//? Analog comparator, and ADC
//? USART, TWI, and timer/counters
//
//Idle mode allows the MCU to wake up from external triggered interrupts as well as internal ones, such as Timer Overflow.
//If wake-up from the analog comparator interrupt is not required, the analog comparator can be powered down by setting
//the ACD bit in ACSRA. See “ACSR1A – Analog Comparator 1 Control and Status Register” on page 129. This will reduce power consumption in Idle mode.
//If the ADC is enabled, a conversion starts automatically when this mode is entered.

}
void pwrmgmt_sleep_adcnoisereduction() {
  LED_BLU_ON
  MCUCR = _BV(SE) | _BV(SM0); // enable sleep instruction, select ADC-noise-reduction
  __builtin_avr_sleep(); // start sleep (sleep instruction)
  MCUCR &= ~_BV(SE); // disable sleep instruction
  LED_BLU_OFF

  //ADC Noise Reduction Mode
  //This sleep mode halts clkI/O, clkCPU, and clkFLASH, while allowing other clocks to run. In ADC Noise Reduction mode, the
  //CPU is stopped but the following peripherals continue to operate:
  //? Watchdog (if enabled), and external interrupts
  //? ADC
  //? USART start frame detector, and TWI
  //
  //This improves the noise environment for the ADC, enabling higher resolution measurements. If the ADC is enabled, a
  //conversion starts automatically when this mode is entered.
  //The following events can wake up the MCU:
  //? Watchdog reset, external reset, and brown-out reset
  //? External level interrupt on INT0, and pin change interrupt
  //? ADC conversion complete interrupt, and SPM/EEPROM ready interrupt
  //? USART start frame detection, and TWI slave address match
}


//adc header
typedef enum adc_chan {AREF, BATT, TEMP1, TEMP2} adc_chan_t;
float read_adc_channel_multiple(adc_chan_t channel, uint8_t num_conversions);
uint16_t read_adc_channel(adc_chan_t channel);
// adc header end

#define ADMUXA_ADC0 0b0000
#define ADMUXA_ADC1 0b0001
#define ADMUXA_ADC2 0b0010
#define ADMUXA_ADC3 0b0011
#define ADMUXA_ADC4 0b0100
#define ADMUXA_ADC5 0b0101
#define ADMUXA_ADC6 0b0110
#define ADMUXA_ADC7 0b0111
//#define ADMUXA_ADC8 0x1000
//#define ADMUXA_ADC9 0x1001
//#define ADMUXA_ADC10 0x1010
//#define ADMUXA_ADC11 0x1011

// only with internal 1.1V reference
#define ADMUXA_ADC_INTTEMP 0b1100

#define ADMUXA_ADC_INT1V1 0b1101
#define ADMUXA_ADC_AGND 0b1110
#define ADMUXA_ADC_VCC 0b1111
// remap to user board
#define ADMUXA_ADC_AREF ADMUXA_ADC0
#define ADMUXA_ADC_BATT ADMUXA_ADC3
#define ADMUXA_ADC_TEMP1 ADMUXA_ADC4
#define ADMUXA_ADC_TEMP2 ADMUXA_ADC5

void select_adc_channel(adc_chan_t channel);
uint16_t read_adc();
bool adc_active = false;

void init_adc() {
  REFVOLT_ON;
  _delay_us(100);
  // todo wait until AREF stabilizes
  ADCSRA |= _BV(ADEN); // activate ADC module
  ADMUXB = 0b10000000; // use ext AREF, GAIN=1;
  ADMUXA = ADMUXA_ADC_BATT; // select BATT voltage
  //ADMUXA = ADMUXA_ADC_TEMP1;

  DIDR0 |= _BV(ADC0D) | _BV(ADC3D) | _BV(ADC4D) | _BV(ADC5D); // disable digital input pin function on analog input pins

  ADCSRA |= _BV(ADPS2); // clk prescaler 16 (125kHz @ 2MHz) 50..200kHz
  //ADCSRA |= _BV(ADATE); // free running
  ADCSRA |= _BV(ADIE); // ADC interrupt necessary to unsleep mcu
  ADCSRA |= _BV(ADIF); // clear any pending interrupt flag
  //ADCSRA |= _BV(ADSC); // start conversion

  adc_active = true;
}

void deinit_adc() {
  __builtin_avr_cli();
  adc_active = false;
  ADCSRA &= ~_BV(ADEN); // deactivate ADC module  
  REFVOLT_OFF;
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

EMPTY_INTERRUPT(ADC_vect)
//ISR(ADC_vect) {}

uint16_t read_adc_channel(adc_chan_t channel) {
  if (!adc_active) {
    init_adc();
  }

  select_adc_channel(channel);

  __builtin_avr_sei();
  ADCSRA |= _BV(ADIF); // clear any pending interrupt flag
  ADCSRA |= _BV(ADSC); // start conversion
  pwrmgmt_sleep_adcnoisereduction();

  while(ADCSRA & _BV(ADSC)); // wait until adc conversion done
  ADCSRA |= _BV(ADIF); // clear adc complete flag

  return read_adc();
}

/// num_conversions < 64
float read_adc_channel_multiple(adc_chan_t channel, uint8_t num_conversions) {
  uint16_t adc_result = 0;
  for(uint8_t i=0; i<num_conversions; i++) {
    adc_result += read_adc_channel(channel);
  }

  return adc_result / (float) num_conversions;
};

void select_adc_channel(adc_chan_t channel) {
  switch(channel) {
    case AREF:
      ADMUXA = ADMUXA_ADC_AREF;
      break;
    case BATT:
      ADMUXA = ADMUXA_ADC_BATT;
      break;
    case TEMP1:
      ADMUXA = ADMUXA_ADC_TEMP1;
      break;
    case TEMP2:
      ADMUXA = ADMUXA_ADC_TEMP2;
      break;
  }
}

uint16_t read_adc() {
  uint8_t low_byte = ADCL;
  return (ADCH<<8) | low_byte;
}

// printf %f needs linker library printf_flt (huge size)
void loop_test_adc() {
  
  for(uint8_t i=0; i<4;i++) {
    LED_RED_ON
    float adc_result = read_adc_channel_multiple(i, 10);
    //float adc_result = read_adc_channel(i); 
    LED_RED_OFF  

	  // ADC = Vin * 1024 / Vref
	  float volt_calib = 1.25 / 1023.0; // 0x03FF is VREF minus one LSB
	  float voltage = adc_result * volt_calib;
	
	  if(BATT == i) {
  	  voltage *= 3.5185185;
	  }
  
	  char data[100] = {0};
	  snprintf(data, 100, "%d %.2f %.3fV\n", i, adc_result, voltage);
	  send_tx0(data);
  }
  deinit_adc();
  send_tx0("\n");
  
  _delay_ms(1000);
}
