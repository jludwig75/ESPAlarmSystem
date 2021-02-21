#pragma once

#include <WebServer.h>
#include <WebServer.h>

class ActivityLog;
class AlarmSensor;
class AlarmSystem;


class AlarmSystemWebServer
{
public:
    AlarmSystemWebServer(AlarmSystem& alarmSystem, ActivityLog& activityLog);
    void begin();
    void onLoop();
private:
    void handleGetState() const;
    void handleGetSensors() const;
    void handleGetSensor() const;
    void handleUpdateSensor();
    void handleGetValidOperations() const;
    void handlePostOperation();
    void handleGetEvents() const;
    AlarmSystem& _alarmSystem;
    ActivityLog& _activityLog;
    mutable WebServer _server;
};
