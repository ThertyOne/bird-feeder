#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

#include <Arduino.h>

class PIRSensor {
private:
    uint8_t pin;
    uint8_t activeLevel;

public:
    PIRSensor(uint8_t sensorPin, uint8_t sensorActiveLevel = HIGH);

    void begin();

    bool readDigital() const;        // true = HIGH, false = LOW
    bool isMotionDetected() const;   // true = signal == activeLevel

    uint8_t getPin() const;
    uint8_t getActiveLevel() const;
};

#endif