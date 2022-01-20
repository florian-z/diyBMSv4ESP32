#include "process_messages.h"
#include "main.h"
#include "eeprom.h"
#include "adc.h"
#include "steinhart.h"
#include "uart.h"
#include <string.h>

uint16_t get_batt_millivolt() {
  float adc_result = read_adc_channel_multiple(BATT, 10);
  // ADC = Vin * 1024 / Vref
  static float volt_calib = 1.25 / 1023.0; // 0x03FF is VREF minus one LSB
  //static float volt_calib = get_config(VOLT_CALIB);
  float voltage = adc_result * volt_calib;
  voltage *= 3.5185185;
  voltage *=1000;
  return (uint16_t) voltage;
}

uint8_t get_temp1_degC() {
  float adc_result = read_adc_channel_multiple(TEMP1, 1);
  // ADC = Vin * 1024 / Vref
  return (uint8_t) thermistorToCelcius(3950, adc_result);
}

uint8_t get_temp2_degC() {
  float adc_result = read_adc_channel_multiple(TEMP2, 1);
  // ADC = Vin * 1024 / Vref
  return (uint8_t) thermistorToCelcius(3950, adc_result);
}

uint8_t parse_chars_to_byte(uint8_t* bytes) {
  uint8_t result = (bytes[0]-'0') * 16;
  result += bytes[1]-'0';
  return result;
}
uint16_t parse_chars_to_word(uint8_t* bytes) {
  uint16_t result = (bytes[0]-'0') * 4096;
  result += (bytes[1]-'0') * 256;
  result += (bytes[2]-'0') * 16;
  result += (bytes[3]-'0');
  return result;
}
void format_byte_to_chars(uint8_t* bytes, const uint8_t data) {
  bytes[0] = (data/16)+'0';
  bytes[1] = (data%16)+'0';
}
void format_word_to_chars(uint8_t* bytes, const uint16_t data) {
  bytes[0] = (data/4096)+'0';
  bytes[1] = ((data%4096)/256)+'0';
  bytes[2] = ((data%256)/16)+'0';
  bytes[3] = (data%16)+'0';
  //bytes[0] = (data/1000)+'0';
  //bytes[1] = ((data%1000)/100)+'0';
  //bytes[2] = ((data%100)/10)+'0';
  //bytes[3] = (data%10)+'0';
}

//void format_short_to_word(uint8_t* bytes, uint16_t shortval) {
  //bytes[0] = (shortval/1000)<<4 | (shortval%1000)/100;
  //bytes[1] = ((shortval%100)/10)<<4 | shortval%10;
//}
//uint16_t format_word_to_short( uint8_t* bytes) {
  //uint16_t shortval = (bytes[0]>>4)*1000;
  //shortval += (bytes[0]&0x0F)*100;
  //shortval += (bytes[1]>>4)*10;
  //shortval += (bytes[1]&0x0F);
  //return shortval;
//}

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
  //if(MSG_START != *ptr) {
    //return false;
  //}
  
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

void process_message(uint8_t* const msg) {
  //if(new_msg_in) {
    if(!is_msg_syntax_valid(msg)) {
      // message has invalid syntax or crc
      // todo sleep idle
      return;
    }
    // message has valid syntax and crc
    
    // clear MSG_CRC, <crc> and MSG_END
    uint8_t* ptr = msg;
    while(MSG_CRC != *++ptr);
    {
      *ptr++ = '\0'; // '*'
      *ptr++ = '\0'; // <crc1>
      *ptr++ = '\0'; // <crc2>
      *ptr = '\0';   //'\n'
    }

    // process message
    uint8_t command = parse_chars_to_byte(&msg[MSG_CMD]);
    uint8_t mod_cnt = parse_chars_to_byte(&msg[MSG_MOD_CNT]);
    switch(command) {
      case GET_BATT_VOLT:
        format_word_to_chars(msg+5+mod_cnt*4, get_batt_millivolt());
        break;
        
      case GET_TEMP:
        format_word_to_chars(msg+5+mod_cnt*4, get_temp1_degC()*256+get_temp2_degC());
        // append value and send
        break;
        
      case SET_BATT_VOLT_CALIB:
        if (!mod_cnt) {
          // message is for this module -> calc and store in eeprom
          // todo;
          set_identify_module();
        }
        break;
        
      case IDENTIFY_MODULE:
        if (!mod_cnt) {
          // message is for this module -> identify
          set_identify_module();
        }
        break;
    }
    
    mod_cnt++;
    format_byte_to_chars(&msg[MSG_MOD_CNT], mod_cnt);
    msg_add_crc_and_end(msg);
    outgoing_msg(msg, strlen(msg));
  //}  
}