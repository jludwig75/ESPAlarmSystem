#pragma once

#include <WebServer.h>
#include <WebServer.h>

#include "ActivityLog.h"


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
    String eventTypeToString(ActivityLog::EventType eventType, uint64_t sensorId) const;
    String sensorDisplayName(uint64_t sensorId) const;
    AlarmSystem& _alarmSystem;
    ActivityLog& _activityLog;
    mutable WebServer _server;
};
