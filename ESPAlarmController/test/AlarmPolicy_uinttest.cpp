#include <catch.hpp>

#include "ActivityLog.h"
#include "AlarmPolicy.h"
#include "AlarmState.h"


SCENARIO( "Test AlarmPolicy::handleSensorState", "" )
{
    ActivityLog log;
    AlarmPolicy policy(log);

    GIVEN( "An unarmed system and a closed sensor" )
    {
        AlarmState state{AlarmState::Disarmed};
        AlarmSensor sensor(1, true, "Front Door", SensorState::Closed);
        sensor.lastUpdate = 5;

        WHEN( "a sensor is reported closed" )
        {
            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Closed, state);

            THEN( "no action is requested" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE_FALSE(actions.playSound);
            }
        }

        WHEN( "a sensor is reported open" )
        {
            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Open, state);

            THEN( "sensor opened chime is played" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE(actions.playSound);
                REQUIRE(actions.sound == SoundPlayer::Sound::SensorChimeOpened);
            }
        }
    }
}