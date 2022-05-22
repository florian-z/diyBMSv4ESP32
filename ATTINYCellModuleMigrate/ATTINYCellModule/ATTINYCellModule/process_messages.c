#include "process_messages.h"
#include "main.h"
#include "util/eeprom.h"
#include "util/adc.h"
#include "util/steinhart.h"
#include "util/uart.h"
#include <string.h>

/** Read config from EEPROM - Only overwrite default config values, if EEPROM is not all zero bytes */
#define VOLT_CALIB_DEFAULT (1.25 / 1023.0) /* see get_batt_millivolt() */
#define TEMP1_B_COEFF_DEFAULT 3950
#define TEMP2_B_COEFF_DEFAULT 3950
static float volt_calib = VOLT_CALIB_DEFAULT;
static uint16_t temp1_b_coeff = TEMP1_B_COEFF_DEFAULT;
static uint16_t temp2_b_coeff = TEMP2_B_COEFF_DEFAULT;
void load_config_from_eeprom() {
  float volt_calib_from_eeprom = get_config_voltcalib();
  if (volt_calib_from_eeprom) {
    volt_calib = volt_calib_from_eeprom;
  } else {
    volt_calib = VOLT_CALIB_DEFAULT;
  }
  
  uint16_t temp_b_coeff_from_eeprom = get_config_temp1bcoeff();
  if (temp_b_coeff_from_eeprom) {
    temp1_b_coeff = temp_b_coeff_from_eeprom;
  } else {
    temp1_b_coeff = TEMP1_B_COEFF_DEFAULT;
  }
  
  temp_b_coeff_from_eeprom = get_config_temp2bcoeff();
  if (temp_b_coeff_from_eeprom) {
    temp2_b_coeff = temp_b_coeff_from_eeprom;
  } else {
    temp2_b_coeff = TEMP2_B_COEFF_DEFAULT;
  }
}

/** Read sensors */
uint16_t get_batt_millivolt_uint16() {
  float adc_result = read_adc_channel_multiple(BATT, 10);
  // ADC = Vin * 1024 / Vref
  // default value: 
  //static float volt_calib = 1.25 / 1023.0; // 0x03FF is VREF minus one LSB
  float voltage = adc_result * volt_calib;
  // *3.5185185 voltage divider
  // *1000 mV/V
  voltage *= 3518.5185;
  return (uint16_t) voltage; // is millivolts
}

float get_batt_volt_float() {
  float adc_result = read_adc_channel_multiple(BATT, 10);
  // ADC = Vin * 1024 / Vref
  // default value:
  //static float volt_calib = 1.25 / 1023.0; // 0x03FF is VREF minus one LSB
  float voltage = adc_result * volt_calib;
  // *3.5185185 voltage divider
  voltage *= 3.5185185;
  return voltage; // is volts
}

uint8_t get_temp1_degC() {
  float adc_result = read_adc_channel_multiple(TEMP1, 1);
  // ADC = Vin * 1024 / Vref
  return (uint8_t) thermistorToCelcius(temp1_b_coeff, adc_result);
}

uint8_t get_temp2_degC() {
  float adc_result = read_adc_channel_multiple(TEMP2, 1);
  // ADC = Vin * 1024 / Vref
  return (uint8_t) thermistorToCelcius(temp2_b_coeff, adc_result);
}

/** Decode / Encode bytes */
uint8_t decode_nibble(const uint8_t nibble_char) {
  return nibble_char < 'A' ? nibble_char - '0' : nibble_char + 10 - 'A';
}
uint8_t parse_chars_to_byte(const uint8_t* bytes) {
  uint8_t result = decode_nibble(bytes[0]) * 16;
  result += decode_nibble(bytes[1]);
  return result;
}
uint16_t parse_chars_to_word(const uint8_t* bytes) {
  uint16_t result = decode_nibble(bytes[0]) * 4096;
  result += decode_nibble(bytes[1]) * 256;
  result += decode_nibble(bytes[2]) * 16;
  result += decode_nibble(bytes[3]);
  return result;
}
uint8_t encode_nibble(const uint8_t nibble_value) {
  return nibble_value < 10 ? nibble_value + '0' : nibble_value - 10 + 'A';
}
void format_byte_to_chars(uint8_t* bytes, const uint8_t data) {
  bytes[0] = encode_nibble(data/16);
  bytes[1] = encode_nibble(data%16);
}
void format_word_to_chars(uint8_t* bytes, const uint16_t data) {
  bytes[0] = encode_nibble(data/4096);
  bytes[1] = encode_nibble((data%4096)/256);
  bytes[2] = encode_nibble((data%256)/16);
  bytes[3] = encode_nibble(data%16);
}

