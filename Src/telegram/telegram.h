// Src/telegram/telegram.h

#ifndef TELEGRAM_H
#define TELEGRAM_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "../config.h"

enum class TelegramCommand {
    // Basic commands
    None,
    Start,                  // not implemented yet
    Unknown,
    Help,
    Status,                 // not implemented yet

    // Initialization commands
    init,
    init_debug,
    init_full,

    // Test commands
    Test_pir,
    WiFi_status,            // not implemented yet
    Test_camera,            // not implemented yet
    Config_info,            // not implemented yet

    // Action commands
    Run_silent,             // not implemented yet
    Run_debug,              // not implemented yet
    Stop,
    Change_configuration,   // not implemented yet
};

class TelegramManager {
private:
    WiFiClientSecure clientTCP;
    UniversalTelegramBot* bot = nullptr;
    
public:
    TelegramManager();
    ~TelegramManager();
    
    void begin();
    bool sendMessage(const String& message);
    bool sendPhoto(uint8_t* jpgBuffer, size_t jpgSize);
    TelegramCommand handleMessages();
    void clearPendingMessages(uint8_t rounds = 3);

    static const char* commandToString(TelegramCommand command);

private:
    String sendPhotoViaTelegram(uint8_t* jpgBuffer, size_t jpgSize);
    TelegramCommand parseCommand(String text);
};

#endif