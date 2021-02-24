#include "ActivityLog.h"

#include <AutoFile.h>
#include <Logging.h>
#include <SPIFFS.h>


namespace
{

const String activityLogFileName = "/activity.log";
const unsigned long flushIntervalMs = 5 * 60 * 1000;   // 5 minutes


}

ActivityLog::ActivityLog()
    :
    _nextLogEntry(0),
    _eventsStored(0),
    _dirty(false),
    _lastFlushTime(0),
    _nextId(0)
{
    memset(_log, 0, sizeof(_log));
}

void ActivityLog::begin()
{
    log_a("Loading actvitiy log");
    if (!SPIFFS.begin())
    {
        log_e("Failed to start SPIFFS");
        return;
    }

    struct tm t;
    getLocalTime(&t);
    _nextId = mktime(&t);

    if (!SPIFFS.exists(activityLogFileName))
    {
        log_a("No activity log file found");
        return;
    }

    auto activityLogFile = AutoFile(SPIFFS.open(activityLogFileName, FILE_READ));
    if (!activityLogFile)
    {
        log_e("Error opening activity log file");
        return;
    }

    if (!activityLogFile->read(reinterpret_cast<uint8_t*>(&_log), sizeof(_log)))
    {
        log_e("Error reading from activity log file");
        memset(_log, 0, sizeof(_log));
        return;
    }

    _nextLogEntry = 0;
    _eventsStored = 0;
    for (size_t i = 0; i < maxEntries(); ++i)
    {
        if (_log[i].event == EventType::Nothing)
        {
            break;
        }

        _eventsStored++;
        if (i > 0 && _log[i].eventTime >= _log[_nextLogEntry].eventTime)
        {
            _nextLogEntry = i;
        }
    }

    _nextLogEntry++;
    if (_nextLogEntry == maxEntries())
    {
        _nextLogEntry = 0;
    }

    log_a("Loadded %u activities from activity log", _eventsStored);
}

void ActivityLog::onLoop()
{
    if (_dirty && (_lastFlushTime == 0 || millis() - _lastFlushTime >= flushIntervalMs))
    {
        flush();
    }
}

size_t ActivityLog::maxEntries() const
{
    return sizeof(_log) / sizeof(_log[0]);
}

void ActivityLog::logEvent(EventType type, uint64_t sensorId)
{
    struct tm t;
    if (!getLocalTime(&t))
    {
        log_e("Failed to get event time");
        return;
    }

    ActivityLogEntry entry{_nextId++, mktime(&t), type, sensorId};

    _log[_nextLogEntry++] = entry;
    if (_nextLogEntry == maxEntries())
    {
        _nextLogEntry = 0;
    }
    if (_eventsStored < maxEntries())
    {
        _eventsStored++;
    }

    _dirty = true;

    if (isCriticalEvent(type))
    {
        flush();
    }
}

bool ActivityLog::isCriticalEvent(EventType eventType) const
{
    switch (eventType)
    {
    case EventType::SystemStart:
    case EventType::AlarmArmed:
    case EventType::AlarmDisarmed:
    case EventType::AlarmTriggered:
    case EventType::AlarmArmingFailed:
        return true;
    default:
        return false;
    }
}

size_t ActivityLog::numberOfEvents() const
{
    return _eventsStored;
}

bool ActivityLog::getEvent(size_t i, unsigned long& id, time_t& eventTime, EventType& eventType, uint64_t& sensorId)
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

    id = _log[i].id;
    eventTime = _log[i].eventTime;
    eventType = _log[i].event;
    sensorId = _log[i].sensorId;
    return true;
}

void ActivityLog::flush()
{
    log_d("Saving %u activities from activity log", _eventsStored);

    auto activityLogFile = AutoFile(SPIFFS.open(activityLogFileName, FILE_WRITE));
    if (!activityLogFile)
    {
        log_e("Error creating activity log file");
        return;
    }

    if (!activityLogFile->write(reinterpret_cast<const uint8_t*>(&_log), sizeof(_log)))
    {
        log_e("Error writing to activity log file");
        return;
    }

    log_i("Saved %u activities from activity log", _eventsStored);

    _lastFlushTime = millis();
    _dirty = false;
}