void format_dword_to_chars(uint8_t* bytes, const uint32_t data) {
  bytes[0] = encode_nibble(data/268435456);
  bytes[1] = encode_nibble((data%268435456)/16777216);
  bytes[2] = encode_nibble((data%16777216)/1048576);
  bytes[3] = encode_nibble((data%1048576)/65536);
  bytes[4] = encode_nibble((data%65536)/4096);
  bytes[5] = encode_nibble((data%4096)/256);
  bytes[6] = encode_nibble((data%256)/16);
  bytes[7] = encode_nibble(data%16);
}

/** MSG tools */
uint8_t calc_crc(const uint8_t* msg) {
  uint8_t crc = 0;
  if(MSG_START == *msg) {
    msg++;
  }
  while(*msg && MSG_END!=*msg && MSG_CRC!=*msg) {
    crc ^= *msg++;
  }
  return crc;
}

void msg_add_crc_and_end(uint8_t* msg) {
  uint8_t* ptr = msg;
  while(*++ptr);
  ptr[0] = MSG_CRC;
  format_byte_to_chars(&ptr[1], calc_crc(msg));
  ptr[3] = MSG_END;
}

bool is_msg_syntax_valid(const uint8_t* const msg) {
  const uint8_t* ptr = msg;
  // check first byte is MSG_START
  if(MSG_START != *ptr) {
    return false;
  }  
  
  // check message contains MSG_CRC
  while(*ptr && MSG_CRC != *ptr) {
    ptr++;
  }
  if(MSG_CRC != *ptr) {
    return false;
  }
  // check message crc
  if (calc_crc(msg) != parse_chars_to_byte(++ptr)) {
    return false;
  }
  //// check message ends with MSG_END
  //while(*ptr && MSG_END != *ptr) {
    //ptr++;
  //}
  //if(MSG_END != *ptr) {
    //return false;
  //}
  //if('\0' != *++ptr) {
    //return false;
  //}
  return true;
}



/** store and process message */
#define MSG_IN_LEN MSG_LEN
static uint8_t msg[MSG_IN_LEN] = {0};
static bool new_msg = false;

void incoming_msg(const uint8_t * const msg_in, const uint8_t len) {
  memcpy(msg, msg_in, len);
  new_msg = true;
}

