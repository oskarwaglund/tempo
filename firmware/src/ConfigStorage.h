#pragma once
#include <Arduino.h>

enum StorageItem
{
    DEVICE_ID,
    WIFI_SSID,
    WIFI_PW,
    API_URL,
    API_KEY
};

class ConfigStorage
{
public:
    static void Initialize();
    void WriteItem(StorageItem item, String value);
    String ReadItem(StorageItem item);
    void Reset();

private:
    void GetAddressAndSizeOf(StorageItem item, int &address, int &size);
};