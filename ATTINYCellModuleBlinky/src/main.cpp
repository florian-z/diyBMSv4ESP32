#include <Arduino.h>
//USING CLOCKWISE PIN MAPPINGS

// A1 - TX
// A6 - LED BLUE
// A7 - REF. VOLTAGE
// B2 - LED RED
#define LED_BLU_ON PORTA |= _BV(PORTA6);
#define LED_RED_ON PORTB |= _BV(PORTB2);
#define REFVOLT_ON PORTA |= _BV(PORTA7);
#define LED_BLU_OFF PORTA &= (~_BV(PORTA6));
#define LED_RED_OFF PORTB &= (~_BV(PORTB2));
#define REFVOLT_OFF PORTA &= (~_BV(PORTA7));
#define READ_RX (PINA & _BV(PORTA2))
#define SET_TX PORTA |= _BV(PORTA1);
#define CLR_TX PORTA &= (~_BV(PORTA1));


void setup()
{
  /* 
   * Boot up will be in 1Mhz CKDIV8 mode from external 8MHz crystal. Swap to /4 to change speed to 2Mhz.
   * Below 2Mhz is required for running ATTINY at low voltages (less than 2V)
   */
  //CCP – Configuration Change Protection Register
  // protected: CLKPR, MCUCR, WDTCSR
  CCP = 0xD8;
  //CLKPR – Clock Prescale Register
  CLKPR = _BV(CLKPS1);
  // 2MHz clock ...
  

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
  delay(4000);
  LED_BLU_ON
  delay(4000);
  REFVOLT_ON
  delay(4000);
  SET_TX
  delay(4000);
  if(READ_RX) {
    LED_RED_ON
  }
  delay(1000);


  LED_BLU_OFF
  CLR_TX
  REFVOLT_OFF
  delay(1);
  if(!READ_RX) {
    LED_RED_OFF;
  }
  delay(1000);
}
