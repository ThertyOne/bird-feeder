// Src/sensors_n_actuators/pir_sensor/pir_sensoe.cpp

#include "pir_sensor.h"

PIRSensor::PIRSensor(uint8_t sensorPin, uint8_t sensorActiveLevel)
    : pin(sensorPin), activeLevel(sensorActiveLevel) {}

void PIRSensor::begin() {
    pinMode(pin, INPUT);

    Serial.print("[PIR] Initialized on GPIO ");
    Serial.println(pin);

    Serial.print("[PIR] Active level: ");
    Serial.println(activeLevel);
}

bool PIRSensor::readDigital() const {
    return digitalRead(pin) == HIGH;
}

bool PIRSensor::isMotionDetected() const {
    return digitalRead(pin) == activeLevel;
}

uint8_t PIRSensor::getPin() const {
    return pin;
}

uint8_t PIRSensor::getActiveLevel() const {
    return activeLevel;
}