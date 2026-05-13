// Src/sensors_n_actuators/camera/camera.h

#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <Arduino.h>
#include "esp_camera.h"

bool initCamera();
camera_fb_t* capturePhoto();
void releasePhoto(camera_fb_t* frameBuffer);
bool testCameraCapture();

#endif