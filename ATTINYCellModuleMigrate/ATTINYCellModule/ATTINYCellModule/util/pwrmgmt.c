#include "pwrmgmt.h"
#include "main.h"

void pwrmgmt_sleep_idle() {
  MCUCR = _BV(SE); // enable sleep instruction, select idle
  __builtin_avr_sleep(); // start sleep (sleep instruction)
  MCUCR &= ~_BV(SE); // disable sleep instruction

//Idle Mode
//This sleep mode basically halts clkCPU and clkFLASH, while allowing other clocks to run. In Idle Mode, the CPU is stopped
//but the following peripherals continue to operate:
// Watchdog and interrupt system
// Analog comparator, and ADC
// USART, TWI, and timer/counters
//
//Idle mode allows the MCU to wake up from external triggered interrupts as well as internal ones, such as Timer Overflow.
//If wake-up from the analog comparator interrupt is not required, the analog comparator can be powered down by setting
//the ACD bit in ACSRA. See “ACSR1A – Analog Comparator 1 Control and Status Register” on page 129. This will reduce power consumption in Idle mode.
//If the ADC is enabled, a conversion starts automatically when this mode is entered.
}

void pwrmgmt_sleep_adcnoisereduction() {
  MCUCR = _BV(SE) | _BV(SM0); // enable sleep instruction, select ADC-noise-reduction
  __builtin_avr_sleep(); // start sleep (sleep instruction)
  MCUCR &= ~_BV(SE); // disable sleep instruction

  //ADC Noise Reduction Mode
  //This sleep mode halts clkI/O, clkCPU, and clkFLASH, while allowing other clocks to run. In ADC Noise Reduction mode, the
  //CPU is stopped but the following peripherals continue to operate:
  // Watchdog (if enabled), and external interrupts
  // ADC
  // USART start frame detector, and TWI
  //
  //This improves the noise environment for the ADC, enabling higher resolution measurements. If the ADC is enabled, a
  //conversion starts automatically when this mode is entered.
  //The following events can wake up the MCU:
  // Watchdog reset, external reset, and brown-out reset
  // External level interrupt on INT0, and pin change interrupt
  // ADC conversion complete interrupt, and SPM/EEPROM ready interrupt
  // USART start frame detection, and TWI slave address match
}