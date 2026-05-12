// Src/sensors/pir_sensor.h

#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

#include <Arduino.h>
#include "../config.h"

class PIRSensor {
private:
    gpio_num_t pin;
    bool isReady = false;
    bool motionDetected = false;
    unsigned long lastTriggerTime = 0;
    unsigned long warmupStartTime = 0;

public:
    PIRSensor(gpio_num_t sensorPin);
    
    void begin();
    bool isMotionDetected();
    bool isReadyForCapture();
    void reset();
    
private:
    static void IRAM_ATTR handleMotionISR();
};

#endif