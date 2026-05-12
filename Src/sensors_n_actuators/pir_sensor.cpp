// Src/sensors/pir_sensor.cpp

#include "pir_sensor.h"

// Global pointer dla ISR (Interrupt Service Routine)
static PIRSensor* pirInstance = nullptr;

void IRAM_ATTR PIRSensor::handleMotionISR() {
    if (pirInstance != nullptr && pirInstance->isReady) {
        pirInstance->motionDetected = true;
    }
}

PIRSensor::PIRSensor(gpio_num_t sensorPin) : pin(sensorPin) {}

void PIRSensor::begin() {
    pirInstance = this;
    
    pinMode(pin, INPUT);
    warmupStartTime = millis();
    
    // Czekaj na kalibrację sensora
    Serial.println("[PIR] Warming up sensor (45 sec)...");
    delay(PIR_WARMUP_TIME);
    
    isReady = true;
    Serial.println("[PIR] Sensor ready!");
    
    // Attach interrupt - HIGH means motion detected
    attachInterrupt(digitalPinToInterrupt(pin), handleMotionISR, RISING);
}

bool PIRSensor::isMotionDetected() {
    return motionDetected;
}

bool PIRSensor::isReadyForCapture() {
    // Sprawdź czy minął czas debounce'a
    if (millis() - lastTriggerTime < PIR_DEBOUNCE_TIME) {
        return false;
    }
    return true;
}

void PIRSensor::reset() {
    motionDetected = false;
    lastTriggerTime = millis();
}