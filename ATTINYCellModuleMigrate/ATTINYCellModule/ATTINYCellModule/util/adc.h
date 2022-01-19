#ifndef _ADC_UTIL_H_
#define _ADC_UTIL_H_

#include <stdint.h>

typedef enum adc_chan {AREF, BATT, TEMP1, TEMP2} adc_chan_t;

float read_adc_channel_multiple(adc_chan_t channel, uint8_t num_conversions);
uint16_t read_adc_channel(adc_chan_t channel);

void deinit_adc();

#endif