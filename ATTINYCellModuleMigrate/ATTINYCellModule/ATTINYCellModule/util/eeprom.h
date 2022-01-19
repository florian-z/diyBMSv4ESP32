#ifndef _EEPROM_UTIL_H_
#define _EEPROM_UTIL_H_

#include <stdint.h>

typedef enum configs { VOLT_CALIB } configs_t;

uint16_t get_config(configs_t config);
void set_config(configs_t config, uint16_t value);

#endif