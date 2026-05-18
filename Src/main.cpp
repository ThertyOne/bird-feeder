// Src/main.cpp

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "sensors_n_actuators/wifi_meneger/wifi_meneger.h"
#include "sensors_n_actuators/pir_sensor/pir_sensor.h"
#include "sensors_n_actuators/camera/camera.h"
#include "telegram/telegram.h"


// Setup
PIRSensor pirSensor(PIR_SENSOR_PIN, PIR_ACTIVE_LEVEL);
TelegramManager telegramBot;
bool wifiReady = false;
bool telegramReady = false;
bool cameraReady = false;

// Utils (definitions at the end of this file)
bool captureAndSendPhoto();
String buildStatusMessage();
void telegramIntroMessage();

void setup() {
    String fullDebugMessage = "";
    String briefDebugMessage = "";

    // Initialize Serial
    Serial.begin(115200);
    delay(3000); // Coucious delay to allow Serial monitor to connect
    Serial.println("\n\n[SYSTEM] Starting up ESP32-CAM bird feeder system...\n\n");

// DEBUG - config info
#if DEBUG_SERIAL
    //Serial.println("\n");
    //Serial.println("<<<----- DEBUG MODE ----->>>");
    Serial.println("[CONFIG] Loaded successfully");
    Serial.print("[CONFIG] SSID: ");
    Serial.println(SSID_1);
    Serial.print("[CONFIG] PIR pin: ");
    Serial.println(PIR_SENSOR_PIN);
    Serial.print("[CONFIG] Flash LED pin: ");
    Serial.println(FLASH_LED_PIN);
    //Serial.println("<<<--- END DEBUG MODE --->>>\n");
#endif

    // Config info
    briefDebugMessage += "Config loaded successfully.\n";
    fullDebugMessage += "Config loaded successfully:\n";
    fullDebugMessage += "Debug mode:\n\tSERIAL=" + String(DEBUG_SERIAL) + ",\n\tWIFI=" + String(DEBUG_SERIAL_WIFI) + ",\n\tTELEGRAM=" + String(DEBUG_SERIAL_TELEGRAM) + ",\n\tCHECK_CONNECTION=" + String(CHECK_TELEGRAM_CONNECTION) + "\n\n";
    
    // Connect to WiFi
    delay(100); // Small cautious delay before starting WiFi connection
    wifiReady = connectToWiFi(120000); // 120 seconds timeout for WiFi connection
    if (!wifiReady) {
        Serial.println("[SYSTEM] WiFi not available. Stopping further initialization.");
        return;
    }
    Serial.println("[SYSTEM] WiFi connected!");
    briefDebugMessage += "WiFi connected.\n";
    fullDebugMessage += "WiFi connected successfully:\n";
    fullDebugMessage += "SSID:\t" + WiFi.SSID()+ "\n";
    fullDebugMessage += "IP address:\t" + WiFi.localIP().toString() + "\n";
    fullDebugMessage += "Signal strength (RSSI):\t" + String(WiFi.RSSI()) + " dBm\n";
    fullDebugMessage += "WiFi channel:\t" + String(WiFi.channel()) + "\n\n";

    // Initialize Telegram bot
    delay(400); // Cautious delay before initializing Telegram
    telegramBot.begin();
    telegramBot.clearPendingMessages();
    Serial.println("[SYSTEM] Telegram bot initialized, testing connection...");
    if (!telegramReady) {
        Serial.println("[SYSTEM] Telegram test failed.");
    } else {
        Serial.println("[SYSTEM] Telegram test passed.");
    }
    telegramReady = telegramBot.sendMessage("ESP32-CAM bird feeder online . . .");
    telegramIntroMessage();
    briefDebugMessage += "Telegram bot initialized.\n";
    fullDebugMessage += "Telegram bot initialized successfully:\n";
    fullDebugMessage += "Bot token:\t" + String(BOT_TOKEN).substring(0, 2) + "  ...  " + String(BOT_TOKEN).substring(String(BOT_TOKEN).length() - 2) + "\n";
    fullDebugMessage += "Chat ID:\t" + String(CHAT_ID).substring(0, 2) + "  ...  " + String(CHAT_ID).substring(String(CHAT_ID).length() - 2) + "\n";
    fullDebugMessage += "Request delay:\t" + String(BOT_REQUEST_DELAY) + " ms\n";
    fullDebugMessage += "Response delay:\t" + String(BOT_RESPONSE_DELAY) + " ms\n\n";

    // Initialize camera
    delay(600); // Cautious delay before initializing camera
    Serial.println("[SYSTEM] Initializing camera...");
    cameraReady = initCamera();
    // Test photo capture
    delay(100); // Cautious delay before testing photo capture
    Serial.println("[SYSTEM] Single photo capture test.");
    if (!cameraReady) {
        Serial.println("[PHOTO] Skipped: camera not ready.");
    } else{
        camera_fb_t* photo = capturePhoto();
        if (photo == nullptr) {
            Serial.println("[PHOTO] Capture failed.");
        } else {
            Serial.println("[PHOTO] Photo captured.");
            releasePhoto(photo); // Release the photo buffer after testing capture to prevent memory leaks
        }
    }
    briefDebugMessage += "Camera capture tested successfully.\n";
    fullDebugMessage += "Camera initialized successfully:\n";
    fullDebugMessage += "Camera model:\tOV2640-75MM\n";
    fullDebugMessage += "Camera resolution:\t<Not implemented>\n";
    fullDebugMessage += "Camera frame size:\t<Not implemented>\n";
    fullDebugMessage += "Camera capture tested successfully.\n\n";

    // Initialize PIR sensor
    Serial.println("[SYSTEM] Initializing PIR sensor...");
    pirSensor.begin();
    Serial.println("[SYSTEM] PIR sensor initialized.");
    briefDebugMessage += "PIR sensor initialized.\n";
    fullDebugMessage += "PIR sensor initialized successfully:\n";
    fullDebugMessage += "PIR sensor pin:\t" + String(PIR_SENSOR_PIN) + "\n";
    fullDebugMessage += "PIR sensor active level:\t" + String(PIR_ACTIVE_LEVEL) + "\n";
    fullDebugMessage += "PIR sensor warm-up time:\t" + String(PIR_WARMUP_TIME) + " ms\n";
    fullDebugMessage += "PIR sensor debounce time:\t" + String(PIR_DEBOUNCE_TIME) + " ms\n";
    fullDebugMessage += "PIR sensor cooldown time:\t" + String(PIR_CAPTURE_COOLDOWN) + " ms\n\n";

    // Setup complete message
    Serial.println("[SYSTEM] Initialization complete.");
    while (true) {
        TelegramCommand introCommand = telegramBot.handleMessages();

        if (introCommand == TelegramCommand::None) {
            delay(BOT_RESPONSE_DELAY);
            continue;
        } if (introCommand == TelegramCommand::init) {
            break;
        } if (introCommand == TelegramCommand::init_debug) {
            telegramBot.sendMessage(briefDebugMessage);
            break;
        } if (introCommand == TelegramCommand::init_full) {
            telegramBot.sendMessage(fullDebugMessage);
            break;
        } telegramBot.sendMessage("Use /init, /init_debug, or /init_full to finish initialization.");

        delay(BOT_RESPONSE_DELAY);
    }

    delay(100); // Small delay before entering main loop
}

