#include "eeprom.h"
#include "main.h"
#include <avr/eeprom.h>

static EEMEM uint16_t volt_calib;

uint16_t get_config(configs_t config) {
  uint16_t* read_ptr;
  switch(config) {
    case VOLT_CALIB:
      read_ptr = &volt_calib;
      break;
  }
  return eeprom_read_word(read_ptr);
}

void set_config(configs_t config, uint16_t value) {
  uint16_t* write_ptr;
  switch(config) {
    case VOLT_CALIB:
    write_ptr = &volt_calib;
    break;
  }
  eeprom_write_word(write_ptr, value);
}