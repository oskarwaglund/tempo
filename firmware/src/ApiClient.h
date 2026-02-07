#pragma once
#include <Arduino.h>

class ApiClient
{
public:
    void Config(String deviceId, String apiUrl, String apiKey);
    bool PostTemperature(float temperature);

private:
    String _deviceId;
    String _apiUrl;
    String _apiKey;
};