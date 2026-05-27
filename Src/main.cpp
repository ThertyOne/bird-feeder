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
String mode = "None";

// Utils (Prototypes, definitions at the end of this file)
bool captureAndSendPhoto();
bool telegramIntroMessage();
void telegramMainLoopIntroMessage();
void handleTelegramCommandInMainLoop();
void helpMessage();
String buildStatusMessage();
void testPirSensor();
void runSilentMode();


// Initialization
void setup() {
#if DEBUG_STARTUP

    // Initialize Serial
    Serial.begin(115200);
    delay(3000); // Coucious delay to allow Serial monitor to connect

    // Test PIR
    Serial.println("Testing PIR sensor.");
    pirSensor.begin();
    Serial.println("Outputting PIR state every third of a second.");
    while (true) {
        delay(333);

        bool rawHigh = pirSensor.readDigital();
        bool motion = pirSensor.isMotionDetected();

        Serial.print("raw=");
        Serial.print(rawHigh ? "HIGH" : "LOW");
        Serial.print(" motion=");
        Serial.println(motion ? "YES" : "NO");
    }
#else // Normal setup() ( DEBUG_STARTUP = 0 )
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
    fullDebugMessage += "Debug mode:\n\tSERIAL=" + String(DEBUG_SERIAL ? "ON" : "OFF");
    fullDebugMessage += ",\n\tWIFI=" + String(DEBUG_SERIAL_WIFI ? "ON" : "OFF");
    fullDebugMessage += ",\n\tTELEGRAM=" + String(DEBUG_SERIAL_TELEGRAM ? "ON" : "OFF");
    fullDebugMessage += ",\n\tMESSAGE_COUNTER=" + String(DEBUG_MESSAGE_COUNTER ? "ON" : "OFF") + ".\n";
    fullDebugMessage += ",\nPIR sensor test - message frequency=" + String(float(DEBUG_PIR_TEST_MESSAGE_FREQUENCY)/10) + "s.\n\n";
    
    // Connect to WiFi
    delay(100); // Small cautious delay before starting WiFi connection
    wifiReady = connectToWiFi(WIFI_CONNECTION_TIMEOUT); // 2 minutes timeout for WiFi connection
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
    fullDebugMessage += "WiFi channel:\t" + String(WiFi.channel()) + "\n";
    fullDebugMessage += "WiFi connection timeout: " + String(float(WIFI_CONNECTION_TIMEOUT)/6000) + "min\n\n";

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
    telegramReady = telegramIntroMessage();
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
    bool sensorTest = pirSensor.isMotionDetected();
    Serial.println("[SYSTEM] PIR sensor initialized.");
    briefDebugMessage += "PIR sensor initialized.\n";
    fullDebugMessage += "PIR sensor initialized successfully:\n";
    fullDebugMessage += "PIR sensor pin:\t" + String(PIR_SENSOR_PIN) + "\n";
    fullDebugMessage += "PIR sensor active level:\t" + String(PIR_ACTIVE_LEVEL) + "\n";
    fullDebugMessage += "PIR sensor state:\t" + String(sensorTest ? "Motion detected" : "Currently no motion") + "\n";
    fullDebugMessage += "PIR sensor cooldown time:\t" + String(PIR_CAPTURE_COOLDOWN) + "ms.\n\n";

    // Setup complete message and intro information
    Serial.println("[SYSTEM] Initialization complete.");
    while (true) {
        TelegramCommand introCommand = telegramBot.handleMessages();

        if (introCommand == TelegramCommand::None) {
            delay(BOT_RESPONSE_DELAY);
            continue;
        } if (introCommand == TelegramCommand::init) {
            telegramBot.sendMessage("Initialization complete. Running in silent automatic mode.");
            mode = "Silent";
            break;
        } if (introCommand == TelegramCommand::init_debug) {
            telegramBot.sendMessage(briefDebugMessage);
            break;
        } if (introCommand == TelegramCommand::init_full) {
            telegramBot.sendMessage(fullDebugMessage);
            telegramBot.sendMessage("Sending full help message with available commands...");
            helpMessage();
            break;
        } telegramBot.sendMessage("Use /init, /init_debug, or /init_full to finish initialization.");

        delay(BOT_RESPONSE_DELAY);
    }

#endif
    delay(100); // Small delay before entering main loop (sanity check)
}

// Entry point
void loop() {
#if DEBUG_STARTUP
    // pass
#else // DEBUG_STARTUP = 0

    // Util for controlled delays
    unsigned long startTime = millis();

    // If we're in "None" mode, we wait for the user to choose a mode via Telegram commands.
    if (mode == "None") {
        // Send intro message with available commands
        telegramMainLoopIntroMessage();

        // Check for Telegram commands
        while (mode == "None") {
            const unsigned long now = millis();
            if (now - startTime >= BOT_RESPONSE_DELAY){
                handleTelegramCommandInMainLoop();
                startTime = now;
            }
            delay(10);
        }
    }

    // Main loop
    startTime = millis();
    while (mode != "None") {

        // Utils for controlled delays
        const unsigned long now = millis();
        if (now - startTime >= BOT_REQUEST_DELAY){
            // Check for Telegram commands
            handleTelegramCommandInMainLoop();
            // Reset start time
            startTime = now;
        }
        delay(10);
    }
#endif
}   


