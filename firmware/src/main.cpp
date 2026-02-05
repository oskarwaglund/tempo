#include <Arduino.h>

#include "TempSensor.h"
#include "ConfigShell.h"
#include "ConfigStorage.h"
#include "ApiClient.h"

ApiClient client;

void setup()
{
    Serial.begin(115200);
    delay(300);

    ConfigStorage::Initialize();

    ConfigShell configShell;
    configShell.Initialize();

    ConfigStorage config;

    String ssid = config.ReadItem(WIFI_SSID);
    String pw = config.ReadItem(WIFI_PW);
    String deviceId = config.ReadItem(DEVICE_ID);
    String apiKey = config.ReadItem(API_KEY);
    String apiUrl = config.ReadItem(API_URL);

    client.Connect(ssid, pw);
    client.Config(deviceId, apiUrl, apiKey);

    Serial.println();
    Serial.println("Starting main loop...");
}


const unsigned long READ_PERIOD_MS = 10000;
unsigned long nextReadEvent = 0;

void loop()
{
    unsigned long now = millis();
    if ((long)(now - nextReadEvent) >= 0) {
        nextReadEvent = now + READ_PERIOD_MS;
        
        TempSensor temp;
        float temperature = temp.GetTemperature();
        client.PostTemperature(temperature);
    }
    delay(1);
}
