#include <Arduino.h>

#include "TempSensor.h"

float TempSensor::GetTemperature()
{
    const int N_READINGS = 10;
    float temperature = 0;
    for(int i = 0; i < 10; i++)
    {
        temperature += Read();
        delay(10);
    }
    temperature /= N_READINGS;
    return temperature;
}

float TempSensor::Read()
{
    const float ADC_MAX_VOLTAGE = 3.0;
    const int ADC_STEPS = 1023;
    
    int adcReading = analogRead(A0);
    float voltage = adcReading * (ADC_MAX_VOLTAGE / ADC_STEPS);
    
    //These values are for a TMP36 TO-92 sensor
    const int BASE_TEMPERATURE = 25; //°C
    const float BASE_VOLTAGE = 0.75; //V
    const float SCALE_FACTOR = 100; //°C/V

    float temperature = BASE_TEMPERATURE + (voltage-BASE_VOLTAGE)*SCALE_FACTOR;
    return temperature;
}