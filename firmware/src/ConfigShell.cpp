#include <Arduino.h>

#include "ConfigStorage.h"
#include "ConfigShell.h"

void ConfigShell::Initialize()
{
    const int SERIAL_CONFIG_REQUEST_TIME = 5; // s
    int secondsRemaining = SERIAL_CONFIG_REQUEST_TIME;
    bool enterConfigMode = false;
    while (secondsRemaining >= 0)
    {
        Serial.printf("Press enter to start config shell, booting in %i...\n", secondsRemaining);
        secondsRemaining--;
        delay(1000);
        if (Serial.available())
        {
            enterConfigMode = true;
            break;
        }
    }
    if (enterConfigMode)
    {
        Serial.println("Entering serial config mode...");
        while(Serial.available())
        {
            Serial.read();
        }
        ConfigMode();
    }
    Serial.println("Leaving serial config...");
}

void ConfigShell::PrintHelp()
{
    Serial.println("*** Config shell ***");
    Serial.println("Commands:");
    Serial.printf("%c                   - Show this help\n", C_HELP);
    Serial.printf("%c                   - Print current config\n", C_SHOW_CONFIG);
    Serial.printf("%c                   - Reset (!) configuration\n", C_RESET);
    Serial.println();

    Serial.printf("%c <Device ID>       - Set device ID\n", C_DEVICE_ID);
    Serial.printf("%c <SSID>            - Set WiFi SSID\n", C_WIFI_SSID);
    Serial.printf("%c <Password>        - Set WiFi password\n", C_WIFI_PW);
    Serial.printf("%c <API URL>         - Set API URL\n", C_API_URL);
    Serial.printf("%c <API key>         - Set API key\n", C_API_KEY);
    Serial.println();

    Serial.printf("%c                   - Quit and start loop\n", C_QUIT);
    Serial.println();
}

void ConfigShell::ConfigMode()
{
    bool exitRequested = false;
    PrintHelp();
    while (!exitRequested)
    {
        String command = AwaitCommand();
        Serial.flush();
        delay(100);
        ProcessCommand(command, exitRequested);
    }
}

bool ConfigShell::IsValidChar(char c)
{
    return c >= 0x20 && c <= 0x7E;
}

String ConfigShell::AwaitCommand()
{
    String command = "";
    bool commandCompleted = false;
    while (!commandCompleted)
    {
        while (Serial.available())
        {
            char c = Serial.read();
            if (c == '\n')
            {
                commandCompleted = true;
            }
            else if (IsValidChar(c))
            {
                command += c;
            }
        }
        delay(50);
    }
    return command;
}

void ConfigShell::ProcessCommand(String command, bool &exitRequested)
{
    if(command.length() == 0)
    {
        return;
    }

    ConfigStorage configStorage;

    char operation = command[0];
    String parameter = command.substring(2);
    
    exitRequested = false;
    switch (operation)
    {
    case C_QUIT:
        exitRequested = true;
        break;
    case C_RESET:
        configStorage.Reset();
        break;
    case C_HELP:
        PrintHelp();
        break;

    case C_SHOW_CONFIG:
        Serial.println("Current config:");
        Serial.printf("Device name:   %s\n", configStorage.ReadItem(StorageItem::DEVICE_ID).c_str());
        Serial.printf("WiFi SSID:     %s\n", configStorage.ReadItem(StorageItem::WIFI_SSID).c_str());
        Serial.printf("WiFi Password: %s\n", configStorage.ReadItem(StorageItem::WIFI_PW).c_str());
        Serial.printf("API URL:       %s\n", configStorage.ReadItem(StorageItem::API_URL).c_str());
        Serial.printf("API key:       %s\n", configStorage.ReadItem(StorageItem::API_KEY).c_str());
        break;

    case C_DEVICE_ID:
        configStorage.WriteItem(StorageItem::DEVICE_ID, parameter);
        break;
    case C_WIFI_SSID:
        configStorage.WriteItem(StorageItem::WIFI_SSID, parameter);
        break;
    case C_WIFI_PW:
        configStorage.WriteItem(StorageItem::WIFI_PW, parameter);
        break;
    case C_API_URL:
        Serial.print("API URL: ");
        Serial.println(parameter);
        configStorage.WriteItem(StorageItem::API_URL, parameter);
        break;
    case C_API_KEY:
        configStorage.WriteItem(StorageItem::API_KEY, parameter);
        break;
    default:
        Serial.printf("Unknown command %c!\n", operation);
    }
}