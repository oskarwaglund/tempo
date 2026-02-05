#include <EEPROM.h>

#include "ConfigStorage.h"

#define CASE_FIELD(itemName) \
    case itemName: \
        address = offsetof(MemoryStructure, itemName); \
        size = sizeof(MemoryStructure::itemName); \
        break;

struct MemoryStructure
{
    char DEVICE_ID[4]; //A number e.g. 1
    char WIFI_SSID[32];
    char WIFI_PW[32];
    char API_URL[64];
    char API_KEY[32];
};

void ConfigStorage::Initialize()
{
    EEPROM.begin(sizeof(MemoryStructure));
    Serial.printf("Initialized %u bytes of EEPROM for config storage!\n", sizeof(MemoryStructure));
}

void ConfigStorage::WriteItem(StorageItem item, String value)
{
    int itemAddress;
    int itemSize;
    GetAddressAndSizeOf(item, itemAddress, itemSize);
    Serial.println();
    Serial.printf("Trying to write %s to addr %i (%i)...\n", value.c_str(), itemAddress, itemSize);
    int valueStringLength = value.length();
    if(valueStringLength >= itemSize)
    {
        Serial.printf("ERROR: %i >= %i\n", value.length(), itemSize);
    }
    else
    {
        for(int i = 0; i < valueStringLength; i++)
        {
            EEPROM.write(itemAddress+i, value[i]);
        }
        EEPROM.write(itemAddress + value.length(), 0);
        EEPROM.commit();
        Serial.printf("Wrote %i bytes to EEPROM\n", value.length()+1);
    }
}

String ConfigStorage::ReadItem(StorageItem item)
{
    int itemAddress;
    int itemSize;
    GetAddressAndSizeOf(item, itemAddress, itemSize);
    Serial.printf("Trying to read from addr %i (%i)...\n", itemAddress, itemSize);

    String value;
    value.reserve(itemSize);
    int offset = 0;
    while(offset < itemSize)
    {
        char c = EEPROM.read(itemAddress + offset);
        if(c == '\n' || c == 0 || c == 0xFF)
        {
            break;
        }
        value += c;
        offset++;
    }
    return value;
}

void ConfigStorage::GetAddressAndSizeOf(StorageItem item, int &address, int &size)
{
    switch(item)
    {
        CASE_FIELD(DEVICE_ID)
        CASE_FIELD(WIFI_SSID)
        CASE_FIELD(WIFI_PW)
        CASE_FIELD(API_URL)
        CASE_FIELD(API_KEY)
        default:
            Serial.println("Unknown StorageItem requested!");
    }
}

void ConfigStorage::Reset()
{
    Serial.println("Resetting configuration...");
    WriteItem(DEVICE_ID, "");
    WriteItem(WIFI_SSID, "");
    WriteItem(WIFI_PW, "");
    WriteItem(API_URL, "");
    WriteItem(API_KEY, "");
}