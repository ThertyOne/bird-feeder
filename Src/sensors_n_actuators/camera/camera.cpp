// Src/sensors_n_actuators/camera/camera.cpp

#include "camera.h"

#include <Arduino.h>
#include "esp_camera.h"

// ESP32-CAM pinout
namespace {
    constexpr int PWDN_GPIO_NUM  = 32;
    constexpr int RESET_GPIO_NUM = -1;
    constexpr int XCLK_GPIO_NUM  = 0;
    constexpr int SIOD_GPIO_NUM  = 26;
    constexpr int SIOC_GPIO_NUM  = 27;

    constexpr int Y9_GPIO_NUM    = 35;
    constexpr int Y8_GPIO_NUM    = 34;
    constexpr int Y7_GPIO_NUM    = 39;
    constexpr int Y6_GPIO_NUM    = 36;
    constexpr int Y5_GPIO_NUM    = 21;
    constexpr int Y4_GPIO_NUM    = 19;
    constexpr int Y3_GPIO_NUM    = 18;
    constexpr int Y2_GPIO_NUM    = 5;

    constexpr int VSYNC_GPIO_NUM = 25;
    constexpr int HREF_GPIO_NUM  = 23;
    constexpr int PCLK_GPIO_NUM  = 22;
}

bool initCamera() {
    Serial.println("[CAMERA] Initializing converter module OV2640...");

    // Configuration structure for esp_camera_init()
    camera_config_t config;

    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;

    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;

    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;

    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;

    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;

    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // Low resolution settings for initial testing
    if (psramFound()) { // PSRAM is an additional external memory that allows for higher resolution and better performance
        Serial.println("[CAMERA] PSRAM detected");

        config.frame_size = FRAMESIZE_VGA; // 640x480 resolution
        // Example resolution settings for later:
        // config.frame_size = FRAMESIZE_QQVGA; // 160x120
        // config.frame_size = FRAMESIZE_QVGA;  // 320x240
        // config.frame_size = FRAMESIZE_VGA;   // 640x480
        // config.frame_size = FRAMESIZE_SVGA; // 800x600 
        // config.frame_size = FRAMESIZE_XGA;  // 1024x768  
        // config.frame_size = FRAMESIZE_SXGA; // 1280x1024
        // config.frame_size = FRAMESIZE_UXGA; // 1600x1200
        // config.frame_size = FRAMESIZE_FHD;  // 1920x1080
        config.jpeg_quality = 12; // 0-63 compresion - lower means higher quality, default is 10
        config.fb_count = 1; // Number of frame buffers to be allocated.
        config.fb_location = CAMERA_FB_IN_PSRAM; // Store frame buffer in PSRAM for better performance
        config.grab_mode = CAMERA_GRAB_LATEST; // Always return the latest frame, even if it means dropping some frames.
    } else {
        Serial.println("[CAMERA] PSRAM not detected, using lower settings");

        config.frame_size = FRAMESIZE_QVGA;
        config.jpeg_quality = 15;
        config.fb_count = 1;
        config.fb_location = CAMERA_FB_IN_DRAM;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    }

    // pass configuration to the camera driver and initialize the camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("[CAMERA] Init failed. Error: 0x%x\n", err);
        return false;
    }

    // Get sensor pointer to access sensor settings and information
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor == nullptr) {
        Serial.println("[CAMERA] Sensor pointer is null");
        return false;
    }

    Serial.println("[CAMERA] Initialized successfully");
    Serial.printf("[CAMERA] Sensor PID: 0x%02x\n\r", sensor->id.PID);

    return true;
}

camera_fb_t* capturePhoto() {
    Serial.println("[CAMERA] Capturing frame...");

    // First frame capture can be worse due to sensor warm-up, discard it
    camera_fb_t* frameBuffer = esp_camera_fb_get();
    if (frameBuffer != nullptr) {
        esp_camera_fb_return(frameBuffer);
        delay(100);
    }
    frameBuffer = esp_camera_fb_get();
    if (frameBuffer == nullptr) {
        Serial.println("[CAMERA] Capture failed");
        return nullptr;
    }

    Serial.print("[CAMERA] Captured frame size: ");
    Serial.print(frameBuffer->len);
    Serial.println(" bytes");

    return frameBuffer;
}

void releasePhoto(camera_fb_t* frameBuffer) {
    // Return the frame buffer back to the driver for reuse to avoid memory leaks !!!
    if (frameBuffer != nullptr) {
        esp_camera_fb_return(frameBuffer);
    }
}

// Temporary function - not tested
bool testCameraCapture() {
    camera_fb_t* frameBuffer = capturePhoto();

    if (frameBuffer == nullptr) {
        Serial.println("[CAMERA] Test capture failed");
        return false;
    }

    Serial.println("[CAMERA] Test capture OK");
    releasePhoto(frameBuffer);

    return true;
}