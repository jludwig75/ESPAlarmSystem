#include "ActivityLog.h"



namespace
{

const String activityLogFileName = "/activity.log";
const unsigned long maxFlushIntervalMs = 60 * 100;


}

ActivityLog::ActivityLog()
    :
    _nextLogEntry(0),
    _eventsStored(0)
{
    memset(_log, 0, sizeof(_log));
}

size_t ActivityLog::maxEntries() const
{
    return sizeof(_log) / sizeof(_log[0]);
}

void ActivityLog::logEvent(EventType type, uint64_t sensorId)
{
    ActivityLogEntry entry{{}, type, sensorId};
    if (!getLocalTime(&entry.eventTime))
    {
        log_e("Failed to get event time");
        return;
    }

    _log[_nextLogEntry++] = entry;
    if (_nextLogEntry == maxEntries())
    {
        _nextLogEntry = 0;
    }
    if (_eventsStored < maxEntries())
    {
        _eventsStored++;
    }
}

size_t ActivityLog::numberOfEvents() const
{
    return _eventsStored;
}

bool ActivityLog::getEvent(size_t i, time_t& eventTime, EventType& eventType, uint64_t& sensorId)
{
    if (i >= numberOfEvents())
    {
        return false;
    }

    size_t oldestEntry;
    if (_eventsStored < maxEntries())
    {
        // We know the log isn't full yet
        // and the first entry will be 0
        assert(_eventsStored == _nextLogEntry);
        oldestEntry = 0;
    }
    else
    {
        // The log is full. _nextLogEntry
        // will be the oldest entry
        oldestEntry = _nextLogEntry;
    }

    i += oldestEntry;
    if (i >= maxEntries())
    {
        // Handle wrapping
        i -= maxEntries();
    }

    if (_log[i].event == EventType::Nothing)
    {
        return false;
    }

    eventTime = mktime(&_log[i].eventTime);
    eventType = _log[i].event;
    sensorId = _log[i].sensorId;
    return true;
}
