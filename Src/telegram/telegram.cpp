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

bool TelegramManager::sendPhoto(uint8_t* jpgBuffer, size_t jpgSize) {
    // Check buffer validity and return early if invalid
    if (jpgBuffer == nullptr || jpgSize == 0) {
        Serial.println("[TELEGRAM] ERR: Invalid photo buffer");
        return false;
    }
    
    // Send photo via Telegram API and check response
    String response = sendPhotoViaTelegram(jpgBuffer, jpgSize);
    
    // Check if response indicates success
    if (response.length() > 0 && response.indexOf("\"ok\":true") >= 0) {
        Serial.println("[TELEGRAM] Photo sent!");
        return true;
    } else {
        Serial.println("[TELEGRAM] ERR: Failed to send photo");
        Serial.println(response);
        return false;
    }
}

String TelegramManager::sendPhotoViaTelegram(uint8_t* jpgBuffer, size_t jpgSize) {
    const char* domain = "api.telegram.org"; // Telegram API server domain
    String getBody = ""; // To store JSON/HTTP response body from Telegram API
    
    if (clientTCP.connect(domain, 443)) { // 443 is the standard port for HTTPS
        Serial.println("[TELEGRAM] Connected to api.telegram.org");
        
        // Prepare multipart/form-data headers and body for photo upload
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
        
        // Send photo data in 1kB chunks to avoid memory issues with large photos
        uint8_t* buf = jpgBuffer;
        size_t remaining = jpgSize;
        while (remaining > 0) {
            size_t toSend = (remaining > 1024) ? 1024 : remaining;
            clientTCP.write(buf, toSend);
            buf += toSend;
            remaining -= toSend;
        }
        
        clientTCP.print(tail);
        
        // Get response
        unsigned long startTime = millis();

        while (millis() - startTime < 10000) {
            while (clientTCP.available()) {
                char c = clientTCP.read();
                getBody += c;

                // Telegram confirmed success, no need to keep reading until TLS close.
                if (getBody.indexOf("\"ok\":true") >= 0) {
                    clientTCP.stop();
                    return getBody;
                }
            }

            if (!clientTCP.connected()) {
                break;
            }

            delay(10);
        }
        
        clientTCP.stop();
    } else {
        Serial.println("[TELEGRAM] ERR: Connection failed");
    }
    
    return getBody;
}

void TelegramManager::clearPendingMessages(uint8_t rounds) {
    if (bot == nullptr) {
        return;
    }

    for (uint8_t i = 0; i < rounds; i++) {
        int numMessages = bot->getUpdates(bot->last_message_received + 1);

        if (numMessages <= 0) {
            break;
        }

        delay(100);
    }
#if DEBUG_SERIAL_TELEGRAM
    Serial.println("[TELEGRAM] Pending messages cleared");
#endif
}

TelegramCommand TelegramManager::parseCommand(String text) {
    text.trim();

    int mentionPos = text.indexOf('@');
    if (mentionPos > 0) {
        text = text.substring(0, mentionPos);
    }

    // basic commands
    if (text == "/start") return TelegramCommand::Start;
    if (text == "/help") return TelegramCommand::Help;  
    if (text == "/status") return TelegramCommand::Status;
    // initialization commands
    if (text == "/init") return TelegramCommand::init;
    if (text == "/init_debug") return TelegramCommand::init_debug;
    if (text == "/init_full") return TelegramCommand::init_full;
    // test commands
    if (text == "/test_pir") return TelegramCommand::Test_pir;
    if (text == "/wifi_status") return TelegramCommand::WiFi_status;
    if (text == "/test_camera") return TelegramCommand::Test_camera;
    if (text == "/config_info") return TelegramCommand::Config_info;
    // action commands
    if (text == "/run_silent") return TelegramCommand::Run_silent;
    if (text == "/run_debug") return TelegramCommand::Run_debug;
    if (text == "/stop") return TelegramCommand::Stop;

    // default for unknown command
    return TelegramCommand::Unknown;
}

TelegramCommand TelegramManager::handleMessages() {
    if (bot == nullptr) {
        Serial.println("[TELEGRAM] ERR: Bot not initialized");
        return TelegramCommand::None;
    }

    int numNewMessages = bot->getUpdates(bot->last_message_received + 1);

    if (numNewMessages <= 0) {
        return TelegramCommand::None;
    }

    TelegramCommand lastCommand = TelegramCommand::None;

    for (int i = 0; i < numNewMessages; i++) {
        String chatId = String(bot->messages[i].chat_id);

        if (chatId != String(CHAT_ID)) {
            Serial.println("[TELEGRAM] Ignoring message from unauthorized chat ID: " + chatId);
            bot->sendMessage(chatId, "Unauthorized access. This bot is private and only responds to the owner.");
            continue;
        }

        lastCommand = parseCommand(bot->messages[i].text);
    }

    return lastCommand;
}

const char* TelegramManager::commandToString(TelegramCommand command) {
    switch (command) {
        // basic commands
        case TelegramCommand::None:
            return "None";

        case TelegramCommand::Start:
            return "Start";

        case TelegramCommand::Unknown:
            return "Unknown";

        case TelegramCommand::Help:
            return "Help";

        case TelegramCommand::Status:
            return "Status";

        // initialization commands
        case TelegramCommand::init:
            return "init";

        case TelegramCommand::init_debug:
            return "init_debug";

        case TelegramCommand::init_full:
            return "init_full";

        // test commands
        case TelegramCommand::Test_pir:
            return "Test_PIR_sensor";

        case TelegramCommand::WiFi_status:
            return "WiFi_status";

        case TelegramCommand::Test_camera:
            return "Test_camera";

        case TelegramCommand::Config_info:
            return "Config_info";

        // action commands
        case TelegramCommand::Run_silent:
            return "Run_silent";

        case TelegramCommand::Run_debug:
            return "Run_debug"; 

        case TelegramCommand::Stop:
            return "Stop";

        case TelegramCommand::Change_configuration:
            return "Change_configuration";

        default:
            return "Invalid";
    }
}
