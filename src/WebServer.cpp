#include "WebServer.h"
#include "HardwareControl.h"
#include "config.h"
#include "VolumeUnit.h"
#include <WiFi.h>
#include <Preferences.h>
#include <LittleFS.h>
#include <Update.h>

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

    _server->on("/api/presets", HTTP_GET, [this](AsyncWebServerRequest* request) {
        Preferences prefs;
        StaticJsonDocument<256> doc;

        if (prefs.begin(PREFS_NAMESPACE, true)) {
            doc["preset1"] = prefs.getInt("preset1_ml", PRESET_1_ML);
            doc["preset2"] = prefs.getInt("preset2_ml", PRESET_2_ML);
            doc["preset3"] = prefs.getInt("preset3_ml", PRESET_3_ML);
            doc["preset4"] = prefs.getInt("preset4_ml", PRESET_4_ML);
            prefs.end();
        }

        String output;
        serializeJson(doc, output);
        request->send(200, "application/json", output);
    });

    _server->on("/api/presets", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (request->hasParam("preset1", true) && request->hasParam("preset2", true) &&
            request->hasParam("preset3", true) && request->hasParam("preset4", true)) {

            int preset1 = request->getParam("preset1", true)->value().toInt();
            int preset2 = request->getParam("preset2", true)->value().toInt();
            int preset3 = request->getParam("preset3", true)->value().toInt();
            int preset4 = request->getParam("preset4", true)->value().toInt();

            if (preset1 > 0 && preset2 > 0 && preset3 > 0 && preset4 > 0) {
                Preferences prefs;
                if (prefs.begin(PREFS_NAMESPACE, false)) {
                    prefs.putInt("preset1_ml", preset1);
                    prefs.putInt("preset2_ml", preset2);
                    prefs.putInt("preset3_ml", preset3);
                    prefs.putInt("preset4_ml", preset4);
                    prefs.end();
                    request->send(200, "application/json", "{\"success\":true}");
                } else {
                    request->send(500, "application/json", "{\"success\":false,\"error\":\"Failed to save presets\"}");
                }
            } else {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"All presets must be greater than 0\"}");
            }
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameters\"}");
        }
    });

    _server->on("/api/volumeunit", HTTP_GET, [this](AsyncWebServerRequest* request) {
        Preferences prefs;
        int unitType = UNIT_MILLILITERS;

        if (prefs.begin(PREFS_NAMESPACE, true)) {
            unitType = prefs.getInt("volume_unit", UNIT_MILLILITERS);
            prefs.end();
        }

        String unitStr = (unitType == UNIT_LITERS) ? "l" : "ml";
        String json = "{\"unit\":\"" + unitStr + "\"}";
        request->send(200, "application/json", json);
    });

    _server->on("/api/volumeunit", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (request->hasParam("unit", true)) {
            String unit = request->getParam("unit", true)->value();

            if (unit == "ml" || unit == "l") {
                int unitType = (unit == "l") ? UNIT_LITERS : UNIT_MILLILITERS;

                Preferences prefs;
                if (prefs.begin(PREFS_NAMESPACE, false)) {
                    prefs.putInt("volume_unit", unitType);
                    prefs.end();
                    request->send(200, "application/json", "{\"success\":true}");
                } else {
                    request->send(500, "application/json", "{\"success\":false,\"error\":\"Failed to save unit preference\"}");
                }
            } else {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid unit (must be 'ml' or 'l')\"}");
            }
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing unit parameter\"}");
        }
    });

    // OTA Update endpoint
    _server->on("/update", HTTP_POST,
        [](AsyncWebServerRequest* request) {
            // This is called after upload completes
            bool shouldReboot = !Update.hasError();
            AsyncWebServerResponse* response = request->beginResponse(200, "text/plain",
                shouldReboot ? "Update successful! Restarting..." : "Update failed");
            response->addHeader("Connection", "close");
            request->send(response);

            if (shouldReboot) {
                delay(500);
                ESP.restart();
            }
        },
        [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
            // This is called for each chunk of data uploaded
            if (!index) {
                Serial.printf("OTA Update Start: %s\n", filename.c_str());

                // Get update type from query parameter
                int cmd = U_FLASH; // Default to firmware
                if (request->hasParam("type")) {
                    String type = request->getParam("type")->value();
                    if (type == "filesystem") {
                        cmd = U_SPIFFS;
                    }
                }

                if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
                    Update.printError(Serial);
                }
            }

            if (!Update.hasError()) {
                if (Update.write(data, len) != len) {
                    Update.printError(Serial);
                }
            }

            if (final) {
                if (Update.end(true)) {
                    Serial.printf("OTA Update Success: %uB\n", index + len);
                } else {
                    Update.printError(Serial);
                }
            }
        }
    );

    // Serve static files from LittleFS with gzip support
    // The server will automatically look for .gz versions of files
    _server->serveStatic("/", LittleFS, "/")
        .setDefaultFile("index.html")
        .setCacheControl("max-age=600");  // Cache for 10 minutes

    _server->begin();
    Serial.println("Web server started on port 80");
}

void WebServerManager::update() {
    if (!_ws) return;

    _ws->cleanupClients();

    // Broadcast status every 1000ms (1 second) when clients are connected
    if (_ws->count() > 0 && millis() - _lastBroadcast > 1000) {
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

    // Note: Preset values and volume unit are fetched separately via
    // /api/presets and /api/volumeunit to avoid blocking on every broadcast

    String output;
    serializeJson(doc, output);
    return output;
}
