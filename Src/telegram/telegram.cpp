// Src/telegram/telegram.cpp

#include "telegram.h"

TelegramManager::TelegramManager() {}

TelegramManager::~TelegramManager() {
    if (bot != nullptr) {
        delete bot;
    }
#if DEBUG_SERIAL_TELEGRAM
    //Serial.println("\n");
    //Serial.println("<<<----- DEBUG MODE ----->>>");
    Serial.println("[TELEGRAM] Checking bot instance...");
    if (bot == nullptr) {
        Serial.println("[TELEGRAM] Bot instance already nonexistent");
    } else {
        Serial.println("[TELEGRAM] Bot instance still exists, deleting...");
    }
    Serial.println("[TELEGRAM] Bot instance deleted");
    //Serial.println("<<<--- END DEBUG MODE --->>>\n");
#endif
}

void TelegramManager::begin() {
    clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT);
    bot = new UniversalTelegramBot(BOT_TOKEN, clientTCP);
    Serial.println("[TELEGRAM] Bot initialized");
#if DEBUG_SERIAL_TELEGRAM
    //Serial.println("\n");
    //Serial.println("<<<----- DEBUG MODE ----->>>");
    Serial.println("[TELEGRAM] Bot instance created");
    //Serial.println("<<<--- END DEBUG MODE --->>>\n");
#endif
}

bool TelegramManager::sendMessage(const String& message) {
    if (bot == nullptr) {
        Serial.println("[TELEGRAM] Bot not initialized");
        return false;
    }

#if DEBUG_SERIAL_TELEGRAM
    //Serial.println("\n");
    //Serial.println("<<<----- DEBUG MODE ----->>>");
    Serial.println("[TELEGRAM] Sending message: " + message + " to chat ID: " + String(CHAT_ID) + "...");
    //Serial.println("<<<--- END DEBUG MODE --->>>\n");
#endif

    bool successfully_sent = bot->sendMessage(CHAT_ID, message, "");

    if (successfully_sent) {
        Serial.println("[TELEGRAM] Message [" + message + "] sent");
    } else {
        Serial.println("[TELEGRAM] Message [" + message + "] sending failed");
    }

    return successfully_sent;
}

// temporary method - NOT TESTED
bool TelegramManager::sendPhoto(uint8_t* jpgBuffer, size_t jpgSize) {
    if (jpgBuffer == nullptr || jpgSize == 0) {
        Serial.println("[TELEGRAM] ✗ Invalid photo buffer");
        return false;
    }
    
    String response = sendPhotoViaTelegram(jpgBuffer, jpgSize);
    
    if (response.length() > 0 && response.indexOf("ok") > 0) {
        Serial.println("[TELEGRAM] ✓ Photo sent!");
        return true;
    } else {
        Serial.println("[TELEGRAM] ✗ Failed to send photo");
        Serial.println(response);
        return false;
    }
}

// temporary method - NOT TESTED
String TelegramManager::sendPhotoViaTelegram(uint8_t* jpgBuffer, size_t jpgSize) {
    const char* domain = "api.telegram.org";
    String getBody = "";
    
    if (clientTCP.connect(domain, 443)) {
        Serial.println("[TELEGRAM] Connected to api.telegram.org");
        
        String head = "--Boundary123\r\n";
        head += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n";
        head += String(CHAT_ID) + "\r\n";
        head += "--Boundary123\r\n";
        head += "Content-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\n";
        head += "Content-Type: image/jpeg\r\n\r\n";
        
        String tail = "\r\n--Boundary123--\r\n";
        
        size_t totalLen = head.length() + jpgSize + tail.length();
        
        clientTCP.println("POST /bot" + String(BOT_TOKEN) + "/sendPhoto HTTP/1.1");
        clientTCP.println("Host: " + String(domain));
        clientTCP.println("Content-Length: " + String(totalLen));
        clientTCP.println("Content-Type: multipart/form-data; boundary=Boundary123");
        clientTCP.println("Connection: close");
        clientTCP.println();
        clientTCP.print(head);
        
        // Wyślij zdjęcie po 1KB chunkami
        uint8_t* buf = jpgBuffer;
        size_t remaining = jpgSize;
        while (remaining > 0) {
            size_t toSend = (remaining > 1024) ? 1024 : remaining;
            clientTCP.write(buf, toSend);
            buf += toSend;
            remaining -= toSend;
        }
        
        clientTCP.print(tail);
        
        // Czytaj response
        unsigned long timeout = millis() + 10000;
        while (millis() < timeout && clientTCP.connected()) {
            while (clientTCP.available()) {
                char c = clientTCP.read();
                getBody += c;
            }
        }
        
        clientTCP.stop();
    } else {
        Serial.println("[TELEGRAM] ✗ Connection failed");
    }
    
    return getBody;
}

// temporary method - NOT TESTED
void TelegramManager::handleMessages() {
    if (bot == nullptr) return;
    
    int numNewMessages = bot->getUpdates(bot->last_message_received + 1);
    
    if (numNewMessages > 0) {
        Serial.print("[TELEGRAM] Got ");
        Serial.print(numNewMessages);
        Serial.println(" messages");
        
        for (int i = 0; i < numNewMessages; i++) {
            String text = bot->messages[i].text;
            String chatId = String(bot->messages[i].chat_id);
            
            // Sprawdź czy message z poprawnego chat_id
            if (chatId != String(CHAT_ID)) {
                bot->sendMessage(chatId, "❌ Unauthorized", "");
                continue;
            }
            
            if (text == "/start") {
                String welcome = "🐦 Bird Feeder System Ready!\n";
                welcome += "Commands:\n";
                welcome += "/photo - Take photo now\n";
                welcome += "/status - System status\n";
                bot->sendMessage(CHAT_ID, welcome, "");
            }
            else if (text == "/photo") {
                bot->sendMessage(CHAT_ID, "📸 Taking photo...", "");
            }
            else if (text == "/status") {
                bot->sendMessage(CHAT_ID, "✅ System online", "");
            }
        }
    }
}