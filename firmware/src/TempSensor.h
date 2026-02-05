#pragma once

class TempSensor
{
public:
    float GetTemperature();
private:
    float Read();
};