// Utils - definitions
bool telegramIntroMessage(){
    telegramBot.sendMessage(
    "🌿🟢🌿🟢🌿🟢🌿🟢🌿\n"
    "\n"
    "  🐦 BIRD FEEDER SYSTEM\n"
    "\n"
    "  🚀 STARTING UP...\n"
    "\n"
    "🌿🟢🌿🟢🌿🟢🌿🟢🌿");

    String intro = "Hello! I'm your friendly neighborhood bird feeder system, powered by ESP32-CAM.";
    intro += " I'm here to keep an eye on your feathered friends and share their photos with you!\n";
    intro += "What should I do now?\nHere are some commands you can use:\n";
    intro += "/init - initialize the system quaietly, report success and run silent automatic mode\n";
    intro += "/init_debug - initialize the system with brief debug output\n";
    intro += "/init_full - initialize the system with full debug output\n";
    intro += "Keep in mind that initialization might take up to a minute ;)";
    return telegramBot.sendMessage(intro);
}

void telegramMainLoopIntroMessage() {
    telegramBot.sendMessage(
        "🌿🟢🌿🟢🌿🟢🌿🟢🌿\n"
        " 🐦 BIRD FEEDER SYSTEM\n"
        "🌿🟢🌿🟢🌿🟢🌿🟢🌿\n"
        "\n"
        "System ready.\n"
        "\n"
        "Available basic commands:\n"
        "\n"
        "📖 /help\n"
        "Show available commands.\n"
        "\n"
        "🌙 /run_silent\n"
        "Start silent automatic mode.\n"
        "The feeder sends a photo only when PIR detects motion.\n"
        "\n"
        "🛠 /run_debug\n"
        "Start automatic mode with debug messages.\n"
        "\n"
        "🛑 /stop\n"
        "Stop automatic mode.\n"
    );
}

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

void handleTelegramCommandInMainLoop() {    
    // Get the latest command from Telegram
    TelegramCommand command = telegramBot.handleMessages();
    Serial.println("[TELEGRAM] Checking for a command: " + String(telegramBot.commandToString(command)));

    switch (command) {
        // None - No command received, do nothing
        case TelegramCommand::None:
            mode = "None";
            break;

        // Start - not implemented yet

        // Help - Send help message. Do not change mode
        case TelegramCommand::Help:
            helpMessage();
            break;

        // Status - not implemented yet
        
        // Test_pir - Run test pir sensor and get back to none
        case TelegramCommand::Test_pir:
            testPirSensor();
            mode = "None";
            break;

        //WiFi_status - not implemented yet

        //Test_camera - not implemented yet

        //Config_info - not implemented yet

        // Run_silent - Run silent mode until it finnishes, then reset to None mode
        case TelegramCommand::Run_silent:
            runSilentMode();
            mode = "None";
            break;

        // Run_debug - Not implemented yet
        case TelegramCommand::Run_debug:
            telegramBot.sendMessage("Starting automatic debug mode. The feeder will send a photo when motion is detected by the PIR sensor.\n"
                                    "Also there will be debug messages sent to the serial monitor and Telegram.");
            // Debug mode here;
            mode = "None";
            break;
        
        // Change_configuration - not implemented yet

        // Reset - Reset ESP with all peripherals
        case TelegramCommand::Reset:
            telegramBot.sendMessage("Feeder will reset now. It might take few seconds...");
            mode = "None";
            Serial.flush();
            delay(200);

            ESP.restart();
            break; // Redundant break to at least reset main loop if ESP.restart() fails.

        // Unknown command. Inform user. Do not change mode.
        default:
            telegramBot.sendMessage("Unsupported command received. Try again.\nUse /help to see available commands.");
            break;
    }
}

void helpMessage() {
    telegramBot.sendMessage(
        "BIRD FEEDER SYSTEM - COMMANDS\n"
        "\n"
        "\n"
        "Basic commands:\n"
        "/start - show intro message\n"
        "/help - show this command list\n"
        "/status - show general system status [in development]\n"
        "\n"
        "Initialization commands:\n"
        "/init - initialize quietly and run silent automatic mode\n"
        "/init_debug - initialize with brief debug output\n"
        "/init_full - initialize with full debug output\n"
        "\n"
        "Test commands:\n"
        "/test_pir - test PIR sensor\n"
        "/wifi_status - show WiFi status [in development]\n"
        "/test_camera - test camera capture [in development]\n"
        "/config_info - show selected config info [in development]\n"
        "\n"
        "Run commands:\n"
        "/run_silent - start silent automatic mode\n"
        "/run_debug - start automatic debug mode [in development]\n"
        "/stop - stop automatic mode\n"
        "/change_config - change selected config parameters [in development]\n"
        "/restart - resets the ESP board with all periferies."
    );
}

