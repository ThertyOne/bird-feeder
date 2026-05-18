// Src/sensors_n_actuators/wifi_meneger/wifi_meneger.h

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

bool connectToWiFi(uint32_t timeoutMs = 60000);
bool isWiFiConnected();

#endif