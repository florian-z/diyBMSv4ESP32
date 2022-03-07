#ifndef _EEPROM_UTIL_H_
#define _EEPROM_UTIL_H_

#include <stdint.h>

typedef enum configs { EEPROM_VOLT_CALIB, EEPROM_TEMP1_B_COEFF, EEPROM_TEMP2_B_COEFF } configs_t;

float get_config_voltcalib();
uint16_t get_config_temp1bcoeff();
uint16_t get_config_temp2bcoeff();

void set_config(configs_t config, void* value);
void clear_config();

#endif