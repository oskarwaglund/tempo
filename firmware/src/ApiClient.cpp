#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include "ApiClient.h"

void ApiClient::Config(String deviceId, String apiUrl, String apiKey)
{
    _deviceId = deviceId;
    _apiUrl = apiUrl;
    _apiKey = apiKey;
}

bool ApiClient::PostTemperature(float temperature)
{
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

    Serial.print("Request body: ");
    Serial.println(body);

    int httpCode = https.POST(body);
    if (httpCode > 0)
    {
        String responseBody = https.getString();
        Serial.printf("Got HTTP code %d\n", httpCode);
        Serial.print("Response body: ");
        Serial.println(responseBody);
    }
    else
    {
        Serial.printf("Request failed: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();

    return httpCode == 200;
}
