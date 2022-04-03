#include "pwrmgmt.h"
#include "../main.h"

/*
 * RUN
 * IDLE
 * ADC noise reduction
 * Standy
 * Power-down
 */

void pwrmgmt_sleep_idle() {
  __builtin_avr_cli();
  MCUCR = _BV(SE) | SLEEP_MODE_IDLE; // enable sleep instruction, select idle
  __builtin_avr_sei();
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
  __builtin_avr_cli();
  MCUCR = _BV(SE) | SLEEP_MODE_ADC; // enable sleep instruction, select ADC-noise-reduction
  __builtin_avr_sei();
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

void pwrmgmt_sleep_standby() {
  __builtin_avr_cli();
  MCUCR = _BV(SE) | SLEEP_MODE_STANDBY; // enable sleep instruction, select idle
  __builtin_avr_sei();
  __builtin_avr_sleep(); // start sleep (sleep instruction)
  MCUCR &= ~_BV(SE); // disable sleep instruction

  //Standby Mode
  //Standby Mode is identical to power-down, with the exception that the oscillator is kept running. From Standby mode, the
  //device wakes up in six clock cycles.
  //Only recommended with external crystal or resonator as clock source.
}

void pwrmgmt_sleep_powerdown() {
  __builtin_avr_cli();
  MCUCR = _BV(SE) | SLEEP_MODE_PWR_DOWN; // enable sleep instruction, select idle
  __builtin_avr_sei();
  __builtin_avr_sleep(); // start sleep (sleep instruction)
  MCUCR &= ~_BV(SE); // disable sleep instruction

  //Power-Down Mode
  //This sleep mode halts all generated clocks, allowing operation of asynchronous modules, only. In Power-down Mode the
  //oscillator is stopped, while the following peripherals continue to operate:
  // Watchdog (if enabled), external interrupts
  //
  //The following events can wake up the MCU:
  // Watchdog reset, external reset, and brown-out reset
  // External level interrupt on INT0, and pin change interrupt
  // USART start frame detection, and TWI slave address match
}