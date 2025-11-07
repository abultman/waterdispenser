#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <ArduinoOTA.h>

class OTAManager {
public:
    OTAManager();

    // Initialize OTA with optional password
    void begin(const char* hostname = "waterdispenser", const char* password = nullptr);

    // Update OTA - call this in loop()
    void update();

    // Check if OTA update is in progress
    bool isUpdating();

    // Get OTA progress (0-100)
    uint8_t getProgress();

    // Set callback for OTA start (optional)
    void onStart(void (*callback)());

    // Set callback for OTA end (optional)
    void onEnd(void (*callback)());

    // Set callback for OTA progress (optional)
    void onProgress(void (*callback)(unsigned int progress, unsigned int total));

    // Set callback for OTA error (optional)
    void onError(void (*callback)(ota_error_t error));

private:
    bool _isUpdating;
    uint8_t _progress;
    void (*_startCallback)();
    void (*_endCallback)();
    void (*_progressCallback)(unsigned int progress, unsigned int total);
    void (*_errorCallback)(ota_error_t error);
};

// Global instance
extern OTAManager otaManager;

#endif // OTA_MANAGER_H