// Main loop
void loop() {
    // Chceck WiFi status
    bool wifiStatus = isWiFiConnected();

    // Check status of all critical components.
    if (!wifiReady || !telegramReady || !cameraReady) {
        Serial.println("[SYSTEM] Critical component(s) not ready:");
        if (!wifiReady) Serial.println("- WiFi not ready");
        if (!telegramReady) Serial.println("- Telegram bot not ready");
        if (!cameraReady) Serial.println("- Camera not ready");
        delay(1000);
        return;
    }

    delay(1000);
}   


// Utils - definitions
// temporary function - not tested
bool captureAndSendPhoto() {
    if (!isWiFiConnected()) {
        Serial.println("[PHOTO] Skipped: WiFi disconnected.");
        return false;
    }

    if (!cameraReady) {
        Serial.println("[PHOTO] Skipped: camera not ready.");
        telegramBot.sendMessage("Photo skipped: camera not ready.");
        return false;
    }

    camera_fb_t* photo = capturePhoto();

    if (photo == nullptr) {
        Serial.println("[PHOTO] Capture failed.");
        telegramBot.sendMessage("Photo capture failed.");
        return false;
    }

    Serial.println("[PHOTO] Sending photo to Telegram...");

    const bool sent = telegramBot.sendPhoto(photo->buf, photo->len);

    releasePhoto(photo);

    if (sent) {
        Serial.println("[PHOTO] Photo sent successfully.");
    } else {
        Serial.println("[PHOTO] Photo sending failed.");
        telegramBot.sendMessage("Photo sending failed.");
    }

    return sent;
}

// temporary function - not tested
String buildStatusMessage() {
    String status = "Bird Feeder status\n";

    status += "WiFi: ";
    status += isWiFiConnected() ? "OK\n" : "DISCONNECTED\n";

    if (isWiFiConnected()) {
        status += "RSSI: ";
        status += String(WiFi.RSSI());
        status += " dBm\n";
    }

    status += "Camera: ";
    status += cameraReady ? "OK\n" : "NOT READY\n";

    status += "Free heap: ";
    status += String(ESP.getFreeHeap());
    status += " B\n";

    status += "Uptime: ";
    status += String(millis() / 1000);
    status += " s";

    return status;
}

void telegramIntroMessage(){
    telegramBot.sendMessage("🌿🟢🌿🟢🌿🟢🌿🟢🌿🟢\n"
    "\n"
    "     🐦 BIRD FEEDER SYSTEM\n"
    "\n"
    "     🚀 STARTING UP...\n"
    "\n"
    "🌿🟢🌿🟢🌿🟢🌿🟢🌿🟢");

    String intro = "Hello! I'm your friendly neighborhood bird feeder system, powered by ESP32-CAM.";
    intro += " I'm here to keep an eye on your feathered friends and share their photos with you!\n";
    intro += "What should I do now?\nHere are some commands you can use:\n";
    intro += "/init - initialize the system quaietly and report success\n";
    intro += "/init_debug - initialize the system with brief debug output\n";
    intro += "/init_full - initialize the system with full debug output\n";
    telegramBot.sendMessage(intro);
}










