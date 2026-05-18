#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

#include <Arduino.h>

class PIRSensor {
private:
    uint8_t pin;
    uint8_t activeLevel;

    unsigned long warmupStartTime = 0;
    unsigned long lastStateChangeTime = 0;
    unsigned long lastCaptureTime = 0;

    bool lastRawState = false;
    bool stableMotionState = false;

public:
    PIRSensor(uint8_t sensorPin, uint8_t sensorActiveLevel = HIGH);

    void begin();

    bool isWarmedUp() const;
    bool isMotionRaw() const;
    bool isMotionDetected();
    bool isReadyForCapture();

    void markCaptureDone();
    void reset();
};

#endif