//USING CLOCKWISE PIN MAPPINGS
#define dump_enable 3
#define green_led 6
#define blue_led 5
#define enable_2048 7

#include <Arduino.h>

void setup()
{
  //Boot up will be in 1Mhz CKDIV8 mode, swap to /4 to change speed to 2Mhz
  //CCP – Configuration Change Protection Register
  CCP = 0xD8;
  //CLKPR – Clock Prescale Register
  CLKPR = _BV(CLKPS1);

  //below 2Mhz is required for running ATTINY at low voltages (less than 2V)

  //PUEA – Port A Pull-Up Enable Control Register (All disabled)
  PUEA = 0;
  //PUEB – Port B Pull-Up Enable Control Register (All disabled)
  PUEB = 0;

  //DDRA – Port A Data Direction Register
  //When DDAn is set, the pin PAn is configured as an output. When DDAn is cleared, the pin is configured as an input
  DDRA |= _BV(DDA1) | _BV(DDA6) | _BV(DDA7);
  //DDRB – Port B Data Direction Register
  //Spare pin is output
  DDRB |= _BV(DDB2);

  //Set the extra high sink capability of pin PA7 is enabled.
  PHDE |= _BV(PHDEA1);
}

void loop()
{
  PORTA |= _BV(PORTA6);
  PORTA |= _BV(PORTA1);
  delay(10);
  if(PINA & _BV(PORTA2)) {
    PORTB |= _BV(PORTB2);
  }
  
  delay(1000);
  PORTA &= (~_BV(PORTA6));
  PORTA &= (~_BV(PORTA1));
  
  if(PINA & _BV(PORTA2)) {
    PORTB &= (~_BV(PORTB2));
  }
  delay(1000);
}
