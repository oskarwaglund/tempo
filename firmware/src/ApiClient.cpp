#include <ESP8266WiFi.h> 
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include "ApiClient.h"

void ApiClient::Connect(String ssid, String pw)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pw.c_str());
    Serial.printf("Connecting to %s:%s...\n", ssid.c_str(), pw.c_str());
    
    while(WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println();

    Serial.println("Connected!");
    Serial.print("Local IP address is ");
    Serial.println(WiFi.localIP());
}

void ApiClient::Config(String deviceId, String apiUrl, String apiKey)
{
    _deviceId = deviceId;
    _apiUrl = apiUrl;
    _apiKey = apiKey;
}

bool ApiClient::PostTemperature(float temperature)
{
    Serial.println("Posting temperature...");
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient https;

    if (!https.begin(client, _apiUrl))
        return false;

    https.addHeader("Content-Type", "application/json");
    https.addHeader("X-API-Key", _apiKey);

    String body;
    body.reserve(96);
    body += "{\"deviceId\":";
    body += _deviceId;
    body += ",\"temperature\":";
    body += String(temperature, 2);
    body += "}";

    Serial.printf("Posting %u bytes...\n", body.length());
    Serial.println(body);

    int httpCode = https.POST(body);
    if (httpCode > 0)
    {
        Serial.printf("HTTP status: %d\n", httpCode);

        String responseBody = https.getString();
        Serial.println("Response body:");
        Serial.println(responseBody);
    }
    else
    {
        Serial.printf("Request failed: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();

    return (httpCode == 200);
}
