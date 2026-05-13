// Src/sensors_n_actuators/wifi_meneger/wifi_meneger.cpp

#include "sensors_n_actuators/wifi_meneger/wifi_meneger.h"

#include <WiFi.h>
#include "config.h"

static bool tryConnect(const char* ssid, const char* password, uint32_t timeoutMs) {
    Serial.print("[WIFI] Connecting to: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    const unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeoutMs) {
        Serial.print(".");
        delay(500);
    }

    Serial.print("\r");

#if DEBUG_SERIAL_WIFI
    //Serial.println("\n");
    //Serial.println("<<<----- DEBUG MODE ----->>>");
    Serial.print("[WIFI] Connection attempt to ");
    Serial.print(ssid);
    Serial.print(" - Status: ")
    Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Failed");
    //Serial.println("<<<--- END DEBUG MODE --->>>\n");
#endif

    return WiFi.status() == WL_CONNECTED;
}

bool connectToWiFi(uint32_t timeoutMs) {
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);

    bool connected = tryConnect(SSID_1, PASSWORD_1, timeoutMs);

#if defined(SSID_2) && defined(PASSWORD_2)
    if (!connected && strlen(SSID_2) > 0) {
        Serial.println("[WIFI] Trying backup network...");
        connected = tryConnect(SSID_2, PASSWORD_2, timeoutMs);
    }
#endif

#if defined(SSID_3) && defined(PASSWORD_3)
    if (!connected && strlen(SSID_3) > 0) {
        Serial.println("[WIFI] Trying backup network...");
        connected = tryConnect(SSID_3, PASSWORD_3, timeoutMs);
    }
#endif

    if (!connected) {
        Serial.println("[WIFI] Connection failed");
        Serial.print("[WIFI] Status code: ");
        Serial.println(WiFi.status());
        return false;
    }

    Serial.println("[WIFI] Connected!");
    Serial.print("[WIFI] IP: ");
    Serial.println(WiFi.localIP());

#if DEBUG_SERIAL
    //Serial.println("\n");
    //Serial.println("<<<----- DEBUG MODE ----->>>");
    Serial.println("[WIFI] Connection details:");
    Serial.print("  SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("  BSSID: ");
    Serial.println(WiFi.BSSIDstr());
    Serial.print("  RSSI: ");
    Serial.println(WiFi.RSSI());
    Serial.print("  Channel: ");
    Serial.println(WiFi.channel());
    //Serial.println("<<<--- END DEBUG MODE --->>>\n");
#endif

    return true;
}

bool isWiFiConnected() {
#if DEBUG_SERIAL_WIFI
    //Serial.println("\n");
    //Serial.println("<<<----- DEBUG MODE ----->>>");
    Serial.print("[WIFI] Checking current connection status: ");
    Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Not connected");
    //Serial.println("<<<--- END DEBUG MODE --->>>\n");
#endif

    return WiFi.status() == WL_CONNECTED;
}