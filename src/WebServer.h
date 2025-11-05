#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>

class WebServerManager {
public:
    WebServerManager();
    void begin();
    void update();
    void broadcastStatus();

private:
    AsyncWebServer* _server;
    AsyncWebSocket* _ws;

    // WebSocket handlers
    void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                         AwsEventType type, void* arg, uint8_t* data, size_t len);

    // Helper methods
    String getStatusJSON();

    // Timing for periodic updates
    unsigned long _lastBroadcast;
};

// Global instance
extern WebServerManager webServer;

#endif // WEB_SERVER_H
