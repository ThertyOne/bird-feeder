#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

bool connectToWiFi(uint32_t timeoutMs = 10000);
bool isWiFiConnected();

#endif