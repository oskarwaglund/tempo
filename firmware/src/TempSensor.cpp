#include <Arduino.h>

#include "TempSensor.h"

float TempSensor::GetTemperature()
{
    if(!_hasValidData) return 0;

    float result = 0;
    for (int i = 0; i < nStoredReadings; i++)
    {
        result += _lastReads[i];
        Serial.println(_lastReads[i]);
    }
    result /= nStoredReadings;
    return result;
}

void TempSensor::Read()
{
    float currentTemp = CalculateTempFromAdc();
    if(_hasValidData)
    {
        _lastReads[_readIndex++] = currentTemp;
        _readIndex %= nStoredReadings;
    }
    else
    {
        for(int i = 0; i < nStoredReadings; i++)
        {
            _lastReads[i] = currentTemp;
        }
        _hasValidData = true;
    }
}

float TempSensor::CalculateTempFromAdc()
{
    const float ADC_MAX_VOLTAGE = 3.0;
    const int ADC_STEPS = 1023;
    
    int adcReading = analogRead(A0);
    float voltage = adcReading * (ADC_MAX_VOLTAGE / ADC_STEPS);
    
    //These values are for a TMP36 TO-92 sensor
    const int BASE_TEMPERATURE = 25; //°C
    const float BASE_VOLTAGE = 0.75; //V
    const float SCALE_FACTOR = 100; //°C/V

    return BASE_TEMPERATURE + (voltage-BASE_VOLTAGE)*SCALE_FACTOR;
}

bool TempSensor::HasValidData()
{
    return _hasValidData;
}