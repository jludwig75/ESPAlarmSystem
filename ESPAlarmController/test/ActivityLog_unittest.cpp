#include <catch.hpp>

#include "ActivityLog.h"

#include <SPIFFS.h>

#include <deque>


SCENARIO( "Test ActivityLog", "" )
{
    REQUIRE(SPIFFS.format());

    ActivityLog log;

    WHEN( "the activity log is initialized" )
    {
        log.begin();

        THEN( "there are no events" )
        {
            REQUIRE(log.numberOfEvents() == 0);
            unsigned long eventId;
            time_t eventTime;
            ActivityLog::EventType eventType;
            uint64_t sensorId;
            REQUIRE_FALSE(log.getEvent(0, eventId, eventTime, eventType, sensorId));
        }

        WHEN( "events are logged" )
        {
            const uint64_t testSensorId = 99;
            log.logEvent(ActivityLog::EventType::SystemStart);
            log.logEvent(ActivityLog::EventType::NewSensor, testSensorId);
            log.logEvent(ActivityLog::EventType::SensorClosed, testSensorId);

            THEN( "the events can be retrieved" )
            {
                unsigned long eventId;
                time_t eventTime;
                ActivityLog::EventType eventType;
                uint64_t sensorId;

                REQUIRE(log.numberOfEvents() == 3);
                REQUIRE(log.getEvent(0, eventId, eventTime, eventType, sensorId));
                REQUIRE(eventType == ActivityLog::EventType::SystemStart);
                REQUIRE(sensorId == 0);

                REQUIRE(log.getEvent(1, eventId, eventTime, eventType, sensorId));
                REQUIRE(eventType == ActivityLog::EventType::NewSensor);
                REQUIRE(sensorId == testSensorId);

                REQUIRE(log.getEvent(2, eventId, eventTime, eventType, sensorId));
                REQUIRE(eventType == ActivityLog::EventType::SensorClosed);
                REQUIRE(sensorId == testSensorId);

                REQUIRE_FALSE(log.getEvent(3, eventId, eventTime, eventType, sensorId));
            }

            WHEN( "the activity log is reloaded")
            {
                log = ActivityLog();
                log.begin();

                THEN( "all but the trailing non-critical events are still present ")
                {
                    unsigned long eventId;
                    time_t eventTime;
                    ActivityLog::EventType eventType;
                    uint64_t sensorId;

                    REQUIRE(log.numberOfEvents() == 1);
                    REQUIRE(log.getEvent(0, eventId, eventTime, eventType, sensorId));
                    REQUIRE(eventType == ActivityLog::EventType::SystemStart);
                    REQUIRE(sensorId == 0);

                    REQUIRE_FALSE(log.getEvent(1, eventId, eventTime, eventType, sensorId));
                }
            }

            WHEN( "onLoop is called for the first time" )
            {
                log.onLoop();

                WHEN( "the activity log is reloaded")
                {
                    log = ActivityLog();
                    log.begin();

                    THEN( "the events can be retrieved" )
                    {
                        unsigned long eventId;
                        time_t eventTime;
                        ActivityLog::EventType eventType;
                        uint64_t sensorId;

                        REQUIRE(log.numberOfEvents() == 3);
                        REQUIRE(log.getEvent(0, eventId, eventTime, eventType, sensorId));
                        REQUIRE(eventType == ActivityLog::EventType::SystemStart);
                        REQUIRE(sensorId == 0);

                        REQUIRE(log.getEvent(1, eventId, eventTime, eventType, sensorId));
                        REQUIRE(eventType == ActivityLog::EventType::NewSensor);
                        REQUIRE(sensorId == testSensorId);

                        REQUIRE(log.getEvent(2, eventId, eventTime, eventType, sensorId));
                        REQUIRE(eventType == ActivityLog::EventType::SensorClosed);
                        REQUIRE(sensorId == testSensorId);

                        REQUIRE_FALSE(log.getEvent(3, eventId, eventTime, eventType, sensorId));
                    }
                }
            }

            WHEN( "the last event is a critical event" )
            {
                log.logEvent(ActivityLog::EventType::AlarmArmed);

                WHEN( "the activity log is reloaded")
                {
                    log = ActivityLog();
                    log.begin();

                    THEN( "all of the events are still present ")
                    {
                        unsigned long eventId;
                        time_t eventTime;
                        ActivityLog::EventType eventType;
                        uint64_t sensorId;

                        REQUIRE(log.numberOfEvents() == 4);
                        REQUIRE(log.getEvent(0, eventId, eventTime, eventType, sensorId));
                        REQUIRE(eventType == ActivityLog::EventType::SystemStart);
                        REQUIRE(sensorId == 0);

                        REQUIRE(log.getEvent(1, eventId, eventTime, eventType, sensorId));
                        REQUIRE(eventType == ActivityLog::EventType::NewSensor);
                        REQUIRE(sensorId == testSensorId);

                        REQUIRE(log.getEvent(2, eventId, eventTime, eventType, sensorId));
                        REQUIRE(eventType == ActivityLog::EventType::SensorClosed);
                        REQUIRE(sensorId == testSensorId);

                        REQUIRE(log.getEvent(3, eventId, eventTime, eventType, sensorId));
                        REQUIRE(eventType == ActivityLog::EventType::AlarmArmed);
                        REQUIRE(sensorId == 0);

                        REQUIRE_FALSE(log.getEvent(4, eventId, eventTime, eventType, sensorId));
                    }
                }
            }
        }

        WHEN( "more than the maximum number of events are logged" )
        {
            struct EventRecord
            {
                ActivityLog::EventType type;
                uint64_t sensorId;
            };

            std::deque<EventRecord> loggedEvents;
            const uint64_t testSensorId = 99;
            loggedEvents.push_back(EventRecord{ActivityLog::EventType::SystemStart, 0});
            log.logEvent(loggedEvents.back().type, loggedEvents.back().sensorId);

            loggedEvents.push_back(EventRecord{ActivityLog::EventType::NewSensor, testSensorId});
            log.logEvent(loggedEvents.back().type, loggedEvents.back().sensorId);

            loggedEvents.push_back(EventRecord{ActivityLog::EventType::SensorClosed, testSensorId});
            log.logEvent(loggedEvents.back().type, loggedEvents.back().sensorId);

            // Open and close sensor 20 times
            for (auto i = 0; i < 20; i++)
            {
                loggedEvents.push_back(EventRecord{ActivityLog::EventType::SensorOpened, testSensorId});
                log.logEvent(loggedEvents.back().type, loggedEvents.back().sensorId);

                loggedEvents.push_back(EventRecord{ActivityLog::EventType::SensorClosed, testSensorId});
                log.logEvent(loggedEvents.back().type, loggedEvents.back().sensorId);
            }

            loggedEvents.push_back(EventRecord{ActivityLog::EventType::AlarmArmed, 0});
            log.logEvent(loggedEvents.back().type, loggedEvents.back().sensorId);

            loggedEvents.push_back(EventRecord{ActivityLog::EventType::AlarmTriggered, testSensorId});
            log.logEvent(loggedEvents.back().type, loggedEvents.back().sensorId);

            // End with a critical event so the log is flushed.
            loggedEvents.push_back(EventRecord{ActivityLog::EventType::AlarmDisarmed, 0});
            log.logEvent(loggedEvents.back().type, loggedEvents.back().sensorId);

            // Trim down to the last 16 events
            while (loggedEvents.size() > 16)
            {
                loggedEvents.pop_front();
            }

            THEN( "the last 16 events can be retrieved" )
            {
                REQUIRE(log.numberOfEvents() == 16);
                for (auto i = 0; i < log.numberOfEvents(); i++)
                {
                    unsigned long eventId;
                    time_t eventTime;
                    ActivityLog::EventType eventType;
                    uint64_t sensorId;

                    REQUIRE(log.getEvent(i, eventId, eventTime, eventType, sensorId));

                    const auto& eventRecord = *(loggedEvents.begin() + i);
                    REQUIRE(eventType == eventRecord.type);
                    REQUIRE(sensorId == eventRecord.sensorId);
                }
            }

            WHEN( "the activity log is reloaded")
            {
                log = ActivityLog();
                log.begin();

                THEN( "the last 16 events can be retrieved" )
                {
                    REQUIRE(log.numberOfEvents() == 16);
                    for (auto i = 0; i < log.numberOfEvents(); i++)
                    {
                        unsigned long eventId;
                        time_t eventTime;
                        ActivityLog::EventType eventType;
                        uint64_t sensorId;

                        REQUIRE(log.getEvent(i, eventId, eventTime, eventType, sensorId));

                        const auto& eventRecord = *(loggedEvents.begin() + i);
                        REQUIRE(eventType == eventRecord.type);
                        REQUIRE(sensorId == eventRecord.sensorId);
                    }
                }
            }
       }
    }
}