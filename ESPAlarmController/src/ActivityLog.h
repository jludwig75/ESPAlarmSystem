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
    void begin();
    void onLoop();
    void logEvent(EventType type, uint64_t sensorId = 0);
    size_t numberOfEvents() const;
    bool getEvent(size_t i, unsigned long& id, time_t& eventTime, EventType& eventType, uint64_t& sensorId);
private:
    size_t maxEntries() const;
    void flush();
    bool isCriticalEvent(EventType eventType) const;
    struct ActivityLogEntry
    {
        unsigned long id;
        time_t eventTime;
        ActivityLog::EventType event;
        uint64_t sensorId;
    };
    ActivityLogEntry _log[16];
    size_t _nextLogEntry;
    size_t _eventsStored;
    bool _dirty;
    unsigned long _lastFlushTime;
    unsigned long _nextId;
};
