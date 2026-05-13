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

// ============================================
// TELEGRAM SECRETS
// ============================================
#define BOT_TOKEN "xxx"
#define CHAT_ID "xxx"

// ============================================
// PIN DEFINITIONS
// ============================================
#define PIR_SENSOR_PIN 14   // motion sensor (PIR)
#define FLASH_LED_PIN 4     // LED flash (status)
#define SERVO_PIN 15        // Servo (future use)

// ============================================
// SENSOR CONFIGURATION
// ============================================
#define PIR_DEBOUNCE_TIME 5000UL    // 5s - time to ignore subsequent triggers after the first one
#define PIR_WARMUP_TIME 45000UL     // 45s - time for the PIR sensor to stabilize after power-up

// ============================================
// TELEGRAM BOT
// ============================================
#define BOT_REQUEST_DELAY 1000UL    // Check for new messages every 1 second

// ============================================
// DEBUG
// ============================================
#define DEBUG_SERIAL 1      // 1 = debug logs on, 0 = disable logs

#endif