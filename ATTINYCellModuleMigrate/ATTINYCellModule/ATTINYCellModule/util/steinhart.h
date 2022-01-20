#ifndef _STEINHART_H_
#define _STEINHART_H_

#include <stdint.h>

int8_t thermistorToCelcius(const uint16_t b_coeff, const uint16_t raw_adc);
//uint8_t temperatureToByte(int16_t tempInCelcius);

#endif
