#include "adc.h"
#include "../main.h"
#include "pwrmgmt.h"


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



static bool adc_active = false;

void init_adc() {
    REFVOLT_ON;
    _delay_us(100); // wait until AREF stabilizes
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
    adc_active = false;
    ADCSRA &= ~_BV(ADEN); // deactivate ADC module  
    REFVOLT_OFF;
}

uint16_t read_adc() {
  uint8_t low_byte = ADCL;
  return (ADCH<<8) | low_byte;
}

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

/* ADC Conversion Complete */
EMPTY_INTERRUPT(ADC_vect)
//ISR(ADC_vect) {}

uint16_t read_adc_channel(adc_chan_t channel) {
  if (!adc_active) {
    init_adc();
  }

  select_adc_channel(channel);

  ADCSRA |= _BV(ADIF); // clear any pending interrupt flag
  //ADCSRA |= _BV(ADSC); // start conversion
  pwrmgmt_sleep_adcnoisereduction(); // automatically starts ADC conversion

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
}