#pragma once

#include <WebServer.h>
#include <ESPAsyncWebServer.h>
#include <WebServer.h>

class AlarmSystem;


class AlarmSystemWebServer
{
public:
    AlarmSystemWebServer(AlarmSystem& alarmSystem);
    void begin();
private:
    void handleGetState(AsyncWebServerRequest *request) const;
    void handleGetSensor(AsyncWebServerRequest *request) const;
    void handleGetSensors(AsyncWebServerRequest *request) const;
    void handleGetValidOperations(AsyncWebServerRequest *request) const;
    void handlePostOperation(AsyncWebServerRequest *request);
    AlarmSystem& _alarmSystem;
    mutable AsyncWebServer _server;
};