void testPirSensor() {
    bool state = false;
    String message;
    message.reserve(2048);
    String frequencyForMessages = String(float(DEBUG_PIR_TEST_MESSAGE_FREQUENCY)/10);
    message = "PIR raport from last " + frequencyForMessages + "s:\n\n";
    unsigned long testStartTime = millis();
    unsigned long lastSampleTime = 0;
    unsigned long lastReportTime = millis();
    unsigned long lastTelegramCheckTime = millis();

    // Initialize test
    telegramBot.sendMessage(
        "PIR test started.\n"
        "Send /stop to finish.\n\n"
        "Pin: " + String(pirSensor.getPin()) + "\n"
        "Active level: " + String(pirSensor.getActiveLevel()) + "\n\n"
        "Feeder will collect PIR sensor state every tenth of a second and send it acumulated every" + frequencyForMessages + 
        "s. It will also send live data via UART interface.\n\n"
        "Listening for motion..."
    );
    Serial.println("[PIR TEST] Simple test started.");

    while (true){
        const unsigned long now = millis();

        // Check telegram messages for \stop and \help
        if (now - lastTelegramCheckTime >= BOT_REQUEST_DELAY) {
            lastTelegramCheckTime = now;
            TelegramCommand lastMessage = telegramBot.handleMessages();

            // /stop
            if (lastMessage == TelegramCommand::Stop) {
                telegramBot.sendMessage("Stopping PIR sensor test.");
                telegramBot.sendMessage(message);
                mode = "None";
                return;
            
            // /help
            } else if (lastMessage == TelegramCommand::Help) helpMessage();
        }

        // Get PIR state
        bool rawHigh = pirSensor.readDigital();
        bool motion = pirSensor.isMotionDetected();

        // Write messages
        float t = (now - testStartTime) / 1000.0f;
        Serial.print("[PIR TEST] Raw signal: ");
        Serial.print(rawHigh ? "HIGH" : "LOW");
        Serial.print(", motion: ");
        Serial.println(motion ? "YES" : "NO");
        message += String(t, 1) + "s  -  Raw signal: " + String(rawHigh ? " < HIGH > " : " < LOW > ");
        message += ", Motion: " + String(motion ? " < YES > " : " < NO > ") + ".\n";
        message += ".\n";

        // send telegram message
        if (now - lastReportTime >= DEBUG_PIR_TEST_MESSAGE_FREQUENCY) {
            lastReportTime = now;

            telegramBot.sendMessage(message);
            message = "PIR raport from last 5s:\n\n";
        }

        delay(10);
    }
}

void runSilentMode() {
    telegramBot.sendMessage("Starting silent automatic mode.\n"
                            "The feeder will send a photo only when motion is detected by the PIR sensor.\n\n"
                            "Waiting for birds to arrive...");

    unsigned long automaticStartTime = millis();
    unsigned long lastSampleTime = 0;
    unsigned long lastTelegramCheckTime = millis();
    unsigned long lastCaptureTime = 0;
    bool pirWasActive = false;

    // Main loop
    while (true){
        const unsigned long now = millis();
        
        // Check for \stop and \help
        if (now - lastTelegramCheckTime >= BOT_REQUEST_DELAY) {
            lastTelegramCheckTime = now;
            TelegramCommand lastMessage = telegramBot.handleMessages();

            // /stop
            if (lastMessage == TelegramCommand::Stop) {
                telegramBot.sendMessage("Stopping automatic silent mode...");
                mode = "None";
                return;
            // /help
            } else if (lastMessage == TelegramCommand::Help) helpMessage();
        }

        if (now - lastSampleTime >= BOT_REQUEST_DELAY) {
            lastSampleTime = now;
            
            // Get PIR state
            bool motion = pirSensor.isMotionDetected();
        
            // React to new motion
            if (motion && !pirWasActive) {
                const unsigned long now2 = millis();

                if (lastCaptureTime == 0 || now2 - lastCaptureTime >= PIR_CAPTURE_COOLDOWN) {
                    Serial.println("[SILENT MODE] Motion detected.");

                    telegramBot.sendMessage(
                        "Motion detected near the feeder.\n"
                        "Taking photo..."
                    );

                    delay(200); // short delay before capture
                    bool sent = captureAndSendPhoto();

                    lastCaptureTime = millis();

                    if (sent) {
                        Serial.println("[SILENT MODE] Photo sent.");
                    } else {
                        Serial.println("[SILENT MODE] Photo sending failed.");
                    }
                } else {
                    Serial.println("[SILENT MODE] Motion ignored due to cooldown.");
                }
            }
            // Update PIR state memory
            pirWasActive = motion;
        }

        delay(10);
    }
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



