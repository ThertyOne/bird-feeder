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
    Start,
    Unknown,
    // Initialization commands
    init,
    init_debug,
    init_full,
    // Action commands
    Photo,
    Status
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

private:
    String sendPhotoViaTelegram(uint8_t* jpgBuffer, size_t jpgSize);
    TelegramCommand parseCommand(String text);
};

#endif