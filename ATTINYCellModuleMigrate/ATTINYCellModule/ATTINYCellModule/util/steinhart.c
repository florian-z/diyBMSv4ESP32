#include "steinhart.h"
#include "main.h"

int8_t thermistorToCelcius(const uint16_t b_coeff, const uint16_t raw_adc) {
  //The thermistor is connected in series with another 47k resistor
  //and across the 2.048V reference giving 50:50 weighting

  //We can calculate the  Steinhart-Hart Thermistor Equation based on the B Coefficient of the thermistor
  // at 25 degrees C rating
  #define NOMINAL_TEMPERATURE 25

  //If we get zero its likely the ADC is connected to ground
  if (raw_adc>0){
    //47000 = resistor value
    //https://arduinodiy.wordpress.com/2015/11/10/measuring-temperature-with-ntc-the-steinhart-hart-formula/
    //float Resistance = 47000.0 * (1023.0F/(float)RawADC - 1.0);
    //float steinhart;
    //steinhart = Resistance / 47000.0; // (R/Ro)

    float steinhart = (1023.0F/(float)raw_adc - 1.0);

    steinhart = log(steinhart); // ln(R/Ro)
    steinhart /= b_coeff; // 1/B * ln(R/Ro)
    steinhart += 1.0 / (NOMINAL_TEMPERATURE + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart; // Invert
    steinhart -= 273.15; // convert to oC

    return (int8_t)steinhart;

    //Temp = log(Temp);
    //Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]^3}
    //Temp = 1.0 / (A + (B*Temp) + (C * Temp * Temp * Temp ));
  }

  return (int8_t)-99;
}