#include "OTAManager.h"
#include <WiFi.h>

// Global instance
OTAManager otaManager;

OTAManager::OTAManager() {
    _isUpdating = false;
    _progress = 0;
    _startCallback = nullptr;
    _endCallback = nullptr;
    _progressCallback = nullptr;
    _errorCallback = nullptr;
}

void OTAManager::begin(const char* hostname, const char* password) {
    // Set hostname
    ArduinoOTA.setHostname(hostname);

    // Set password if provided
    if (password != nullptr && strlen(password) > 0) {
        ArduinoOTA.setPassword(password);
    }

    // Configure OTA callbacks
    ArduinoOTA.onStart([this]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "firmware";
        } else { // U_SPIFFS or U_FS
            type = "filesystem";
        }
        Serial.println("OTA Update Started: " + type);
        _isUpdating = true;
        _progress = 0;

        // Call user callback if set
        if (_startCallback != nullptr) {
            _startCallback();
        }
    });

    ArduinoOTA.onEnd([this]() {
        Serial.println("\nOTA Update Complete!");
        _isUpdating = false;
        _progress = 100;

        // Call user callback if set
        if (_endCallback != nullptr) {
            _endCallback();
        }
    });

    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
        _progress = (progress / (total / 100));
        Serial.printf("OTA Progress: %u%%\r", _progress);

        // Call user callback if set
        if (_progressCallback != nullptr) {
            _progressCallback(progress, total);
        }
    });

    ArduinoOTA.onError([this](ota_error_t error) {
        Serial.printf("OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
        _isUpdating = false;

        // Call user callback if set
        if (_errorCallback != nullptr) {
            _errorCallback(error);
        }
    });

    // Start OTA service
    ArduinoOTA.begin();
    Serial.println("OTA service started");
}

void OTAManager::update() {
    ArduinoOTA.handle();
}

bool OTAManager::isUpdating() {
    return _isUpdating;
}

uint8_t OTAManager::getProgress() {
    return _progress;
}

void OTAManager::onStart(void (*callback)()) {
    _startCallback = callback;
}

void OTAManager::onEnd(void (*callback)()) {
    _endCallback = callback;
}

void OTAManager::onProgress(void (*callback)(unsigned int progress, unsigned int total)) {
    _progressCallback = callback;
}

void OTAManager::onError(void (*callback)(ota_error_t error)) {
    _errorCallback = callback;
}
