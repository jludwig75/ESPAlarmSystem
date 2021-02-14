#pragma once

#include <WebServer.h>


class AlarmSystem;


class AlarmSystemWebServer
{
public:
    AlarmSystemWebServer(AlarmSystem& alarmSystem);
    void begin();
    void onLoop();
private:
    void handleGetState() const;
    void handleGetSensors() const;
    void handleGetValidOperations() const;
    void handlePostOperation();
    AlarmSystem& _alarmSystem;
    mutable WebServer _server;
};
