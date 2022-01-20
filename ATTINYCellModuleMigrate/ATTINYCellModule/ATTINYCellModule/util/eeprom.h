#ifndef _EEPROM_UTIL_H_
#define _EEPROM_UTIL_H_

#include <stdint.h>

typedef enum configs { VOLT_CALIB } configs_t;

float get_config(configs_t config);
void set_config(configs_t config, float value);

#endif