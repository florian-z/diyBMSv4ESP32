#ifndef _STEINHART_H_
#define _STEINHART_H_

#include <stdint.h>

int16_t thermistorToCelcius(uint16_t b_coeff, uint16_t raw_adc);
uint8_t temperatureToByte(int16_t tempInCelcius);

#endif
