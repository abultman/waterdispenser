#include "WebServer.h"
#include "HardwareControl.h"
#include "config.h"
#include <WiFi.h>
#include <Preferences.h>
#include <LittleFS.h>

// Global instance
WebServerManager webServer;

WebServerManager::WebServerManager() {
    _server = nullptr;
    _ws = nullptr;
    _lastBroadcast = 0;
}

void WebServerManager::begin() {
    // Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("ERROR: Failed to mount LittleFS!");
        Serial.println("Web interface will not be available.");
        return;
    }
    Serial.println("LittleFS mounted successfully");

    _server = new AsyncWebServer(80);
    _ws = new AsyncWebSocket("/ws");

    // Setup WebSocket
    _ws->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client,
                       AwsEventType type, void* arg, uint8_t* data, size_t len) {
        this->onWebSocketEvent(server, client, type, arg, data, len);
    });
    _server->addHandler(_ws);

    // Serve static files from LittleFS
    _server->serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    _server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        request->send(200, "application/json", getStatusJSON());
    });

    _server->on("/api/start", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (request->hasParam("amount", true)) {
            float amount = request->getParam("amount", true)->value().toFloat();
            if (amount > 0 && amount <= 10000) {
                hardwareControl.startDispensing(amount);
                request->send(200, "application/json", "{\"success\":true}");
            } else {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid amount\"}");
            }
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing amount parameter\"}");
        }
    });

    _server->on("/api/pause", HTTP_POST, [this](AsyncWebServerRequest* request) {
        hardwareControl.pauseDispensing();
        request->send(200, "application/json", "{\"success\":true}");
    });

    _server->on("/api/resume", HTTP_POST, [this](AsyncWebServerRequest* request) {
        hardwareControl.resumeDispensing();
        request->send(200, "application/json", "{\"success\":true}");
    });

    _server->on("/api/stop", HTTP_POST, [this](AsyncWebServerRequest* request) {
        hardwareControl.stopDispensing();
        request->send(200, "application/json", "{\"success\":true}");
    });

    _server->on("/api/wifi", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            String ssid = request->getParam("ssid", true)->value();
            String password = request->getParam("password", true)->value();

            // Save credentials
            Preferences prefs;
            if (prefs.begin(PREFS_NAMESPACE, false)) {
                prefs.putString("wifi_ssid", ssid);
                prefs.putString("wifi_pass", password);
                prefs.end();
            }

            // Attempt connection
            WiFi.begin(ssid.c_str(), password.c_str());
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Connecting...\"}");
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameters\"}");
        }
    });

    _server->on("/api/hostname", HTTP_GET, [this](AsyncWebServerRequest* request) {
        Preferences prefs;
        String hostname = DEFAULT_MDNS_HOSTNAME;
        if (prefs.begin(PREFS_NAMESPACE, true)) {
            hostname = prefs.getString("mdns_hostname", DEFAULT_MDNS_HOSTNAME);
            prefs.end();
        }
        String json = "{\"hostname\":\"" + hostname + "\"}";
        request->send(200, "application/json", json);
    });

    _server->on("/api/hostname", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (request->hasParam("hostname", true)) {
            String hostname = request->getParam("hostname", true)->value();

            // Validate hostname (alphanumeric and hyphens, 1-63 chars)
            if (hostname.length() > 0 && hostname.length() <= 63) {
                bool valid = true;
                for (size_t i = 0; i < hostname.length(); i++) {
                    char c = hostname[i];
                    if (!isalnum(c) && c != '-') {
                        valid = false;
                        break;
                    }
                }

                if (valid) {
                    // Save hostname
                    Preferences prefs;
                    if (prefs.begin(PREFS_NAMESPACE, false)) {
                        prefs.putString("mdns_hostname", hostname);
                        prefs.end();
                    }
                    request->send(200, "application/json", "{\"success\":true,\"message\":\"Hostname saved. Restart required.\"}");
                } else {
                    request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid hostname format\"}");
                }
            } else {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Hostname must be 1-63 characters\"}");
            }
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing hostname parameter\"}");
        }
    });

    _server->on("/api/calibration", HTTP_GET, [this](AsyncWebServerRequest* request) {
        float factor = hardwareControl.getCalibrationFactor();
        String json = "{\"pulsesPerLiter\":" + String(factor, 2) + "}";
        request->send(200, "application/json", json);
    });

    _server->on("/api/calibration", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (request->hasParam("pulsesPerLiter", true)) {
            float factor = request->getParam("pulsesPerLiter", true)->value().toFloat();
            if (factor > 0) {
                hardwareControl.setCalibrationFactor(factor);
                request->send(200, "application/json", "{\"success\":true}");
            } else {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid calibration factor\"}");
            }
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameter\"}");
        }
    });

    _server->begin();
    Serial.println("Web server started on port 80");
}

void WebServerManager::update() {
    _ws->cleanupClients();

    // Broadcast status every 500ms when clients are connected
    if (_ws->count() > 0 && millis() - _lastBroadcast > 500) {
        broadcastStatus();
        _lastBroadcast = millis();
    }
}

void WebServerManager::broadcastStatus() {
    String status = getStatusJSON();
    _ws->textAll(status);
}

void WebServerManager::onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                                        AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        // Send current status to new client
        client->text(getStatusJSON());
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
    }
}

String WebServerManager::getStatusJSON() {
    StaticJsonDocument<512> doc;

    // System status
    doc["wifi"]["connected"] = WiFi.status() == WL_CONNECTED;
    doc["wifi"]["ssid"] = WiFi.SSID();
    doc["wifi"]["ip"] = WiFi.localIP().toString();
    doc["wifi"]["rssi"] = WiFi.RSSI();

    // Dispensing status
    DispensingState state = hardwareControl.getState();
    String stateStr;
    switch (state) {
        case IDLE: stateStr = "idle"; break;
        case DISPENSING: stateStr = "dispensing"; break;
        case PAUSED: stateStr = "paused"; break;
        case STOPPING: stateStr = "stopping"; break;
        case COMPLETED: stateStr = "completed"; break;
        case ERROR_TIMEOUT: stateStr = "error_timeout"; break;
        case ERROR_NO_FLOW: stateStr = "error_no_flow"; break;
    }
    doc["dispensing"]["state"] = stateStr;
    doc["dispensing"]["target"] = hardwareControl.getTargetAmount();
    doc["dispensing"]["dispensed"] = hardwareControl.getDispensedAmount();
    doc["dispensing"]["remaining"] = hardwareControl.getRemainingAmount();
    doc["dispensing"]["progress"] = hardwareControl.getProgress();
    doc["dispensing"]["valveOpen"] = hardwareControl.isValveOpen();

    // Calibration
    doc["calibration"]["pulsesPerLiter"] = hardwareControl.getCalibrationFactor();

    // Preset values
    doc["presets"][0] = PRESET_1_ML;
    doc["presets"][1] = PRESET_2_ML;
    doc["presets"][2] = PRESET_3_ML;
    doc["presets"][3] = PRESET_4_ML;

    String output;
    serializeJson(doc, output);
    return output;
}
