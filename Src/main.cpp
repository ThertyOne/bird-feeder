// Src/main.cpp

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "sensors_n_actuators/wifi_meneger.h"
#include "telegram/telegram.h"

/*
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "sensors_n_actuators/pir_sensor.h"
#include "telegram/telegram.h"


// Camera pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Global objects
PIRSensor pirSensor(PIR_SENSOR_PIN);
TelegramManager telegramBot;
unsigned long lastPhotoCaptureTime = 0;

void setupCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_LATEST;

    if (psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 1;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("[CAMERA] Init failed: 0x%x\n", err);
        ESP.restart();
    }
    
    Serial.println("[CAMERA] ✓ Initialized");
}

void captureAndSendPhoto() {
    Serial.println("[PHOTO] Capturing...");
    
    // Wyrzuć pierwsze zdjęcie (zwykle słabej jakości)
    camera_fb_t* fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    
    delay(100);
    
    // Weź nowe zdjęcie
    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("[PHOTO] ✗ Capture failed!");
        return;
    }
    
    Serial.print("[PHOTO] Size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
    
    // Flash LED
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(200);
    digitalWrite(FLASH_LED_PIN, LOW);
    
    // Wyślij na Telegram
    bool sent = telegramBot.sendPhoto(fb->buf, fb->len);
    
    esp_camera_fb_return(fb);
    
    if (sent) {
        Serial.println("[PHOTO] ✓ Sent to Telegram!");
    }
    
    // Reset PIR sensora
    pirSensor.reset();
}
*/

// Setup
TelegramManager telegramBot;

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
    const bool wifiOk = connectToWiFi();
    if (!wifiOk) {
        Serial.println("[SYSTEM] WiFi not available. Stopping further initialization.");
        return;
    }
    Serial.println("[SYSTEM] WiFi connected!");

    // Initialize Telegram bot
    delay(400); // Cautious delay before initializing Telegram
    telegramBot.begin();
    Serial.println("[SYSTEM] Telegram bot initialized, testing connection...");
    bool telegramOk = telegramBot.sendMessage("ESP32-CAM bird feeder online.");
    if (!telegramOk) {
        Serial.println("[SYSTEM] Telegram test failed.");
    } else {
        Serial.println("[SYSTEM] Telegram test passed.");
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
