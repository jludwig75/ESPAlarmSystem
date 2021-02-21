#pragma once

#include <Arduino.h>


class ActivityLog
{
public:
    enum class EventType
    {
        Nothing = 0,
        SystemStart,
        NewSensor,
        SensorOpened,
        SensorClosed,
        SensorFault,
        AlarmArmed,
        AlarmDisarmed,
        AlarmTriggered,
        AlarmArmingFailed
    };
    ActivityLog();
    bool begin();
    void logEvent(EventType type, uint64_t sensorId = 0);
    size_t numberOfEvents() const;
    bool getEvent(size_t i, time_t& eventTime, EventType& eventType, uint64_t& sensorId);
private:
    size_t maxEntries() const;
    struct ActivityLogEntry
    {
        struct tm eventTime;
        ActivityLog::EventType event;
        uint64_t sensorId;
    };
    ActivityLogEntry _log[32];
    size_t _nextLogEntry;
    size_t _eventsStored;
};
