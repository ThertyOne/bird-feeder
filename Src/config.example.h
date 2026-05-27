// Src/config.example.h
// Example configuration file, copy to config.h and fill in your credentials and settings.

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// WIFI CREDENTIALS
// ============================================
#define SSID_1 "xxx"
#define PASSWORD_1 "xxx"
//#define SSID_2 "XXX"
//#define PASSWORD_2 "XXX"
//#define SSID_3 "XXX"
//#define PASSWORD_3 "XXX"
// ============================================
// WIFI CONFIGURATION
// ============================================
#define WIFI_CONNECTION_TIMEOUT 12000UL // 2 minutes timeout for WiFi connection

// ============================================
// TELEGRAM SECRETS
// ============================================
#define BOT_TOKEN "xxx"
#define CHAT_ID "xxx"
// ============================================
// TELEGRAM BOT CONFIGURATION
// ============================================
#define BOT_REQUEST_DELAY 2000UL        // 2s - delay between Telegram Bot API requests
#define BOT_RESPONSE_DELAY 500UL        // 500ms - delay to wait for a response from Telegram Bot API before timing out

// ============================================
// PIN DEFINITIONS
// ============================================
#define PIR_SENSOR_PIN 15   // motion sensor (PIR)
#define FLASH_LED_PIN 4     // LED flash (status)
#define SERVO_PIN 14        // Servo (future use)

// ============================================
// SENSOR CONFIGURATION
// ============================================
#define PIR_DEBOUNCE_TIME 200UL         // 0.2ms - time to ignore subsequent triggers after the first one
#define PIR_CAPTURE_COOLDOWN 45000UL    // 45s - minimum time between captures
#define PIR_ACTIVE_LEVEL 0              // PIR sensor active level (1 = HIGH, 0 = LOW)

// ============================================
// DEBUG
// ============================================
#define DEBUG_SERIAL 1                          // 1 = debug logs on, 0 = disable logs
#define DEBUG_SERIAL_WIFI 1                     // 1 = log WiFi_meneger logs, 0 = only main logs
#define DEBUG_SERIAL_TELEGRAM 1                 // 1 = log TelegramBot logs, 0 = only main logs
#define DEBUG_STARTUP 0                         // 1 = debuging setup() and loop(), 0 normal setup() and loop()
#define DEBUG_MESSAGE_COUNTER 1                 // 1 = start telegram messages with number
#define DEBUG_PIR_TEST_MESSAGE_FREQUENCY 4000UL // Send messages every 4s

#endif