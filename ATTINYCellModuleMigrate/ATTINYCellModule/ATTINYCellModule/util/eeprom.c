#include "eeprom.h"
#include "main.h"
#include <avr/eeprom.h>

static EEMEM float volt_calib;
static EEMEM uint16_t temp1_b_coeff;
static EEMEM uint16_t temp2_b_coeff;

float get_config_voltcalib() {
  if (eeprom_read_dword((uint32_t*)&volt_calib) == UINT32_MAX) {
    return 0.0;
  }
  return eeprom_read_float(&volt_calib);
}

uint16_t get_config_temp1bcoeff() {
  if (eeprom_read_word(&temp1_b_coeff) == UINT16_MAX) {
    return 0;
  }
  return eeprom_read_word(&temp1_b_coeff);
}

uint16_t get_config_temp2bcoeff() {
  if (eeprom_read_word(&temp2_b_coeff) == UINT16_MAX) {
    return 0;
  }
  return eeprom_read_word(&temp2_b_coeff);
}

void set_config(configs_t config, void* value) {
  switch(config) {
    case EEPROM_VOLT_CALIB:
      eeprom_write_float(&volt_calib, *(float*)value);
      break;
    case EEPROM_TEMP1_B_COEFF:
      eeprom_write_word(&temp1_b_coeff, *(uint16_t*)value);
      break;
    case EEPROM_TEMP2_B_COEFF:
      eeprom_write_word(&temp2_b_coeff, *(uint16_t*)value);
      break;
  }
}

void clear_config() {
  eeprom_write_dword((uint32_t*)&volt_calib, UINT32_MAX);
  eeprom_write_word(&temp1_b_coeff, UINT16_MAX);
  eeprom_write_word(&temp2_b_coeff, UINT16_MAX);
}