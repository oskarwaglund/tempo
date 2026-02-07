#pragma once

#include <Arduino.h>

class ConfigShell
{
public:
    void Initialize();

private:
    void ConfigMode();

    void PrintHelp();

    String AwaitCommand();
    void ProcessCommand(String command, bool &exitRequested);
    bool IsValidChar(char c);

    static constexpr char C_HELP = 'h';
    static constexpr char C_QUIT = 'q';
    static constexpr char C_RESET = 'r';
    static constexpr char C_SHOW_CONFIG = 's';

    static constexpr char C_DEVICE_ID = 'd';
    static constexpr char C_WIFI_SSID = 'w';
    static constexpr char C_WIFI_PW = 'p';
    static constexpr char C_API_URL = 'a';
    static constexpr char C_API_KEY = 'k';
};