#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "TempSensor.h"
#include "ConfigShell.h"
#include "ConfigStorage.h"
#include "ApiClient.h"

ApiClient apiClient;
TempSensor temp;

bool schedulingSetupDone = false;

unsigned long readLast;
unsigned long readPeriod = 10000; // ms

unsigned long postLast;
unsigned long postPeriod = 60000; // ms
unsigned long postOffset = 5000;

void setupWifi(ConfigStorage &config);
void setupApiClient(ConfigStorage &config);

void setup()
{
    Serial.begin(115200);

    ConfigStorage::Initialize();

    ConfigShell configShell;
    configShell.Initialize();

    ConfigStorage config;
    setupWifi(config);
    setupApiClient(config);

    Serial.println("Starting main loop...");
}

void loop()
{
    unsigned long now = millis();
    if (!schedulingSetupDone)
    {
        readLast = now;
        postLast = now + postOffset - postPeriod;
        schedulingSetupDone = true;
    }

    if (now - postLast >= postPeriod)
    {
        postLast += postPeriod;
        if (temp.HasValidData())
        {
            float temperature = temp.GetTemperature();
            apiClient.PostTemperature(temperature);
        }
    }

    if (now - readLast >= readPeriod)
    {
        readLast += readPeriod;
        temp.Read();
        Serial.printf("Temp: %.2f\n", temp.GetTemperature());
    }

    delay(1);
}

void setupWifi(ConfigStorage &config)
{
    String ssid = config.ReadItem(WIFI_SSID);
    String pw = config.ReadItem(WIFI_PW);

    WiFi.mode(WIFI_STA);
    WiFi.setPhyMode(WIFI_PHY_MODE_11G);
    WiFi.begin(ssid, pw);
    Serial.printf("Connecting to %s:%s...\n", ssid.c_str(), pw.c_str());

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        yield();
    }
    Serial.println();

    Serial.println("Connected!");
    Serial.print("Local IP address is ");
    Serial.println(WiFi.localIP());
}

void setupApiClient(ConfigStorage &config)
{
    String deviceId = config.ReadItem(DEVICE_ID);
    String apiKey = config.ReadItem(API_KEY);
    String apiUrl = config.ReadItem(API_URL);
    apiClient.Config(deviceId, apiUrl, apiKey);
}