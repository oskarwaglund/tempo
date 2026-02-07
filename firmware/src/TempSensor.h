#pragma once

class TempSensor
{
public:
    float GetTemperature();
    void Read();
    bool HasValidData();
private:
    float CalculateTempFromAdc();

    static const int nStoredReadings = 6;

    int _readIndex;
    bool _hasValidData = false;
    float _lastReads[nStoredReadings];
};