void process_message() {
  if(new_msg) {
    new_msg = false;
    if(!is_msg_syntax_valid(msg)) {
      // message has invalid syntax or crc
      LED_RED_ON
      set_processing_done();
      return;
    }
    // message has valid syntax and crc
    
    set_disable_deepsleep(); // reception of a valid message will disable deepsleep
    
    // clear MSG_CRC, <crc> and MSG_END
    {
      uint8_t* ptr = msg;
      while(MSG_CRC != *++ptr);
      *ptr++ = '\0'; // '*'
      *ptr++ = '\0'; // <crc1>
      *ptr++ = '\0'; // <crc2>
      *ptr++ = '\0'; //'\r'
      *ptr = '\0';   //'\n'
    }

    // process message
    uint8_t command = parse_chars_to_byte(&msg[MSG_CMD]);
    uint8_t mod_cnt = parse_chars_to_byte(&msg[MSG_MOD_CNT]);
    switch(command) {
      
      case GET_BATT_VOLT:
        // append value and send
        format_word_to_chars(&msg[MSG_DATA_BEGIN]+mod_cnt*4, get_batt_millivolt_uint16());
        // TODO check max message length
        // TODO check static/dynamic RAM usage (512bytes max)
        break;
        
      case GET_TEMP:
        // append value and send
        format_word_to_chars(&msg[MSG_DATA_BEGIN]+mod_cnt*4, get_temp1_degC()*256+get_temp2_degC());
        // TODO check max message length
        // TODO check static/dynamic RAM usage (512bytes max)
        break;
        
      case IDENTIFY_MODULE:
        if (!mod_cnt) {
          // message is for this module -> identify
          set_identify_module();
        }
        break;
        
      case ACTIVATE_POWERSAVE:
        LED_RED_OFF // TODO flo
        set_enable_deepsleep();
        break;
        
      case SET_CONFIG_BATT_VOLT_CALIB:
        if (!mod_cnt && strlen((char*)msg)==9) {
          // message is for this module -> calc and store in eeprom
          volt_calib = VOLT_CALIB_DEFAULT; // set volt_calib to default value for reference measurement
          float adc_volt_default = get_batt_volt_float(); // reference measurement
          float adc_volt_target = parse_chars_to_word(&msg[MSG_DATA_BEGIN]) / 1000.0;
          float volt_calib_new = adc_volt_target / adc_volt_default * VOLT_CALIB_DEFAULT;
          set_config(EEPROM_VOLT_CALIB, &volt_calib_new);
          load_config_from_eeprom();
          set_identify_module();
          // +4 recv target volts
          format_word_to_chars(&msg[MSG_DATA_BEGIN]+4, (adc_volt_default*1000.0)/1);
          // +4 recv target volts +4 adc reading with default volt_calib
          format_word_to_chars(&msg[MSG_DATA_BEGIN]+8, get_batt_millivolt_uint16());
          // +4 recv target volts +4 adc reading with default volt_calib +4 adc reading with new volt_calib
          format_dword_to_chars(&msg[MSG_DATA_BEGIN]+12, volt_calib*1e11);
        }
        break;
        
      case SET_CONFIG_TEMP1_B_COEFF:
        if (!mod_cnt && strlen((char*)msg)==9) {
          // message is for this module -> store in eeprom
          uint8_t temp_before = get_temp1_degC(); // with old temp_b_coeff
          uint16_t temp_b_coeff_from_msg = parse_chars_to_word(&msg[MSG_DATA_BEGIN]);
          set_config(EEPROM_TEMP1_B_COEFF, &temp_b_coeff_from_msg);
          load_config_from_eeprom();
          set_identify_module();
          uint8_t temp_after = get_temp1_degC(); // with new temp_b_coeff
          // +4 recv temp_b_coeff
          format_word_to_chars(&msg[MSG_DATA_BEGIN]+4, temp1_b_coeff);
          // +4 recv temp_b_coeff +4 confirm temp_b_coeff
          format_word_to_chars(&msg[MSG_DATA_BEGIN]+4+4, temp_before*256+temp_after);
        }
        break;
        
      case SET_CONFIG_TEMP2_B_COEFF:
        if (!mod_cnt && strlen((char*)msg)==9) {
          // message is for this module -> store in eeprom
          uint8_t temp_before = get_temp2_degC(); // with old temp_b_coeff
          uint16_t temp_b_coeff_from_msg = parse_chars_to_word(&msg[MSG_DATA_BEGIN]);
          set_config(EEPROM_TEMP2_B_COEFF, &temp_b_coeff_from_msg);
          load_config_from_eeprom();
          set_identify_module();
          uint8_t temp_after = get_temp2_degC(); // with new temp_b_coeff
          // +4 recv temp_b_coeff
          format_word_to_chars(&msg[MSG_DATA_BEGIN]+4, temp2_b_coeff);
          // +4 recv temp_b_coeff +4 confirm temp_b_coeff
          format_word_to_chars(&msg[MSG_DATA_BEGIN]+4+4, temp_before*256+temp_after);
        }
        break;
        
      case CLEAR_CONFIG:
        if (!mod_cnt) {
          // message is for this module -> clear data in eeprom
          clear_config();
          load_config_from_eeprom();
          set_identify_module();
        }
        // no break, here -> this should give the same output as GET_CONFIG
      case GET_CONFIG:
        if (!mod_cnt) {
          // message is for this module -> give current settings
          //format_word_to_chars(&msg[MSG_DATA_BEGIN], volt_calib*1000000);
          //format_word_to_chars(&msg[MSG_DATA_BEGIN]+4, (volt_calib*1000000.0 - (uint32_t)(volt_calib*1000000))*1000.0);
          format_dword_to_chars(&msg[MSG_DATA_BEGIN], volt_calib*1e11);
          // +8 volt_calib
          format_word_to_chars(&msg[MSG_DATA_BEGIN]+8, temp1_b_coeff);
          // +8 volt_calib +4 temp1_b_coeff
          format_word_to_chars(&msg[MSG_DATA_BEGIN]+12, temp2_b_coeff);
        }
        break;
    }
    
    mod_cnt++;
    format_byte_to_chars(&msg[MSG_MOD_CNT], mod_cnt);
    msg_add_crc_and_end(msg);
    outgoing_msg(msg, strlen((char*)msg));
    memset(msg, '\0', MSG_IN_LEN);
  }  
}


///** test program */
//void proc_msg_test() {
  ////get_batt_volt_float();
  //format_word_to_chars(&msg[0], get_batt_millivolt_uint16());
  //msg[1*4] = ' ';
  ////get_temp1_degC();
  ////get_temp2_degC();
  //format_word_to_chars(&msg[0]+1*4+1, get_temp1_degC()*256+get_temp2_degC());
  ////msg[2*4+1] = ' ';
  //
  //msg[2*4+1] = '\n';
  //outgoing_msg(msg, strlen((char*)msg));
  //memset(msg, '\0', MSG_IN_LEN);
  //
  //
  //for(uint8_t i = 0; i<17; i++) {
    //format_byte_to_chars(&msg[0], i);
    //
    //msg[2] = parse_chars_to_byte(msg);
    //msg[3] = '\n';
    //
    //outgoing_msg(msg, strlen((char*)msg));
    //memset(msg, '\0', MSG_IN_LEN);
  //}    
//}