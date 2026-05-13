// Src/telegram/telegram.h

#ifndef TELEGRAM_H
#define TELEGRAM_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "../config.h"

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
    void handleMessages();
    
private:
    String sendPhotoViaTelegram(uint8_t* jpgBuffer, size_t jpgSize);
};

#endif