// Src/main.cpp

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "sensors_n_actuators/wifi_meneger/wifi_meneger.h"
#include "sensors_n_actuators/camera/camera.h"
#include "telegram/telegram.h"


// Setup
TelegramManager telegramBot;
bool wifiReady = false;
bool telegramReady = false;
bool cameraReady = false;

void setup() {
    // temporary solution to prevent brownout resets during WiFi connection 
    // (caused by no 5V power delivery to the ESP32)
    //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector
    
    // Initialize Serial
    Serial.begin(115200);
    delay(3000); // Coucious delay to allow Serial monitor to connect
    Serial.println("\n");
    Serial.println("╔═══════════════════════════════╗");
    Serial.println("║      BIRD FEEDER SYSTEM       ║");
    Serial.println("║         Starting up...        ║");
    Serial.println("╚═══════════════════════════════╝");
    Serial.println("\n");

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

    // Connect to WiFi
    delay(100); // Small cautious delay before starting WiFi connection
    wifiReady = connectToWiFi();
    if (!wifiReady) {
        Serial.println("[SYSTEM] WiFi not available. Stopping further initialization.");
        return;
    }
    Serial.println("[SYSTEM] WiFi connected!");

    // Initialize Telegram bot
    delay(400); // Cautious delay before initializing Telegram
    telegramBot.begin();
    Serial.println("[SYSTEM] Telegram bot initialized, testing connection...");
    telegramReady = telegramBot.sendMessage("ESP32-CAM bird feeder online.");
    if (!telegramReady) {
        Serial.println("[SYSTEM] Telegram test failed.");
    } else {
        Serial.println("[SYSTEM] Telegram test passed.");
    }

    // Initialize camera
    delay(600); // Cautious delay before initializing camera
    Serial.println("[SYSTEM] Initializing camera...");
    cameraReady = initCamera();
    if (!cameraReady) {
        Serial.println("[SYSTEM] Camera initialization failed. Stopping further initialization.");
        return;
    }
    Serial.println("[SYSTEM] Camera initialized, testing capture...");
    if (!testCameraCapture()) {
        Serial.println("[SYSTEM] Camera capture test failed.");
        return;
    }
    Serial.println("[SYSTEM] Camera capture test passed.");

    // Test Telegram photo sending
    delay(100); // Cautious delay before testing Telegram photo sending
    Serial.println("[SYSTEM] Single photo send test.");
    if (!cameraReady) {
        Serial.println("[PHOTO] Skipped: camera not ready.");
    } else if (!telegramReady) {
        Serial.println("[PHOTO] Skipped: Telegram not ready.");
    } else{
        camera_fb_t* photo = capturePhoto();
        if (photo == nullptr) {
            Serial.println("[PHOTO] Capture failed.");
        } else {
            Serial.println("[PHOTO] Sending photo to Telegram...");
            const bool sent = telegramBot.sendPhoto(photo->buf, photo->len);
            releasePhoto(photo); // Important to release the photo buffer after sending to avoid memory leaks !!!
            if (sent) {
                Serial.println("[PHOTO] Photo sent successfully.");
            } else {
                Serial.println("[PHOTO] Photo sending failed.");
            }
        }
    }

    // Setup complete message
    Serial.println("[SYSTEM] Initialization complete.");
    delay(100); // Small delay before entering main loop
}

// Main loop
void loop() {
    // Chceck WiFi status
    bool wifiStatus = isWiFiConnected();
    
// DEBUG - whole system status
#if DEBUG_SERIAL
    //Serial.println("\n");
    //Serial.println("<<<----- DEBUG MODE ----->>>");
    
    // System status
    Serial.println("[SYSTEM] System running...");

    // WiFi status
    if (wifiStatus) {
        Serial.println("[SYSTEM] WiFi OK");
    } else {
        Serial.println("[SYSTEM] WiFi disconnected");
    }

    // Telegram status
    if (CHECK_TELEGRAM_CONNECTION) {
        Serial.println("[SYSTEM] Checking Telegram connection (to disable this check, set CHECK_TELEGRAM_CONNECTION in config.h to 0).");
        if (wifiStatus) {
            telegramBot.sendMessage("Bird feeder running smoothly. WiFi is connected.");
        } else {
            Serial.println("[SYSTEM] Skipping Telegram check due to WiFi issues.");
        }
    }

    //Serial.println("<<<--- END DEBUG MODE --->>>\n");
#endif

    delay(10000);
}

/*
void loop() {
    // check PIR sensor
    //if (pirSensor.isMotionDetected() && pirSensor.isReadyForCapture()) {
    //    Serial.println("\n[MOTION] Motion detected!");
    //    captureAndSendPhoto();
    //}
    // temporary workaround: capture photo every 10 seconds
        if (millis() - lastPhotoCaptureTime > 10000) {
            Serial.println("\n[MOTION] Motion detected! (simulated)");
            captureAndSendPhoto();
            lastPhotoCaptureTime = millis();
        }
    
    delay(100); // Small delay to prevent high CPU usage
}
*/
