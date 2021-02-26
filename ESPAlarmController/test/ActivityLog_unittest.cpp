#include <catch.hpp>

#include "ActivityLog.h"

#include <SPIFFS.h>


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
    }
}