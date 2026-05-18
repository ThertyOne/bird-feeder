// Src/sensors_n_actuators/pir_sensor/pir_sensor.cpp

#include "pir_sensor.h"
#include "config.h"

PIRSensor::PIRSensor(uint8_t sensorPin, uint8_t sensorActiveLevel)
    : pin(sensorPin), activeLevel(sensorActiveLevel) {}

void PIRSensor::begin() {
    pinMode(pin, INPUT);

    warmupStartTime = millis();
    lastStateChangeTime = millis();
    lastCaptureTime = 0;

    lastRawState = isMotionRaw();
    stableMotionState = lastRawState;

    Serial.print("[PIR] Initialized on GPIO ");
    Serial.println(pin);

    Serial.print("[PIR] Warmup time: ");
    Serial.print(PIR_WARMUP_TIME);
    Serial.println(" ms");
}

bool PIRSensor::isWarmedUp() const {
    return millis() - warmupStartTime >= PIR_WARMUP_TIME;
}

bool PIRSensor::isMotionRaw() const {
    return digitalRead(pin) == activeLevel;
}

bool PIRSensor::isMotionDetected() {
    const bool currentRawState = isMotionRaw();
    const unsigned long now = millis();

    if (currentRawState != lastRawState) {
        lastRawState = currentRawState;
        lastStateChangeTime = now;
    }

    if (now - lastStateChangeTime >= PIR_DEBOUNCE_TIME) {
        stableMotionState = currentRawState;
    }

    return stableMotionState;
}

bool PIRSensor::isReadyForCapture() {
    if (!isWarmedUp()) {
        return false;
    }

    if (!isMotionDetected()) {
        return false;
    }

    if (lastCaptureTime == 0) {
        return true;
    }

    return millis() - lastCaptureTime >= PIR_CAPTURE_COOLDOWN;
}

void PIRSensor::markCaptureDone() {
    lastCaptureTime = millis();
}

void PIRSensor::reset() {
    stableMotionState = false;
    lastRawState = isMotionRaw();
    lastStateChangeTime = millis();
    lastCaptureTime = millis();
}